#include <math.h>
#include <stdlib.h>
#include "BaseLib/Float4x4.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/LogToMemory.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Mallocator.h"
#include "SupportLib/AnimationPlayer.h"
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
#define TEST_GOOBER_FILE	".." _FS_ ".." _FS_ "TestSupportLib" _FS_ "TestSupportLib" _FS_ "Input" _FS_ "Scene" _FS_ "Goober.DRG"
#define TEST_GOOBER_SCALE	(1 << 16)
#define TEST_GOOBER_PARTS	4	//How many parts in 1 << RASTER_AXIS_BITS of its distance from the origin an integer pose may be from a float one.


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
	pInstanceMesh->mapConnections.Add(pConnection);
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
// renderer's scale.  A mirror is wound the other way.  A skin with nothing
// to pose it by isn't drawn.
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
// A triangle whose first corner is all the first bone's, whose second is
// all the second's and whose third is half each, bound where the bones
// are when they're at the origin, in an instance of the bones' skeleton.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CInstance> TestFrameAddSkinned(Ptr<CScene> pScene, Ptr<CSkeleton> pSkeleton)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CInstance>		pInstance;
	CMesh*				pcMesh;
	SSkinWeight*		psWeight;
	SFloat4x4			sIdentity;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->mszName.Append("Skinned");
	pcMesh = &pEditor->mpMesh;
	pcMesh->mcSkin.mbInUse = true;
	pEditor->AddCorner(0, 0, 0);
	pEditor->AddCorner(1, 0, 0);
	pEditor->AddCorner(0, 1, 0);
	pEditor->AddFace(0, 1, 2);
	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(0)->mcWeights.Add();
	psWeight->iMatrix = 0;
	psWeight->fWeight = 1.0f;
	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(1)->mcWeights.Add();
	psWeight->iMatrix = 1;
	psWeight->fWeight = 1.0f;
	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(2)->mcWeights.Add();
	psWeight->iMatrix = 0;
	psWeight->fWeight = 0.5f;
	psWeight = pcMesh->mcSkin.mcSkinVerts.Get(2)->mcWeights.Add();
	psWeight->iMatrix = 1;
	psWeight->fWeight = 0.5f;
	sIdentity.Identity();
	pcMesh->mcSkin.mcInverseSkinMatricies.Add(&sIdentity);
	pcMesh->mcSkin.mcInverseSkinMatricies.Add(&sIdentity);
	pcMesh->mcSkin.mapBones.Add(pSkeleton->GetBone(0));
	pcMesh->mcSkin.mapBones.Add(pSkeleton->GetBone(1));

	pInstance = OMalloc<CInstance>();
	pInstance->mszName.Append("Skinned");
	pInstance->MakeBones(pSkeleton, NULL);
	pInstance->AddMesh(pEditor);
	pScene->mapInstances.Add(pInstance);
	return pInstance;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times szText is in szLog.
