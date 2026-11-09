#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshModifierStack.h"
#include "SupportLib/MeshPlaneEditor.h"
#include "SupportLib/MeshSmoothFaces.h"
#include "SupportLib/MeshSmoothGroups.h"
#include "TestLib/Assert.h"


size CountMeshMemoryObjects(const char* szClassName);


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
// Two faces sharing corners 0 and 2, each with its own name.  No normals.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshNormalTwoFaces(int iName1, int iName2)
{
	Ptr<CMesh>	pMesh;

	pMesh = OMalloc<CMesh>();
	pMesh->AddCorner(0, 0, 0);
	pMesh->AddCorner(1, 0, 0);
	pMesh->AddCorner(1, 1, 0);
	pMesh->AddCorner(0, 1, 0);
	pMesh->AddFace(0, 1, 2);
	pMesh->AddFace(0, 2, 3);
	pMesh->SetFaceName(0, iName1);
	pMesh->SetFaceName(1, iName2);
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// Each makes a new mesh with normals from one that has none, which stays
// as it was.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshNormalModifiersMakeNewMeshes(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SMeshNormalFace*	psNormalFace;

	pSource = TestMeshNormalTwoFaces(1, 2);

	cStack.Init(pSource);
	cStack.AddModifier<CMeshFlatFaces>()->Init();
	pOutput = cStack.GetOutput();
	AssertFalse(pOutput == pSource);
	AssertFalse(pSource->mcNormals.InUse());
	AssertTrue(pOutput->mcNormals.InUse());
	AssertSize(2, pOutput->mcNormals.mcNormals.NumElements());
	psNormalFace = pOutput->mcNormals.mcFaces.Get(1);
	AssertInt(1, psNormalFace->aiCornerNormals[0]);
	AssertInt(1, psNormalFace->aiCornerNormals[2]);
	AssertInt(4, pOutput->NumCorners());
	AssertInt(2, pOutput->GetFaceName(1));
	cStack.Kill();

	cStack.Init(pSource);
	cStack.AddModifier<CMeshSmoothFaces>()->Init();
	pOutput = cStack.GetOutput();
	AssertSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	psNormalFace = pOutput->mcNormals.mcFaces.Get(0);
	AssertInt(2, psNormalFace->aiCornerNormals[0]);
	AssertFloat(1.0f, pOutput->mcNormals.mcNormals.Get(psNormalFace->aiCornerNormals[1])->z, 3);
	cStack.Kill();

	AssertFalse(pSource->mcNormals.InUse());
	AssertInt(2, pSource->NumFaces());
}


