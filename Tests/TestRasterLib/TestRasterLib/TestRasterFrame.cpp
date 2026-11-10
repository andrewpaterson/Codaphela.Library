#include <math.h>
#include <stdlib.h>
#include "BaseLib/Float4x4.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Camera.h"
#include "SupportLib/Instance.h"
#include "SupportLib/Light.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/Scene.h"
#include "RenderLib/RenderScene.h"
#include "RasterRenderLib/RasterFrame.h"
#include "RasterRenderLib/RasterRenderFactory.h"
#include "TestLib/Assert.h"


#define TEST_FRAME_SCALE	1000


//////////////////////////////////////////////////////////////////////////
//
// A triangle with corners at 0 and 1, in an instance of its own placed by
// psWorld.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMeshEditor> TestFrameAddInstance(Ptr<CScene> pScene, SFloat4x4* psWorld)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CInstance>		pInstance;
	Ptr<CInstanceMesh>	pInstanceMesh;
	Ptr<CConnection>	pConnection;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddCorner(0, 0, 0);
	pEditor->AddCorner(1, 0, 0);
	pEditor->AddCorner(0, 1, 0);
	pEditor->AddFace(0, 1, 2);

	pConnection = OMalloc<CConnection>();
	pConnection->msWorldMatrix = *psWorld;
	pConnection->msLocalMatrix = *psWorld;
	pInstanceMesh = OMalloc<CInstanceMesh>(pEditor);
	pInstanceMesh->maiConnectionSlots.Add(0);
	pInstance = OMalloc<CInstance>();
	pInstance->mapConnections.Add(pConnection);
	pInstance->mapMeshes.Add(pInstanceMesh);
	pScene->mapInstances.Add(pInstance);
	return pEditor;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestFrameAxis(int32 x, int32 y, int32 z, SInt32Vec3* psAxis)
{
	AssertInt(x, psAxis->x);
	AssertInt(y, psAxis->y);
	AssertInt(z, psAxis->z);
}


