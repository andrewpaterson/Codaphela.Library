# Unwrapping and Virtual Textures

A mesh with no UVs (an isosurface, a procedural mesh) is given a UV layer so it can have a texture of its own, and
that texture is baked from materials that give a colour at any point in space.  The aim is a single megatexture, as
Rage's (id Tech 5's MegaTexture): every surface its own texels, nothing tiled, kept as pages of which only those in
view are in memory.  This page is the unwrap as built (charts, flattening, packing, baking), the virtual texture as
planned, and the user's decisions.

Both plans are being built from.  [`SupportLib/UnwrapPlan.txt`] has steps 1 to 8 built and 9 (the virtual texture)
and 10 (loaded textures) not started.  [`SupportLib/VirtualTexturePlan.txt`] is an outline: nothing in it is built,
and only what the unwrap plan records as decided is decided.  The texture generators a bake reads came first, in
[`ProceduralLib/TexturePlan.txt`].

## UV layers and layouts

A [`CMesh`]'s UVs are a [`CMeshUVCoords`]: layers ([`CMeshUVLayer`]), each its UVs and, for each face, its corners'
UVs ([`SMeshUVFace`], none for a face without).  Faces of different charts can share a corner position and still have
different UVs, so a seam needs no extra corners.

A mesh's layout is its charts packed into a rectangle measured in texels at the scene's texel density.  The UV layer
is each corner's place in that rectangle, 0 to 1 across it, and the layer keeps the layout's size
(miLayoutWidth and miLayoutHeight, saved with it; 0 when the UVs aren't a layout), so a bake needs only its texture.
A face anticlockwise seen from outside is anticlockwise in UV, u to the right and v up.

Instances share a mesh, so where a copy of the layout sits in the virtual texture is to belong to the instance: its
address, in pages.  Repeated props are to point at one copy; an instance that bakes lighting has its own.  Until the
virtual texture is built the layout is an ordinary image the size of the rectangle, the mesh's material's diffuse
image.

## The unwrap

[`CMeshUnwrap`] is a mesh modifier that gives a mesh UV layer 0 in three stages, as every unwrapper does them
(Hormann, Levy and Sheffer's course notes; xatlas), and keeps everything else the mesh has.  It goes on an editor's
stack after any modifier that rebuilds faces, as those refuse a mesh with UVs ([`CMeshModifier::CanRebuild`]).

| Setting | Default | Meaning |
|---|---|---|
| mfTexelsPerUnit | (given) | The scene's texel density: texels to a unit of the mesh. |
| meCharts | [`MUC_Grown`] | [`MUC_Box`] or [`MUC_Grown`]. |
| meFlatten | [`MUF_Rigid`] | [`MUF_Projected`], [`MUF_Conformal`] or [`MUF_Rigid`]. |
| mfChartAngle | 60 degrees | How far a grown chart's faces may be from its average normal. |
| miPageSize | 128 | The virtual texture's page: a layout's sides are whole pages. |
| miGutter | 4 | Texels between charts at the finest level, and the packing cell's size. |

### Charts

[`CMeshCharts`] cuts the faces into charts, pieces that flatten without much stretch; their edges are the seams.

- [`CMeshCharts::MakeBox`] (box projection): each face goes to whichever of the six axis directions its normal is
  nearest, and faces of one direction joined by an edge are a chart.  It can't fold, but stretches on slopes (up to
  1 / cos 54.7 degrees) and makes many seams on a curve.
- [`CMeshCharts::Grow`]: from each face not yet in a chart, a chart grows across edges, taking whichever face beside
  it is nearest its area weighted average normal while that is within the angle.  A face that ends up more than 85
  degrees from the final average leaves, so a chart laid flat across its normal can't fold.
- [`CMeshCharts::Regrow`]: a chart that folds or overlaps itself once flattened (found by the overlap measure) has its
  faces grown again at half the angle, until none does.

There is no check that a chart is a disc: one only has to lie flat without overlapping itself.

### Flattening

- Projected: across the chart's normal.  Faces are stretched where they slope from it.
- Conformal: Least Squares Conformal Maps (Levy, Petitjean, Ray and Maillot 2002), [`FlattenConformal`].  Each
  triangle's Cauchy-Riemann equations in its own plane, weighted by the root of its area, with the two UVs furthest
  apart pinned; angles are kept, so a texture isn't sheared.  The chart is then scaled so its area in texels is its
  area at the density.  It is solved directly ([`Float64SparseLeastSquaresDirect`]: A'A in a
  [`CFloat64SkylineMatrix`], its unknowns in reverse Cuthill-McKee order, Cholesky factored).
- Rigid: As-Rigid-As-Possible (Liu, Zhang, Xu, Gotsman and Gortler 2008) from LSCM's UVs, [`FlattenRigid`], ten
  local/global rounds.  The global step's matrix never changes, so it is factored once
  ([`CFloat64SparseLeastSquares`]) and solved for u and v each round; a triangle's nearest turn comes from its map
  directly, with no SVD.  Areas are kept as well as angles, so texel density is even across a chart.

Each chart is then turned to its smallest bounding rectangle, which has a side along a side of the chart's convex
hull.  BaseLib also has conjugate gradients ([`Float64SparseSolve`]) and CGLS ([`Float64SparseLeastSquares`]) over a
[`CFloat64SparseMatrix`], which LSCM used first.

| Flattening | Angles kept to (sphere; isosurface of five spheres) | Texels per area against the density (sphere; isosurface) |
|---|---|---|
| Box projection | 1.7 times | |
| LSCM | 1.11; 1.10 | 0.77 to 1.48; 0.63 to 1.44 |
| ARAP | 1.11 to 1.21; 1.10 to 1.32 | 0.88 to 1.14; 0.76 to 1.20 |

These, from TestMeshUnwrap's meshes (2026-11-15 and 11-16), are why grown charts by LSCM became the default and ARAP
then replaced LSCM as it.

### Packing

[`CBitmapPacker`] packs shapes made of rows of bits ([`CBitRows`]), each where it first fits, lowest then leftmost.
A chart is the cells (miGutter texels square) its triangles cover, grown a cell all round for its gutter; of its four
quarter turns the one that reaches least high is kept.  Charts go biggest first across a layout as wide as the root
of their cells' area, and the layout's sides are rounded up to whole pages.  Charts fit into each other's hollows,
which bounding rectangles can't: packed as rectangles along a skyline, charts filled 0.31 to 0.53 of their layouts;
packed as cells, 0.44 to 0.63 (2026-11-15).  Charts are not yet placed on page boundaries.

### Measures

[`SupportLib/MeshUVMeasures.h`] is what an unwrap is judged by: faces missing UVs ([`CountMeshUVFacesMissing`]),
turned over ([`CountMeshUVFacesTurned`]), outside the layout ([`CountMeshUVsOutside`]), overlapping
([`CountMeshUVOverlaps`]: insides sharing area, found through a grid of cells), how much of the layout is covered
([`MeasureMeshUVArea`]), and how much each face is stretched ([`MeasureMeshUVStretch`]: the singular values of its
map from 3D to texels; their ratio is angle distortion, their product against the density squared area distortion).
The same measures work on one chart's triangles as it is laid flat.

## Baking

Baking is why seams are cheap: every chart is baked from its own 3D points, so neighbouring charts match at a seam
without their UVs lining up, as a painted texture's must.  Little stretch matters more than short seams.

A material is a [`CSolidTexture`]: a [`CObject`] that gives a sample ([`SSolidSample`]: colour, height, shine, gloss
and glow) at a point and normal in the world, through a placement ([`CSolidTexture::Place`]: origin, axes, scale) in
world units, so texel density changes how sharp a surface is, never how it looks.  It is told the texel's width, so a
texture finer than a texel gives its average, not its noise.  [`CSolidChecker`] is a box filtered 3D checker, which
is also the test of stretch: squares that stay square and the same size everywhere mean a good unwrap.  The
generators and materials built on it are ProceduralLib's ([`ProceduralLib/TexturePlan.txt`]).

[`BakeMeshTexture`] bakes one image through any UV layer at any size: each texel whose centre a face's UV triangle
covers takes the texture at the 3D point and normal its barycentric position gives; then every texel no face covers
takes its nearest covered one's colour, breadth first (dilation), so filtering at a seam or gutter finds the chart's
colour.

[`CMeshBake`] is the modifier that bakes a mesh: through UV layer 0 at the layer's layout size, every mip level (each
level baked from the texture at half the size, never shrunk from the level above), into one white material whose
diffuse image and its levels ([`CMaterialImage`]'s mapLevels) the bake is.  Each face's old diffuse colour tints its
texels.  Since the texture generators it also takes more samples a texel at coarser levels, shades the colour by the
texture's height, can bake the tilted normals as a bump image, shares its rows among threads
([`CMeshBake::SetJobs`]) and can be stopped early ([`CMeshBake::SetStop`]).  Both renderers draw the baked levels
([`CTextureLevels`], [Scenes](scenes.md)); neither reads the normals yet.

## The virtual texture (planned)

The plan, [`SupportLib/VirtualTexturePlan.txt`], suits this project because the object database already pages: a
page of texels is a small object named by its level and place, read when wanted and freed when not.  And the textures
are procedural, so a page needn't be stored before it is wanted: it can be baked when first asked for, kept, and
baked again after a material or a light changes.  Rage stored terabytes of painted texels; here detail costs bake
time.

How it works, after Rage (van Waveren's and Barrett's notes):

- The virtual texture is very large, with a full mip chain, every level cut into pages with a few texels of border
  copied from their neighbours so filtering works across a page's edge.  Each instance's layout has an address in it.
- Each frame the renderer finds which pages, at which levels, the pixels on screen want (a cheap draw of page numbers,
  read back).
- Wanted pages not in memory are read from the database, or baked, into a page cache of fixed size.  A page table
  says where each page is; drawing a texel looks up its page in the table, then reads the cache.  A page not there
  yet is drawn from a coarser level that is, so nothing is missing, only blurred for a frame or two.  The cache lets
  go of the least recently used.

Proposed, to decide when it starts: pages of 128 texels with a 4 texel border (120 used) and a cache 4096 texels
square (1024 pages); pages as database objects, compressed as images are saved now; a page baked at its own level
when first wanted, and marked to bake again by a change it touches; both renderers drawing it (Vulkan with the table
and cache as textures and a feedback draw, the rasteriser with the same lookup in software); lighting baked into the
pages.  The outline's steps: pages and the page table as objects with a bake of one page; the cache, lookup and
coarser pages in the rasteriser; feedback, loading and letting go in the rasteriser; Vulkan; baking in the
background; baked lighting.

## Decisions

The user said texturing is outside their expertise, so choices here are explained and a recommendation made each
time (2026-11-15).

1. Unwrapping lives in SupportLib: it is for any mesh, not only isosurfaces (the user, 2026-11-15).
2. Textures will be both baked and loaded; baked first (the user, 2026-11-15).
3. The texturing is a megatexture, as Rage's, on the object database (the user, 2026-11-15).
4. The user took every recommendation for it (2026-11-15):
    - texel density is one setting for the whole scene, texels per world unit, chosen by eye; every chart is laid out
      at its true size in texels, not scaled to fill an image;
    - every mip level is baked from the materials, not shrunk from the level above, and each level's gutters dilated;
    - charts are packed onto the page grid where they can be, with gutters of about 4 texels at the finest level;
    - one UV layer of unique texels, with lighting and decals baked into it, so no lightmap layer; tiling loaded
      textures are a second layer, for those materials only, later;
    - where a mesh's charts are is kept, so a change of material or light re-bakes the pages it touches without
      unwrapping again;
    - materials give a colour at a 3D point and are scaled in world units;
    - an instance that bakes lighting has texels of its own; small repeated props share their mesh's;
    - UVs are made ready for the virtual texture now, but the first textures are ordinary images, one a mesh.
5. After steps 1 to 8, asked one at a time (the user, 2026-11-16): the layout's size is kept on the UV layer; solid
   textures have a placement; every mip level is baked; a bake keeps a face's old colour as a tint; grown charts by
   LSCM stay the default; the solver is made faster and TestMeshUnwrap runs on the W65C816 again with coarser meshes
   there; ARAP is built (it became the default); the textured scene's three hundredths allowance stays and FMA stays
   on; texture generators come before steps 9 and 10.
6. LSCM is solved directly rather than by conjugate gradients (2026-11-16): conjugate gradients took about as many
   iterations as a chart has UVs, and TestMeshUnwrap fell from 12.5G to 3.66G cycles on the 65C64 and from 60.7G to
   13.5G on the W65C816 (with spheres of 16 by 8 and tori of 24 by 8 there), inside its 4G instructions.
7. Texel density, first look (2026-11-16): spheres of radius 1 with a quarter unit checker in WindowInputTester's
   isosurface tab looked "much better" to the user at 128 texels to the unit than at 64.  The value is still open.

## Not done

- The virtual texture (unwrap step 9): layouts placed at instances' addresses, pages baked on demand.  Open in its
  plan: the virtual texture's size (from the density and surface area), how the rasteriser finds feedback (noting
  pages as it draws), baking in the background on ThreadLib's jobs, page compression in the database and on the GPU
  (BC1 or BC7, or none at first), and what a page holds for each texel (colour alone now; height, shine, gloss and glow
  each cost bytes in every page once a renderer reads them).
- Loaded textures (unwrap step 10): a mesh's own UVs kept, tiling materials on a second layer.
- The scene's texel density value.
- How large a chart may be before it is split for packing: one wider than the page cache can't be drawn at its finest
  level all at once.
- Charts aren't placed on page boundaries, and each chart's rectangle (an array saved with the unwrap) isn't kept;
  both wait for the virtual texture.
- An instance scaled up has fewer texels to the unit than the density.  Proposed: allowed for now; an instance with
  its own copy could have a layout at its scale later.
- Charts too small aren't merged into a neighbour.
- Neither renderer reads a bake's normals (the bump image).
