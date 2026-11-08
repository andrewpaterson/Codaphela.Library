#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshFixReversed.h"
#include "SupportLib/MeshModifierStack.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshFixReversedAssertCorners(CMesh* pcMesh, int iFace, int iCorner1, int iCorner2, int iCorner3)
{
	int*	piCorners;

	piCorners = pcMesh->GetFaceCorners(iFace);
	AssertInt(iCorner1, piCorners[0]);
	AssertInt(iCorner2, piCorners[1]);
	AssertInt(iCorner3, piCorners[2]);
}


//////////////////////////////////////////////////////////////////////////
//
// Three triangles with corner normals facing up +Z, two of which wind the
// other way, and one with none of its own that can't be wrong.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshFixReversedMesh(void)
{
	Ptr<CMesh>			pMesh;
	SMeshNormalFace*	psNormalFace;
	int					i;
	int					k;

	pMesh = OMalloc<CMesh>();
	pMesh->mcNormals.mbInUse = true;
	pMesh->mcColours.mbInUse = true;
	pMesh->mcColours.mcColours.Add(0xFF111111);
	pMesh->mcColours.mcColours.Add(0xFF222222);
	pMesh->mcColours.mcColours.Add(0xFF333333);

	pMesh->AddCorner(0, 0, 0);
	pMesh->AddCorner(100, 0, 0);
	pMesh->AddCorner(0, 100, 0);
	pMesh->AddFace(0, 1, 2);
	pMesh->AddFace(0, 2, 1);
	pMesh->AddFace(0, 2, 1);
	pMesh->AddFace(2, 1, 0);

	pMesh->SetFaceName(1, 9);

	pMesh->AddNormal(0, 0, 1);
	pMesh->AddNormal(0, 0.6f, 0.8f);
	for (i = 0; i < 4; i++)
	{
		if (i != 2)
		{
			psNormalFace = pMesh->mcNormals.mcFaces.Get(i);
			psNormalFace->aiCornerNormals[0] = 4;
			psNormalFace->aiCornerNormals[1] = 5;
			psNormalFace->aiCornerNormals[2] = 4;
		}
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			pMesh->mcColours.mcFaces.Get(i)->aiCornerColours[k] = k;
		}
	}
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshFixReversedTurnsFaces(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	CMeshFixReversed*	pcFix;
	SMeshNormalFace*	psNormalFace;
	int*				piColours;

	pSource = TestMeshFixReversedMesh();
	cStack.Init(pSource);
	pcFix = cStack.AddModifier<CMeshFixReversed>();
	pcFix->Init();

	pOutput = cStack.GetOutput();
	AssertFalse(pOutput == pSource);
	AssertSize(2, pcFix->NumFixed());

	TestMeshFixReversedAssertCorners(&pOutput, 0, 0, 1, 2);
	TestMeshFixReversedAssertCorners(&pOutput, 1, 0, 1, 2);
	TestMeshFixReversedAssertCorners(&pOutput, 2, 0, 2, 1);
	TestMeshFixReversedAssertCorners(&pOutput, 3, 2, 0, 1);

	AssertInt(9, pOutput->GetFaceName(1));
	AssertInt(0, pOutput->GetFaceName(0));

	//The source stays reversed.
	TestMeshFixReversedAssertCorners(&pSource, 1, 0, 2, 1);
	TestMeshFixReversedAssertCorners(&pSource, 3, 2, 1, 0);

	//What was on each corner goes with it.
	psNormalFace = pOutput->mcNormals.mcFaces.Get(1);
	AssertInt(4, psNormalFace->aiCornerNormals[0]);
	AssertInt(4, psNormalFace->aiCornerNormals[1]);
	AssertInt(5, psNormalFace->aiCornerNormals[2]);
	piColours = pOutput->mcColours.mcFaces.Get(1)->aiCornerColours;
	AssertInt(0, piColours[0]);
	AssertInt(2, piColours[1]);
	AssertInt(1, piColours[2]);

	//Corner normals and colours are at the same indices, and flat normals face as the face now winds.
	AssertSize(6, pOutput->mcNormals.mcNormals.NumElements());
	AssertFloat(0.8f, pOutput->mcNormals.mcNormals.Get(5)->z, 3);
	AssertFloat(1.0f, pOutput->mcNormals.mcNormals.Get(1)->z, 3);
	AssertFloat(-1.0f, pOutput->mcNormals.mcNormals.Get(2)->z, 3);
	AssertInt((int)0xFF333333, (int)pOutput->mcColours.mcColours.GetValue(2));

	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With nothing reversed the mesh itself comes back, and that's so of a fixed
// one, one with no normals and one with what a new mesh wouldn't keep.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshFixReversedNothingToDo(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pMesh;
	Ptr<CMesh>			pOutput;
	CMeshFixReversed*	pcFix;
	SLogConfig			sLogConfig;

	pMesh = TestMeshFixReversedMesh();
	cStack.Init(pMesh);
	pcFix = cStack.AddModifier<CMeshFixReversed>();
	pcFix->Init();
	pOutput = cStack.GetOutput();

	cStack.Flatten();
	pcFix = cStack.AddModifier<CMeshFixReversed>();
	pcFix->Init();
	AssertTrue(cStack.GetOutput() == pOutput);
	AssertSize(0, pcFix->NumFixed());
	cStack.Kill();

	pMesh = OMalloc<CMesh>();
	pMesh->AddCorner(0, 0, 0);
	pMesh->AddCorner(100, 0, 0);
	pMesh->AddCorner(0, 100, 0);
	pMesh->AddFace(0, 1, 2);
	cStack.Init(pMesh);
	pcFix = cStack.AddModifier<CMeshFixReversed>();
	pcFix->Init();
	AssertTrue(cStack.GetOutput() == pMesh);
	cStack.Kill();

	pMesh = TestMeshFixReversedMesh();
	pMesh->mcMaterials.mbInUse = true;
	cStack.Init(pMesh);
	pcFix = cStack.AddModifier<CMeshFixReversed>();
	pcFix->Init();
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(cStack.GetOutput() == pMesh);
	gcLogger.SetConfig(&sLogConfig);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshFixReversed(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshFixReversedTurnsFaces();
	TestMeshFixReversedNothingToDo();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
