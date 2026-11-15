#include <stdlib.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/Float4x4.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Camera.h"
#include "SupportLib/Instance.h"
#include "SupportLib/MeshEditor.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshMergeNormals.h"
#include "SupportLib/Scene.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/Torus.h"
#include "SupportLib/ShapeUnion.h"
#include "SupportLib/ShapeSmoothUnion.h"
#include "SupportLib/ShapeSubtraction.h"
#include "RenderLib/RenderScene.h"
#include "RasterRenderLib/RasterFrame.h"
#include "RasterRenderLib/RasterRenderFactory.h"
#include "ProceduralLib/MeshShapeEditor.h"
#include "TestLib/Assert.h"


#define TEST_SHAPE_SCENE_WIDTH		128
#define TEST_SHAPE_SCENE_HEIGHT		80
#define TEST_SHAPE_SCENE_SCALE		1000
#define TEST_SHAPE_SCENE_CHANNEL	8		//How far a channel may be from what was saved before its pixel counts as different.


//////////////////////////////////////////////////////////////////////////
//
// How many pixels differ from the saved frame by more than
// TEST_SHAPE_SCENE_CHANNEL in any channel, or -1 if it can't be read.
// Builds differ in floats' last bits (g++ contracts a * b + c into one
// FMA instruction when it optimises, and Visual Studio needn't), so a
// curve's corners and shading can move a little.
//
//////////////////////////////////////////////////////////////////////////
static int TestShapeSceneDifferences(const char* szExpected, uint32* puiPixels, int iNumPixels)
{
	CFileBasic	cFile;
	uint32*		puiExpected;
	int			i;
	int			iChannel;
	int			iDifferent;
	int			iExpected;
	int			iActual;
	lsize		uiRead;

	puiExpected = (uint32*)malloc(iNumPixels * sizeof(uint32));
	cFile.Init(DiskFile(szExpected));
	uiRead = 0;
	if (cFile.Open(EFM_Read))
	{
		uiRead = cFile.Read(puiExpected, sizeof(uint32), (lsize)iNumPixels);
		cFile.Close();
	}
	cFile.Kill();
	if (uiRead != (lsize)iNumPixels)
	{
		free(puiExpected);
		return -1;
	}

	iDifferent = 0;
	for (i = 0; i < iNumPixels; i++)
	{
		for (iChannel = 0; iChannel < 4; iChannel++)
		{
			iExpected = (int)((puiExpected[i] >> (iChannel * 8)) & 0xFF);
			iActual = (int)((puiPixels[i] >> (iChannel * 8)) & 0xFF);
			if (abs(iExpected - iActual) > TEST_SHAPE_SCENE_CHANNEL)
			{
				iDifferent++;
				break;
			}
		}
	}
	free(puiExpected);
	return iDifferent;
}


//////////////////////////////////////////////////////////////////////////
//
// pShape meshed by a CMeshShapeEditor with normals merged after it, in an
// instance of its own where the shape put it.
//
//////////////////////////////////////////////////////////////////////////
static void TestShapeSceneAdd(Ptr<CScene> pScene, Ptr<CShape> pShape, float32 fSmallest, float32 fLargest)
{
	Ptr<CMeshEditor>	pEditor;
	Ptr<CInstance>		pInstance;
	Ptr<CInstanceMesh>	pInstanceMesh;
	Ptr<CConnection>	pConnection;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pShape, fSmallest, fLargest);
	pEditor->AddModifier<CMeshFlatFaces>();
	pEditor->AddModifier<CMeshMergeNormals>(MESH_SHAPE_SPLIT_ANGLE);

	pConnection = OMalloc<CConnection>();
	pConnection->msWorldMatrix.Identity();
	pConnection->msLocalMatrix.Identity();
	pInstanceMesh = OMalloc<CInstanceMesh>(pEditor);
	pInstanceMesh->mapConnections.Add(pConnection);
	pInstance = OMalloc<CInstance>();
	pInstance->mapConnections.Add(pConnection);
	pInstance->mapMeshes.Add(pInstanceMesh);
	pScene->mapInstances.Add(pInstance);
}


//////////////////////////////////////////////////////////////////////////
//
// A camera at psEye looking at the origin, with z up in the scene.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CCamera> TestShapeSceneCamera(SFloat32Vec3* psEye)
{
	Ptr<CCamera>	pCamera;
	SFloat4x4		sWorld;
	SFloat32Vec3	sBack;
	SFloat32Vec3	sUp;
	SFloat32Vec3	sRight;
	SFloat32Vec3	sSceneUp;

	Float32Vec3Normalise(&sBack, psEye);
	sSceneUp.Init(0, 0, 1);
	Float32Vec3Cross(&sRight, &sSceneUp, &sBack);
	sRight.Normalize();
	Float32Vec3Cross(&sUp, &sBack, &sRight);

	sWorld.Identity();
	sWorld.x.Init(sRight.x, sRight.y, sRight.z, 0.0f);
	sWorld.y.Init(sUp.x, sUp.y, sUp.z, 0.0f);
	sWorld.z.Init(sBack.x, sBack.y, sBack.z, 0.0f);
	sWorld.pos.Init(psEye->x, psEye->y, psEye->z, 1.0f);

	pCamera = OMalloc<CCamera>();
	pCamera->mfFOV = 3.14159265f / 3.0f;
	pCamera->mpConnection = OMalloc<CConnection>();
	pCamera->mpConnection->msWorldMatrix = sWorld;
	pCamera->mpConnection->msLocalMatrix = sWorld;
	return pCamera;
}


