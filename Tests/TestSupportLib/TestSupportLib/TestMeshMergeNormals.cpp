#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshMergeNormals.h"
#include "SupportLib/MeshModifierStack.h"
#include "TestLib/Assert.h"


#define TEST_MESH_MERGE_NORMALS_RADIANS	0.01745329252f


//////////////////////////////////////////////////////////////////////////
//
// iNumFaces faces fanning around corner 0, each with iNumFaces's normals to be
// given by TestMeshMergeNormalsSetFace.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshMergeNormalsFan(int iNumFaces)
{
	Ptr<CMesh>	pMesh;
	int			i;

	pMesh = OMalloc<CMesh>();
	pMesh->mcNormals.mbInUse = true;
	pMesh->mcColours.mbInUse = true;
	pMesh->mcColours.mcColours.Add(0xFF808080);
	pMesh->mcColours.mcColours.Add(0xFF404040);
	pMesh->AddCorner(0, 0, 0);
	for (i = 0; i <= iNumFaces; i++)
	{
		pMesh->AddCorner((float32)(10 * (i + 1)), (float32)(10 * i * i), 0);
	}
	for (i = 0; i < iNumFaces; i++)
	{
		pMesh->AddFace(0, i + 1, i + 2);
		pMesh->mcColours.mcFaces.Get(i)->aiCornerColours[0] = i % 2;
		pMesh->mcColours.mcFaces.Get(i)->aiCornerColours[1] = i % 2;
		pMesh->mcColours.mcFaces.Get(i)->aiCornerColours[2] = i % 2;
		pMesh->SetFaceName(i, i + 5);
	}
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// A normal fDegrees from up towards x, used by all of face iFace's corners.
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshMergeNormalsSetFace(CMesh* pcMesh, int iFace, float32 fDegrees)
{
	SMeshNormalFace*	psNormalFace;
	int					iNormal;
	int					k;

	iNormal = (int)pcMesh->mcNormals.mcNormals.NumElements();
	pcMesh->AddNormal(sinf(fDegrees * TEST_MESH_MERGE_NORMALS_RADIANS), 0, cosf(fDegrees * TEST_MESH_MERGE_NORMALS_RADIANS));
	psNormalFace = pcMesh->mcNormals.mcFaces.Get(iFace);
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		psNormalFace->aiCornerNormals[k] = iNormal;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshMergeNormalsOutput(CMeshModifierStack* pcStack, Ptr<CMesh> pSource, float32 fDegrees)
{
	pcStack->Init(pSource);
	pcStack->AddModifier<CMeshMergeNormals>()->Init(fDegrees * TEST_MESH_MERGE_NORMALS_RADIANS);
	return pcStack->GetOutput();
}


//////////////////////////////////////////////////////////////////////////
//
// The normal of a corner of a face in degrees from up towards x.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestMeshMergeNormalsDegrees(CMesh* pcMesh, int iFace, int iCorner)
{
	SFloat32Vec3*	psNormal;

	psNormal = pcMesh->mcNormals.mcNormals.Get(pcMesh->mcNormals.mcFaces.Get(iFace)->aiCornerNormals[iCorner]);
	return atan2f(psNormal->x, psNormal->z) / TEST_MESH_MERGE_NORMALS_RADIANS;
}


//////////////////////////////////////////////////////////////////////////
//
// Normals close together at a corner become their average and ones far apart,
// as at a sharp edge, stay apart.  Nothing else changes, in either mesh.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMergeNormalsMerges(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	int					i;

	pSource = TestMeshMergeNormalsFan(3);
	TestMeshMergeNormalsSetFace(&pSource, 0, 0);
	TestMeshMergeNormalsSetFace(&pSource, 1, 6);
	TestMeshMergeNormalsSetFace(&pSource, 2, 90);

	pOutput = TestMeshMergeNormalsOutput(&cStack, pSource, 10);

	AssertInt(6, (int)pSource->mcNormals.mcNormals.NumElements());
	AssertFalse(pSource->GetConnectivity()->IsGenerated());

	AssertInt(5, pOutput->NumCorners());
	AssertInt(3, pOutput->NumFaces());
	AssertFalse(pOutput->GetConnectivity()->IsGenerated());

	AssertFloat(3, TestMeshMergeNormalsDegrees(&pOutput, 0, 0), 1);
	AssertFloat(3, TestMeshMergeNormalsDegrees(&pOutput, 1, 0), 1);
	AssertInt(pOutput->mcNormals.mcFaces.Get(0)->aiCornerNormals[0], pOutput->mcNormals.mcFaces.Get(1)->aiCornerNormals[0]);
	AssertFloat(90, TestMeshMergeNormalsDegrees(&pOutput, 2, 0), 1);

	for (i = 0; i < 3; i++)
	{
		AssertInt(i + 5, pOutput->GetFaceName(i));
		AssertInt(i % 2, pOutput->mcColours.mcFaces.Get(i)->aiCornerColours[0]);
		AssertInt(i + 1, pOutput->GetFaceCorners(i)[1]);
	}
	AssertFloat(20, pOutput->GetPosition(2)->x, 3);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Normals only merge at the same corner, as each corner has its own vertex.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMergeNormalsCornersApart(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;

	pSource = TestMeshMergeNormalsFan(2);
	TestMeshMergeNormalsSetFace(&pSource, 0, 0);
	TestMeshMergeNormalsSetFace(&pSource, 1, 4);

	pOutput = TestMeshMergeNormalsOutput(&cStack, pSource, 10);

	//Corners 0 and 2 are on both faces, and each has its own merged normal.  Corners 1 and 3 are on one each.
	AssertInt(2 + 4, (int)pOutput->mcNormals.mcNormals.NumElements());
	AssertFloat(2, TestMeshMergeNormalsDegrees(&pOutput, 0, 0), 1);
	AssertFloat(0, TestMeshMergeNormalsDegrees(&pOutput, 0, 1), 1);
	AssertFloat(2, TestMeshMergeNormalsDegrees(&pOutput, 0, 2), 1);
	AssertFloat(4, TestMeshMergeNormalsDegrees(&pOutput, 1, 2), 1);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A slow curve can't all merge into one normal, a little at a time.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMergeNormalsDoesntCreep(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;

	pSource = TestMeshMergeNormalsFan(3);
	TestMeshMergeNormalsSetFace(&pSource, 0, 0);
	TestMeshMergeNormalsSetFace(&pSource, 1, 8);
	TestMeshMergeNormalsSetFace(&pSource, 2, 16);

	pOutput = TestMeshMergeNormalsOutput(&cStack, pSource, 10);

	AssertFloat(4, TestMeshMergeNormalsDegrees(&pOutput, 0, 0), 1);
	AssertFloat(4, TestMeshMergeNormalsDegrees(&pOutput, 1, 0), 1);
	AssertFloat(16, TestMeshMergeNormalsDegrees(&pOutput, 2, 0), 1);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With no angle allowed only normals that are the same merge, and a face with no
// normal of its own at a corner keeps its flat one.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMergeNormalsSameOnly(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;

	pSource = TestMeshMergeNormalsFan(4);
	TestMeshMergeNormalsSetFace(&pSource, 0, 30);
	TestMeshMergeNormalsSetFace(&pSource, 1, 30);
	TestMeshMergeNormalsSetFace(&pSource, 2, 31);

	pOutput = TestMeshMergeNormalsOutput(&cStack, pSource, 0);

	AssertInt(pOutput->mcNormals.mcFaces.Get(0)->aiCornerNormals[0], pOutput->mcNormals.mcFaces.Get(1)->aiCornerNormals[0]);
	AssertFalse(pOutput->mcNormals.mcFaces.Get(0)->aiCornerNormals[0] == pOutput->mcNormals.mcFaces.Get(2)->aiCornerNormals[0]);
	AssertInt(-1, pOutput->mcNormals.mcFaces.Get(3)->aiCornerNormals[0]);
	AssertFloat(31, TestMeshMergeNormalsDegrees(&pOutput, 2, 0), 1);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With nothing to merge the mesh itself comes back, and that's so of a merged
// one, one with no normals and one with what a new mesh wouldn't keep.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMergeNormalsNothingToDo(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SLogConfig			sLogConfig;

	pSource = TestMeshMergeNormalsFan(2);
	TestMeshMergeNormalsSetFace(&pSource, 0, 0);
	TestMeshMergeNormalsSetFace(&pSource, 1, 4);
	pOutput = TestMeshMergeNormalsOutput(&cStack, pSource, 10);
	AssertFalse(pOutput == pSource);
	cStack.Flatten();
	cStack.AddModifier<CMeshMergeNormals>()->Init(10 * TEST_MESH_MERGE_NORMALS_RADIANS);
	AssertTrue(cStack.GetOutput() == pOutput);
	cStack.Kill();

	pSource = TestMeshMergeNormalsFan(2);
	TestMeshMergeNormalsSetFace(&pSource, 0, 0);
	TestMeshMergeNormalsSetFace(&pSource, 1, 90);
	AssertTrue(TestMeshMergeNormalsOutput(&cStack, pSource, 10) == pSource);
	cStack.Kill();

	pSource = OMalloc<CMesh>();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(0, 10, 0);
	pSource->AddFace(0, 1, 2);
	AssertTrue(TestMeshMergeNormalsOutput(&cStack, pSource, 10) == pSource);
	cStack.Kill();

	pSource = TestMeshMergeNormalsFan(2);
	TestMeshMergeNormalsSetFace(&pSource, 0, 0);
	TestMeshMergeNormalsSetFace(&pSource, 1, 4);
	pSource->mcSkin.mbInUse = true;
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(TestMeshMergeNormalsOutput(&cStack, pSource, 10) == pSource);
	gcLogger.SetConfig(&sLogConfig);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshMergeNormals(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshMergeNormalsMerges();
	TestMeshMergeNormalsCornersApart();
	TestMeshMergeNormalsDoesntCreep();
	TestMeshMergeNormalsSameOnly();
	TestMeshMergeNormalsNothingToDo();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
