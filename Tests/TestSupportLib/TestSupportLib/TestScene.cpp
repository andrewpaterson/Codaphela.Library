#include <math.h>
#include "BaseLib/ArrayFloat32.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Mallocator.h"
#include "StandardLib/Objects.h"
#include "SupportLib/AnimationPlayer.h"
#include "SupportLib/MeshSmoothGroups.h"
#include "SupportLib/Scene.h"
#include "SupportLib/SceneFileReader.h"
#include "SupportLib/SceneFileWriter.h"
#include "TestLib/Assert.h"
#include "TestLib/AssertFile.h"


#define SCENE_SAMPLE		"Input" _FS_ "Scene" _FS_ "Sample.DRG"
#define SCENE_GOOBER		"Input" _FS_ "Scene" _FS_ "Goober.DRG"
#define SCENE_OUTPUT_DIR	"Output" _FS_ "SceneObjects"
#define SCENE_OUTPUT_A		SCENE_OUTPUT_DIR _FS_ "A.DRG"
#define SCENE_OUTPUT_B		SCENE_OUTPUT_DIR _FS_ "B.DRG"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestSceneReadMesh(Ptr<CScene> pScene)
{
	Ptr<CInstance>		pInstance;
	Ptr<CInstanceMesh>	pInstanceMesh;
	Ptr<CMeshEditor>	pEditor;
	CMesh*				pcMesh;
	CMeshSkinVert*		pcVert;
	SFloat32Vec3*		psPosition;
	int					iNumFlat;

	AssertSize(1, pScene->mapInstances.NumElements());
	pInstance = pScene->mapInstances.Get(0);
	AssertString("Thing", pInstance->mszName.Text());
	AssertSize(2, pInstance->mapMeshes.NumElements());

	pInstanceMesh = pInstance->mapMeshes.Get(0);
	pEditor = pInstanceMesh->mpEditor;
	pcMesh = &pEditor->mpMesh;
	AssertString("Pyramid", pEditor->mszName.Text());
	AssertInt(5, pcMesh->NumCorners());
	psPosition = pcMesh->GetPosition(4);
	AssertFloat(0.5f, psPosition->x, 3);
	AssertFloat(1.0f, psPosition->z, 3);

	//The quad is cut in two, and the editor still sees it as one polygon.
	AssertInt(3, pcMesh->NumFaces());
	AssertInt(2, pEditor->NumPolygons());

	//The file's normals, after a flat one for each face.
	iNumFlat = pcMesh->NumFaces();
	AssertTrue(pcMesh->mcNormals.InUse());
	AssertLSize((size)iNumFlat + 2, pcMesh->mcNormals.mcNormals.NumElements());
	AssertInt(iNumFlat, pcMesh->mcNormals.mcFaces.Get(0)->aiCornerNormals[0]);
	AssertInt(iNumFlat + 1, pcMesh->mcNormals.mcFaces.Get(2)->aiCornerNormals[0]);
	AssertInt(-1, pcMesh->mcNormals.mcFaces.Get(2)->aiCornerNormals[2]);

	AssertSize(2, pcMesh->mcUVs.mcLayers.NumElements());
	AssertLSize(4, pcMesh->mcUVs.mcLayers.Get(0)->mcUVs.NumElements());
	AssertLSize(1, pcMesh->mcUVs.mcLayers.Get(1)->mcUVs.NumElements());
	AssertInt(-1, pcMesh->mcUVs.mcLayers.Get(0)->mcFaces.Get(2)->aiUVs[2]);

	AssertTrue(pcMesh->mcColours.InUse());
	AssertLSize(2, pcMesh->mcColours.mcColours.NumElements());
	AssertInt(1, pcMesh->mcColours.mcFaces.Get(2)->aiCornerColours[0]);

	//Slots in the mesh's own materials, which are the file's second then first.
	AssertSize(2, pcMesh->mcMaterials.mapMaterials.NumElements());
	AssertString("Bare", pcMesh->mcMaterials.mapMaterials.Get(0)->mszName.Text());
	AssertString("Red", pcMesh->mcMaterials.mapMaterials.Get(1)->mszName.Text());
	AssertInt(0, pcMesh->mcMaterials.mcFaces.GetValue(0));
	AssertInt(0, pcMesh->mcMaterials.mcFaces.GetValue(1));
	AssertInt(-1, pcMesh->mcMaterials.mcFaces.GetValue(2));

	//A scene file's groups are the mesh's, not a modifier's on the editor's stack.
	AssertNull(pEditor->mcModifiers.GetModifier<CMeshSmoothGroups>());
	AssertTrue(pcMesh->mcSmoothing.InUse());
	AssertTrue(pcMesh->mcSmoothing.mauiGroups.GetValue(0) == 1);
	AssertTrue(pcMesh->mcSmoothing.mauiGroups.GetValue(1) == 1);
	AssertTrue(pcMesh->mcSmoothing.mauiGroups.GetValue(2) == 0x80000002);

	AssertTrue(pcMesh->mcSkin.InUse());
	AssertLSize(5, pcMesh->mcSkin.mcSkinVerts.NumElements());
	AssertSize(2, pcMesh->mcSkin.mcInverseSkinMatricies.NumElements());
	pcVert = pcMesh->mcSkin.mcSkinVerts.Get(4);
	AssertSize(2, pcVert->mcWeights.NumElements());
	AssertFloat(0.75f, pcVert->mcWeights.Get(0)->fWeight, 3);
	AssertInt(0, pcVert->mcWeights.Get(0)->iMatrix);
	AssertFloat(0.25f, pcVert->mcWeights.Get(1)->fWeight, 3);
	AssertInt(1, pcVert->mcWeights.Get(1)->iMatrix);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestSceneReadMaterials(Ptr<CScene> pScene)
{
	Ptr<CInstanceMesh>	pInstanceMesh;
	Ptr<CMaterial>		pRed;
	Ptr<CImage>			pImage;
	CMaterialImage*		pcBump;
	CMaterialProperties*	psProperties;

	pInstanceMesh = pScene->mapInstances.Get(0)->mapMeshes.Get(0);
	pRed = pInstanceMesh->mpEditor->mpMesh->mcMaterials.mapMaterials.Get(1);
	psProperties = &pRed->msProperties;
	AssertFloat(1.0f, psProperties->mcDiffuseColour.mfRed, 3);
	AssertFloat(0.5f, psProperties->mcDiffuseColour.mfAlpha, 3);
	AssertFloat(0.25f, psProperties->mcSpecularColour.mfGreen, 3);
	AssertFloat(1.5f, psProperties->mfShininessPower, 3);
	AssertFloat(0.25f, psProperties->mfShininessBase, 3);

	AssertTrue(pRed->GetImage(MI_Diffuse)->mpImage.IsNull());
	pcBump = pRed->GetImage(MI_Bump);
	AssertString("Checks", pcBump->mszImageName.Text());
	pImage = pcBump->mpImage;
	AssertTrue(pImage.IsNotNull());
	AssertInt(2, (int)pImage->GetWidth());
	AssertInt(2, (int)pImage->GetHeight());
	AssertInt(3, (int)pImage->NumChannels());
	AssertInt(1, ((uint8*)pImage->GetData())[0]);
	AssertInt(12, ((uint8*)pImage->GetData())[11]);

	//What the instance uses in place of the mesh's, the same object as a mesh in the file refers to.
	AssertSize(1, pInstanceMesh->mapMaterials.NumElements());
	AssertTrue(&pInstanceMesh->mapMaterials.Get(0) == &pInstanceMesh->mpEditor->mpMesh->mcMaterials.mapMaterials.Get(0));
}


//////////////////////////////////////////////////////////////////////////
//
// One object for each of the file's connections, whatever refers to it.
//
//////////////////////////////////////////////////////////////////////////
static void TestSceneReadConnections(Ptr<CScene> pScene)
{
	Ptr<CInstance>			pInstance;
	Ptr<CConnection>		pChild;
	Ptr<CConnection>		pRoot;
	Ptr<CLight>				pLight;
	Ptr<CCamera>			pCamera;
	Ptr<CSceneAnimation>	pAnimation;
	Ptr<CConnectionAnimation>	pTrack;

	pInstance = pScene->mapInstances.Get(0);
	AssertSize(2, pInstance->mapConnections.NumElements());
	pChild = pInstance->mapConnections.Get(0);
	pRoot = pInstance->mapConnections.Get(1);
	AssertString("Child", pChild->mszName.Text());
	AssertString("Root", pRoot->mszName.Text());
	AssertTrue(&pChild->mpParent == &pRoot);
	AssertTrue(pRoot->mpParent.IsNull());
	AssertTrue(pChild->mbParity);
	AssertFalse(pRoot->mbParity);
	AssertFloat(2.0f, pChild->msWorldMatrix.pos.x, 3);
	AssertFloat(4.0f, pChild->msLocalMatrix.pos.z, 3);

	AssertSize(2, pInstance->mapMeshes.Get(0)->mapConnections.NumElements());
	AssertTrue(&pInstance->mapMeshes.Get(0)->mapConnections.Get(1) == &pRoot);

	AssertSize(1, pScene->mapLights.NumElements());
	pLight = pScene->mapLights.Get(0);
	AssertString("Lamp", pLight->mszName.Text());
	AssertInt(LT_Spot, pLight->meType);
	AssertInt(LDT_Square, pLight->meDecay);
	AssertFloat(0.25f, pLight->msColour.mfRed, 3);
	AssertFloat(1.0f, pLight->mfDecayStart, 3);
	AssertFloat(10.0f, pLight->mfRange, 3);
	AssertFloat(0.5f, pLight->mfIntensity, 3);
	AssertTrue(pLight->mbCastShadows);
	AssertTrue(&pLight->mpConnection == &pChild);

	AssertSize(1, pScene->mapCameras.NumElements());
	pCamera = pScene->mapCameras.Get(0);
	AssertString("Eye", pCamera->mszName.Text());
	AssertInt(CAT_Orthonormal, pCamera->meCameraType);
	AssertFloat(1.0f, pCamera->mfFOV, 3);
	AssertFloat(0.25f, pCamera->mfNear, 3);
	AssertFloat(100.0f, pCamera->mfFar, 3);
	AssertTrue(&pCamera->mpConnection == &pRoot);

	AssertSize(1, pScene->mapAnimations.NumElements());
	pAnimation = pScene->mapAnimations.Get(0);
	AssertString("Move", pAnimation->mszName.Text());
	AssertSize(2, pAnimation->mapTracks.NumElements());
	pTrack = pAnimation->mapTracks.Get(0);
	AssertTrue(&pTrack->mpConnection == &pRoot);
	AssertSize(2, pTrack->NumKeys());
	AssertFloat(0.5f, pTrack->GetKey(1)->fTime, 3);
	AssertTrue(&pAnimation->mapTracks.Get(1)->mpConnection == &pChild);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestSceneReadParticles(Ptr<CScene> pScene)
{
	Ptr<CSceneParticles>	pStars;
	Ptr<CSceneParticles>	pDust;

	AssertSize(2, pScene->mapParticles.NumElements());
	pStars = pScene->mapParticles.Get(0);
	AssertString("Stars", pStars->mszName.Text());
	AssertLSize(2, pStars->NumParticles());
	AssertFloat(6.0f, *pStars->mafPositions.Get(5), 3);
	AssertFloat(0.5f, *pStars->mafColours.Get(4), 3);
	AssertFloat(2.0f, *pStars->mafRadii.Get(1), 3);
	AssertFloat(0.25f, *pStars->mafIntensities.Get(1), 3);

	pDust = pScene->mapParticles.Get(1);
	AssertString("Dust", pDust->mszName.Text());
	AssertLSize(2, pDust->NumParticles());
	AssertLSize(0, pDust->mafColours.NumElements());
	AssertLSize(0, pDust->mafRadii.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneRead(void)
{
	Ptr<CScene>	pScene;

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_SAMPLE));

	TestSceneReadMesh(pScene);
	TestSceneReadMaterials(pScene);
	TestSceneReadConnections(pScene);
	TestSceneReadParticles(pScene);

	//An empty mesh is still a mesh.
	AssertString("Empty", pScene->mapInstances.Get(0)->mapMeshes.Get(1)->mpEditor->mszName.Text());
	AssertInt(0, pScene->mapInstances.Get(0)->mapMeshes.Get(1)->mpEditor->mpMesh->NumFaces());
}


//////////////////////////////////////////////////////////////////////////
//
// What's written is what was read, by reading it back.  A polygon starts
// at its lowest corner now, and what the sample left out has its default.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneWrite(void)
{
	Ptr<CScene>			pScene;
	Ptr<CScene>			pAgain;
	CSceneFileReader	cReader;
	int32				aiSizes[2];
	int32				aiCorners[7];
	uint32				auiGroups[2];

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_SAMPLE));
	AssertTrue(pScene->WriteFile(SCENE_OUTPUT_A));

	cReader.Init(SCENE_OUTPUT_A);
	AssertTrue(cReader.Open());
	AssertSize(2, cReader.NumMeshes());
	AssertSize(1, cReader.NumImages());
	AssertSize(2, cReader.NumMaterials());
	AssertSize(2, cReader.NumConnections());
	AssertSize(1, cReader.NumInstances());

	AssertTrue(cReader.BeginMesh(0));
	AssertLSize(2, cReader.NumPolygons());
	AssertTrue(cReader.ReadPolygonSizes(aiSizes));
	AssertInt((int32)4, aiSizes[0]);
	AssertInt((int32)3, aiSizes[1]);
	AssertLSize(7, cReader.NumCorners());
	AssertTrue(cReader.ReadCornerPositions(aiCorners));
	AssertInt((int32)0, aiCorners[0]);
	AssertInt((int32)3, aiCorners[1]);
	AssertInt((int32)2, aiCorners[2]);
	AssertInt((int32)1, aiCorners[3]);
	AssertInt((int32)0, aiCorners[4]);
	AssertInt((int32)1, aiCorners[5]);
	AssertInt((int32)4, aiCorners[6]);
	AssertLSize(2, cReader.NumNormals());
	AssertSize(2, cReader.NumUVLayers());
	AssertLSize(2, cReader.NumColours());
	AssertLSize(2, cReader.NumMaterialSlots());
	AssertLSize(2, cReader.NumSmoothGroups());
	AssertTrue(cReader.ReadSmoothGroups(auiGroups));
	AssertTrue(auiGroups[0] == 1);
	AssertTrue(auiGroups[1] == 0x80000002);
	AssertLSize(5, cReader.NumSkinCounts());
	AssertLSize(2, cReader.NumInverseMatrices());
	AssertTrue(cReader.End());
	cReader.Kill();

	pAgain = OMalloc<CScene>();
	AssertTrue(pAgain->ReadFile(SCENE_OUTPUT_A));
	TestSceneReadMesh(pAgain);
	TestSceneReadMaterials(pAgain);
	TestSceneReadConnections(pAgain);
	TestSceneReadParticles(pAgain);

	//Once it's been through a scene it stays as it is.
	AssertTrue(pAgain->WriteFile(SCENE_OUTPUT_B));
	AssertFile(SCENE_OUTPUT_A, SCENE_OUTPUT_B);
}


