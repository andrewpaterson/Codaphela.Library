#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalMemory.h"
#include "BaseLib/Logger.h"
#include "OldDragonLib/OldDragonConverter.h"
#include "OldDragonLib/OldDragonReader.h"
#include "SupportLib/SceneFileReader.h"
#include "TestLib/Assert.h"


#define CONVERTER_OUTPUT_DIR	"Output" _FS_ "Converter"
#define CONVERTED				CONVERTER_OUTPUT_DIR _FS_ "Converted.DRG"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetTestName(char* szName, const char* szValue)
{
	strcpy(szName, szValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SetTestMatrix(SOldDragonMatrix* psMatrix, float32 fValue)
{
	int32	i;

	for (i = 0; i < 16; i++)
	{
		psMatrix->af[i] = fValue + (float32)i;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static COldDragonMesh* AddTestMesh(COldDragonScene* pcScene, const char* szName)
{
	COldDragonMesh*	pcMesh;
	SOldDragonVec3*	psPosition;
	SOldDragonFace*	psFace;
	int32			i;

	pcMesh = pcScene->AddMesh();
	SetTestName(pcMesh->mszName, szName);
	pcMesh->miID = (int32)pcScene->macMeshes.NumElements() - 1;
	for (i = 0; i < 3; i++)
	{
		psPosition = pcMesh->macPositions.Add();
		psPosition->x = (float32)i;
		psPosition->y = 1.0f;
		psPosition->z = 2.0f;
	}
	psFace = pcMesh->macFaces.Add();
	for (i = 0; i < 3; i++)
	{
		psFace->aiPositions[i] = i;
		psFace->aiNormals[i] = OLD_DRAGON_NO_INDEX;
		psFace->aiUVs[i] = OLD_DRAGON_NO_INDEX;
	}
	psFace->iMaterial = OLD_DRAGON_NO_INDEX;
	psFace->uiSmoothing = 0;
	return pcMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SOldDragonConnection* AddTestConnection(COldDragonScene* pcScene, const char* szName, int32 iObjectType, int32 iID)
{
	SOldDragonConnection*	psConnection;

	psConnection = pcScene->AddConnection();
	SetTestName(psConnection->szName, szName);
	psConnection->iID = iID;
	psConnection->iObjectType = iObjectType;
	SetTestMatrix(&psConnection->sWorld, 1.0f);
	SetTestMatrix(&psConnection->sLocal, 100.0f);
	psConnection->bSelected = false;
	psConnection->bTopLevel = false;
	psConnection->bParity = false;
	return psConnection;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static COldDragonInstance* AddTestInstance(COldDragonScene* pcScene, int32 iType, int32 iObject, int32 iConnection)
{
	COldDragonInstance*	pcInstance;

	pcInstance = pcScene->AddInstance();
	pcInstance->miType = iType;
	pcInstance->miObject = iObject;
	pcInstance->maiConnections.Add(&iConnection);
	return pcInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SOldDragonLight* AddTestLight(COldDragonScene* pcScene, const char* szName, int32 iType)
{
	SOldDragonLight*	psLight;

	psLight = pcScene->AddLight();
	memset(psLight, 0, sizeof(SOldDragonLight));
	SetTestName(psLight->szName, szName);
	psLight->iID = (int32)pcScene->macLights.NumElements() - 1;
	psLight->iType = iType;
	psLight->fIntensity = 0.5f;
	return psLight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SOldDragonCamera* AddTestCamera(COldDragonScene* pcScene, const char* szName, float32 fFOV)
{
	SOldDragonCamera*	psCamera;

	psCamera = pcScene->AddCamera();
	memset(psCamera, 0, sizeof(SOldDragonCamera));
	SetTestName(psCamera->szName, szName);
	psCamera->iID = (int32)pcScene->macCameras.NumElements() - 1;
	psCamera->fFOV = fFOV;
	psCamera->fNear = 1.0f;
	psCamera->fFar = 1000.0f;
	return psCamera;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SOldDragonMaterial* AddTestMaterial(COldDragonScene* pcScene, const char* szName)
{
	SOldDragonMaterial*	psMaterial;

	psMaterial = pcScene->AddMaterial();
	memset(psMaterial, 0, sizeof(SOldDragonMaterial));
	SetTestName(psMaterial->szName, szName);
	return psMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddTestKey(COldDragonSequence* pcSequence, float32 fValue, float32 fTime)
{
	SOldDragonKeyFrame*	psKey;

	psKey = pcSequence->macKeyFrames.Add();
	SetTestMatrix(&psKey->sMatrix, fValue);
	psKey->fTime = fTime;
}


//////////////////////////////////////////////////////////////////////////
//
// A scene that's written and then read back, to look at what's in it.
//
//////////////////////////////////////////////////////////////////////////
static void AssertConverts(COldDragonScene* pcScene)
{
	AssertTrue(WriteOldDragonSceneFile(pcScene, CONVERTED));
	AssertTrue(VerifyOldDragonSceneFile(pcScene, CONVERTED));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConvertFaces(void)
{
	COldDragonScene		cScene;
	COldDragonMesh*		pcMesh;
	SOldDragonFace*		psFace;
	CSceneFileReader	cReader;
	int32				aiSlots[2];
	int32				aiPolygonSlots[2];
	int32				aiSizes[2];
	uint32				auiGroups[2];

	cScene.Init();
	cScene.meVersion = ODV_September2009;
	AddTestMaterial(&cScene, "Unused");
	AddTestMaterial(&cScene, "Second");
	AddTestMaterial(&cScene, "Third");

	pcMesh = AddTestMesh(&cScene, "Mesh");
	pcMesh->macFaces.Get(0)->iMaterial = 2;
	pcMesh->macFaces.Get(0)->uiSmoothing = 4;
	psFace = pcMesh->macFaces.Add();
	*psFace = *pcMesh->macFaces.Get(0);
	psFace->iMaterial = OLD_DRAGON_NO_INDEX;
	psFace->uiSmoothing = 8;

	AddTestMesh(&cScene, "Plain");
	AssertConverts(&cScene);

	cReader.Init(CONVERTED);
	AssertTrue(cReader.Open());
	AssertSize(2, cReader.NumMeshes());

	AssertTrue(cReader.BeginMesh(0));
	AssertSize(2, cReader.NumPolygons());
	AssertSize(6, cReader.NumCorners());
	AssertTrue(cReader.ReadPolygonSizes(aiSizes));
	AssertInt(3, aiSizes[0]);
	AssertInt(3, aiSizes[1]);
	AssertSize(1, cReader.NumMaterialSlots());
	AssertTrue(cReader.ReadMaterialSlots(aiSlots));
	AssertInt(2, aiSlots[0]);
	AssertSize(2, cReader.NumPolygonSlots());
	AssertTrue(cReader.ReadPolygonSlots(aiPolygonSlots));
	AssertInt(0, aiPolygonSlots[0]);
	AssertInt(SCENE_FILE_NONE, aiPolygonSlots[1]);
	AssertTrue(cReader.ReadSmoothGroups(auiGroups));
	AssertInt(4, auiGroups[0]);
	AssertInt(8, auiGroups[1]);
	AssertSize(0, cReader.NumNormals());
	AssertSize(0, cReader.NumCornerNormals());
	AssertSize(0, cReader.NumUVLayers());
	AssertTrue(cReader.End());

	AssertTrue(cReader.BeginMesh(1));
	AssertSize(0, cReader.NumMaterialSlots());
	AssertSize(0, cReader.NumPolygonSlots());
	AssertSize(0, cReader.NumSmoothGroups());
	AssertTrue(cReader.End());

	cReader.Kill();
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddTestImage(COldDragonScene* pcScene, const char* szName, int32 iChannels, const uint8* puiPixels)
{
	COldDragonImage*	pcImage;

	pcImage = pcScene->AddImage();
	SetTestName(pcImage->mszName, szName);
	pcImage->miWidth = 2;
	pcImage->miHeight = 1;
	pcImage->miChannels = iChannels;
	pcImage->SetPixels(puiPixels, 2 * (size)iChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConvertImages(void)
{
	COldDragonScene		cScene;
	CSceneFileReader	cReader;
	CChars				szChannels;
	uint8				auiPixels[8];
	const uint8			auiBGR[6] = { 1, 2, 3, 4, 5, 6 };
	const uint8			auiBGRA[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	const uint8			auiMask[2] = { 9, 10 };

	cScene.Init();
	cScene.meVersion = ODV_September2009;
	AddTestImage(&cScene, "BGR", 3, auiBGR);
	AddTestImage(&cScene, "BGRA", 4, auiBGRA);
	AddTestImage(&cScene, "Mask", 1, auiMask);
	AssertConverts(&cScene);

	cReader.Init(CONVERTED);
	AssertTrue(cReader.Open());
	AssertSize(3, cReader.NumImages());

	szChannels.Init();
	AssertTrue(cReader.BeginImage(0));
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("RGB", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiPixels));
	AssertInt(3, auiPixels[0]);
	AssertInt(2, auiPixels[1]);
	AssertInt(1, auiPixels[2]);
	AssertInt(6, auiPixels[3]);
	AssertInt(5, auiPixels[4]);
	AssertInt(4, auiPixels[5]);
	AssertTrue(cReader.End());
	szChannels.Kill();

	szChannels.Init();
	AssertTrue(cReader.BeginImage(1));
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("RGBA", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiPixels));
	AssertInt(3, auiPixels[0]);
	AssertInt(1, auiPixels[2]);
	AssertInt(4, auiPixels[3]);
	AssertInt(7, auiPixels[4]);
	AssertInt(5, auiPixels[6]);
	AssertInt(8, auiPixels[7]);
	AssertTrue(cReader.End());
	szChannels.Kill();

	szChannels.Init();
	AssertTrue(cReader.BeginImage(2));
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("A", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiPixels));
	AssertInt(9, auiPixels[0]);
	AssertInt(10, auiPixels[1]);
	AssertTrue(cReader.End());
	szChannels.Kill();

	cReader.Kill();
	cScene.Kill();

	//The Max exporter's are already the right way round.
	cScene.Init();
	cScene.meVersion = ODV_Max;
	AddTestImage(&cScene, "RGB", 3, auiBGR);
	AddTestImage(&cScene, "RGBA", 4, auiBGRA);
	AssertConverts(&cScene);

	cReader.Init(CONVERTED);
	AssertTrue(cReader.Open());

	szChannels.Init();
	AssertTrue(cReader.BeginImage(0));
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("RGB", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiPixels));
	AssertInt(1, auiPixels[0]);
	AssertInt(3, auiPixels[2]);
	AssertInt(4, auiPixels[3]);
	AssertInt(6, auiPixels[5]);
	AssertTrue(cReader.End());
	szChannels.Kill();

	szChannels.Init();
	AssertTrue(cReader.BeginImage(1));
	AssertTrue(cReader.ReadChannels(&szChannels));
	AssertString("RGBA", szChannels.Text());
	AssertTrue(cReader.ReadPixels(auiPixels));
	AssertInt(1, auiPixels[0]);
	AssertInt(3, auiPixels[2]);
	AssertInt(4, auiPixels[3]);
	AssertInt(8, auiPixels[7]);
	AssertTrue(cReader.End());
	szChannels.Kill();

	cReader.Kill();
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConvertKeys(void)
{
	SLogConfig			sLogConfig;
	COldDragonScene		cScene;
	COldDragonSequence*	pcSequence;
	CSceneFileReader	cReader;
	float32				afTimes[3];
	float32				af4x4s[48];

	cScene.Init();
	cScene.meVersion = ODV_September2009;
	AddTestConnection(&cScene, "Bone", OLD_DRAGON_NO_INDEX, 0);
	pcSequence = cScene.AddSequence();
	SetTestName(pcSequence->mszName, "Bone");
	pcSequence->miConnection = 0;
	AddTestKey(pcSequence, 0.0f, 0.0f);
	AddTestKey(pcSequence, 20.0f, 0.5f);
	AddTestKey(pcSequence, 40.0f, 0.25f);
	AddTestKey(pcSequence, 40.0f, 0.0f);
	AssertConverts(&cScene);

	cReader.Init(CONVERTED);
	AssertTrue(cReader.Open());
	AssertSize(1, cReader.NumAnimations());
	AssertTrue(cReader.BeginAnimation(0));
	AssertSize(1, cReader.NumTracks());
	AssertTrue(cReader.BeginTrack(0));
	AssertInt(0, cReader.GetAttached());
	AssertSize(3, cReader.NumKeys());
	AssertTrue(cReader.ReadKeyTimes(afTimes));
	AssertFloat(0.0f, afTimes[0], 3);
	AssertFloat(0.5f, afTimes[1], 3);
	AssertFloat(0.75f, afTimes[2], 3);
	AssertTrue(cReader.ReadKeyMatrices(af4x4s));
	AssertFloat(40.0f, af4x4s[32], 3);
	AssertTrue(cReader.End());
	AssertTrue(cReader.End());
	cReader.Kill();
	cScene.Kill();

	//A key with no time after the first, that isn't a repeat of the one before and the last, isn't understood.
	cScene.Init();
	cScene.meVersion = ODV_September2009;
	AddTestConnection(&cScene, "Bone", OLD_DRAGON_NO_INDEX, 0);
	pcSequence = cScene.AddSequence();
	SetTestName(pcSequence->mszName, "Bone");
	pcSequence->miConnection = 0;
	AddTestKey(pcSequence, 0.0f, 0.0f);
	AddTestKey(pcSequence, 20.0f, 0.0f);
	AddTestKey(pcSequence, 40.0f, 0.5f);

	sLogConfig = gcLogger.SetSilent();
	AssertFalse(WriteOldDragonSceneFile(&cScene, CONVERTED));
	gcLogger.SetConfig(&sLogConfig);
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConvertNumbering(void)
{
	COldDragonScene		cScene;
	COldDragonSequence*	pcSequence;
	CSceneFileReader	cReader;
	SLogConfig			sLogConfig;
	int32				aiConnections[2];
	int32				iNumber;

	//As in Goober: the nodes in the middle of the list aren't counted, and their sequences aren't numbered.
	cScene.Init();
	cScene.meVersion = ODV_September2009;
	AddTestMesh(&cScene, "Mesh");
	AddTestConnection(&cScene, "Mesh Place", OLD_DRAGON_NO_INDEX, 0);
	AddTestConnection(&cScene, "Bone A", OLD_DRAGON_NO_INDEX, 1);
	AddTestConnection(&cScene, "IK Chain", OLD_DRAGON_NO_INDEX, 2);
	AddTestConnection(&cScene, "Bone B", OLD_DRAGON_NO_INDEX, 3);

	pcSequence = cScene.AddSequence();
	SetTestName(pcSequence->mszName, "Bone A");
	pcSequence->miConnection = 1;
	AddTestKey(pcSequence, 0.0f, 0.0f);

	pcSequence = cScene.AddSequence();
	SetTestName(pcSequence->mszName, "IK Chain");
	AddTestKey(pcSequence, 0.0f, 0.0f);

	pcSequence = cScene.AddSequence();
	SetTestName(pcSequence->mszName, "Bone B");
	pcSequence->miConnection = 2;
	AddTestKey(pcSequence, 0.0f, 0.0f);

	iNumber = 2;
	AddTestInstance(&cScene, OLD_DRAGON_2009_INSTANCE_MESH, 0, 1)->maiConnections.Add(&iNumber);
	AssertConverts(&cScene);

	cReader.Init(CONVERTED);
	AssertTrue(cReader.Open());
	AssertTrue(cReader.BeginInstance(0));
	AssertSize(2, cReader.NumInstanceConnections());
	AssertTrue(cReader.ReadInstanceConnections(aiConnections));
	AssertInt(1, aiConnections[0]);
	AssertInt(3, aiConnections[1]);
	AssertTrue(cReader.End());

	AssertTrue(cReader.BeginAnimation(0));
	AssertSize(3, cReader.NumTracks());
	AssertTrue(cReader.BeginTrack(0));
	AssertInt(1, cReader.GetAttached());
	AssertTrue(cReader.End());
	AssertTrue(cReader.BeginTrack(1));
	AssertInt(2, cReader.GetAttached());
	AssertTrue(cReader.End());
	AssertTrue(cReader.BeginTrack(2));
	AssertInt(3, cReader.GetAttached());
	AssertTrue(cReader.End());
	AssertTrue(cReader.End());
	cReader.Kill();
	cScene.Kill();

	//With no connection of its name, and with two, which one it moves isn't known.
	cScene.Init();
	cScene.meVersion = ODV_September2009;
	AddTestConnection(&cScene, "Other", OLD_DRAGON_NO_INDEX, 0);
	pcSequence = cScene.AddSequence();
	SetTestName(pcSequence->mszName, "Nothing");
	pcSequence->miConnection = 0;
	AddTestKey(pcSequence, 0.0f, 0.0f);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(WriteOldDragonSceneFile(&cScene, CONVERTED));
	gcLogger.SetConfig(&sLogConfig);
	cScene.Kill();

	cScene.Init();
	cScene.meVersion = ODV_September2009;
	AddTestConnection(&cScene, "Twin", OLD_DRAGON_NO_INDEX, 0);
	AddTestConnection(&cScene, "Twin", OLD_DRAGON_NO_INDEX, 1);
	pcSequence = cScene.AddSequence();
	SetTestName(pcSequence->mszName, "Twin");
	pcSequence->miConnection = 0;
	AddTestKey(pcSequence, 0.0f, 0.0f);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(WriteOldDragonSceneFile(&cScene, CONVERTED));
	gcLogger.SetConfig(&sLogConfig);
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConvertFromInstances(void)
{
	SLogConfig			sLogConfig;
	COldDragonScene		cScene;
	CSceneFileReader	cReader;
	CChars				szName;
	float32				fFOV;
	float32				fNear;
	float32				fFar;
	float32				fDecayStart;
	float32				fRange;
	float32				fIntensity;
	int32				aiConnections[1];

	cScene.Init();
	cScene.meVersion = ODV_Max;
	AddTestMesh(&cScene, "Mesh");
	AddTestLight(&cScene, "FDirect01", 1);
	AddTestLight(&cScene, "Omni01", 0);
	AddTestLight(&cScene, "Unplaced Spot", 1);
	AddTestCamera(&cScene, "Camera", 90.0f);
	AddTestConnection(&cScene, "Mesh Place", OLD_DRAGON_NO_INDEX, 0);
	AddTestConnection(&cScene, "Light Place", OLD_DRAGON_NO_INDEX, 1);
	AddTestConnection(&cScene, "Omni Place", OLD_DRAGON_NO_INDEX, 2);
	AddTestConnection(&cScene, "Camera Place A", OLD_DRAGON_NO_INDEX, 3);
	AddTestConnection(&cScene, "Camera Place B", OLD_DRAGON_NO_INDEX, 4);
	AddTestInstance(&cScene, 1, 0, 0);
	AddTestInstance(&cScene, 2, 0, 1);
	AddTestInstance(&cScene, 2, 1, 2);
	AddTestInstance(&cScene, 3, 0, 3);
	AddTestInstance(&cScene, 3, 0, 4);
	AssertConverts(&cScene);

	cReader.Init(CONVERTED);
	AssertTrue(cReader.Open());

	AssertSize(1, cReader.NumInstances());
	szName.Init();
	AssertTrue(cReader.BeginInstance(0));
	AssertTrue(cReader.ReadName(&szName));
	AssertString("Mesh Place", szName.Text());
	AssertSize(1, cReader.NumInstanceConnections());
	AssertTrue(cReader.ReadInstanceConnections(aiConnections));
	AssertInt(0, aiConnections[0]);
	AssertTrue(cReader.End());
	szName.Kill();

	AssertSize(3, cReader.NumLights());
	AssertTrue(cReader.BeginLight(0));
	AssertInt(SFLT_Directional, cReader.GetLightType());
	AssertInt(1, cReader.GetAttached());
	AssertTrue(cReader.ReadLightValues(&fDecayStart, &fRange, &fIntensity));
	AssertFloat(0.5f, fIntensity, 3);
	AssertTrue(cReader.End());
	AssertTrue(cReader.BeginLight(1));
	AssertInt(SFLT_Omni, cReader.GetLightType());
	AssertInt(2, cReader.GetAttached());
	AssertTrue(cReader.End());
	AssertTrue(cReader.BeginLight(2));
	AssertInt(SFLT_Spot, cReader.GetLightType());
	AssertInt(SCENE_FILE_NONE, cReader.GetAttached());
	AssertTrue(cReader.End());

	AssertSize(2, cReader.NumCameras());
	AssertTrue(cReader.BeginCamera(0));
	AssertInt(3, cReader.GetAttached());
	AssertTrue(cReader.ReadLens(&fFOV, &fNear, &fFar));
	AssertFloat(1.5708f, fFOV, 3);
	AssertTrue(cReader.End());
	AssertTrue(cReader.BeginCamera(1));
	AssertInt(4, cReader.GetAttached());
	AssertTrue(cReader.End());

	cReader.Kill();
	cScene.Kill();

	//Not a kind that's known.
	cScene.Init();
	cScene.meVersion = ODV_Max;
	AddTestMesh(&cScene, "Mesh");
	AddTestConnection(&cScene, "Mesh Place", OLD_DRAGON_NO_INDEX, 0);
	AddTestInstance(&cScene, 77, 0, 0);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(WriteOldDragonSceneFile(&cScene, CONVERTED));
	gcLogger.SetConfig(&sLogConfig);
	cScene.Kill();

	//Not there.
	cScene.Init();
	cScene.meVersion = ODV_Max;
	AddTestMesh(&cScene, "Mesh");
	AddTestConnection(&cScene, "Mesh Place", OLD_DRAGON_NO_INDEX, 0);
	AddTestInstance(&cScene, 1, 1, 0);
	sLogConfig = gcLogger.SetSilent();
	AssertFalse(WriteOldDragonSceneFile(&cScene, CONVERTED));
	gcLogger.SetConfig(&sLogConfig);
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConvertFromIntoOblivionNodes(void)
{
	COldDragonScene		cScene;
	CSceneFileReader	cReader;
	float32				fFOV;
	float32				fNear;
	float32				fFar;
	float32				fDecayStart;
	float32				fRange;
	float32				fIntensity;
	int32				aiConnections[1];

	cScene.Init();
	cScene.meVersion = ODV_IOX;
	AddTestMesh(&cScene, "First");
	AddTestMesh(&cScene, "Second");
	AddTestLight(&cScene, "Omni01", 0);
	AddTestCamera(&cScene, "Camera", 0.589f);
	AddTestConnection(&cScene, "Root", OIOX_Root, 0);
	AddTestConnection(&cScene, "Second Node", OIOX_Mesh, 1);
	AddTestConnection(&cScene, "First Node", OIOX_Mesh, 0);
	AddTestConnection(&cScene, "Light Node", OIOX_Light, 0);
	AddTestConnection(&cScene, "Camera Node", OIOX_Camera, 0);
	//The link is one instance in the old file, with no object.  Its nodes are what place things.
	cScene.AddInstance();
	AssertConverts(&cScene);

	cReader.Init(CONVERTED);
	AssertTrue(cReader.Open());
	AssertSize(5, cReader.NumConnections());
	AssertSize(2, cReader.NumInstances());
	AssertTrue(cReader.BeginInstance(0));
	AssertTrue(cReader.ReadInstanceConnections(aiConnections));
	AssertInt(1, aiConnections[0]);
	AssertTrue(cReader.BeginInstanceMesh(0));
	AssertInt(1, cReader.GetInstanceMesh());
	AssertTrue(cReader.End());
	AssertTrue(cReader.End());
	AssertTrue(cReader.BeginInstance(1));
	AssertTrue(cReader.ReadInstanceConnections(aiConnections));
	AssertInt(2, aiConnections[0]);
	AssertTrue(cReader.End());

	AssertSize(1, cReader.NumLights());
	AssertTrue(cReader.BeginLight(0));
	AssertInt(3, cReader.GetAttached());
	AssertTrue(cReader.ReadLightValues(&fDecayStart, &fRange, &fIntensity));
	AssertFloat(1.0f, fIntensity, 3);
	AssertTrue(cReader.End());

	AssertSize(1, cReader.NumCameras());
	AssertTrue(cReader.BeginCamera(0));
	AssertInt(4, cReader.GetAttached());
	AssertTrue(cReader.ReadLens(&fFOV, &fNear, &fFar));
	AssertFloat(0.589f, fFOV, 3);
	AssertTrue(cReader.End());

	cReader.Kill();
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The comparison is what the whole conversion is trusted on, so it has to
// notice things being wrong.
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonVerifyNoticesDifferences(void)
{
	SLogConfig			sLogConfig;
	COldDragonScene	cScene;
	COldDragonMesh*	pcMesh;

	cScene.Init();
	cScene.meVersion = ODV_September2009;
	pcMesh = AddTestMesh(&cScene, "Mesh");
	AddTestMaterial(&cScene, "Material");
	AddTestConnection(&cScene, "Place", OLD_DRAGON_NO_INDEX, 0);
	AddTestInstance(&cScene, OLD_DRAGON_2009_INSTANCE_MESH, 0, 0);
	AssertConverts(&cScene);

	sLogConfig = gcLogger.SetSilent();

	pcMesh->macPositions.Get(1)->y = 7.0f;
	AssertFalse(VerifyOldDragonSceneFile(&cScene, CONVERTED));
	pcMesh->macPositions.Get(1)->y = 1.0f;
	AssertTrue(VerifyOldDragonSceneFile(&cScene, CONVERTED));

	pcMesh->macFaces.Get(0)->aiPositions[1] = 2;
	AssertFalse(VerifyOldDragonSceneFile(&cScene, CONVERTED));
	pcMesh->macFaces.Get(0)->aiPositions[1] = 1;

	cScene.macMaterials.Get(0)->sDiffuse.g = 0.5f;
	AssertFalse(VerifyOldDragonSceneFile(&cScene, CONVERTED));
	cScene.macMaterials.Get(0)->sDiffuse.g = 0.0f;

	cScene.macConnections.Get(0)->sWorld.af[5] = -1.0f;
	AssertFalse(VerifyOldDragonSceneFile(&cScene, CONVERTED));
	SetTestMatrix(&cScene.macConnections.Get(0)->sWorld, 1.0f);

	cScene.macInstances.Get(0)->miObject = 1;
	AssertFalse(VerifyOldDragonSceneFile(&cScene, CONVERTED));
	cScene.macInstances.Get(0)->miObject = 0;

	AddTestMaterial(&cScene, "Another");
	AssertFalse(VerifyOldDragonSceneFile(&cScene, CONVERTED));

	AssertFalse(VerifyOldDragonSceneFile(&cScene, CONVERTER_OUTPUT_DIR _FS_ "NotThere.DRG"));

	gcLogger.SetConfig(&sLogConfig);
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConvertOldFiles(void)
{
	COldDragonScene	cScene;

	cScene.Init();
	AssertTrue(ReadOldDragonFile("Input" _FS_ "Laser.DRG", &cScene));
	AssertConverts(&cScene);
	cScene.Kill();

	cScene.Init();
	AssertTrue(ReadOldDragonFile("Input" _FS_ "Plane.DRG", &cScene));
	AssertConverts(&cScene);
	cScene.Kill();

	cScene.Init();
	AssertTrue(ReadOldDragonFile("Input" _FS_ "Box.DRG", &cScene));
	AssertConverts(&cScene);
	cScene.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestOldDragonConverter(void)
{
	CFileUtil	cFileUtil;

	BeginTests();

	cFileUtil.MakeDir("Output");
	cFileUtil.RemoveDir(CONVERTER_OUTPUT_DIR);
	cFileUtil.MakeDir(CONVERTER_OUTPUT_DIR);

	TestOldDragonConvertFaces();
	TestOldDragonConvertImages();
	TestOldDragonConvertKeys();
	TestOldDragonConvertNumbering();
	TestOldDragonConvertFromInstances();
	TestOldDragonConvertFromIntoOblivionNodes();
	TestOldDragonVerifyNoticesDifferences();
	TestOldDragonConvertOldFiles();

	cFileUtil.RemoveDir(CONVERTER_OUTPUT_DIR);

	TestStatistics();
}

