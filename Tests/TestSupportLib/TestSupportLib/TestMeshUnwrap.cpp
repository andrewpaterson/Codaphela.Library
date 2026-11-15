#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshSphereEditor.h"
#include "SupportLib/MeshTorusEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshUnwrap.h"
#include "SupportLib/MeshUVMeasures.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Every face has UVs, none is turned over or outside, no two overlap, and
// the layout is whole pages.
//
//////////////////////////////////////////////////////////////////////////
static void AssertUnwrapped(CMesh* pcMesh, CMeshUnwrap* pcUnwrap)
{
	AssertTrue(pcMesh->mcUVs.InUse());
	AssertInt(0, CountMeshUVFacesMissing(pcMesh, 0));
	AssertInt(0, CountMeshUVFacesTurned(pcMesh, 0));
	AssertInt(0, CountMeshUVsOutside(pcMesh, 0));
	AssertInt(0, CountMeshUVOverlaps(pcMesh, 0));
	AssertTrue(pcUnwrap->miLayoutWidth > 0);
	AssertInt(0, pcUnwrap->miLayoutWidth % MESH_UNWRAP_PAGE);
	AssertInt(0, pcUnwrap->miLayoutHeight % MESH_UNWRAP_PAGE);
}


//////////////////////////////////////////////////////////////////////////
//
// A box 2 by 1 by 0.5 at 64 texels to the unit: six charts, one for each
// side, none stretched, all at 64 texels to the unit.  Its normals (made
// before the unwrap) are kept.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrapBox(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sZ;
	SFloat32Vec3		sStart;
	SMeshUVStretch		sStretch;

	sX.Init(2, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 0.5f);
	sStart.Init(-1, -0.5f, -0.25f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 2, 2, 2);
	pEditor->AddModifier<CMeshFlatFaces>();
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(64.0f);
	pMesh = pEditor->GetOutput();

	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertInt(6, pUnwrap->miNumCharts);
	AssertTrue(pMesh->mcNormals.InUse());
	MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 64.0f);
	AssertInt(pMesh->NumFaces(), sStretch.iNumFaces);
	AssertFloat(1.0f, sStretch.fWorstAngle, 4);
	AssertFloat(1.0f, sStretch.fLeastArea, 4);
	AssertFloat(1.0f, sStretch.fMostArea, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere: six charts, one about each axis, stretched as box projection
// stretches a slope (at most root 3 between two directions, and never
// more texels than the density).  A torus: unwrapped as well, in more
// charts.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrapCurved(void)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMeshUnwrap>	pUnwrap;
	Ptr<CMesh>			pMesh;
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sCentre;
	SMeshUVStretch		sStretch;

	sAxis.Init(0, 0, 1);
	sCentre.Init(0.3f, -0.2f, 0.1f);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshSphereEditor>(&sAxis, &sCentre, 1.0f, 24, 12);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(100.0f);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertInt(6, pUnwrap->miNumCharts);
	MeasureMeshUVStretch(&sStretch, &pMesh, 0, (float32)pUnwrap->miLayoutWidth, (float32)pUnwrap->miLayoutHeight, 100.0f);
	AssertTrue(sStretch.fWorstAngle < 1.75f);
	AssertTrue(sStretch.fMostArea < 1.001f);
	AssertTrue(sStretch.fLeastArea > 0.5f);

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshTorusEditor>(&sAxis, &sCentre, 2.0f, 0.5f, 32, 12);
	pUnwrap = pEditor->AddModifier<CMeshUnwrap>(100.0f);
	pMesh = pEditor->GetOutput();
	AssertUnwrapped(&pMesh, &pUnwrap);
	AssertTrue(pUnwrap->miNumCharts > 6);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshUnwrap(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestMeshUnwrapBox();
	ObjectsFlush();
	TestMeshUnwrapCurved();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