//////////////////////////////////////////////////////////////////////////
//
// All the kinds of image, with two that have the same name, as Melee has.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneImages(void)
{
	CSceneFileWriter	cWriter;
	CSceneFileReader	cReader;
	Ptr<CScene>			pScene;
	CChars				szName;
	CChars				szChannels;
	float32				afPositions[9];
	int32				aiSizes[1];
	int32				aiCorners[3];
	int32				aiMaterials[1];
	int32				aiSlots[1];
	uint8				auiRGBA[8];
	uint8				auiRGB[6];
	uint8				auiA[2];
	uint8				auiRead[8];
	int32				iConnection;
	int32				iSlot;
	int					i;

	for (i = 0; i < 9; i++)
	{
		afPositions[i] = (float32)((i * 5) % 3);
	}
	for (i = 0; i < 8; i++)
	{
		auiRGBA[i] = (uint8)(10 + i);
	}
	for (i = 0; i < 6; i++)
	{
		auiRGB[i] = (uint8)(30 + i);
	}
	auiA[0] = 50;
	auiA[1] = 51;
	aiSizes[0] = 3;
	aiCorners[0] = 0;
	aiCorners[1] = 1;
	aiCorners[2] = 2;
	aiMaterials[0] = 0;
	aiSlots[0] = 0;
	iConnection = 0;
	iSlot = 0;

	cWriter.Init(SCENE_OUTPUT_A);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Mesh"));
	AssertTrue(cWriter.WritePositions(afPositions, 3));
	AssertTrue(cWriter.WritePolygons(aiSizes, 1, aiCorners, 3));
	AssertTrue(cWriter.WriteMaterialSlots(aiMaterials, 1));
	AssertTrue(cWriter.WritePolygonSlots(aiSlots, 1));
	AssertTrue(cWriter.EndMesh());
	AssertTrue(cWriter.BeginImage("Same"));
	AssertTrue(cWriter.WritePixels(2, 1, "RGBA", auiRGBA));
	AssertTrue(cWriter.EndImage());
	AssertTrue(cWriter.BeginImage("Same"));
	AssertTrue(cWriter.WritePixels(2, 1, "A", auiA));
	AssertTrue(cWriter.EndImage());
	AssertTrue(cWriter.BeginImage("Other"));
	AssertTrue(cWriter.WritePixels(2, 1, "RGB", auiRGB));
	AssertTrue(cWriter.EndImage());
	AssertTrue(cWriter.BeginMaterial("Material"));
	AssertTrue(cWriter.WriteMaterialTexture(SFTX_Diffuse, 0));
	AssertTrue(cWriter.WriteMaterialTexture(SFTX_Opacity, 1));
	AssertTrue(cWriter.WriteMaterialTexture(SFTX_Bump, 2));
	AssertTrue(cWriter.EndMaterial());
	AssertTrue(cWriter.BeginConnection("Place"));
	AssertTrue(cWriter.EndConnection());
	AssertTrue(cWriter.BeginInstance("Thing"));
	AssertTrue(cWriter.WriteConnections(&iConnection, 1));
	AssertTrue(cWriter.WriteInstanceMesh(0, &iSlot, 1, NULL, 0));
	AssertTrue(cWriter.EndInstance());
	AssertTrue(cWriter.Close());
	cWriter.Kill();

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_OUTPUT_A));
	AssertTrue(pScene->WriteFile(SCENE_OUTPUT_B));

	cReader.Init(SCENE_OUTPUT_B);
	AssertTrue(cReader.Open());
	AssertSize(3, cReader.NumImages());

	szName.Init();
	szChannels.Init();
	AssertTrue(cReader.BeginImage(1));
	AssertTrue(cReader.ReadName(&szName));
	AssertString("Same", szName.Text());
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("A", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiRead));
	AssertInt(50, auiRead[0]);
	AssertInt(51, auiRead[1]);
	AssertTrue(cReader.End());
	szChannels.Kill();
	szName.Kill();

	szChannels.Init();
	AssertTrue(cReader.BeginImage(0));
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("RGBA", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiRead));
	AssertInt(10, auiRead[0]);
	AssertInt(17, auiRead[7]);
	AssertTrue(cReader.End());
	szChannels.Kill();

	szChannels.Init();
	AssertTrue(cReader.BeginImage(2));
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("RGB", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiRead));
	AssertInt(30, auiRead[0]);
	AssertInt(35, auiRead[5]);
	AssertTrue(cReader.End());
	szChannels.Kill();

	cReader.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With no normals in its file a mesh gets them from its groups, and a face
