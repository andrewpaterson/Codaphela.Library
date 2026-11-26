# Isosurfaces

ProceduralLib makes a [`CMesh`] from any shape that can say how far a point is from its surface: SupportLib's
[`CShape`]s (spheres, boxes, cylinders, convex hulls, transforms) and what is made by joining and cutting them. The
surface is found by dual contouring on an octree, with each cell's vertex fitted to the planes at its crossings (Ju,
Losasso, Schaefer and Warren 2002), so a crease, a corner or a notch narrower than a cell comes out where it is.
Marching cubes is built too, as the baseline the dual contouring is measured against. The mesh is an ordinary
[`CMesh`], so a [`CMeshEditor`], a [`CScene`] and both renderers take it as they take any other. HCP65-Sculpt meshes
its shapes this way.

ProceduralLib isn't in an indexed solution, so its classes are named here by their headers.

## The parts

| Header | Holds | Is |
|---|---|---|
| [`ProceduralLib/IsosurfaceGrid.h`] | CIsosurfaceGrid, IsosurfaceInside, IsosurfaceFindCrossing | a uniform grid's corner distances and edge crossings |
| [`ProceduralLib/MarchingCubes.h`] | CMarchingCubes, SMarchingCubesCase | the baseline, and the cases manifold dual contouring splits cells by |
| [`ProceduralLib/IsosurfaceFit.h`] | SIsosurfaceFit | a cell's vertex: where the planes at its crossings best meet |
| [`ProceduralLib/DualContouring.h`] | CDualContouring, IsosurfaceJoinPieces, IsosurfaceAddQuad | dual contouring on a uniform grid |
| [`ProceduralLib/IsosurfaceOctree.h`] | CIsosurfaceOctree | dual contouring on a balanced octree |
| [`ProceduralLib/MeshShapeEditor.h`] | CMeshShapeEditor | any [`CShape`] to a mesh, as a source modifier |
| [`ProceduralLib/MeshShapeNormals.h`] | CMeshShapeNormals | a shape's mesh's normals from the shape |
| [`BaseLib/Float32Symmetric3x3.h`] | [`SFloat32Symmetric3x3`] | the 3x3 symmetric Jacobi eigen solver the fit uses |

They are plain Init/Kill classes, except the two modifiers, which are objects holding a `Ptr<CShape>`.

## What a shape must give

[`CShape::Distance`] is negative inside, positive outside and zero on the surface, and [`CShape::Gradient`] points
out. What the isosurface needs of them:

- Exactly zero is outside. A box whose faces lie on the grid's planes samples zeros on them, and taking zero both
  ways gives those faces twice or not at all. Every sign test goes through IsosurfaceInside.
- The distance may be a bound, never more than the true distance, as empty space is skipped by it. min and max of
  exact distances are bounds, and so is a smooth union, so joining and cutting keep this.
- A shape that can give its gradient exactly should: central differences across a crease average its two faces, and
  the fit then can't find the crease. The gradient needn't be a unit; the crossing normalises it.

A convex hull's gradient is its winning face's normal, exactly the plane the fit wants, so a hull's faces come out
flat and its edges sharp. Where two shapes meet, each crossing takes its normal from whichever shape wins there, so a
cylinder pushed into a box has a sharp seam.

## Sampling

CIsosurfaceGrid works out the shape's distance at each corner of a uniform grid once, and on each edge whose ends
differ in sign finds the crossing by the Illinois method (the secant through the bracket, the end that stays twice
halved; 16 steps at most), with the shape's gradient there as its normal. Linear interpolation alone puts a crossing
off a curved surface. Its arrays are large ones ([`CArrayLargeTemplate`]), as a grid passes 64 KB with the W65C816's
16-bit size.

## Marching cubes

