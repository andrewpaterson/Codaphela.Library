#ifndef __MESH_MEASURES_H__
#define __MESH_MEASURES_H__
#include "SupportLib/Mesh.h"
#include "SupportLib/Shape.h"


//What the isosurface tests check a mesh by, as counts and distances for the tests to assert on.  TestSupportLib's own checks are
//static in another program.

//Edges with a face on one side only.  A closed mesh has none.
int		CountMeshOpenEdges(CMesh* pcMesh);

//Corners less edges plus faces: 2 for a ball, 0 for a ring.
int		MeshEulerCharacteristic(CMesh* pcMesh);

//Faces whose normal points into pcShape: against its gradient at the face's middle.  Faces with no area aren't counted.
int		CountMeshFacesFacingIn(CMesh* pcMesh, CShape* pcShape);

//Faces whose corners are in a line, or on each other.
int		CountMeshFacesWithNoArea(CMesh* pcMesh);

//The furthest any corner is from pcShape's surface.
float32	FurthestMeshCornerFromSurface(CMesh* pcMesh, CShape* pcShape);

//A face's normal, as CMeshNormals makes it (anticlockwise seen from outside), its length twice the face's area, and the middle
//of its corners.
void	MeshFaceNormal(CMesh* pcMesh, int iFace, SFloat32Vec3* psNormal, SFloat32Vec3* psMiddle);


#endif // __MESH_MEASURES_H__

