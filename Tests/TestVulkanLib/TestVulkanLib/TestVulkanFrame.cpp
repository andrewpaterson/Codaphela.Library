#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "BaseLib/Float4x4.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Camera.h"
#include "SupportLib/Instance.h"
#include "SupportLib/Light.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/Scene.h"
#include "RenderLib/RenderScene.h"
#include "RasterLib/RasterTarget.h"
#include "RasterRenderLib/RasterFrame.h"
#include "RasterRenderLib/RasterRenderFactory.h"
#include "VulkanLib/VulkanFrame.h"
#include "VulkanLib/VulkanRenderFactory.h"
#include "TestLib/Assert.h"


#define TEST_VULKAN_WIDTH		200
#define TEST_VULKAN_HEIGHT		150
#define TEST_VULKAN_BACKGROUND	0xFF102030
#define TEST_VULKAN_SCALE		1000


//How alike the two renderers' frames are.
struct STestVulkanLikeness
{
	int		iCovered;		//Pixels either drew on.
	int		iDisagree;		//Of those, pixels only one drew on.
	float64	fMeanDifference;	//The mean of the largest difference of a channel, where both drew.
};


//////////////////////////////////////////////////////////////////////////
//
// A quad wound anticlockwise from outside the box whose middle is
// psMiddle, whatever order its corners are given in around it.
//
//////////////////////////////////////////////////////////////////////////
static void AddTestVulkanQuad(CMeshEditor* pcEditor, SFloat32Vec3* asCorners, int a, int b, int c, int d, SFloat32Vec3* psMiddle)
{
	SFloat32Vec3	sAB;
	SFloat32Vec3	sAC;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sOut;
	int				iSwap;

	sAB.Init(asCorners[b].x - asCorners[a].x, asCorners[b].y - asCorners[a].y, asCorners[b].z - asCorners[a].z);
	sAC.Init(asCorners[c].x - asCorners[a].x, asCorners[c].y - asCorners[a].y, asCorners[c].z - asCorners[a].z);
	sNormal.Init(sAB.y * sAC.z - sAB.z * sAC.y, sAB.z * sAC.x - sAB.x * sAC.z, sAB.x * sAC.y - sAB.y * sAC.x);
	sOut.Init((asCorners[a].x + asCorners[c].x) * 0.5f - psMiddle->x, (asCorners[a].y + asCorners[c].y) * 0.5f - psMiddle->y, (asCorners[a].z + asCorners[c].z) * 0.5f - psMiddle->z);
	if (sNormal.x * sOut.x + sNormal.y * sOut.y + sNormal.z * sOut.z < 0.0f)
	{
		iSwap = b;
		b = d;
		d = iSwap;
	}
	pcEditor->AddFace(a, b, c);
	pcEditor->AddFace(a, c, d);
}


