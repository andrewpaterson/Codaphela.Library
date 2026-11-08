#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshModifierStack.h"
#include "SupportLib/MeshWeldCorners.h"
#include "TestLib/Assert.h"


#define TEST_MESH_WELD_GREY		0
#define TEST_MESH_WELD_DARK		1


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshWeldNewMesh(void)
{
	Ptr<CMesh>	pMesh;

	pMesh = OMalloc<CMesh>();
	pMesh->mcNormals.mbInUse = true;
	pMesh->mcColours.mbInUse = true;
	pMesh->mcColours.mcColours.Add(0xFF808080);
	pMesh->mcColours.mcColours.Add(0xFF404040);
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshWeldAddFace(CMesh* pcMesh, int iCorner1, int iCorner2, int iCorner3, int iColour, int iNormal)
{
	CMFRet				r;
	SMeshNormalFace*	psNormalFace;
	int					i;

	r = pcMesh->AddFace(iCorner1, iCorner2, iCorner3);
	psNormalFace = pcMesh->mcNormals.mcFaces.Get(r.miFirstFace);
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		pcMesh->mcColours.mcFaces.Get(r.miFirstFace)->aiCornerColours[i] = iColour;
		psNormalFace->aiCornerNormals[i] = iNormal;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshWeldOutput(CMeshModifierStack* pcStack, Ptr<CMesh> pSource)
{
	pcStack->Init(pSource);
	pcStack->AddModifier<CMeshWeldCorners>()->Init();
	return pcStack->GetOutput();
}


//////////////////////////////////////////////////////////////////////////
//
// A folded square of two faces with six corners between them comes back with
// four, which didn't move.  The faces keep their own normals, so the fold stays
// sharp, and their names.  The source is left as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshWeldCornersMerges(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SFloat32Vec3*		psNormal;
	int*				piFirst;
	int*				piSecond;

	pSource = TestMeshWeldNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(0, 10, 5);
	pSource->AddCorner(0, 0, 0);
	TestMeshWeldAddFace(&pSource, 0, 1, 2, TEST_MESH_WELD_GREY, 2);
	TestMeshWeldAddFace(&pSource, 3, 4, 5, TEST_MESH_WELD_GREY, 3);
	pSource->SetFaceName(1, 8);
	pSource->AddNormal(0, 0, 1);
	pSource->AddNormal(0.6f, 0, 0.8f);

	pOutput = TestMeshWeldOutput(&cStack, pSource);

	AssertInt(6, pSource->NumCorners());
	AssertInt(2, pSource->NumFaces());
	AssertFalse(pSource->GetConnectivity()->IsGenerated());

	AssertInt(4, pOutput->NumCorners());
	AssertInt(2, pOutput->NumFaces());
	AssertFalse(pOutput->GetConnectivity()->IsGenerated());
	AssertInt(0, pOutput->GetFaceName(0));
	AssertInt(8, pOutput->GetFaceName(1));

	piFirst = pOutput->GetFaceCorners(0);
	piSecond = pOutput->GetFaceCorners(1);
	AssertInt(piFirst[2], piSecond[0]);
	AssertInt(piFirst[0], piSecond[2]);
	AssertFloat(10, pOutput->GetPosition(piFirst[2])->y, 3);
	AssertFloat(5, pOutput->GetPosition(piSecond[1])->z, 3);

	psNormal = pOutput->mcNormals.mcNormals.Get(pOutput->mcNormals.mcFaces.Get(0)->aiCornerNormals[0]);
	AssertFloat(1, psNormal->z, 3);
	psNormal = pOutput->mcNormals.mcNormals.Get(pOutput->mcNormals.mcFaces.Get(1)->aiCornerNormals[0]);
	AssertFloat(0.6f, psNormal->x, 3);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Faces of different colours don't join, even where they shared a corner to
// start with, and corners nearly in the same place aren't the same.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshWeldCornersKeepsApart(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;

	pSource = TestMeshWeldNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(0, 10, 0);
	TestMeshWeldAddFace(&pSource, 0, 1, 2, TEST_MESH_WELD_GREY, -1);
	TestMeshWeldAddFace(&pSource, 2, 3, 0, TEST_MESH_WELD_DARK, -1);

	pOutput = TestMeshWeldOutput(&cStack, pSource);
	AssertInt(6, pOutput->NumCorners());
	AssertInt(2, pOutput->NumFaces());
	cStack.Kill();

	pSource = TestMeshWeldNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(10, 10, 0.001f);
	pSource->AddCorner(0, 10, 0);
	pSource->AddCorner(0, 0, 0.001f);
	TestMeshWeldAddFace(&pSource, 0, 1, 2, TEST_MESH_WELD_GREY, -1);
	TestMeshWeldAddFace(&pSource, 3, 4, 5, TEST_MESH_WELD_GREY, -1);

	AssertTrue(TestMeshWeldOutput(&cStack, pSource) == pSource);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A face whose corners become one goes, and normals only it used go with it.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshWeldCornersRemovesDegenerate(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SFloat32Vec3*		psNormal;

	pSource = TestMeshWeldNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(5, 5, 0);
	pSource->AddCorner(5, 5, 0);
	pSource->AddCorner(5, 6, 0);
	TestMeshWeldAddFace(&pSource, 3, 4, 5, TEST_MESH_WELD_GREY, 2);
	TestMeshWeldAddFace(&pSource, 0, 1, 2, TEST_MESH_WELD_GREY, 3);
	pSource->AddNormal(1, 0, 0);
	pSource->AddNormal(0, 0, 1);

	pOutput = TestMeshWeldOutput(&cStack, pSource);
	AssertInt(3, pOutput->NumCorners());
	AssertInt(1, pOutput->NumFaces());
	AssertInt(2, (int)pOutput->mcNormals.mcNormals.NumElements());
	AssertInt(1, pOutput->mcNormals.mcFaces.Get(0)->aiCornerNormals[0]);
	psNormal = pOutput->mcNormals.mcNormals.Get(1);
	AssertFloat(1, psNormal->z, 3);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With nothing to weld the mesh itself comes back, and that's so of a welded
// one, an empty one and one with what a new mesh wouldn't keep.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshWeldCornersNothingToDo(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SLogConfig			sLogConfig;

	pSource = TestMeshWeldNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(0, 10, 0);
	pSource->AddCorner(0, 0, 0);
	TestMeshWeldAddFace(&pSource, 0, 1, 2, TEST_MESH_WELD_GREY, -1);
	TestMeshWeldAddFace(&pSource, 3, 4, 5, TEST_MESH_WELD_GREY, -1);

	pOutput = TestMeshWeldOutput(&cStack, pSource);
	AssertFalse(pOutput == pSource);
	cStack.Flatten();
	cStack.AddModifier<CMeshWeldCorners>()->Init();
	AssertTrue(cStack.GetOutput() == pOutput);
	cStack.Kill();

	pSource = TestMeshWeldNewMesh();
	AssertTrue(TestMeshWeldOutput(&cStack, pSource) == pSource);
	cStack.Kill();

	pSource = TestMeshWeldNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(10, 10, 0);
	pSource->AddCorner(10, 10, 0);
	TestMeshWeldAddFace(&pSource, 0, 1, 2, TEST_MESH_WELD_GREY, -1);
	pSource->mcMaterials.mbInUse = true;
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(TestMeshWeldOutput(&cStack, pSource) == pSource);
	gcLogger.SetConfig(&sLogConfig);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshWeldCorners(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshWeldCornersMerges();
	TestMeshWeldCornersKeepsApart();
	TestMeshWeldCornersRemovesDegenerate();
	TestMeshWeldCornersNothingToDo();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
