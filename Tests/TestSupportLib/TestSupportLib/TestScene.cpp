#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Mallocator.h"
#include "StandardLib/Objects.h"
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
	AssertSize((size)iNumFlat + 2, pcMesh->mcNormals.mcNormals.NumElements());
	AssertInt(iNumFlat, pcMesh->mcNormals.mcFaces.Get(0)->aiCornerNormals[0]);
	AssertInt(iNumFlat + 1, pcMesh->mcNormals.mcFaces.Get(2)->aiCornerNormals[0]);
	AssertInt(-1, pcMesh->mcNormals.mcFaces.Get(2)->aiCornerNormals[2]);

	AssertSize(2, pcMesh->mcUVs.mcLayers.NumElements());
	AssertSize(4, pcMesh->mcUVs.mcLayers.Get(0)->mcUVs.NumElements());
	AssertSize(1, pcMesh->mcUVs.mcLayers.Get(1)->mcUVs.NumElements());
	AssertInt(-1, pcMesh->mcUVs.mcLayers.Get(0)->mcFaces.Get(2)->aiUVs[2]);

	AssertTrue(pcMesh->mcColours.InUse());
	AssertSize(2, pcMesh->mcColours.mcColours.NumElements());
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
	AssertSize(5, pcMesh->mcSkin.mcSkinVerts.NumElements());
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

	AssertSize(2, pInstance->mapMeshes.Get(0)->maiConnectionSlots.NumElements());
	AssertInt(1, pInstance->mapMeshes.Get(0)->maiConnectionSlots.GetValue(1));

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
	AssertSize(2, pTrack->masKeyFrames.NumElements());
	AssertFloat(0.5f, pTrack->masKeyFrames.Get(1)->mfTime, 3);
	AssertInt(KFT_Matrix, pTrack->masKeyFrames.Get(1)->miType);
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
	AssertSize(2, pStars->NumParticles());
	AssertFloat(6.0f, *pStars->mafPositions.Get(5), 3);
	AssertFloat(0.5f, *pStars->mafColours.Get(4), 3);
	AssertFloat(2.0f, *pStars->mafRadii.Get(1), 3);
	AssertFloat(0.25f, *pStars->mafIntensities.Get(1), 3);

	pDust = pScene->mapParticles.Get(1);
	AssertString("Dust", pDust->mszName.Text());
	AssertSize(2, pDust->NumParticles());
	AssertSize(0, pDust->mafColours.NumElements());
	AssertSize(0, pDust->mafRadii.NumElements());
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
	AssertSize(2, cReader.NumPolygons());
	AssertTrue(cReader.ReadPolygonSizes(aiSizes));
	AssertInt((int32)4, aiSizes[0]);
	AssertInt((int32)3, aiSizes[1]);
	AssertSize(7, cReader.NumCorners());
	AssertTrue(cReader.ReadCornerPositions(aiCorners));
	AssertInt((int32)0, aiCorners[0]);
	AssertInt((int32)3, aiCorners[1]);
	AssertInt((int32)2, aiCorners[2]);
	AssertInt((int32)1, aiCorners[3]);
	AssertInt((int32)0, aiCorners[4]);
	AssertInt((int32)1, aiCorners[5]);
	AssertInt((int32)4, aiCorners[6]);
	AssertSize(2, cReader.NumNormals());
	AssertSize(2, cReader.NumUVLayers());
	AssertSize(2, cReader.NumColours());
	AssertSize(2, cReader.NumMaterialSlots());
	AssertSize(2, cReader.NumSmoothGroups());
	AssertTrue(cReader.ReadSmoothGroups(auiGroups));
	AssertTrue(auiGroups[0] == 1);
	AssertTrue(auiGroups[1] == 0x80000002);
	AssertSize(5, cReader.NumSkinCounts());
	AssertSize(2, cReader.NumInverseMatrices());
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
	AssertSize(2, cReader.NumPolygons());
	AssertSize(2, cReader.NumSmoothGroups());
	AssertSize(6, cReader.NumCornerNormals());
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
void TestSceneFailures(void)
{
	Ptr<CScene>			pScene;
	Ptr<CMaterial>		pMaterial;
	Ptr<CInstance>		pInstance;
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
	AssertSize(67, pInstanceMesh->maiConnectionSlots.NumElements());
	pcMesh = &pInstanceMesh->mpEditor->mpMesh;
	AssertInt(3241, pcMesh->NumCorners());
	AssertInt(6134, pcMesh->NumFaces());
	AssertSize(3241, pcMesh->mcSkin.mcSkinVerts.NumElements());
	AssertSize(67, pcMesh->mcSkin.mcInverseSkinMatricies.NumElements());
	AssertSize(1, pcMesh->mcUVs.mcLayers.NumElements());

	//It has no normals in its file: they come from its smoothing groups.
	AssertTrue(pcMesh->mcNormals.mcNormals.NumElements() > (size)pcMesh->NumFaces());
	AssertTrue(pcMesh->mcSmoothing.InUse());
	AssertSize(6134, pcMesh->mcSmoothing.mauiGroups.NumElements());

	AssertSize(1, pScene->mapAnimations.NumElements());
	AssertSize(140, pScene->mapAnimations.Get(0)->mapTracks.NumElements());
	AssertSize(80, pScene->mapAnimations.Get(0)->mapTracks.Get(0)->masKeyFrames.NumElements());
	AssertSize(1, pScene->mapParticles.NumElements());
	AssertSize(1105, pScene->mapParticles.Get(0)->NumParticles());

	AssertTrue(pcMesh->mcMaterials.mapMaterials.Get(0)->GetImage(MI_Diffuse)->mpImage.IsNotNull());
	AssertString("Goober_D.tga", pcMesh->mcMaterials.mapMaterials.Get(0)->GetImage(MI_Diffuse)->mszImageName.Text());
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