//////////////////////////////////////////////////////////////////////////
//
// A box of one material, with a flat normal on each face.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMeshEditor> TestVulkanBox(float32 fSize, float32 fAlpha, float32 fRed, float32 fGreen, float32 fBlue)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CMaterial>		pMaterial;
	CMesh*				pcMesh;
	SFloat32Vec3		asCorners[8];
	SFloat32Vec3		sMiddle;
	int					i;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pcMesh = &pEditor->mpMesh;
	pcMesh->mcNormals.mbInUse = true;
	pcMesh->mcMaterials.mbInUse = true;
	pMaterial = OMalloc<CMaterial>();
	pMaterial->msProperties.mcDiffuseColour.Init(fAlpha, fRed, fGreen, fBlue);
	pcMesh->mcMaterials.mapMaterials.Add(pMaterial);

	for (i = 0; i < 8; i++)
	{
		asCorners[i].Init((i & 1) ? fSize * 0.5f : -fSize * 0.5f, (i & 2) ? fSize * 0.5f : -fSize * 0.5f, (i & 4) ? fSize * 0.5f : -fSize * 0.5f);
		pEditor->AddCorner(asCorners[i].x, asCorners[i].y, asCorners[i].z);
	}
	sMiddle.Init(0.0f, 0.0f, 0.0f);
	AddTestVulkanQuad(&pEditor, asCorners, 0, 1, 3, 2, &sMiddle);
	AddTestVulkanQuad(&pEditor, asCorners, 4, 5, 7, 6, &sMiddle);
	AddTestVulkanQuad(&pEditor, asCorners, 0, 1, 5, 4, &sMiddle);
	AddTestVulkanQuad(&pEditor, asCorners, 2, 3, 7, 6, &sMiddle);
	AddTestVulkanQuad(&pEditor, asCorners, 0, 2, 6, 4, &sMiddle);
	AddTestVulkanQuad(&pEditor, asCorners, 1, 3, 7, 5, &sMiddle);
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		pcMesh->mcMaterials.mcFaces.SetValue((size)i, 0);
	}
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AddTestVulkanInstance(Ptr<CScene> pScene, Ptr<CMeshEditor> pEditor, SFloat4x4* psWorld)
{
	Ptr<CInstance>		pInstance;
	Ptr<CConnection>	pConnection;

	pConnection = OMalloc<CConnection>();
	pConnection->msLocalMatrix = *psWorld;
	pConnection->msWorldMatrix = *psWorld;
	pInstance = OMalloc<CInstance>();
	pInstance->mapConnections.Add(pConnection);
	pInstance->AddMesh(pEditor)->mapConnections.Add(pConnection);
	pScene->mapInstances.Add(pInstance);
}


//////////////////////////////////////////////////////////////////////////
//
// A red box turned about y, a green one turned about x and y, a half
// transparent blue one in front, and a camera a little above them looking
// along -z, the scene's first.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CScene> TestVulkanScene(void)
{
	Ptr<CScene>		pScene;
	Ptr<CCamera>	pCamera;
	SFloat4x4		sTurn;
	SFloat4x4		sTilt;
	SFloat4x4		sWorld;

	pScene = OMalloc<CScene>();
	Float4x4RotationY(&sWorld, 0.5f);
	AddTestVulkanInstance(pScene, TestVulkanBox(2.0f, 1.0f, 0.9f, 0.2f, 0.2f), &sWorld);

	Float4x4RotationY(&sTurn, 0.8f);
	Float4x4RotationX(&sTilt, 0.6f);
	Float4x4Multiply(&sWorld, &sTurn, &sTilt);
	sWorld.pos.Init(2.5f, 0.3f, -1.0f, 1.0f);
	AddTestVulkanInstance(pScene, TestVulkanBox(1.2f, 1.0f, 0.2f, 0.8f, 0.3f), &sWorld);

	Float4x4RotationY(&sWorld, -0.3f);
	sWorld.pos.Init(-1.2f, -0.2f, 1.5f, 1.0f);
	AddTestVulkanInstance(pScene, TestVulkanBox(1.0f, 0.5f, 0.2f, 0.3f, 1.0f), &sWorld);

	pCamera = OMalloc<CCamera>();
	pCamera->mfFOV = 0.9f;
	pCamera->mfNear = 0.1f;
	pCamera->mfFar = 100.0f;
	pCamera->mpConnection = OMalloc<CConnection>();
	Float4x4RotationX(&sWorld, -0.15f);
	sWorld.pos.Init(0.0f, 1.2f, 7.0f, 1.0f);
	pCamera->mpConnection->msLocalMatrix = sWorld;
	pCamera->mpConnection->msWorldMatrix = sWorld;
	pScene->mapCameras.Add(pCamera);
	return pScene;
}


