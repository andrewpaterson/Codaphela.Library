# Shapes

A shape is a solid that says how far a point is from its surface: [`CShape`] and nothing else.  Isosurfaces mesh
them, and collision and picking are to ask them.  This page is the shape model: the five layers it sits in, the
shapes and their combinations, how shapes and meshes meet, and why the 2009 collision classes were deleted rather
than repaired.

The plan it was built from, [`SupportLib/ShapePlan.txt`], is finished.  It is kept as the record that
[`SupportLib/Shape.h`], [`ProceduralLib/MeshShapeEditor.h`] and [`ProceduralLib/Isosurface.txt`] cite by section and
step; its list of the 2009 classes' bugs is there.

## Five layers

Each layer has one job, and they meet in two places: the primitive shapes' mesh editors, and the bridges.

| Layer | Where | What |
|---|---|---|
| 1. Value geometry | BaseLib | Plain classes holding values and doing the arithmetic: [`SFloat32Plane`], [`SFloat32Sphere`], [`SFloat32Box`], [`SFloat32Bounds`], [`SFloat32Triangle`]. |
| 2. Point set algorithms | SupportLib | [`CQuickHull`]; bounding fits when something needs them. |
| 3. Meshes | SupportLib | [`CMesh`] and its details are the data; [`CMeshEditor`] and its modifiers make and change meshes. |
| 4. Shapes | SupportLib | [`CShape`] and everything derived from it. |
| 5. Bridges | ProceduralLib, SupportLib | Shape to mesh ([`ProceduralLib/MeshShapeEditor.h`]); mesh to shape (`CMeshShape`, not built). |

"Shape" means only a [`CShape`]: the file that cuts a polygon into faces for the OBJ and scene readers was renamed
from MeshPolygonShape.h to [`SupportLib/MeshPolygonCut.h`] for that.  [`SupportLib/VectorShapes.h`] and the geometry
map are 2D outlines for the vector rasteriser and are not shapes in this sense.

## Value geometry

The value classes sit beside [`SFloat32Vec3`] and [`SFloat4x4`] in BaseLib.  They hold values and no pointers into
other arrays, and aren't [`CObject`]s, so they go in arrays, free lists and a renderer's per-frame data, and RasterLib
(BaseLib and StandardLib only) can use them.

| Class | Holds | Gives |
|---|---|---|
| [`SFloat32Plane`] | a unit normal and a distance | Distance (n.p - d, negative behind it), Contains, ClosestPoint, Flip |
| [`SFloat32Sphere`] | a centre and a radius | Distance, Contains (a point or a triangle), Volume, IntersectionVolume, a fit to points |
| [`SFloat32Box`] | a centre, three unit axes (longest first) and half lengths | an oriented box: exact Distance, Contains, Volume, GetCorner |
| [`SFloat32Bounds`] | a minimum and a maximum | an axis aligned box: empty, Add, centre, size, exact Distance, Contains |
| [`SFloat32Triangle`] | three corners | Normal, Plane, Area, Center, ClosestPoint, unsigned Distance |

They keep the shapes' rules: Distance is negative inside, positive outside, and exactly zero is outside (Contains is
Distance < 0).  A triangle has no inside, so its distance is unsigned.  [`CArrayFloat32Plane`] is an array of planes.
Only what something needs is written: ray tests, segments and the like wait for a user.

The sphere fitted to points is centred on their average and reaches the furthest; the 2009 "best fit" (the two
points furthest apart) didn't always hold them all.

## Convex hulls

