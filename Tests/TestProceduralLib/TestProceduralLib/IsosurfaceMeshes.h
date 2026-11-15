#ifndef __ISOSURFACE_MESHES_H__
#define __ISOSURFACE_MESHES_H__
#include "SupportLib/Mesh.h"
#include "SupportLib/Shape.h"


//pcShape meshed on a grid of fCellSize around it, a cell bigger than its bounds every way.
Ptr<CMesh>	MarchingCubesMesh(CShape* pcShape, float32 fCellSize);
Ptr<CMesh>	DualContouringMesh(CShape* pcShape, float32 fCellSize);

//pcShape meshed on an octree from fSmallest to fLargest cells, split at 30 degrees and a centre fCentreTolerance off its
//corners.  piLeaves is how many leaves it had, if not NULL.
Ptr<CMesh>	OctreeMesh(CShape* pcShape, float32 fSmallest, float32 fLargest, float32 fCentreTolerance, int* piLeaves);


#endif // __ISOSURFACE_MESHES_H__