//////////////////////////////////////////////////////////////////////////
//
// Ambient, a light far away above and to the right, and a point light in
// front that falls off.
//
//////////////////////////////////////////////////////////////////////////
static void AddTestVulkanLights(Ptr<CScene> pScene)
{
	Ptr<CLight>	pLight;
	SFloat4x4	sTurn;
	SFloat4x4	sTilt;

	pLight = OMalloc<CLight>();
	pLight->meType = LT_Ambient;
	pLight->mfIntensity = 0.2f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pScene->mapLights.Add(pLight);

	pLight = OMalloc<CLight>();
	pLight->meType = LT_Directional;
	pLight->mfIntensity = 0.6f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pLight->mpConnection = OMalloc<CConnection>();
	Float4x4RotationY(&sTurn, 0.7f);
	Float4x4RotationX(&sTilt, -0.8f);
	Float4x4Multiply(&pLight->mpConnection->msWorldMatrix, &sTilt, &sTurn);
	pScene->mapLights.Add(pLight);

	pLight = OMalloc<CLight>();
	pLight->meType = LT_Omni;
	pLight->mfIntensity = 0.8f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pLight->meDecay = LDT_Linear;
	pLight->mfDecayStart = 2.0f;
	pLight->mfRange = 12.0f;
	pLight->mpConnection = OMalloc<CConnection>();
	Float4x4Translation(&pLight->mpConnection->msWorldMatrix, -2.0f, 2.0f, 3.0f);
	pScene->mapLights.Add(pLight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int TestVulkanChannelDifference(uint32 uiA, uint32 uiB)
{
	int	iWorst;
	int	iDifference;
	int	iShift;

	iWorst = 0;
	for (iShift = 0; iShift < 24; iShift += 8)
	{
		iDifference = abs((int)((uiA >> iShift) & 0xFF) - (int)((uiB >> iShift) & 0xFF));
		iWorst = (iDifference > iWorst) ? iDifference : iWorst;
	}
	return iWorst;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestVulkanCompare(uint32* puiRaster, uint32* puiVulkan, STestVulkanLikeness* psLikeness)
{
	int		i;
	int		iBoth;
	bool	bRaster;
	bool	bVulkan;
	float64	fTotal;

	psLikeness->iCovered = 0;
	psLikeness->iDisagree = 0;
	iBoth = 0;
	fTotal = 0.0;
	for (i = 0; i < TEST_VULKAN_WIDTH * TEST_VULKAN_HEIGHT; i++)
	{
		bRaster = (puiRaster[i] & 0xFFFFFF) != (TEST_VULKAN_BACKGROUND & 0xFFFFFF);
		bVulkan = (puiVulkan[i] & 0xFFFFFF) != (TEST_VULKAN_BACKGROUND & 0xFFFFFF);
		if (bRaster || bVulkan)
		{
			psLikeness->iCovered++;
		}
		if (bRaster != bVulkan)
		{
			psLikeness->iDisagree++;
		}
		else if (bRaster)
		{
			iBoth++;
			fTotal += TestVulkanChannelDifference(puiRaster[i], puiVulkan[i]);
		}
	}
	psLikeness->fMeanDifference = (iBoth > 0) ? fTotal / iBoth : 0.0;
}


//////////////////////////////////////////////////////////////////////////
//
// The same scene drawn by the raster renderer and by Vulkan.  They round
// differently at the edges of triangles and the rasteriser shades in
// integers, so they're alike, not the same: nearly every pixel drawn by one
// is drawn by the other, in nearly the same colour.
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestVulkanLikeRaster(Ptr<CScene> pScene, Ptr<CCamera> pCamera, CVulkanRenderFactory* pcVulkanFactory)
{
	CRasterRenderFactory	cRasterFactory;
	CRasteriser				cRasteriser;
	CRasterTarget			cTarget;
	Ptr<CRasterFrame>		pRasterFrame;
	Ptr<CVulkanFrame>		pVulkanFrame;
	STestVulkanLikeness		sLikeness;
	uint32*					puiRaster;
	uint32*					puiVulkan;

	puiRaster = (uint32*)malloc(TEST_VULKAN_WIDTH * TEST_VULKAN_HEIGHT * sizeof(uint32));
	puiVulkan = (uint32*)malloc(TEST_VULKAN_WIDTH * TEST_VULKAN_HEIGHT * sizeof(uint32));

	cRasterFactory.Init(TEST_VULKAN_SCALE);
	cRasteriser.Init();
	cTarget.Init(puiRaster, TEST_VULKAN_WIDTH, TEST_VULKAN_HEIGHT, TEST_VULKAN_WIDTH);
	cTarget.Clear(TEST_VULKAN_BACKGROUND);
	pRasterFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cRasterFactory, cRasterFactory.CreateRenderer()), pCamera);
	pRasterFrame->Draw(&cRasteriser, &cTarget);

	pVulkanFrame = OMalloc<CVulkanFrame>(OMalloc<CRenderScene>(pScene, pcVulkanFactory, pcVulkanFactory->CreateRenderer()), pCamera);
	AssertTrue(pVulkanFrame->Draw(puiVulkan, TEST_VULKAN_WIDTH, TEST_VULKAN_HEIGHT, TEST_VULKAN_BACKGROUND));
	AssertSize(3, pVulkanFrame->NumInstances());

	TestVulkanCompare(puiRaster, puiVulkan, &sLikeness);
	AssertTrue(sLikeness.iCovered > TEST_VULKAN_WIDTH * TEST_VULKAN_HEIGHT / 10);
	AssertTrue(sLikeness.iDisagree * 50 < sLikeness.iCovered);
	AssertTrue(sLikeness.fMeanDifference < 4.0);

	pVulkanFrame = NULL;
	pRasterFrame = NULL;
	cTarget.Kill();
	cRasteriser.Kill();
	cRasterFactory.Kill();
	free(puiVulkan);
	free(puiRaster);
}


//////////////////////////////////////////////////////////////////////////
//
// Lit by the scene's lights, and by the light the rasteriser has always
// had when it has none.
//
//////////////////////////////////////////////////////////////////////////
void TestVulkanFrameLikeRaster(CVulkanRenderFactory* pcFactory)
{
	Ptr<CScene>		pScene;
	Ptr<CCamera>	pCamera;

	pScene = TestVulkanScene();
	pCamera = pScene->mapCameras.Get(0);
	AssertTestVulkanLikeRaster(pScene, pCamera, pcFactory);
	AddTestVulkanLights(pScene);
	AssertTestVulkanLikeRaster(pScene, pCamera, pcFactory);
}


//////////////////////////////////////////////////////////////////////////
//
// With no camera there's nothing to draw.
//
//////////////////////////////////////////////////////////////////////////
void TestVulkanFrameNoCamera(CVulkanRenderFactory* pcFactory)
{
	Ptr<CScene>			pScene;
	Ptr<CCamera>		pCamera;
	Ptr<CVulkanFrame>	pFrame;
	uint32				auiPixels[16];

	pScene = TestVulkanScene();
	pCamera = pScene->mapCameras.Get(0);
	pFrame = OMalloc<CVulkanFrame>(OMalloc<CRenderScene>(pScene, pcFactory, pcFactory->CreateRenderer()), Ptr<CCamera>());
	AssertFalse(pFrame->Draw(auiPixels, 4, 4, TEST_VULKAN_BACKGROUND));
}


//////////////////////////////////////////////////////////////////////////
//
// What's here needs a device that has Vulkan 1.3.  With none it's said,
// and there's nothing to test.
//
//////////////////////////////////////////////////////////////////////////
void TestVulkanFrame(void)
{
	CVulkanRenderFactory	cFactory;
	Ptr<CVulkanRenderer>	pRenderer;

	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	cFactory.Init();
	pRenderer = cFactory.CreateRenderer();
	if (!pRenderer->IsReady())
	{
		gcLogger.Info("TestVulkanFrame: there's no device with Vulkan 1.3, so nothing is tested.");
	}
	else
	{
		AssertTrue(strlen(pRenderer->GetDeviceName()) > 0);
		pRenderer = NULL;
		TestVulkanFrameLikeRaster(&cFactory);
		ObjectsFlush();
		TestVulkanFrameNoCamera(&cFactory);
	}
	pRenderer = NULL;
	cFactory.Kill();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