// in no group stays flat.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneSmoothGroups(void)
{
	CSceneFileWriter	cWriter;
	CSceneFileReader	cReader;
	Ptr<CScene>			pScene;
	CMesh*				pcMesh;
	float32				afPositions[12];
	int32				aiSizes[2];
	int32				aiCorners[6];
	int32				aiRead[6];
	uint32				auiGroups[2];
	int32				iConnection;
	int32				iSlot;
	int					i;

	for (i = 0; i < 12; i++)
	{
		afPositions[i] = (float32)((i * 7) % 4);
	}
	aiSizes[0] = 3;
	aiSizes[1] = 3;
	aiCorners[0] = 0;
	aiCorners[1] = 1;
	aiCorners[2] = 2;
	aiCorners[3] = 0;
	aiCorners[4] = 2;
	aiCorners[5] = 3;
	auiGroups[0] = 0;
	auiGroups[1] = 1;
	iConnection = 0;
	iSlot = 0;

	cWriter.Init(SCENE_OUTPUT_A);
	AssertTrue(cWriter.Open());
	AssertTrue(cWriter.BeginMesh("Mesh"));
	AssertTrue(cWriter.WritePositions(afPositions, 4));
	AssertTrue(cWriter.WritePolygons(aiSizes, 2, aiCorners, 6));
	AssertTrue(cWriter.WriteSmoothGroups(auiGroups, 2));
	AssertTrue(cWriter.EndMesh());
	AssertTrue(cWriter.BeginConnection("Place"));
	AssertTrue(cWriter.EndConnection());
	AssertTrue(cWriter.BeginInstance("Thing"));
	AssertTrue(cWriter.WriteConnections(&iConnection, 1));
	AssertTrue(cWriter.WriteInstanceMesh(0, &iSlot, 1, NULL, 0));
	AssertTrue(cWriter.EndInstance());
	AssertTrue(cWriter.Close());
	cWriter.Kill();

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_OUTPUT_A));
	pcMesh = &pScene->mapInstances.Get(0)->mapMeshes.Get(0)->mpEditor->mpMesh;
	AssertTrue(pcMesh->mcNormals.mcNormals.NumElements() > 2);
	AssertTrue(pScene->WriteFile(SCENE_OUTPUT_B));

	cReader.Init(SCENE_OUTPUT_B);
	AssertTrue(cReader.Open());
	AssertTrue(cReader.BeginMesh(0));
	AssertLSize(2, cReader.NumPolygons());
	AssertLSize(2, cReader.NumSmoothGroups());
	AssertLSize(6, cReader.NumCornerNormals());
	AssertTrue(cReader.ReadCornerNormals(aiRead));
	AssertInt((int32)SCENE_FILE_NONE, aiRead[0]);
	AssertInt((int32)SCENE_FILE_NONE, aiRead[1]);
	AssertInt((int32)SCENE_FILE_NONE, aiRead[2]);
	AssertTrue(aiRead[3] >= 0);
	AssertTrue(cReader.End());
	cReader.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CConnection> TestSceneConnection(float32 x, Ptr<CConnection> pParent)
{
	Ptr<CConnection>	pConnection;

	pConnection = OMalloc<CConnection>();
	pConnection->msLocalMatrix.pos.x = x;
	pConnection->mpParent = pParent;
	return pConnection;
}