//
//////////////////////////////////////////////////////////////////////////
static int TestFrameCount(const char* szLog, const char* szText)
{
	const char*	sz;
	int			iCount;

	iCount = 0;
	for (sz = strstr(szLog, szText); sz != NULL; sz = strstr(sz + 1, szText))
	{
		iCount++;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A skinned mesh is posed by its instance's connections for its bones, at
// the renderer's scale, and drawn with no axes of its own.  A bone that
// mirrors is an error logged once, the mesh isn't drawn while it does, and
// it's drawn again when it doesn't.  A mesh with a bone its instance hasn't
// got isn't drawn.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterFrameSkinned(void)
{
	CRasterRenderFactory	cFactory;
	CRasteriser				cRasteriser;
	CLogToMemory			cLog;
	Ptr<CScene>				pScene;
	Ptr<CSkeleton>			pSkeleton;
	Ptr<CSkeleton>			pOther;
	Ptr<CInstance>			pInstance;
	Ptr<CConnection>		pRoot;
	Ptr<CConnection>		pTip;
	Ptr<CRasterFrame>		pFrame;
	SRasterInstance*		psInstance;
	SRasterVertex*			psVertex;
	SFloat4x4				sMatrix;
	char					szLog[4096];
	int32					iOne;

	iOne = 1 << RASTER_AXIS_BITS;
	cFactory.Init(TEST_FRAME_SCALE);
	cRasteriser.Init();
	pScene = OMalloc<CScene>();
	pSkeleton = OMalloc<CSkeleton>();
	sMatrix.Identity();
	pSkeleton->AddBone("Root", NULL, &sMatrix);
	pSkeleton->AddBone("Tip", NULL, &sMatrix);
	pInstance = TestFrameAddSkinned(pScene, pSkeleton);
	pRoot = pInstance->mapConnections.Get(0);
	pTip = pInstance->mapConnections.Get(1);

	//The root moves 2 along x, and the tip turns a quarter about z, taking x to y.
	pRoot->msLocalMatrix.pos.Init(2.0f, 0.0f, 0.0f, 1.0f);
	Float4x4RotationZ(&pTip->msLocalMatrix, 3.14159265f / 2.0f);
	pScene->UpdateConnections();

	pFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), Ptr<CCamera>());
	pFrame->Extract(&cRasteriser, 64);
	AssertSize(1, pFrame->NumInstances());
	psInstance = &pFrame->GetInstances()[0];
	AssertNotNull(psInstance->pcPosed);
	AssertTestFrameAxis(iOne, 0, 0, &psInstance->asAxes[0]);
	AssertInt(0, psInstance->sOffset.x);
	AssertFalse(psInstance->bMirrored);
	psVertex = psInstance->pcPosed->masVertices.Get(0);
	AssertInt(2000, psVertex->sPosition.x);
	AssertInt(0, psVertex->sPosition.y);
	psVertex = psInstance->pcPosed->masVertices.Get(1);
	AssertTrue(abs(psVertex->sPosition.x) <= 1);
	AssertTrue(abs(psVertex->sPosition.y - 1000) <= 1);
	psVertex = psInstance->pcPosed->masVertices.Get(2);
	AssertTrue(abs(psVertex->sPosition.x - 500) <= 1);
	AssertTrue(abs(psVertex->sPosition.y - 500) <= 1);
	AssertInt(2000, psInstance->pcPosed->msMax.x);

	//The tip mirrors for two frames: it's logged the first time, and the mesh comes back after.
	cLog.Start(true);
	pTip->msLocalMatrix.Identity();
	pTip->msLocalMatrix.x.x = -1.0f;
	pScene->UpdateConnections();
	pFrame->Extract(&cRasteriser, 64);
	AssertSize(0, pFrame->NumInstances());
	pFrame->Extract(&cRasteriser, 64);
	AssertSize(0, pFrame->NumInstances());
	cLog.Stop(szLog, 4096);
	AssertInt(1, TestFrameCount(szLog, "which mirrors it"));
	AssertTrue(strstr(szLog, "[Tip]") != NULL);
	pTip->msLocalMatrix.Identity();
	pScene->UpdateConnections();
	pFrame->Extract(&cRasteriser, 64);
	AssertSize(1, pFrame->NumInstances());

	//Bound to an instance of another skeleton, it has no connections for its bones, which binding logs once.
	pOther = OMalloc<CSkeleton>();
	pOther->AddBone("Elsewhere", NULL, &sMatrix);
	cLog.Start(true);
	pInstance = OMalloc<CInstance>();
	pInstance->MakeBones(pOther, NULL);
	pInstance->AddMesh(pScene->mapInstances.Get(0)->mapMeshes.Get(0)->mpEditor);
	pScene->mapInstances.Add(pInstance);
	pScene->UpdateConnections();
	pFrame->Extract(&cRasteriser, 64);
	pFrame->Extract(&cRasteriser, 64);
	cLog.Stop(szLog, 4096);
	AssertSize(1, pFrame->NumInstances());
	AssertInt(1, TestFrameCount(szLog, "isn't drawn"));
	AssertTrue(strstr(szLog, "nor for 1 others") != NULL);

	pFrame = NULL;
	pScene = NULL;
	pSkeleton = NULL;
	pOther = NULL;
	pInstance = NULL;
	pRoot = NULL;
	pTip = NULL;
	cRasteriser.Kill();
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The furthest any of Goober's posed vertices is from where floats put it:
// each raster vertex where it was bound, moved by the same weights and by
// each matrix, its inverse matrix times its connection's world matrix.  Its
// normals must be units.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestFrameGooberWorst(CRasterMesh* pcRaster, CRasterPosed* pcPosed, CInstanceMesh* pcInstanceMesh)
{
	CMeshSkin*			pcSkin;
	SRasterVertex*		psBound;
	SRasterVertex*		psPosed;
	SRasterSkinVertex*	psSkin;
	SRasterSkinWeight*	psWeight;
	SFloat4x4			sMatrix;
	SFloat32Vec3		sPosition;
	SFloat32Vec3		sMoved;
	float64				afSum[3];
	float64				fWorst;
	float64				fDifference;
	int64				iLength;
	size				i;
	uint32				j;
	int					k;

	pcSkin = &pcInstanceMesh->mpEditor->mpMesh->mcSkin;
	fWorst = 0.0;
	for (i = 0; i < pcRaster->masVertices.NumElements(); i++)
	{
		psBound = pcRaster->masVertices.Get(i);
		psPosed = pcPosed->masVertices.Get(i);
		psSkin = pcRaster->masSkinVertices.Get(i);
		sPosition.Init((float32)psBound->sPosition.x / TEST_GOOBER_SCALE, (float32)psBound->sPosition.y / TEST_GOOBER_SCALE, (float32)psBound->sPosition.z / TEST_GOOBER_SCALE);
		afSum[0] = 0.0;
		afSum[1] = 0.0;
		afSum[2] = 0.0;
		for (j = 0; j < psSkin->uiCount; j++)
		{
			psWeight = pcRaster->masSkinWeights.Get(psSkin->uiFirst + j);
			Float4x4Multiply(&sMatrix, pcSkin->mcInverseSkinMatricies.Get(psWeight->uiMatrix), &pcInstanceMesh->mapConnections.Get(psWeight->uiMatrix)->msWorldMatrix);
			Float32Vec3TransformCoord(&sMoved, &sPosition, &sMatrix);
			afSum[0] += sMoved.x * psWeight->uiWeight / RASTER_SKIN_ONE;
			afSum[1] += sMoved.y * psWeight->uiWeight / RASTER_SKIN_ONE;
			afSum[2] += sMoved.z * psWeight->uiWeight / RASTER_SKIN_ONE;
		}
		for (k = 0; k < 3; k++)
		{
			fDifference = fabs(afSum[k] * TEST_GOOBER_SCALE - (&psPosed->sPosition.x)[k]);
			fWorst = (fDifference > fWorst) ? fDifference : fWorst;
		}
		iLength = (int64)psPosed->sNormal.x.miRaw * psPosed->sNormal.x.miRaw + (int64)psPosed->sNormal.y.miRaw * psPosed->sNormal.y.miRaw + (int64)psPosed->sNormal.z.miRaw * psPosed->sNormal.z.miRaw;
		AssertTrue(llabs(iLength - ((int64)1 << (SRasterUnit::Bits * 2))) < ((int64)1 << (SRasterUnit::Bits * 2)) / 200);
	}
	return fWorst;
}


//////////////////////////////////////////////////////////////////////////
//
// Goober, read from its converted file, walks: at each of several times
// the frame poses its one skinned mesh to within rounding of where floats
// put each vertex.  An axis holds RASTER_AXIS_BITS of fraction, so that's
// a few parts in 1 << RASTER_AXIS_BITS of how far the vertices are from the
// origin: Goober is about 6 million units across here, and is out by up to
// 47.  Its images are bigger than the debug allocator's limit.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterFrameGoober(void)
{
	CRasterRenderFactory	cFactory;
	CRasteriser				cRasteriser;
	CFileUtil				cFileUtil;
	Ptr<CScene>				pScene;
	Ptr<CInstance>			pInstance;
	Ptr<CRasterFrame>		pFrame;
	Ptr<CAnimationPlayer>	pPlayer;
	SRasterInstance*		psInstance;
	float64					fWorst;
	float64					fLargest;
	float64					fDuration;
	size					i;
	int						k;
	lsize					uiMaxDebugAllocatorSize;
	int						iTime;

	BeginTests();
	if (!cFileUtil.Exists(TEST_GOOBER_FILE))
	{
		TestStatistics();
		return;
	}

	uiMaxDebugAllocatorSize = guiMaxDebugAllocatorSize;
	InitAllocatorDebug(32 MB);
	DataIOInit();
	NumberInit();
	ObjectsInit();

	cFactory.Init(TEST_GOOBER_SCALE);
	cRasteriser.Init();
	pScene = OMalloc<CScene>();
	AssertTrue(pScene->ReadFile(TEST_GOOBER_FILE));
	pInstance = pScene->mapInstances.Get(0);
	pPlayer = OMalloc<CAnimationPlayer>(pScene->mapAnimations.Get(0), pInstance, true);
	fDuration = pScene->mapAnimations.Get(0)->GetDuration();
	pFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), Ptr<CCamera>());

	fWorst = 0.0;
	for (iTime = 0; iTime < 8; iTime++)
	{
		pPlayer->SetTime((float32)(fDuration * iTime / 8.0));
		pPlayer->Apply();
		pScene->UpdateConnections();
		pFrame->Extract(&cRasteriser, 256);
		AssertSize(1, pFrame->NumInstances());
		psInstance = &pFrame->GetInstances()[0];
		AssertNotNull(psInstance->pcPosed);
		AssertSize(psInstance->pcMesh->masVertices.NumElements(), psInstance->pcPosed->masVertices.NumElements());
		fWorst = fmax(fWorst, TestFrameGooberWorst(psInstance->pcMesh, psInstance->pcPosed, &pInstance->mapMeshes.Get(0)));
	}
	fLargest = 0.0;
	for (i = 0; i < psInstance->pcMesh->masVertices.NumElements(); i++)
	{
		for (k = 0; k < 3; k++)
		{
			fLargest = fmax(fLargest, fabs((float64)(&psInstance->pcMesh->masVertices.Get(i)->sPosition.x)[k]));
		}
	}
	AssertTrue(fWorst > 0.0);
	AssertTrue(fWorst < fLargest * TEST_GOOBER_PARTS / (1 << RASTER_AXIS_BITS) + 2.0);

	pFrame = NULL;
	pPlayer = NULL;
	pInstance = NULL;
	pScene = NULL;
	cRasteriser.Kill();
	cFactory.Kill();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();
	InitAllocatorDebug(uiMaxDebugAllocatorSize);

	TestStatistics();
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
	TestRasterFrameSkinned();
	TestRasterFrameLights();
	TestRasterFrameCamera();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