//////////////////////////////////////////////////////////////////////////
//
// Faces in one group share a normal at a corner, in two they have one each,
// and in none they use their flat ones.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSmoothGroupsModify(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pOutput;
	CMeshSmoothGroups*	pcSmoothing;

	cStack.Init(TestMeshNormalTwoFaces(1, 1));
	pcSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pcSmoothing->Init(1);
	pOutput = cStack.GetOutput();
	AssertSize(2, pOutput->mcSmoothing.mauiGroups.NumElements());
	AssertSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());

	//Again gives the same.
	cStack.Touch();
	pOutput = cStack.GetOutput();
	AssertSize(2, pOutput->mcSmoothing.mauiGroups.NumElements());
	AssertSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	cStack.Kill();

	cStack.Init(TestMeshNormalTwoFaces(1, 2));
	pcSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pcSmoothing->Init();
	pcSmoothing->meGenerationStyle = SGS_Name;
	pOutput = cStack.GetOutput();
	AssertTrue(pOutput->mcSmoothing.mauiGroups.GetValue(0) == 1);
	AssertTrue(pOutput->mcSmoothing.mauiGroups.GetValue(1) == 2);
	AssertSize(2 + 6, pOutput->mcNormals.mcNormals.NumElements());
	cStack.Kill();

	cStack.Init(TestMeshNormalTwoFaces(1, 1));
	pcSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pcSmoothing->Init(0);
	pOutput = cStack.GetOutput();
	AssertSize(2, pOutput->mcNormals.mcNormals.NumElements());
	AssertInt(1, pOutput->mcNormals.mcFaces.Get(1)->aiCornerNormals[0]);
	cStack.Kill();

	cStack.Init(TestMeshNormalTwoFaces(1, 2));
	pcSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pcSmoothing->Init();
	pcSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cStack.GetOutput();
	AssertSize(2, pOutput->mcNormals.mcNormals.NumElements());
	cStack.Kill();

	cStack.Init();
	cStack.AddModifier<CMeshSmoothGroups>()->Init();
	AssertInt(0, cStack.GetOutput()->NumFaces());
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Modifiers follow each other in a stack: a shape, then normals on it.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshNormalModifiersAfterShape(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pOutput;
	SFloat32Vec3		sX;
	SFloat32Vec3		sY;
	SFloat32Vec3		sStart;

	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	sStart.Init(0, 0, 0);

	cStack.Init();
	cStack.AddModifier<CMeshPlaneEditor>()->Init(&sX, &sY, &sStart, 2, 2);
	cStack.AddModifier<CMeshSmoothFaces>()->Init();
	pOutput = cStack.GetOutput();
	AssertInt(8, pOutput->NumFaces());
	AssertSize(8 + 9, pOutput->mcNormals.mcNormals.NumElements());
	AssertSize(2, CountMeshMemoryObjects("CMesh"));
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// On a mesh in an editor, its output has the normals and the mesh doesn't
// until it's collapsed.  Collapsing again doesn't pile up corner normals.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshNormalModifiersInEditorAgain(void)
{
	CMesh				cMesh;
	CMeshEditor			cEditor;
	Ptr<CMesh>			pOutput;

	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;
	cMesh.CopyFrom(&TestMeshNormalTwoFaces(1, 1));
	AssertSize(2, cMesh.mcNormals.mcNormals.NumElements());

	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshSmoothGroups>()->Init(1);
	pOutput = cEditor.GetOutput();
	AssertSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	AssertSize(2, cMesh.mcNormals.mcNormals.NumElements());
	pOutput = NULL;

	cEditor.Collapse();
	AssertSize(2 + 4, cMesh.mcNormals.mcNormals.NumElements());
	AssertSize(2, cMesh.mcSmoothing.mauiGroups.NumElements());
	AssertInt(0, cEditor.mcModifiers.NumEditors());

	cEditor.AddModifier<CMeshSmoothGroups>()->Init(1);
	cEditor.Collapse();
	AssertSize(2 + 4, cMesh.mcNormals.mcNormals.NumElements());
	AssertInt(2, cMesh.NumFaces());
	AssertInt(4, cMesh.NumCorners());
	cEditor.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// They only change normals, so a mesh's UVs, materials and skin come
// through, as Goober needs.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshNormalModifiersKeepDetails(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	Ptr<CMaterial>		pMaterial;
	CMeshSkinVert*		pcVert;
	SSkinWeight			sWeight;
	SFloat4x4			sInverse;
	int					i;

	pSource = TestMeshNormalTwoFaces(1, 1);
	pSource->mcUVs.mbInUse = true;
	pSource->AddUV(0.25f, 0.75f);
	pSource->mcUVs.mcLayers.Get(0)->mcFaces.Get(1)->aiUVs[2] = 0;

	pMaterial = OMalloc<CMaterial>();
	pSource->mcMaterials.mbInUse = true;
	pSource->mcMaterials.mapMaterials.Add(pMaterial);
	pSource->mcMaterials.mcFaces.SetValue(1, 0);

	pSource->mcSkin.mbInUse = true;
	for (i = 0; i < pSource->NumCorners(); i++)
	{
		pSource->mcSkin.mcSkinVerts.Add()->Init();
	}
	sWeight.fWeight = 1.0f;
	sWeight.iMatrix = 0;
	pSource->mcSkin.mcSkinVerts.Get(2)->mcWeights.Add(&sWeight);
	sInverse.Identity();
	pSource->mcSkin.mcInverseSkinMatricies.Add(&sInverse);

	cStack.Init(pSource);
	cStack.AddModifier<CMeshSmoothGroups>()->Init(1);
	pOutput = cStack.GetOutput();
	AssertFalse(pOutput == pSource);
	AssertSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());

	AssertTrue(pOutput->mcUVs.InUse());
	AssertSize(1, pOutput->mcUVs.mcLayers.NumElements());
	AssertFloat(0.75f, pOutput->mcUVs.mcLayers.Get(0)->mcUVs.Get(0)->y, 3);
	AssertInt(0, pOutput->mcUVs.mcLayers.Get(0)->mcFaces.Get(1)->aiUVs[2]);
	AssertInt(-1, pOutput->mcUVs.mcLayers.Get(0)->mcFaces.Get(0)->aiUVs[0]);

	AssertTrue(pOutput->mcMaterials.InUse());
	AssertTrue(&pOutput->mcMaterials.mapMaterials.Get(0) == &pMaterial);
	AssertInt(-1, pOutput->mcMaterials.mcFaces.GetValue(0));
	AssertInt(0, pOutput->mcMaterials.mcFaces.GetValue(1));

	AssertTrue(pOutput->mcSkin.InUse());
	AssertSize(4, pOutput->mcSkin.mcSkinVerts.NumElements());
	pcVert = pOutput->mcSkin.mcSkinVerts.Get(2);
	AssertSize(1, pcVert->mcWeights.NumElements());
	AssertFloat(1.0f, pcVert->mcWeights.Get(0)->fWeight, 3);
	AssertSize(0, pOutput->mcSkin.mcSkinVerts.Get(0)->mcWeights.NumElements());
	AssertSize(1, pOutput->mcSkin.mcInverseSkinMatricies.NumElements());

	//What it was made from is as it was.
	AssertSize(0, pSource->mcNormals.mcNormals.NumElements());
	cStack.Kill();
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
	TestMeshNormalModifiersMakeNewMeshes();
	TestMeshSmoothGroupsModify();
	TestMeshNormalModifiersAfterShape();
	TestMeshNormalModifiersInEditorAgain();
	TestMeshNormalModifiersKeepDetails();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