//////////////////////////////////////////////////////////////////////////
//
// A world matrix is the local one times the parent's, so a child turned by
// its parent moves the way its parent faces.  A parent nothing else holds is
// still done, a mirror sets the parity, and a loop of parents stops.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneUpdateConnections(void)
{
	Ptr<CScene>			pScene;
	Ptr<CInstance>		pInstance;
	Ptr<CConnection>	pRoot;
	Ptr<CConnection>	pChild;
	Ptr<CConnection>	pGrandchild;
	Ptr<CConnection>	pMirrored;
	Ptr<CLight>			pLight;
	Ptr<CConnection>	pFirst;
	Ptr<CConnection>	pSecond;

	pRoot = TestSceneConnection(10, NULL);
	Float4x4RotationZ(&pRoot->msLocalMatrix, 3.14159265f / 2.0f);
	pRoot->msLocalMatrix.pos.x = 10;
	pChild = TestSceneConnection(1, pRoot);
	pGrandchild = TestSceneConnection(2, pChild);
	pGrandchild->msWorldMatrix.pos.z = 99;

	pInstance = OMalloc<CInstance>();
	pInstance->mapConnections.Add(pGrandchild);
	pScene = OMalloc<CScene>();
	pScene->mapInstances.Add(pInstance);

	pMirrored = TestSceneConnection(0, NULL);
	pMirrored->msLocalMatrix.x.x = -1;
	pLight = OMalloc<CLight>();
	pLight->mpConnection = pMirrored;
	pScene->mapLights.Add(pLight);

	pScene->UpdateConnections();

	AssertFloat(10.0f, pRoot->msWorldMatrix.pos.x, 3);
	AssertFloat(10.0f, pChild->msWorldMatrix.pos.x, 3);
	AssertFloat(1.0f, pChild->msWorldMatrix.pos.y, 3);
	AssertFloat(10.0f, pGrandchild->msWorldMatrix.pos.x, 3);
	AssertFloat(3.0f, pGrandchild->msWorldMatrix.pos.y, 3);
	AssertFloat(0.0f, pGrandchild->msWorldMatrix.pos.z, 3);
	AssertFalse(pGrandchild->mbParity);
	AssertTrue(pMirrored->mbParity);

	//A parent moved is seen by the next update.
	pRoot->msLocalMatrix.pos.y = 5;
	pScene->UpdateConnections();
	AssertFloat(8.0f, pGrandchild->msWorldMatrix.pos.y, 3);

	pFirst = TestSceneConnection(1, NULL);
	pSecond = TestSceneConnection(1, pFirst);
	pFirst->mpParent = pSecond;
	pInstance->mapConnections.Add(pFirst);
	pScene->UpdateConnections();
	pFirst->mpParent = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An ambient light needs no connection, and keeps its type through a file.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneAmbientLight(void)
{
	Ptr<CScene>	pScene;
	Ptr<CLight>	pLight;

	pScene = OMalloc<CScene>();
	pLight = OMalloc<CLight>();
	pLight->mszName.Append("Sky");
	pLight->meType = LT_Ambient;
	pLight->mfIntensity = 0.25f;
	pScene->mapLights.Add(pLight);
	AssertTrue(pScene->WriteFile(SCENE_OUTPUT_A));

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_OUTPUT_A));
	AssertSize(1, pScene->mapLights.NumElements());
	pLight = pScene->mapLights.Get(0);
	AssertString("Sky", pLight->mszName.Text());
	AssertInt(LT_Ambient, pLight->meType);
	AssertFloat(0.25f, pLight->mfIntensity, 3);
	AssertTrue(pLight->mpConnection.IsNull());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneFailures(void)
{
	Ptr<CScene>			pScene;
	Ptr<CMaterial>		pMaterial;
	Ptr<CInstance>		pInstance;
	Ptr<CInstance>		pOther;
	Ptr<CMeshEditor>	pEditor;
	Ptr<CInstanceMesh>	pInstanceMesh;
	CFileUtil			cFileUtil;

	pScene = OMalloc<CScene>();
	AssertFalse(pScene->ReadFile(SCENE_OUTPUT_DIR _FS_ "NotThere.DRG"));

	//Bytes in another order would come back as other colours.
	pMaterial = OMalloc<CMaterial>();
	pMaterial->GetImage(MI_Diffuse)->mpImage = OMalloc<CImage>(1, 1, PT_uint8, IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_STOP);
	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->mpMesh->mcMaterials.mbInUse = true;
	pEditor->mpMesh->mcMaterials.mapMaterials.Add(pMaterial);
	pInstanceMesh = OMalloc<CInstanceMesh>(pEditor);
	pInstance = OMalloc<CInstance>();
	pInstance->mapMeshes.Add(pInstanceMesh);
	pScene->mapInstances.Add(pInstance);
	AssertFalse(pScene->WriteFile(SCENE_OUTPUT_DIR _FS_ "Bad.DRG"));
	AssertFalse(cFileUtil.Exists(SCENE_OUTPUT_DIR _FS_ "Bad.DRG"));

	//A file holds a mesh's connections as places in its instance's list, so one using another instance's connection reads
	//back as an error rather than as some other connection.
	pScene = OMalloc<CScene>();
	pOther = OMalloc<CInstance>();
	pOther->mapConnections.Add(OMalloc<CConnection>());
	pScene->mapInstances.Add(pOther);
	pInstance = OMalloc<CInstance>();
	pInstance->mapConnections.Add(OMalloc<CConnection>());
	pInstanceMesh = OMalloc<CInstanceMesh>(OMalloc<CMeshEditor>(OMalloc<CMesh>()));
	pInstanceMesh->mapConnections.Add(pOther->mapConnections.Get(0));
	pInstance->mapMeshes.Add(pInstanceMesh);
	pScene->mapInstances.Add(pInstance);
	AssertTrue(pScene->WriteFile(SCENE_OUTPUT_DIR _FS_ "Elsewhere.DRG"));
	pScene = OMalloc<CScene>();
	AssertFalse(pScene->ReadFile(SCENE_OUTPUT_DIR _FS_ "Elsewhere.DRG"));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestScene(void)
{
	CFileUtil	cFileUtil;
	SLogConfig	sLogConfig;

	BeginTests();

	DataIOInit();
	ObjectsInit();
	cFileUtil.MakeDir("Output");
	cFileUtil.RemoveDir(SCENE_OUTPUT_DIR);
	cFileUtil.MakeDir(SCENE_OUTPUT_DIR);

	TestSceneRead();
	TestSceneWrite();
	TestSceneImages();
	TestSceneSmoothGroups();
	TestSceneUpdateConnections();
	TestSceneAmbientLight();

	sLogConfig = gcLogger.SetSilent();
	TestSceneFailures();
	gcLogger.SetConfig(&sLogConfig);

	cFileUtil.RemoveDir(SCENE_OUTPUT_DIR);
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}


//////////////////////////////////////////////////////////////////////////
//
// Goober's bones are a skeleton, all under the first, and working out
// their world matrices from it leaves them where its file has them.
//
//////////////////////////////////////////////////////////////////////////
static void TestSceneGooberSkeleton(Ptr<CScene> pScene, Ptr<CInstance> pInstance)
{
	Ptr<CConnection>	pConnection;
	CArrayFloat4x4		asWorlds;
	size				uiNumParents;
	size				i;
	int32				j;
	float32				fExpected;
	float32				fDifference;
	float32				fWorst;

	asWorlds.Init();
	uiNumParents = 0;
	for (i = 0; i < pInstance->mapConnections.NumElements(); i++)
	{
		pConnection = pInstance->mapConnections.Get(i);
		asWorlds.Add(&pConnection->msWorldMatrix);
		if (pConnection->mpParent.IsNotNull())
		{
			uiNumParents++;
		}
	}
	AssertSize(66, uiNumParents);
	AssertTrue(pInstance->mapConnections.Get(0)->mpParent.IsNull());

	pScene->UpdateConnections();
	fWorst = 0.0f;
	for (i = 0; i < pInstance->mapConnections.NumElements(); i++)
	{
		pConnection = pInstance->mapConnections.Get(i);
		for (j = 0; j < 16; j++)
		{
			fExpected = ((float32*)asWorlds.Get(i))[j];
			fDifference = fabsf(((float32*)&pConnection->msWorldMatrix)[j] - fExpected) / (fabsf(fExpected) + 1.0f);
			if (fDifference > fWorst)
			{
				fWorst = fDifference;
			}
		}
	}
	AssertTrue(fWorst < 0.0001f);
	asWorlds.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A skinned mesh walking in a field of stars.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneGooberRead(void)
{
	Ptr<CScene>			pScene;
	Ptr<CInstance>		pInstance;
	Ptr<CInstanceMesh>	pInstanceMesh;
	CMesh*				pcMesh;
	size				i;

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_GOOBER));

	AssertSize(1, pScene->mapInstances.NumElements());
	pInstance = pScene->mapInstances.Get(0);
	AssertSize(67, pInstance->mapConnections.NumElements());
	AssertSize(1, pInstance->mapMeshes.NumElements());
	for (i = 0; i < 67; i++)
	{
		AssertTrue(pInstance->mapConnections.Get(i)->mszName.StartsWith("bone") || pInstance->mapConnections.Get(i)->mszName.StartsWith("Bone"));
	}

	pInstanceMesh = pInstance->mapMeshes.Get(0);
	AssertSize(67, pInstanceMesh->mapConnections.NumElements());
	pcMesh = &pInstanceMesh->mpEditor->mpMesh;
	AssertInt(3241, pcMesh->NumCorners());
	AssertInt(6134, pcMesh->NumFaces());
	AssertLSize(3241, pcMesh->mcSkin.mcSkinVerts.NumElements());
	AssertSize(67, pcMesh->mcSkin.mcInverseSkinMatricies.NumElements());
	AssertSize(1, pcMesh->mcUVs.mcLayers.NumElements());

	//It has no normals in its file: they come from its smoothing groups.
	AssertTrue(pcMesh->mcNormals.mcNormals.NumElements() > (size)pcMesh->NumFaces());
	AssertTrue(pcMesh->mcSmoothing.InUse());
	AssertLSize(6134, pcMesh->mcSmoothing.mauiGroups.NumElements());

	AssertSize(1, pScene->mapAnimations.NumElements());
	AssertSize(140, pScene->mapAnimations.Get(0)->mapTracks.NumElements());
	AssertSize(80, pScene->mapAnimations.Get(0)->mapTracks.Get(0)->masKeys.NumElements());
	AssertSize(1, pScene->mapParticles.NumElements());
	AssertLSize(1105, pScene->mapParticles.Get(0)->NumParticles());

	AssertTrue(pcMesh->mcMaterials.mapMaterials.Get(0)->GetImage(MI_Diffuse)->mpImage.IsNotNull());
	AssertString("Goober_D.tga", pcMesh->mcMaterials.mapMaterials.Get(0)->GetImage(MI_Diffuse)->mszImageName.Text());

	TestSceneGooberSkeleton(pScene, pInstance);
}


