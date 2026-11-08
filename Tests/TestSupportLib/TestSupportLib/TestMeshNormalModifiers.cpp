#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Each new face's flat normal goes in before the corner normals, and a face
// that was already there still has the same corner normals.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshAddFaceKeepsCornerNormals(void)
{
	CMesh				cMesh;
	SMeshNormalFace*	psNormalFace;

	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;
	cMesh.AddCorner(0, 0, 0);
	cMesh.AddCorner(1, 0, 0);
	cMesh.AddCorner(0, 1, 0);
	cMesh.AddFace(0, 1, 2);
	cMesh.AddNormal(0, 0.6f, 0.8f);

	psNormalFace = cMesh.mcNormals.mcFaces.Get(0);
	psNormalFace->aiCornerNormals[0] = 1;
	psNormalFace->aiCornerNormals[1] = 0;

	cMesh.AddFace(0, 2, 1);
	cMesh.AddFace(0, 1, 2);

	psNormalFace = cMesh.mcNormals.mcFaces.Get(0);
	AssertInt(3, psNormalFace->aiCornerNormals[0]);
	AssertInt(0, psNormalFace->aiCornerNormals[1]);  //Its flat normal stays where it is.
	AssertInt(-1, psNormalFace->aiCornerNormals[2]);
	AssertInt(0, psNormalFace->iFaceNormal);
	AssertFloat(0.8f, cMesh.mcNormals.mcNormals.Get(3)->z, 3);

	AssertInt(-1, cMesh.mcNormals.mcFaces.Get(2)->aiCornerNormals[0]);
	AssertFloat(1.0f, cMesh.mcNormals.mcNormals.Get(2)->z, 3);
	AssertFloat(-1.0f, cMesh.mcNormals.mcNormals.Get(1)->z, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshNormalModifiers(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshAddFaceKeepsCornerNormals();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
