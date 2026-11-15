#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshBoxEditor.h"
#include "SupportLib/MeshCylinderEditor.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshGeosphereEditor.h"
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
	cStack.AddModifier<CMeshFlatFaces>();
	pOutput = cStack.GetOutput();
	AssertFalse(pOutput == pSource);
	AssertFalse(pSource->mcNormals.InUse());
	AssertTrue(pOutput->mcNormals.InUse());
	AssertLSize(2, pOutput->mcNormals.mcNormals.NumElements());
	psNormalFace = pOutput->mcNormals.mcFaces.Get(1);
	AssertInt(1, psNormalFace->aiCornerNormals[0]);
	AssertInt(1, psNormalFace->aiCornerNormals[2]);
	AssertInt(4, pOutput->NumCorners());
	AssertInt(2, pOutput->GetFaceName(1));
	cStack.Kill();

	cStack.Init(pSource);
	cStack.AddModifier<CMeshSmoothFaces>();
	pOutput = cStack.GetOutput();
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
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
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pOutput;
	Ptr<CMeshSmoothGroups>	pSmoothing;

	cStack.Init(TestMeshNormalTwoFaces(1, 1));
	pSmoothing = cStack.AddModifier<CMeshSmoothGroups>(1);
	pOutput = cStack.GetOutput();
	AssertLSize(2, pOutput->mcSmoothing.mauiGroups.NumElements());
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());

	//Again gives the same.
	cStack.Touch();
	pOutput = cStack.GetOutput();
	AssertLSize(2, pOutput->mcSmoothing.mauiGroups.NumElements());
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	cStack.Kill();

	cStack.Init(TestMeshNormalTwoFaces(1, 2));
	pSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Name;
	pOutput = cStack.GetOutput();
	AssertTrue(pOutput->mcSmoothing.mauiGroups.GetValue(0) == 1);
	AssertTrue(pOutput->mcSmoothing.mauiGroups.GetValue(1) == 2);
	AssertLSize(2 + 6, pOutput->mcNormals.mcNormals.NumElements());
	cStack.Kill();

	cStack.Init(TestMeshNormalTwoFaces(1, 1));
	pSmoothing = cStack.AddModifier<CMeshSmoothGroups>(0);
	pOutput = cStack.GetOutput();
	AssertLSize(2, pOutput->mcNormals.mcNormals.NumElements());
	AssertInt(1, pOutput->mcNormals.mcFaces.Get(1)->aiCornerNormals[0]);
	cStack.Kill();

	//By angle the names don't matter: the faces are flat, so they're one group.
	cStack.Init(TestMeshNormalTwoFaces(1, 2));
	pSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cStack.GetOutput();
	AssertInt((int32)1, pOutput->mcSmoothing.mauiGroups.GetValue(0));
	AssertInt((int32)1, pOutput->mcSmoothing.mauiGroups.GetValue(1));
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	cStack.Kill();

	cStack.Init();
	cStack.AddModifier<CMeshSmoothGroups>();
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
	cStack.AddModifier<CMeshPlaneEditor>(&sX, &sY, &sStart, 2, 2);
	cStack.AddModifier<CMeshSmoothFaces>();
	pOutput = cStack.GetOutput();
	AssertInt(8, pOutput->NumFaces());
	AssertLSize(8 + 9, pOutput->mcNormals.mcNormals.NumElements());
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
	AssertLSize(2, cMesh.mcNormals.mcNormals.NumElements());

	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshSmoothGroups>(1);
	pOutput = cEditor.GetOutput();
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	AssertLSize(2, cMesh.mcNormals.mcNormals.NumElements());
	pOutput = NULL;

	cEditor.Collapse();
	AssertLSize(2 + 4, cMesh.mcNormals.mcNormals.NumElements());
	AssertLSize(2, cMesh.mcSmoothing.mauiGroups.NumElements());
	AssertInt(0, cEditor.mcModifiers.NumModifiers());

	cEditor.AddModifier<CMeshSmoothGroups>(1);
	cEditor.Collapse();
	AssertLSize(2 + 4, cMesh.mcNormals.mcNormals.NumElements());
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
	cStack.AddModifier<CMeshSmoothGroups>(1);
	pOutput = cStack.GetOutput();
	AssertFalse(pOutput == pSource);
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());

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
	AssertLSize(4, pOutput->mcSkin.mcSkinVerts.NumElements());
	pcVert = pOutput->mcSkin.mcSkinVerts.Get(2);
	AssertSize(1, pcVert->mcWeights.NumElements());
	AssertFloat(1.0f, pcVert->mcWeights.Get(0)->fWeight, 3);
	AssertSize(0, pOutput->mcSkin.mcSkinVerts.Get(0)->mcWeights.NumElements());
	AssertSize(1, pOutput->mcSkin.mcInverseSkinMatricies.NumElements());

	//What it was made from is as it was.
	AssertLSize(0, pSource->mcNormals.mcNormals.NumElements());
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Two faces sharing corners 0 and 1, along y.  The first is flat, facing
// +z, and the second is folded up from it by fDegrees.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshNormalFoldedFaces(float32 fDegrees)
{
	Ptr<CMesh>	pMesh;
	float32		fRadians;

	fRadians = fDegrees * (float32)N_PI / 180.0f;

	pMesh = OMalloc<CMesh>();
	pMesh->AddCorner(0, 0, 0);
	pMesh->AddCorner(0, 1, 0);
	pMesh->AddCorner(1, 0, 0);
	pMesh->AddCorner(-cosf(fRadians), 0, sinf(fRadians));
	pMesh->AddFace(0, 2, 1);
	pMesh->AddFace(0, 1, 3);
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest dot product between a face's flat normal and its corners'
// normals: 1 where its corners are sharp, less where they're smoothed.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestMeshSmoothMinCornerDot(CMesh* pcMesh, int iFace)
{
	SMeshNormalFace*	psNormalFace;
	SFloat32Vec3*		psFlat;
	SFloat32Vec3*		psCorner;
	float32				fMin;
	float32				fDot;
	int					i;

	psNormalFace = pcMesh->mcNormals.mcFaces.Get(iFace);
	psFlat = pcMesh->mcNormals.mcNormals.Get(psNormalFace->iFaceNormal);
	fMin = 1.0f;
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		psCorner = pcMesh->mcNormals.mcNormals.Get(psNormalFace->aiCornerNormals[i]);
		fDot = Float32Vec3Dot(psFlat, psCorner);
		if (fDot < fMin)
		{
			fMin = fDot;
		}
	}
	return fMin;
}