//////////////////////////////////////////////////////////////////////////
//
// The most any of an instance's bones has changed in length, from its
// parent's position to its own, against pafLengths, which it fills the
// first time (when it's empty).  A change is against the length and a
// unit, so that a bone of almost no length doesn't make rounding look big.
//
//////////////////////////////////////////////////////////////////////////
static float32 TestSceneGooberBoneChange(Ptr<CInstance> pInstance, CArrayFloat32* pafLengths)
{
	Ptr<CConnection>	pConnection;
	SFloat4x4*			psChild;
	SFloat4x4*			psParent;
	float32				x;
	float32				y;
	float32				z;
	float32				fLength;
	float32				fFirst;
	float32				fChange;
	float32				fWorst;
	size				i;
	size				uiBone;
	bool				bFirst;

	bFirst = pafLengths->NumElements() == 0;
	fWorst = 0.0f;
	uiBone = 0;
	for (i = 0; i < pInstance->mapConnections.NumElements(); i++)
	{
		pConnection = pInstance->mapConnections.Get(i);
		if (pConnection->mpParent.IsNull())
		{
			continue;
		}
		psChild = &pConnection->msWorldMatrix;
		psParent = &pConnection->mpParent->msWorldMatrix;
		x = psChild->pos.x - psParent->pos.x;
		y = psChild->pos.y - psParent->pos.y;
		z = psChild->pos.z - psParent->pos.z;
		fLength = sqrtf(x * x + y * y + z * z);
		if (bFirst)
		{
			pafLengths->Add(fLength);
		}
		else
		{
			fFirst = pafLengths->GetValue(uiBone);
			fChange = fabsf(fLength - fFirst) / (fFirst + 1.0f);
			fWorst = (fChange > fWorst) ? fChange : fWorst;
		}
		uiBone++;
	}
	return fWorst;
}