//////////////////////////////////////////////////////////////////////////
//
// A scene of shapes, drawn by the raster renderer, as it was when first
// drawn and looked at (Input/ShapeScene.raw) but for a hundredth of its
// pixels: a sphere and a torus from their own editors, a cylinder pushed
// up through a box and two spheres smoothly joined, as isosurfaces.
// Output/ShapeScene.raw is what was drawn, ARGB32 rows from the top.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeSceneDraw(void)
{
	CRasterRenderFactory	cFactory;
	CRasteriser				cRasteriser;
	CRasterTarget			cTarget;
	CFileBasic				cFile;
	CFileUtil				cFileUtil;
	Ptr<CScene>				pScene;
	Ptr<CCamera>			pCamera;
	Ptr<CRasterFrame>		pFrame;
	Ptr<CShapeUnion>		pUnion;
	SFloat32Box				sBox;
	SFloat32Vec3			sCentre;
	SFloat32Vec3			asAxes[3];
	SFloat32Vec3			sAxis;
	SFloat32Vec3			sStart;
	SFloat32Vec3			sEye;
	uint32*					puiPixels;
	int						i;
	int						iDrawn;
	int						iDifferent;
	lsize					uiBytes;

	pScene = OMalloc<CScene>();

	sCentre.Init(-3.0f, 1.5f, 0.0f);
	TestShapeSceneAdd(pScene, OMalloc<CSphere>(&sCentre, 1.0f), 0.1f, 1.0f);

	sCentre.Init(3.0f, 1.5f, 0.0f);
	sAxis.Init(0.3f, -0.4f, 1.0f);
	TestShapeSceneAdd(pScene, OMalloc<CTorus>(&sAxis, &sCentre, 1.0f, 0.35f), 0.1f, 1.0f);

	sCentre.Init(-1.2f, -1.0f, -0.4f);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.0f, &asAxes[1], 1.0f, &asAxes[2], 0.6f);
	sAxis.Init(0, 0, 1);
	sStart.Init(-1.2f, -1.0f, -0.5f);
	pUnion = OMalloc<CShapeUnion>();
	pUnion->Add(OMalloc<CBox>(&sBox));
	pUnion->Add(OMalloc<CCylinder>(&sAxis, &sStart, 0.5f, 1.6f));
	TestShapeSceneAdd(pScene, pUnion, 0.05f, 0.4f);

	sCentre.Init(1.6f, -1.2f, 0.0f);
	sStart.Init(2.4f, -0.8f, 0.3f);
	TestShapeSceneAdd(pScene, OMalloc<CShapeSmoothUnion>(OMalloc<CSphere>(&sCentre, 0.8f), OMalloc<CSphere>(&sStart, 0.6f), 0.4f), 0.05f, 0.4f);

	sEye.Init(1.5f, -9.0f, 6.0f);
	pCamera = TestShapeSceneCamera(&sEye);
	pScene->mapCameras.Add(pCamera);

	cFactory.Init(TEST_SHAPE_SCENE_SCALE);
	cRasteriser.Init();
	pFrame = OMalloc<CRasterFrame>(OMalloc<CRenderScene>(pScene, &cFactory, cFactory.CreateRenderer()), pCamera);

	uiBytes = TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT * sizeof(uint32);
	puiPixels = (uint32*)malloc((size_t)uiBytes);
	cTarget.Init(puiPixels, TEST_SHAPE_SCENE_WIDTH, TEST_SHAPE_SCENE_HEIGHT, TEST_SHAPE_SCENE_WIDTH);
	cTarget.Clear(0xFF000000);
	pFrame->Draw(&cRasteriser, &cTarget);
	AssertSize(4, pFrame->NumInstances());

	iDrawn = 0;
	for (i = 0; i < TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT; i++)
	{
		iDrawn += (puiPixels[i] != 0xFF000000) ? 1 : 0;
	}
	AssertTrue(iDrawn > (TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT / 10));

	cFileUtil.TouchDir("Output");
	cFile.Init(DiskFile("Output" _FS_ "ShapeScene.raw"));
	if (cFile.Open(EFM_Write_Create))
	{
		cFile.WriteData(puiPixels, uiBytes);
		cFile.Close();
	}
	cFile.Kill();
	iDifferent = TestShapeSceneDifferences("Input" _FS_ "ShapeScene.raw", puiPixels, TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT);
	AssertTrue(iDifferent >= 0);
	AssertTrue(iDifferent <= (TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT / 100));

	cTarget.Kill();
	free(puiPixels);
	pFrame = NULL;
	pCamera = NULL;
	pUnion = NULL;
	pScene = NULL;
	cRasteriser.Kill();
	cFactory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestShapeScene(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
#ifndef BASELIB_SIZE16
	//RenderLib's cooked mesh lists count with a size, so a mesh this big passes 64KB with a 16 bit one, and RenderLib isn't built
	//for the W65C816.
	TestShapeSceneDraw();
#endif
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