//////////////////////////////////////////////////////////////////////////
//
// Two faces are one group when the fold between them is within the sharp
// angle and two when it isn't.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSmoothGroupsAngleFold(void)
{
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pOutput;
	Ptr<CMeshSmoothGroups>	pSmoothing;
	uint32					uiGroup0;
	uint32					uiGroup1;

	cStack.Init(TestMeshNormalFoldedFaces(15.0f));
	pSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cStack.GetOutput();
	AssertInt((int32)1, pOutput->mcSmoothing.mauiGroups.GetValue(0));
	AssertInt((int32)1, pOutput->mcSmoothing.mauiGroups.GetValue(1));
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	AssertTrue(TestMeshSmoothMinCornerDot(&pOutput, 0) < 0.999f);
	cStack.Kill();

	cStack.Init(TestMeshNormalFoldedFaces(30.0f));
	pSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cStack.GetOutput();
	uiGroup0 = pOutput->mcSmoothing.mauiGroups.GetValue(0);
	uiGroup1 = pOutput->mcSmoothing.mauiGroups.GetValue(1);
	AssertInt((int32)1, uiGroup0);
	AssertInt((int32)2, uiGroup1);
	AssertLSize(2 + 6, pOutput->mcNormals.mcNormals.NumElements());
	AssertFloat(1.0f, TestMeshSmoothMinCornerDot(&pOutput, 0), 3);
	AssertFloat(1.0f, TestMeshSmoothMinCornerDot(&pOutput, 1), 3);

	//A sharper angle than the fold smooths it.
	pSmoothing->mfSharpAngle = 45.0f;
	cStack.Touch();
	pOutput = cStack.GetOutput();
	AssertInt((int32)1, pOutput->mcSmoothing.mauiGroups.GetValue(1));
	AssertLSize(2 + 4, pOutput->mcNormals.mcNormals.NumElements());
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A box's sides are at right angles, so every corner is sharp; the faces of
// one side, split into several polygons, are one group; and sides that meet
// have different groups.  A cylinder's side is smooth and its caps sharp.
// A geosphere is all one group.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSmoothGroupsAngleShapes(void)
{
	CMesh					cMesh;
	CMeshEditor				cEditor;
	Ptr<CMesh>				pOutput;
	Ptr<CMeshSmoothGroups>	pSmoothing;
	SFloat32Vec3			sX;
	SFloat32Vec3			sY;
	SFloat32Vec3			sZ;
	SFloat32Vec3			sStart;
	SFloat32Vec3*			psFlat;
	SFloat32Vec3*			psCorner;
	SMeshNormalFace*		psNormalFace;
	CArrayInt				aiGroups;
	int						i;
	int						j;
	int						iNumFaces;
	uint32					uiGroup;
	bool					bCap;

	sX.Init(1, 0, 0);
	sY.Init(0, 1, 0);
	sZ.Init(0, 0, 1);
	sStart.Init(0, 0, 0);

	cMesh.Init();
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshBoxEditor>(&sX, &sY, &sZ, &sStart, 2, 2, 2);
	pSmoothing = cEditor.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cEditor.GetOutput();
	iNumFaces = pOutput->NumFaces();
	AssertInt(48, iNumFaces);
	aiGroups.Init();
	for (i = 0; i < iNumFaces; i++)
	{
		uiGroup = pOutput->mcSmoothing.mauiGroups.GetValue(i);
		AssertTrue(uiGroup != 0);
		AssertTrue((uiGroup & (uiGroup - 1)) == 0);
		aiGroups.AddIfUnique((int)uiGroup);
		AssertFloat(1.0f, TestMeshSmoothMinCornerDot(&pOutput, i), 3);
		for (j = 0; j < iNumFaces; j++)
		{
			//A group is one side, or two opposite ones.
			if (pOutput->mcSmoothing.mauiGroups.GetValue(j) == uiGroup)
			{
				AssertFloat(1.0f, fabsf(Float32Vec3Dot(pOutput->mcNormals.GetFaceNormal(i), pOutput->mcNormals.GetFaceNormal(j))), 3);
			}
		}
	}
	//Opposite sides don't meet, so they can share a group: three are enough.
	AssertSize(3, aiGroups.NumElements());
	aiGroups.Kill();

	//Each of the 26 positions has a normal for each side it's on.
	AssertLSize(48 + 6 * 1 + 12 * 2 + 8 * 3, pOutput->mcNormals.mcNormals.NumElements());
	pOutput = NULL;
	cEditor.Kill();
	cMesh.Kill();

	cMesh.Init();
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshCylinderEditor>(&sZ, &sStart, 1.0f, 2.0f, 24, 1, 1);
	pSmoothing = cEditor.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cEditor.GetOutput();
	iNumFaces = pOutput->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		psNormalFace = pOutput->mcNormals.mcFaces.Get(i);
		psFlat = pOutput->mcNormals.mcNormals.Get(psNormalFace->iFaceNormal);
		bCap = fabsf(psFlat->z) > 0.999f;
		if (bCap)
		{
			AssertFloat(1.0f, TestMeshSmoothMinCornerDot(&pOutput, i), 3);
		}
		else
		{
			AssertTrue(TestMeshSmoothMinCornerDot(&pOutput, i) < 0.999f);
			for (j = 0; j < NUM_FACE_CORNERS; j++)
			{
				psCorner = pOutput->mcNormals.mcNormals.Get(psNormalFace->aiCornerNormals[j]);
				AssertFloat(0.0f, psCorner->z, 3);
				AssertFloat(1.0f, psCorner->Magnitude(), 3);
			}
		}
	}
	pOutput = NULL;
	cEditor.Kill();
	cMesh.Kill();

	cMesh.Init();
	cEditor.Init(&cMesh);
	cEditor.AddModifier<CMeshGeosphereEditor>(&sZ, &sStart, 1.0f, 3);
	pSmoothing = cEditor.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cEditor.GetOutput();
	iNumFaces = pOutput->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		AssertInt((int32)1, pOutput->mcSmoothing.mauiGroups.GetValue(i));
	}
	AssertLSize(iNumFaces + pOutput->NumCorners(), pOutput->mcNormals.mcNormals.NumElements());
	pOutput = NULL;
	cEditor.Kill();
	cMesh.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// 33 faces meeting only at corner 0 are 33 groups that all touch, one more