//////////////////////////////////////////////////////////////////////////
//
// Goober's walk starts where it stands: played at its start, every bone is
// where its file has it.  As it walks its bones move but stay the same
// length, at its keys and between them, which only comes out so if each
// key is relative to its parent's.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneGooberWalk(void)
{
	Ptr<CScene>					pScene;
	Ptr<CInstance>				pInstance;
	Ptr<CSceneAnimation>		pAnimation;
	Ptr<CConnectionAnimation>	pTrack;
	Ptr<CAnimationPlayer>		pPlayer;
	Ptr<CConnection>			pConnection;
	CArrayFloat4x4				asWorlds;
	CArrayFloat32				afLengths;
	size						i;
	int32						j;
	float32						fDifference;
	float32						fWorst;
	float32						fMoved;

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_GOOBER));
	pInstance = pScene->mapInstances.Get(0);
	pAnimation = pScene->mapAnimations.Get(0);
	pTrack = pAnimation->mapTracks.Get(0);
	AssertTrue(pAnimation->GetDuration() > 1.0f);

	asWorlds.Init();
	for (i = 0; i < pInstance->mapConnections.NumElements(); i++)
	{
		asWorlds.Add(&pInstance->mapConnections.Get(i)->msWorldMatrix);
	}

	pPlayer = OMalloc<CAnimationPlayer>(pAnimation, true);
	pPlayer->Apply();
	pScene->UpdateConnections();
	fWorst = 0.0f;
	for (i = 0; i < pInstance->mapConnections.NumElements(); i++)
	{
		pConnection = pInstance->mapConnections.Get(i);
		for (j = 0; j < 16; j++)
		{
			fDifference = fabsf(((float32*)&pConnection->msWorldMatrix)[j] - ((float32*)asWorlds.Get(i))[j]) / (fabsf(((float32*)asWorlds.Get(i))[j]) + 1.0f);
			fWorst = (fDifference > fWorst) ? fDifference : fWorst;
		}
	}
	AssertTrue(fWorst < 0.0001f);

	afLengths.Init();
	TestSceneGooberBoneChange(pInstance, &afLengths);
	AssertSize(66, afLengths.NumElements());
	fWorst = 0.0f;
	for (i = 4; i < pTrack->NumKeys(); i += 4)
	{
		pPlayer->SetTime(pTrack->GetKey(i)->fTime);
		pPlayer->Apply();
		pScene->UpdateConnections();
		fDifference = TestSceneGooberBoneChange(pInstance, &afLengths);
		fWorst = (fDifference > fWorst) ? fDifference : fWorst;
	}
	AssertTrue(fWorst < 0.0001f);

	pPlayer->SetTime((pTrack->GetKey(40)->fTime + pTrack->GetKey(41)->fTime) * 0.5f);
	pPlayer->Apply();
	pScene->UpdateConnections();
	AssertTrue(TestSceneGooberBoneChange(pInstance, &afLengths) < 0.001f);
	fMoved = 0.0f;
	for (i = 0; i < pInstance->mapConnections.NumElements(); i++)
	{
		pConnection = pInstance->mapConnections.Get(i);
		fDifference = fabsf(pConnection->msWorldMatrix.pos.x - asWorlds.Get(i)->pos.x) + fabsf(pConnection->msWorldMatrix.pos.y - asWorlds.Get(i)->pos.y) + fabsf(pConnection->msWorldMatrix.pos.z - asWorlds.Get(i)->pos.z);
		fMoved = (fDifference > fMoved) ? fDifference : fMoved;
	}
	AssertTrue(fMoved > 0.01f);

	afLengths.Kill();
	asWorlds.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Its file stays the same once it's been through a scene.  Only one scene
