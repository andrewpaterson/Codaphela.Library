# ProceduralLib

ProceduralLib makes meshes and textures from functions of a point in space. It sits on SupportLib, whose shapes
([`CShape`]), meshes ([`CMesh`], [`CMeshEditor`] and its modifiers), solid textures ([`CSolidTexture`]) and bake
([`CMeshBake`]) it extends, and on BaseLib's noise ([`BaseLib/Noise.h`]) and eigen solver
([`SFloat32Symmetric3x3`]). HCP65-Sculpt is built on it: a shape meshed as an isosurface, unwrapped, and a material's
graph baked onto it and edited as nodes. SupportLib's side is in its [Shapes](../supportlib/shapes.md) and
[Unwrapping and Virtual Textures](../supportlib/unwrapping.md) pages.

ProceduralLib isn't in an indexed solution, so its classes are named by their headers. Its tests are
TestProceduralLib (`Tests/TestProceduralLib`), which builds 32- and 64-bit, Debug and Release, ASan and Size16.

| Page | Covers |
|---|---|
| [Isosurfaces](isosurface.md) | any [`CShape`] to a mesh: dual contouring on a balanced octree, the plane fit, manifold cells, marching cubes as the baseline, CMeshShapeEditor and CMeshShapeNormals |
| [Texture Generators](textures.md) | noise, the solid texture generators and operators, the graph and materials built from them, materials as files, filtering and the bake |

## Its parts

| Part | Headers | Is |
|---|---|---|
| Sampling | [`ProceduralLib/IsosurfaceGrid.h`] | a grid's corner distances and edge crossings |
| Surface extraction | [`ProceduralLib/MarchingCubes.h`], [`ProceduralLib/IsosurfaceFit.h`], [`ProceduralLib/DualContouring.h`], [`ProceduralLib/IsosurfaceOctree.h`] | marching cubes, the vertex fit, dual contouring on a grid and on an octree |
| Mesh modifiers | [`ProceduralLib/MeshShapeEditor.h`], [`ProceduralLib/MeshShapeNormals.h`] | a shape as a mesh editor's source, and its normals from the shape |
| Generators | [`ProceduralLib/SolidFlat.h`], [`ProceduralLib/SolidGradient.h`], [`ProceduralLib/SolidNoise.h`], [`ProceduralLib/SolidCells.h`], [`ProceduralLib/SolidBricks.h`] | solid textures from nothing but a point |
| Operators | [`ProceduralLib/SolidOperator.h`] and the Solid* headers beside it | solid textures that read others: ramp, adjust, merge, mask, warp, slope, cavity, channel, combine |
| Materials | [`ProceduralLib/SolidMaterials.h`], [`ProceduralLib/SolidMaterialFile.h`] | graphs built by functions; a graph in a file |

## Decisions

- Generators, operators, materials and the isosurface are in ProceduralLib; what other libraries will want is below
  it: noise and the eigen solver in BaseLib (plain arithmetic, no objects), shapes, solid textures, their sample and
  the bake in SupportLib.
- Everything a scene or an editor holds is an object (the modifiers, every texture), so a graph is saved, loaded and
  edited as any objects are; the extraction classes are plain Init/Kill classes.
