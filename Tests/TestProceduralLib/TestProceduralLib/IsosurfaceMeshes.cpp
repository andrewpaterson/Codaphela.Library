#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "ProceduralLib/MarchingCubes.h"
#include "ProceduralLib/DualContouring.h"
#include "MeshMeasures.h"
#include "IsosurfaceMeshes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> MarchingCubesMesh(CShape* pcShape, float32 fCellSize)
{
	Ptr<CMesh>		pMesh;
	CMeshEditor		cEditor;
	CIsosurfaceGrid	cGrid;
	CMarchingCubes	cMarchingCubes;
	SFloat32Bounds	sBounds;

	GrownShapeBounds(&sBounds, pcShape, fCellSize);
	cGrid.Init(&sBounds, fCellSize);
	cGrid.Sample(pcShape);

	pMesh = OMalloc<CMesh>();
	cEditor.Init(pMesh);
	cMarchingCubes.Init();
	cMarchingCubes.Generate(&cEditor, &cGrid);
	cMarchingCubes.Kill();
	cEditor.Kill();
	cGrid.Kill();
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> DualContouringMesh(CShape* pcShape, float32 fCellSize)
{
	Ptr<CMesh>		pMesh;
	CMeshEditor		cEditor;
	CIsosurfaceGrid	cGrid;
	CDualContouring	cDualContouring;
	SFloat32Bounds	sBounds;

	GrownShapeBounds(&sBounds, pcShape, fCellSize);
	cGrid.Init(&sBounds, fCellSize);
	cGrid.Sample(pcShape);

	pMesh = OMalloc<CMesh>();
	cEditor.Init(pMesh);
	cDualContouring.Init();
	cDualContouring.Generate(&cEditor, &cGrid, pcShape);
	cDualContouring.Kill();
	cEditor.Kill();
	cGrid.Kill();
	return pMesh;
}