// is held at a time, as its images are 33MB.
//
//////////////////////////////////////////////////////////////////////////
void TestSceneGooberWrite(void)
{
	Ptr<CScene>	pScene;

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_GOOBER));
	AssertTrue(pScene->WriteFile(SCENE_OUTPUT_A));
	pScene = NULL;

	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(SCENE_OUTPUT_A));
	AssertTrue(pScene->WriteFile(SCENE_OUTPUT_B));
	pScene = NULL;

	AssertFile(SCENE_OUTPUT_A, SCENE_OUTPUT_B);
}


//////////////////////////////////////////////////////////////////////////
//
// Goober's images are bigger than the debug allocator's limit.
//
//////////////////////////////////////////////////////////////////////////
static void BeginTestSceneGoober(lsize* puiMaxDebugAllocatorSize)
{
	CFileUtil	cFileUtil;

	*puiMaxDebugAllocatorSize = guiMaxDebugAllocatorSize;
	InitAllocatorDebug(32 MB);
	DataIOInit();
	ObjectsInit();
	cFileUtil.MakeDir("Output");
	cFileUtil.RemoveDir(SCENE_OUTPUT_DIR);
	cFileUtil.MakeDir(SCENE_OUTPUT_DIR);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void EndTestSceneGoober(lsize uiMaxDebugAllocatorSize)
{
	CFileUtil	cFileUtil;

	cFileUtil.RemoveDir(SCENE_OUTPUT_DIR);
	ObjectsKill(false);
	DataIOKill();
	InitAllocatorDebug(uiMaxDebugAllocatorSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneGoober(void)
{
	lsize	uiMaxDebugAllocatorSize;

	BeginTests();

	BeginTestSceneGoober(&uiMaxDebugAllocatorSize);
	TestSceneGooberRead();
	ObjectsFlush();
	TestSceneGooberWalk();
	EndTestSceneGoober(uiMaxDebugAllocatorSize);

	TestStatistics();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSceneGooberResave(void)
{
	lsize	uiMaxDebugAllocatorSize;

	BeginTests();

	BeginTestSceneGoober(&uiMaxDebugAllocatorSize);
	TestSceneGooberWrite();
	EndTestSceneGoober(uiMaxDebugAllocatorSize);

	TestStatistics();
}

