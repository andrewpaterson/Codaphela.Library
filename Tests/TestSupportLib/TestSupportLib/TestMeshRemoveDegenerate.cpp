#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshModifierStack.h"
#include "SupportLib/MeshRemoveDegenerate.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshRemoveDegenerateAssertCorners(CMesh* pcMesh, int iFace, int iCorner1, int iCorner2, int iCorner3)
{
	int*	piCorners;

	piCorners = pcMesh->GetFaceCorners(iFace);
	AssertInt(iCorner1, piCorners[0]);
	AssertInt(iCorner2, piCorners[1]);
	AssertInt(iCorner3, piCorners[2]);
}


//////////////////////////////////////////////////////////////////////////
//
// Two faces with area, either side of three without: one in a line, one
// with two corners in the same place and one with a corner twice.  The last
// face has corner normals and colours.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshRemoveDegenerateMesh(void)
{
	Ptr<CMesh>			pMesh;
	SMeshNormalFace*	psNormalFace;
	int					k;

	pMesh = OMalloc<CMesh>();
	pMesh->mcNormals.mbInUse = true;
	pMesh->mcColours.mbInUse = true;
	pMesh->mcColours.mcColours.Add(0xFF111111);
	pMesh->mcColours.mcColours.Add(0xFF222222);

	pMesh->AddCorner(0, 0, 0);
	pMesh->AddCorner(100, 0, 0);
	pMesh->AddCorner(0, 100, 0);
	pMesh->AddCorner(50, 0, 0);
	pMesh->AddCorner(0, 0, 0);
	pMesh->AddCorner(100, 100, 0);
	pMesh->AddFace(0, 1, 2);
	pMesh->AddFace(0, 3, 1);
	pMesh->AddFace(0, 4, 2);
	pMesh->AddFace(1, 1, 2);
	pMesh->AddFace(1, 5, 2);
	pMesh->SetFaceName(0, 7);
	pMesh->SetFaceName(4, 8);

	pMesh->AddNormal(0, 0.6f, 0.8f);
	psNormalFace = pMesh->mcNormals.mcFaces.Get(4);
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		psNormalFace->aiCornerNormals[k] = 5;
		pMesh->mcColours.mcFaces.Get(4)->aiCornerColours[k] = 1;
	}
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// The faces with no area go, and so does the corner only they used.  The
// faces that stay keep their names, normals and colours, and the source is
// left as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshRemoveDegenerateRemovesFaces(void)
{
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pSource;
	Ptr<CMesh>				pOutput;
	CMeshRemoveDegenerate*	pcRemove;
	SMeshNormalFace*		psNormalFace;
	int						k;

	pSource = TestMeshRemoveDegenerateMesh();
	AssertFalse(CMeshRemoveDegenerate::IsDegenerate(&pSource, 0));
	AssertTrue(CMeshRemoveDegenerate::IsDegenerate(&pSource, 1));
	AssertTrue(CMeshRemoveDegenerate::IsDegenerate(&pSource, 2));
	AssertTrue(CMeshRemoveDegenerate::IsDegenerate(&pSource, 3));
	AssertFalse(CMeshRemoveDegenerate::IsDegenerate(&pSource, 4));

	cStack.Init(pSource);
	pcRemove = cStack.AddModifier<CMeshRemoveDegenerate>();
	pcRemove->Init();
	pOutput = cStack.GetOutput();
	AssertFalse(pOutput == pSource);
	AssertSize(3, pcRemove->NumRemoved());

	AssertInt(2, pOutput->NumFaces());
	AssertInt(4, pOutput->NumCorners());
	TestMeshRemoveDegenerateAssertCorners(&pOutput, 0, 0, 1, 2);
	TestMeshRemoveDegenerateAssertCorners(&pOutput, 1, 1, 3, 2);
	AssertFloat(100.0f, pOutput->GetPosition(3)->x, 3);
	AssertFloat(100.0f, pOutput->GetPosition(3)->y, 3);
	AssertInt(7, pOutput->GetFaceName(0));
	AssertInt(8, pOutput->GetFaceName(1));

	//The corner normal is numbered after the two faces' flat normals.
	AssertSize(3, pOutput->mcNormals.mcNormals.NumElements());
	psNormalFace = pOutput->mcNormals.mcFaces.Get(1);
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		AssertInt(2, psNormalFace->aiCornerNormals[k]);
		AssertInt(1, pOutput->mcColours.mcFaces.Get(1)->aiCornerColours[k]);
	}
	AssertFloat(0.8f, pOutput->mcNormals.mcNormals.Get(2)->z, 3);
	AssertInt((int)0xFF222222, (int)pOutput->mcColours.mcColours.GetValue(1));

	AssertInt(5, pSource->NumFaces());
	AssertInt(6, pSource->NumCorners());

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh whose faces all have area comes back as itself, even a sliver far
// thinner than a renderer's unit, and so does one with what a new mesh
// wouldn't keep.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshRemoveDegenerateNothingToDo(void)
{
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pMesh;
	CMeshRemoveDegenerate*	pcRemove;
	SLogConfig				sLogConfig;

	pMesh = OMalloc<CMesh>();
	pMesh->AddCorner(0, 0, 0);
	pMesh->AddCorner(100, 0, 0);
	pMesh->AddCorner(50, 0.000001f, 0);
	pMesh->AddFace(0, 1, 2);
	cStack.Init(pMesh);
	pcRemove = cStack.AddModifier<CMeshRemoveDegenerate>();
	pcRemove->Init();
	AssertTrue(cStack.GetOutput() == pMesh);
	AssertSize(0, pcRemove->NumRemoved());
	cStack.Kill();

	pMesh = TestMeshRemoveDegenerateMesh();
	pMesh->mcMaterials.mbInUse = true;
	cStack.Init(pMesh);
	pcRemove = cStack.AddModifier<CMeshRemoveDegenerate>();
	pcRemove->Init();
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(cStack.GetOutput() == pMesh);
	gcLogger.SetConfig(&sLogConfig);
	AssertSize(0, pcRemove->NumRemoved());
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshRemoveDegenerate(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshRemoveDegenerateRemovesFaces();
	TestMeshRemoveDegenerateNothingToDo();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