// than there are bits.  Every face still gets a bit, and only two share one.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshSmoothGroupsAngleTooManyGroups(void)
{
	CMeshModifierStack		cStack;
	Ptr<CMesh>				pMesh;
	Ptr<CMesh>				pOutput;
	Ptr<CMeshSmoothGroups>	pSmoothing;
	CArrayUint32			aiGroups;
	int						i;
	float32					fAngle;
	uint32					uiGroup;

	pMesh = OMalloc<CMesh>();
	pMesh->AddCorner(0, 0, 0);
	for (i = 0; i < 33; i++)
	{
		fAngle = (float32)i * 2.0f * (float32)N_PI / 33.0f;
		pMesh->AddCorner(cosf(fAngle), sinf(fAngle), 0);
		pMesh->AddCorner(cosf(fAngle + 0.1f), sinf(fAngle + 0.1f), 0);
		pMesh->AddFace(0, 1 + i * 2, 2 + i * 2);
	}

	cStack.Init(pMesh);
	pSmoothing = cStack.AddModifier<CMeshSmoothGroups>();
	pSmoothing->meGenerationStyle = SGS_Angle;
	pOutput = cStack.GetOutput();

	aiGroups.Init();
	for (i = 0; i < 33; i++)
	{
		uiGroup = pOutput->mcSmoothing.mauiGroups.GetValue(i);
		AssertTrue(uiGroup != 0);
		aiGroups.AddIfUnique(uiGroup);
	}
	AssertSize(32, aiGroups.NumElements());
	aiGroups.Kill();
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
	TestMeshSmoothGroupsAngleFold();
	TestMeshSmoothGroupsAngleShapes();
	TestMeshSmoothGroupsAngleTooManyGroups();
	TestMeshNormalModifiersAfterShape();
	TestMeshNormalModifiersInEditorAgain();
	TestMeshNormalModifiersKeepDetails();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