[`CQuickHull`] turns points into their convex hull: faces (coplanar triangles merged into polygons, corners in order,
each with Newell's plane) and the indices of the points they are made of.  It was written fresh, as the 2009 hull
generator never worked:

- it starts from a tetrahedron, so no point is lost in the plane of a flat start;
- a point is outside a face by more than QUICK_HULL_TOLERANCE (1e-5) of how far the points reach along x, y and z
  together;
- the faces a new point sees are found across neighbours from the one it is furthest in front of, so they always join
  up, and all of them are removed (removing only the face the point was given to fails the tests);
- fewer than four points, or points all in one plane, in a line or in one place, fail cleanly with no faces.

[`CConvexHull`] is the shape made from it (below).

## CShape

[`CShape`] is a [`CObject`]:

| Member | Meaning |
|---|---|
| [`CShape::Distance`] | How far a point is from the surface: negative inside, positive outside, zero is outside.  It may be a bound: never more than the true distance. |
| [`CShape::Gradient`] | Points away from the surface; needn't be a unit.  The base class takes central differences over SHAPE_GRADIENT_STEP (1e-4) of its bounds' longest side, or of 1. |
| [`CShape::GetBounds`] | An [`SFloat32Bounds`]; it may be as large as a float32 goes (a half space), and an isosurface is then given a box by its caller. |
| [`CShape::IsExact`] | Whether Distance is the distance, not only a bound.  True unless a class says otherwise. |
| [`CShape::Contains`] | Distance < 0. |
| [`CShape::AddMesh`] | Adds the mesh the shape's own mesh editor makes of it, with the same values; false for a shape with no mesh editor. |

Every shape is a solid.  A shape that can give its gradient exactly overrides [`CShape::Gradient`], as central
differences across a crease average its faces.  There are no open shapes: a mesh's faces are already triangles, and
[`SFloat32Triangle`] does their arithmetic.

Shapes are made with [`OMalloc`], their arguments being their Init's.  The primitives hold their value class as a
field that Class names (msSphere, msBox, msPlane), so the object machinery saves and loads them;
[`CConvexHull`] saves its arrays in a Save and Load of its own.

### Primitives

Each primitive is placed by its own values, as its mesh editor is, and takes them in its editor's order.

| Shape | Holds | Distance | Mesh editor |
|---|---|---|---|
| [`CSphere`] | [`SFloat32Sphere`] | exact; gradient a unit, nothing at the centre | [`CMeshSphereEditor`] |
| [`CBox`] | [`SFloat32Box`] | exact; gradient a unit | [`CMeshBoxEditor`] |
| [`CCylinder`] | start, axis, length, radius | exact, capped | [`CMeshCylinderEditor`] |
| [`CCone`] | start, axis, length, two radii | exact (Quilez's capped cone) | [`CMeshConeEditor`] |
| [`CTube`] | start, axis, length, inner and outer radii | exact | [`CMeshTubeEditor`] |
| [`CTorus`] | centre, axis, two radii | exact | [`CMeshTorusEditor`] |
| [`CCapsule`] | start, axis, length, radius | exact | none |
| [`CHalfSpace`] | [`SFloat32Plane`] | exact; gradient the normal; bounds as large as a float32 goes | none |
| [`CConvexHull`] | its faces' planes and its corners | exact inside and on it, a bound outside | none (made by [`CQuickHull`]) |

The cylinder, cone, tube, torus and capsule are [`CRevolvedShape`]s: a start, a unit axis and a length.  A point is
turned into its profile's plane (how far from the axis, how far along it), where
[`CRevolvedShape::ProfileDistance`] gives a 2D distance and gradient; turning a profile keeps them exact.  The
cylinder's and tube's profiles are rectangles, the cone's a trapezoid, the torus's a circle (its length is nothing)
and the capsule's a line less its radius.

A convex hull's distance is its furthest plane's, its gradient that plane's normal and its bounds its corners'.
Points [`CQuickHull`] can't make a hull of make it nothing, as far as can be from everywhere.

### Combinations

Combinations hold their children as [`Ptr`]s, so shapes form a graph like a scene, rooted by whatever holds it, and
freed with it.  A primitive is placed by its values; [`CShapeTransform`] moves a combination.

| Shape | Holds | Distance |
|---|---|---|
| [`CShapeUnion`] | any number of shapes | the nearest (min): a bound |
| [`CShapeIntersection`] | any number of shapes | the furthest (max): a bound |
| [`CShapeSubtraction`] | a shape and a cut | max(shape, -cut): a bound; the gradient is the cut's turned round where the cut wins |
| [`CShapeSmoothUnion`] | two shapes and a radius | Quilez's polynomial smooth min: a bound |
| [`CShapeTransform`] | one shape, a matrix and its inverse | the shape's at the point turned into its frame, times the smallest scale along the matrix's axes |

The union and intersection are [`CShapeGroup`]s: one [`CArray`] of shapes, the chosen one at each point giving the
distance and gradient (the first where two are as near).  With no shapes a group is nothing.

The smooth union is the blended form of the smooth min, so its gradient is the same blend of its two shapes'
gradients, exactly.  Its bounds grow by a quarter of its radius, as far as the blend can fill the seam.

The transform scales a distance back by the smallest scale along its matrix's axes, not the largest: a world
distance is at least the smallest scale times the shape's own, so only that is a bound.  It is exact for a scale the
same every way, and a matrix with no inverse fails its Init.

## Shapes and meshes

Meshes and shapes meet only through the bridges.  The procedural mesh shapes ([`CMeshBoxEditor`],
[`CMeshSphereEditor`] and the rest) stay mesh editors and are not [`CShape`]s; each primitive shape pairs with one.

Shape to mesh is a source modifier in ProceduralLib ([`ProceduralLib/MeshShapeEditor.h`]) that holds its
[`CShape`].  A primitive is made exactly by its own mesh editor with the same values, through [`CShape::AddMesh`],
with at least 6 segments round a circle ([`ShapeMeshSegments`]); anything else goes through the isosurface
([`ProceduralLib/Isosurface.txt`]).  This needed modifiers to be [`CObject`]s, so one can hold a [`Ptr`] to its
shape: a [`CMeshModifierStack`] keeps its modifiers in a [`CArray`] and makes one with
[`CMeshModifierStack::AddModifier`] (2026-11-15).

Mesh to shape is `CMeshShape`, planned and not built (Not done, below).  It is to hold a [`Ptr`] to a closed
[`CMesh`] and keep its own search structure, rebuilt when the mesh's change counters move, and never write into the
mesh.  The 2009 CMeshShape, which added its spheres' centres and its box's axes to the mesh's own positions and
normals, went.

## The 2009 classes

The collision classes of 2009 were three kinds of thing: pointer views onto a mesh's arrays (CSphere, CBox, CPlane,
CHalfSpace, CTriangle, CPolygon, CConvexHull and ShapeHelper's helpers), index records (CShapeIndexed and its kin)
and a mesh's collision data (CMeshShape with CMeshTriangles, CMeshLeaves, CMeshBoundingBox and CMeshConvexHull).
Nothing called them but four thin tests, and they were broken: the hull generator's free lists were set up with
their arguments the wrong way round once CFreeList's Init changed, it started from two flat faces that lose every
point in their plane, and it only removed faces with points left to see.  They were deleted with their tests, and
the names reused for the new shapes.  BaseLib's RemoveAtNoDeallocate went with them, as only the hull generator used
it.

Every bug found while reading them is listed in [`SupportLib/ShapePlan.txt`] ("Found while reading the 2009
classes").  TestBaseLib's value geometry tests check the ones in the triangle, sphere and box fits, and
[`TestSupportLib/TestSupportLib/TestQuickHull.cpp`] the hull generator's.

## Tests

The shapes are tested in TestSupportLib and the value geometry in TestBaseLib, and both run on the 65C64 and the
W65C816 in their `65` suites.

| Test | What it checks |
|---|---|
| [`TestSupportLib/TestSupportLib/TestQuickHull.cpp`] | A cube with points inside and on its faces and edges (six faces of four, only its corners are corners), a turned box, a cloud of 200 points and 120 on a sphere checked by brute force and Euler, and sets too few, flat, in a line or in one place. |
| [`TestSupportLib/TestSupportLib/TestShape.cpp`] | Distances, gradients against central differences away from creases, bounds, Contains at exactly zero, Save and Load (each shape to a file of its own, as a chunk file holds the object written to it last). |
| [`TestSupportLib/TestSupportLib/TestRevolvedShape.cpp`] | Along a tilted axis, and every corner of the mesh the shape's editor makes with the same values on its surface.  Zero being outside is tested along an axis, the only place a point is exactly on a surface. |
| [`TestSupportLib/TestSupportLib/TestShapeCombination.cpp`] | Each combination a bound against a surface sampled on a grid (bisected where Contains changes), a graph freed with what holds it, Save and Load.  With the largest scale the transform fails it. |
| [`TestSupportLib/TestSupportLib/TestConvexHull.cpp`] | A box's points make a hull whose distance is the box's inside exactly and no more outside; planes, corners and distances the same saved and read back. |

## Decisions

1. The base is [`CShape`]; it is a [`CObject`], and every shape extends it (the user, 2026-11-15).
2. The 2009 collision classes are not repaired: the five-layer model replaces them and they are deleted (the user,
   2026-11-15).  Asked what Claude would do given a free hand, the user took the model whole.
3. Value geometry is plain classes in BaseLib with no pointers and no [`CObject`], so arrays, free lists, per-frame
   render data and RasterLib can hold them (2026-11-15).
4. Distance is negative inside, positive outside, and exactly zero is outside, for the value classes and the shapes
   alike; Distance may be a bound but never more than the true distance, and [`CShape::IsExact`] says which
   (2026-11-15).
5. The procedural mesh editors stay mesh editors and pair with primitive shapes; "shape" means only [`CShape`], so
   MeshPolygonShape.h became MeshPolygonCut.h (2026-11-15).
6. There are no open shapes (2026-11-15).
7. Mesh modifiers are [`CObject`]s, so a modifier can hold a [`Ptr`] to a shape (2026-11-15, CommonLibraries
   c94c16cd8).
8. A transform scales back by its smallest scale, not its largest as the plan first said: only that is a bound
   (2026-11-15).
9. A mesh's own distance (`CMeshShape`) and the bounding fits wait until collision, picking or culling needs them
   (2026-11-15).

## Not done

- `CMeshShape` (plan step 9): a closed mesh's distance, the closest face through a bounding volume hierarchy over its
  faces, inside by winding number or pseudo normals (Baerentzen and Aanaes 2005).  It holds a [`Ptr`] to the
  [`CMesh`], builds its hierarchy (and hull and bounds if wanted) in arrays of its own, again when the mesh's change
  counters move, and never writes into the mesh.  When collision or picking needs it.
- Bounding fits: a minimal sphere, and an oriented box fitted to the hull.  When collision or culling needs them.
  These, and `CMeshShape`, are to be tested against the 2009 bugs that have no new code yet: the box's best fit, the
  leaves, the mesh's hull and its bounding box.
- Ray tests, segments and the like in the value geometry.
- [`CCookedMesh`]'s msMin and msMax, and RasterLib's integer bounds, could become [`SFloat32Bounds`].
- [`CCapsule`] and [`CHalfSpace`] have no mesh editor, so they are meshed only through the isosurface.