CMarchingCubes is the baseline: each crossing one mesh corner, shared by every triangle on it. Its 256 cases are made
in Init, not typed in from Bourke's tables: each face of the cube is walked anticlockwise seen from outside, and a
line runs from each crossing into the inside to where the walk next comes out. A face's lines depend on its four
corners alone, so the two cubes either side agree and nothing cracks, and two inside corners across a face are kept
apart. The lines join into loops (the case's pieces), which are fanned into triangles. DecideCase settles a case's
ambiguous faces by the asymptotic decider (Nielson and Hamann 1991), for manifold dual contouring. Classic marching
cubes' fully disambiguated cases (MC33) are not built: marching cubes is only the baseline.

A gyroid cut by a sphere ([`TestProceduralLib/TestGyroid.h`]) meets every case. Marching cubes bevels every crease:
a box's edge is cut off by up to a cell, and finer cells only make the bevel narrower.

## Placing a cell's vertex

The vertex minimises the sum over the cell's crossings of (n . (x - p))^2: the point where the planes through the
crossings, along their normals, best meet. SIsosurfaceFit keeps A'A (an [`SFloat32Symmetric3x3`]), A'b and b'b, small
sums that add up, with positions relative to the cell's centre so a far away cell loses nothing to cancelling.

Solve works from the crossings' mass point with a pseudo inverse of A'A
([`SFloat32Symmetric3x3::PseudoInverseMultiply`]): eigenvalues under a hundredth of the largest count as zero
(ISOSURFACE_FIT_CUT_OFF). So along a direction no plane fixes (along an edge, across a flat face, anywhere in a
sphere's cell) the vertex stays at the mass point, and two or three planes still give their edge or corner exactly.
Ju et al. drop A's singular values under a tenth, which is a hundredth of A'A's eigenvalues; a tenth of the
eigenvalues dropped a face crossed at one or two of a cell's edges when another was crossed at many, and the vertex
slid off past their edge. A vertex outside its cell is clamped back in, and the mass point is used if that fits worse.
Letting the vertex out of its cell by even a tenth of one folded faces over, so it stays clamped.

The solver is in BaseLib, in float32 (BaseLib compiles for the 65C64 and W65C816), as plain arithmetic something else
may want.

## Making the faces

Each crossed edge is shared by four cells, and their four vertices make a quad, wound from the edge's inside end to its
outside end so it faces out (anticlockwise from outside, as a scene file's polygons are). Each vertex is one mesh
corner shared by every face on it, so the mesh is connected and CMesh's connectivity and the modifiers work on it.

IsosurfaceAddQuad adds the quad as two triangles: the split whose triangles both face along the crossing's normal if
only one does, else the one whose diagonal's middle is nearer the surface (across a box's edge two of the quad's
corners are on the edge, and the diagonal between them follows it), else the one whose smaller triangle is larger.
The split whose triangles agree best is wrong here: across a box's edge it cuts through the box. So the quads go in
as two faces, not through [`CMesh::AddQuad`], which picks the flattest split ([`IsMeshQuadCutFirstToThird`]), right
for a general mesh.

## The octree

CIsosurfaceOctree covers a box from cells of a largest size down to a smallest. Build splits a cell from the root
down:

- not at all where its centre is further from the surface than half its diagonal: there is no surface in it, even by
  a bound. This is what a distance buys over an inside test, and a scene mostly of empty space costs little;
- always while it's bigger than the largest size;
- where its centre's distance differs from its corners' average by more than the centre tolerance (a notch or bump
  thinner than the cell, a curve the corners can't see), or its crossings' normals differ by more than the split
  angle (a crease, or a bend tighter than the cell).

Generate makes faces on the minimal edges, each leaf's edges with no smaller leaf around them: each crossed one is a
quad, or a triangle where one leaf is two of the four places round it, of the leaves around it. Nothing cracks
between sizes, as every face is made from the smallest edges. Each crossing is added to the fit of every leaf
around its edge, so a big leaf beside small ones still has a vertex.

Two things against faces folding over where sizes change: the tree is balanced (leaves beside each other differ by
one size at most), and a crossing counts in a leaf's fit by its edge's length over the leaf's size, so a big leaf's
many small neighbours don't pull its vertex over theirs. Balancing took a turned box from 43 faces facing in to 1.

Corner distances are worked out again where cells meet, not shared: a distance is the same each time, so cells agree
on their signs, and a shared table keyed by finest-grid position would need a map counting past a 16-bit size.
Splitting by distance needs a bound, so the test gyroid is divided by its steepest slope (root 6 times its scale).

## Manifold dual contouring

One vertex per cell joins two sheets of surface at one point where a cell holds parts of the surface that don't
touch, and the mesh is then not a manifold. CDualContouring gives a cell a vertex for each of its sheets (Schaefer,
Ju and Warren 2007): the cell's marching cubes pieces, its ambiguous faces settled by the asymptotic decider, joined
(IsosurfaceJoinPieces) unless their average normals are more than about 100 degrees apart
(ISOSURFACE_SHEETS_COSINE). Without the joining a box's edge just outside a cell, which leaves its two faces as two
pieces, dented the box. The octree splits only its smallest leaves' vertices; bigger leaves keep one, as their normals
agree. A gyroid had 9 pinched corners on a uniform grid and 12 on the octree, and has 0 and 1 (2026-11-15).

## Shape to mesh

CMeshShapeEditor is a [`CMeshSourceModifier`] holding a `Ptr<CShape>`, so it sits on a [`CMeshEditor`]'s stack, and
a change to the shape meshes it again. A primitive with a mesh editor of its own ([`CShape::AddMesh`]: sphere, box,
cylinder, cone, tube, torus) is that editor's mesh exactly, 32 segments round its circles. Anything else is an
isosurface: CIsosurfaceOctree over the shape's bounds grown by a smallest cell, split at 30 degrees and a centre
tolerance of a tenth of the smallest cell. A shape with endless bounds (a half space) and no editor makes nothing.

It makes faces only. CMeshShapeNormals, a [`CMeshModifier`] after it, gives normals from the shape: at a corner whose
faces' sides (the gradient a tenth of the way toward each face's middle) agree within 45 degrees, every face shares
the gradient at the corner; at a crease each face corner has its own side's, so creases stay sharp and a cylinder is
round right up to its rim. A face corner whose normal would be more than 45 degrees from its face keeps its flat
normal. A mesh with UVs, skin or materials is left as it was ([`CMeshModifier::CanRebuild`]).

[`CMeshFlatFaces`] then [`CMeshMergeNormals`] were used first. Merging at 30 degrees split corners wherever coarse
faces fanned through more (4 of 5300 corners on five spheres smoothly joined), which showed as creases; the user found
the shape's normals much better (2026-11-15).

## What it measured

Counts from TestProceduralLib when each step was built (2026-11-15), at cells of 0.25 unless said:

| Case | Marching cubes | Dual contouring |
|---|---|---|
| A turned box's volume | 4% short | within 1%, corners exact, area right to 2 decimals |
| Corners on a cylinder's rims | 16 | 80, volume within 0.5% |
| Corners on a box-cylinder union's seam | 0 | 22 |

On the octree, cells 1 to 0.0625: a sphere has a quarter of a uniform grid's corners and a box half, a V notch a
third of the largest cell is kept, and a gyroid cut by a sphere is closed with none of its own faces folded.

## Known limits

- A pinch can still be left where two sheets' normals are nearer than about 100 degrees, or in an octree leaf
  bigger than the smallest.
- Marching cubes' pieces are only what the corners show: a cell can hold pieces of two faces whose edge passes
  outside it, where the vertex, clamped back in, is up to 0.3 of a cell off.
- A corner can be cut off when the cell holding it doesn't cross all three of its faces: an octree box's corners are
  within a quarter of the smallest cell. Splitting by normal angle usually reaches it.
- The vertex is a fit, so a surface may cross itself near a tight crease; the clamp keeps it within its cell. A
  union's two slivers at a cylinder's rim, a thousandth apart, can face either way.
- Nothing thinner than the smallest cell is expected to survive; a sphere smaller than it may vanish.

## Tests

TestProceduralLib, one file per class ([`TestProceduralLib/TestIsosurfaceGrid.cpp`],
[`TestProceduralLib/TestMarchingCubes.cpp`], [`TestProceduralLib/TestIsosurfaceFit.cpp`],
[`TestProceduralLib/TestDualContouring.cpp`], [`TestProceduralLib/TestIsosurfaceOctree.cpp`],
[`TestProceduralLib/TestMeshShapeEditor.cpp`], [`TestProceduralLib/TestMeshShapeNormals.cpp`]). Its own mesh checks
are in [`TestProceduralLib/MeshMeasures.h`]: open edges, the Euler characteristic, faces facing in, faces with no
area, pinched corners, the furthest corner from the surface, volume. They are tried on SupportLib's editors' meshes
and on a mesh turned inside out ([`TestProceduralLib/TestMeshMeasures.cpp`]). [`TestProceduralLib/IsosurfaceMeshes.h`]
meshes a shape each way; [`TestProceduralLib/TestGyroid.h`] is the gyroid.

[`TestProceduralLib/TestShapeScene.cpp`] draws a small scene of shapes with [`CRasterFrame`] and compares it with
saved pixels within 8 a channel in all but a hundredth of them, not byte for byte: optimised g++ contracts
`a * b + c` into FMA, so Release and Debug floats differ in the last bits and move a few pixels a level or two. It is
left out of Size16 builds, as RenderLib's cooked mesh lists count with a size.

[`ProceduralLib/Isosurface.txt`] is the plan this was built from, step by step (8.1 to 8.8 of SupportLib's
[`SupportLib/ShapePlan.txt`]). The code's comments and ShapePlan cite its sections and steps, so it is kept as that
record.

## Decisions

- Dual contouring on an octree with a plane fit for each vertex, not plain marching cubes, so a V, a mitred elbow or
  a cube's edge keeps its crease; detail goes where the shape needs it (the user, 2026-11-15).
- Spheres, cylinders, cubes and convex hulls must all work, and so must what is made by joining and cutting them (the
  user, 2026-11-15).
- Marching cubes is built as the baseline, without MC33's cases (2026-11-15).
- The shape is a [`CShape`] and the isosurface takes a `Ptr<CShape>` (the user's decision in ShapePlan).
- The 3x3 symmetric Jacobi solver is in BaseLib (the user, 2026-11-15).
- The fit's cut off is a hundredth of A'A's largest eigenvalue, and the vertex stays clamped in its cell
  (2026-11-15, measured).
- Normals come from the shape (CMeshShapeNormals). The user first took [`CMeshMergeNormals`] after flat faces
  (2026-11-15), then judged the shape's normals much better the same day.
- A closed [`CMesh`] as a shape (closest triangle through a spatial index) waits until something needs it: it is far
  slower than the shapes above.

## Not done

- Corner distances aren't shared between neighbouring cells.
- The fit is float32, relative to the cell's centre; whether far away cells need float64 is untested.
- UVs: an isosurface has none. A box projection is the simplest; triplanar blending needs three samples and isn't
  something a [`CMesh`] can hold. (HCP65-Sculpt unwraps the mesh with [`CMeshUnwrap`] instead.)
- Materials: a distance query could also say which shape won, so each face gets its shape's material slot.
- Coordinates: SupportLib's scene plan's open question applies (a scene file is right handed with y up).
- Threads: cells are independent once their corners are known, so the octree's leaves could be done on [`CJobs`].
- Level of detail between blocks of an octree made separately (Transvoxel, Lengyel 2010) waits for something big
  enough to need it.
