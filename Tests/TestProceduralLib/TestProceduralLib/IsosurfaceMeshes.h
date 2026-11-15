#ifndef __ISOSURFACE_MESHES_H__
#define __ISOSURFACE_MESHES_H__
#include "SupportLib/Mesh.h"
#include "SupportLib/Shape.h"


//pcShape meshed on a grid of fCellSize around it, a cell bigger than its bounds every way.
Ptr<CMesh>	MarchingCubesMesh(CShape* pcShape, float32 fCellSize);
Ptr<CMesh>	DualContouringMesh(CShape* pcShape, float32 fCellSize);


#endif // __ISOSURFACE_MESHES_H__