//////////////////////////////////////////////////////////////////////////
//
// Each rigid mesh is placed by its connection: its axes are the world
// matrix's columns, its light axes their rotation, and it's moved at the
// renderer's scale.  A mirror is wound the other way, and a skinned mesh
// isn't drawn yet.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterFrameInstances(void)
{
	CRasterRenderFactory	cFactory;
	CRasteriser				cRasteriser;
	Ptr<CScene>				pScene;
	Ptr<CRasterFrame>		pFrame;
	Ptr<CMeshEditor>		pSkinned;
	SRasterInstance*		psInstance;
	SFloat4x4				sWorld;
	int32					iOne;

	iOne = 1 << RASTER_AXIS_BITS;
	cFactory.Init(TEST_FRAME_SCALE);
	cRasteriser.Init();
	pScene = OMalloc<CScene>();

	Float4x4RotationZ(&sWorld, 3.14159265f / 2.0f);
	sWorld.pos.Init(2.0f, 0.0f, 0.0f, 1.0f);
	TestFrameAddInstance(pScene, &sWorld);

	sWorld.Identity();
	sWorld.x.x = -1.0f;
	TestFrameAddInstance(pScene, &sWorld);

	sWorld.Identity();
	sWorld.x.x = 2.0f;
	sWorld.y.y = 2.0f;
	sWorld.z.z = 2.0f;
	TestFrameAddInstance(pScene, &sWorld);

	sWorld.Identity();
	pSkinned = TestFrameAddInstance(pScene, &sWorld);
	pSkinned->mpMesh->mcSkin.mbInUse = true;

	pFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), Ptr<CCamera>());
	pFrame->Extract(&cRasteriser, 64);
	AssertSize(3, pFrame->NumInstances());

	//A quarter turn around z: the new x is the old -y.
	psInstance = &pFrame->GetInstances()[0];
	AssertTestFrameAxis(0, -iOne, 0, &psInstance->asAxes[0]);
	AssertTestFrameAxis(iOne, 0, 0, &psInstance->asAxes[1]);
	AssertTestFrameAxis(0, 0, iOne, &psInstance->asAxes[2]);
	AssertTestFrameAxis(0, -iOne, 0, &psInstance->asLightAxes[0]);
	AssertInt(2000, psInstance->sOffset.x);
	AssertFalse(psInstance->bMirrored);
	AssertInt(TEST_FRAME_SCALE, psInstance->pcMesh->msMax.x);

	psInstance = &pFrame->GetInstances()[1];
	AssertTestFrameAxis(-iOne, 0, 0, &psInstance->asAxes[0]);
	AssertTestFrameAxis(-iOne, 0, 0, &psInstance->asLightAxes[0]);
	AssertTrue(psInstance->bMirrored);

	psInstance = &pFrame->GetInstances()[2];
	AssertTestFrameAxis(2 * iOne, 0, 0, &psInstance->asAxes[0]);
	AssertTestFrameAxis(iOne, 0, 0, &psInstance->asLightAxes[0]);
	AssertTestFrameAxis(0, 0, iOne, &psInstance->asLightAxes[2]);
	AssertFalse(psInstance->bMirrored);

	pFrame = NULL;
	pScene = NULL;
	pSkinned = NULL;
	cRasteriser.Kill();
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The scene's lights: ambient ones add up, a directional light is towards
// its +z, and an omni light is a point light at the renderer's scale.  With
// none the rasteriser keeps the light it starts with.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterFrameLights(void)
{
	CRasterRenderFactory	cFactory;
	CRasteriser				cRasteriser;
	CRasterLights*			pcLights;
	Ptr<CScene>				pScene;
	Ptr<CRasterFrame>		pFrame;
	Ptr<CLight>				pLight;
	SFloat4x4				sWorld;

	cFactory.Init(TEST_FRAME_SCALE);
	cRasteriser.Init();
	pcLights = cRasteriser.GetLights();
	pScene = OMalloc<CScene>();
	pFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), Ptr<CCamera>());

	pcLights->Clear();
	pFrame->Extract(&cRasteriser, 64);
	AssertInt(64, pcLights->miAmbient);
	AssertSize(1, pcLights->masDirectional.NumElements());
	AssertTrue(pcLights->masDirectional.Get(0)->bOnScreen);

	pLight = OMalloc<CLight>();
	pLight->meType = LT_Ambient;
	pLight->mfIntensity = 0.25f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pScene->mapLights.Add(pLight);
	pLight = OMalloc<CLight>();
	pLight->meType = LT_Ambient;
	pLight->mfIntensity = 0.25f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pScene->mapLights.Add(pLight);

	//Turned a quarter around x, its +z is the old -y.
	pLight = OMalloc<CLight>();
	pLight->meType = LT_Directional;
	pLight->mfIntensity = 0.5f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pLight->mpConnection = OMalloc<CConnection>();
	Float4x4RotationX(&pLight->mpConnection->msWorldMatrix, 3.14159265f / 2.0f);
	pScene->mapLights.Add(pLight);

	pLight = OMalloc<CLight>();
	pLight->meType = LT_Omni;
	pLight->mfIntensity = 1.0f;
	pLight->msColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pLight->meDecay = LDT_Linear;
	pLight->mfDecayStart = 2.0f;
	pLight->mfRange = 3.0f;
	pLight->mpConnection = OMalloc<CConnection>();
	Float4x4Translation(&sWorld, 1.0f, 2.0f, 3.0f);
	pLight->mpConnection->msWorldMatrix = sWorld;
	pScene->mapLights.Add(pLight);

	pFrame->Extract(&cRasteriser, 64);
	AssertInt(128, pcLights->miAmbient);
	AssertSize(1, pcLights->masDirectional.NumElements());
	AssertFalse(pcLights->masDirectional.Get(0)->bOnScreen);
	AssertInt(128, pcLights->masDirectional.Get(0)->iAmount);
	AssertInt(0, pcLights->masDirectional.Get(0)->sTowards.x.miRaw);
	AssertInt(-(1 << SRasterUnit::Bits), pcLights->masDirectional.Get(0)->sTowards.y.miRaw);
	AssertSize(1, pcLights->masPoint.NumElements());
	AssertInt(1000, pcLights->masPoint.Get(0)->sPosition.x);
	AssertInt(3000, pcLights->masPoint.Get(0)->sPosition.z);
	AssertInt(256, pcLights->masPoint.Get(0)->iAmount);
	AssertInt(RLD_Linear, pcLights->masPoint.Get(0)->eDecay);
	AssertInt(2000, pcLights->masPoint.Get(0)->iStart);
	AssertInt(3000, pcLights->masPoint.Get(0)->iReach);

	pFrame = NULL;
	pScene = NULL;
	cRasteriser.Kill();
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Where two cameras put a point on a 64 by 64 screen agree but for rounding.
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestFrameSameView(CRasterCamera* pcA, CRasterCamera* pcB)
{
	SRasterScreenVertex	sA;
	SRasterScreenVertex	sB;
	SInt32Vec3			sPoint;
	int					i;

	pcA->Prepare(64, 64);
	pcB->Prepare(64, 64);
	for (i = 0; i < 8; i++)
	{
		sPoint.Init((i & 1) ? 1500 : -500, (i & 2) ? 1200 : -800, (i & 4) ? 1000 : -2000);
		pcA->Project(&sA, &sPoint);
		pcB->Project(&sB, &sPoint);
		AssertTrue(abs(sA.sPosition.x.miRaw - sB.sPosition.x.miRaw) <= 2);
		AssertTrue(abs(sA.sPosition.y.miRaw - sB.sPosition.y.miRaw) <= 2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The camera is the scene camera's, looking at the middle of what's drawn,
// with the focal length its field of view makes across the target.  What the
// controls do is written back to the scene camera, so a frame made from it
// sees the same, and the camera isn't set again from what it wrote.  When the
// scene camera moves the camera follows it.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterFrameCamera(void)
{
	CRasterRenderFactory	cFactory;
	CRasteriser				cRasteriser;
	Ptr<CScene>				pScene;
	Ptr<CCamera>			pCamera;
	Ptr<CRasterFrame>		pFrame;
	Ptr<CRasterFrame>		pOther;
	CRasterCamera*			pcCamera;
	CRasterTarget			cTarget;
	SFloat4x4				sWorld;
	SFloat4x4				sWritten;
	uint32*					puiPixels;
	int						i;
	int						iDrawn;

	cFactory.Init(TEST_FRAME_SCALE);
	cRasteriser.Init();
	pScene = OMalloc<CScene>();
	sWorld.Identity();
	TestFrameAddInstance(pScene, &sWorld);

	pCamera = OMalloc<CCamera>();
	pCamera->mfFOV = 3.14159265f / 2.0f;
	pCamera->mpConnection = OMalloc<CConnection>();
	Float4x4Translation(&pCamera->mpConnection->msWorldMatrix, 0.5f, 0.5f, 10.0f);
	pCamera->mpConnection->msLocalMatrix = pCamera->mpConnection->msWorldMatrix;
	pScene->mapCameras.Add(pCamera);

	pFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), pCamera);
	pFrame->Extract(&cRasteriser, 64);
	pcCamera = pFrame->GetCamera();
	AssertInt(32, pcCamera->GetFocalLength());
	AssertInt(10000, pcCamera->GetDistance());
	AssertInt(500, pcCamera->GetPivot()->x);
	AssertInt(500, pcCamera->GetPivot()->y);
	AssertInt(0, pcCamera->GetPivot()->z);

	//The triangle is in front of it, half a unit square ten away across a quarter turn: 0.5 * (32 / 10)^2 pixels.
	puiPixels = (uint32*)malloc(64 * 64 * sizeof(uint32));
	cTarget.Init(puiPixels, 64, 64, 64);
	cTarget.Clear(0xFF000000);
	pFrame->Draw(&cRasteriser, &cTarget);
	iDrawn = 0;
	for (i = 0; i < 64 * 64; i++)
	{
		iDrawn += (puiPixels[i] != 0xFF000000) ? 1 : 0;
	}
	AssertTrue((iDrawn >= 2) && (iDrawn <= 12));
	cTarget.Kill();
	free(puiPixels);

	//Panned and turned by the controls, which the next frame writes back.
	pcCamera->Pan(5, -3);
	pcCamera->Drag(20, 30, 28, 25, 64, 64);
	pFrame->Extract(&cRasteriser, 64);
	sWritten = pCamera->mpConnection->msWorldMatrix;
	AssertFalse(Float4x4Equals(&sWritten, &sWorld));
	AssertTrue(pCamera->mpConnection->msLocalMatrix.pos.z == sWritten.pos.z);

	pOther = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), pCamera);
	pOther->Extract(&cRasteriser, 64);
	AssertTestFrameSameView(pFrame->GetCamera(), pOther->GetCamera());

	//Nothing moved, so nothing is written or set again.
	pFrame->Extract(&cRasteriser, 64);
	AssertTrue(Float4x4Equals(&sWritten, &pCamera->mpConnection->msWorldMatrix));
	AssertTestFrameSameView(pFrame->GetCamera(), pOther->GetCamera());

	//The scene camera moves, and the camera follows at the distance it was.
	Float4x4Translation(&pCamera->mpConnection->msWorldMatrix, 0.0f, 0.0f, 20.0f);
	pFrame->Extract(&cRasteriser, 64);
	AssertInt(10000, pcCamera->GetDistance());
	AssertInt(10000, pcCamera->GetPivot()->z);
	AssertInt(0, pcCamera->GetPan()->x);

	pOther = NULL;
	pFrame = NULL;
	pCamera = NULL;
	pScene = NULL;
	cRasteriser.Kill();
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterFrame(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRasterFrameInstances();
	TestRasterFrameLights();
	TestRasterFrameCamera();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

