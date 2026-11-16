#include <stdlib.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "StandardLib/Objects.h"
#include "SupportLib/MeshFlatFaces.h"
#include "SupportLib/MeshMergeNormals.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "SupportLib/Cylinder.h"
#include "SupportLib/Torus.h"
#include "SupportLib/ShapeUnion.h"
#include "SupportLib/ShapeSmoothUnion.h"
#include "SupportLib/ShapeSubtraction.h"
#include "ProceduralLib/MeshShapeEditor.h"
#include "SupportLib/SolidChecker.h"
#include "TestLib/Assert.h"
#include "ShapeSceneHelper.h"


#define TEST_SHAPE_SCENE_WIDTH		128
#define TEST_SHAPE_SCENE_HEIGHT		80
#define TEST_SHAPE_SCENE_CHANNEL	8		//How far a channel may be from what was saved before its pixel counts as different,
#define TEST_SHAPE_SCENE_BLOCK		12		//and a textured scene's four by four block's mean.


//////////////////////////////////////////////////////////////////////////
//
// How many iBlock by iBlock blocks of pixels differ from the saved frame's
// by more than iTolerance in the mean of any channel, or -1 if it can't be
// read.  Builds differ in floats' last bits (g++ contracts a * b + c into
// one FMA instruction when it optimises, and Visual Studio needn't), so a
// curve's corners and shading can move a little; a textured surface's
// charts can be grown differently, so its texels are laid out differently
// and its mottle, where a mip level blends them, differs pixel for pixel
// but not block for block.
//
//////////////////////////////////////////////////////////////////////////
static int TestShapeSceneDifferences(const char* szExpected, uint32* puiPixels, int iBlock, int iTolerance)
{
	CFileBasic	cFile;
	uint32*		puiExpected;
	int			iNumPixels;
	int			iBlockX;
	int			iBlockY;
	int			x;
	int			y;
	int			i;
	int			iChannel;
	int			iDifferent;
	int			iExpected;
	int			iActual;
	lsize		uiRead;

	iNumPixels = TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT;
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
	for (iBlockY = 0; iBlockY < TEST_SHAPE_SCENE_HEIGHT; iBlockY += iBlock)
	{
		for (iBlockX = 0; iBlockX < TEST_SHAPE_SCENE_WIDTH; iBlockX += iBlock)
		{
			for (iChannel = 0; iChannel < 4; iChannel++)
			{
				iExpected = 0;
				iActual = 0;
				for (y = iBlockY; y < iBlockY + iBlock; y++)
				{
					for (x = iBlockX; x < iBlockX + iBlock; x++)
					{
						i = (y * TEST_SHAPE_SCENE_WIDTH) + x;
						iExpected += (int)((puiExpected[i] >> (iChannel * 8)) & 0xFF);
						iActual += (int)((puiPixels[i] >> (iChannel * 8)) & 0xFF);
					}
				}
				if (abs(iExpected - iActual) > iTolerance * iBlock * iBlock)
				{
					iDifferent++;
					break;
				}
			}
		}
	}
	free(puiExpected);
	return iDifferent;
}


//////////////////////////////////////////////////////////////////////////
//
// pShape meshed by a CMeshShapeEditor with flat normals merged after it.
//
//////////////////////////////////////////////////////////////////////////
static void TestShapeSceneAdd(Ptr<CScene> pScene, Ptr<CShape> pShape, float32 fSmallest, float32 fLargest)
{
	Ptr<CMeshEditor>	pEditor;

	pEditor = OMalloc<CMeshEditor>(OMalloc<CMesh>());
	pEditor->AddModifier<CMeshShapeEditor>(pShape, fSmallest, fLargest);
	pEditor->AddModifier<CMeshFlatFaces>();
	pEditor->AddModifier<CMeshMergeNormals>(MESH_SHAPE_SPLIT_ANGLE);
	ShapeSceneInstance(pScene, pEditor);
}


//////////////////////////////////////////////////////////////////////////
//
// pScene drawn by the raster renderer from psEye, 128 by 80, as it was
// when first drawn and looked at (Input/<szName>.raw) but for
// iPercentDifferent of its iBlock by iBlock blocks (each channel's mean
// within iTolerance).  Output/<szName>.raw is what was drawn, ARGB32 rows
// from the top.
//
//////////////////////////////////////////////////////////////////////////
static void TestShapeSceneRender(Ptr<CScene> pScene, SFloat32Vec3* psEye, size uiInstances, const char* szName, int iBlock, int iTolerance, int iPercentDifferent)
{
	CFileBasic	cFile;
	CFileUtil	cFileUtil;
	CChars		szFile;
	uint32*		puiPixels;
	int			i;
	int			iDrawn;
	int			iDifferent;
	lsize		uiBytes;

	uiBytes = TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT * sizeof(uint32);
	puiPixels = (uint32*)malloc((size_t)uiBytes);
	AssertSize(uiInstances, ShapeSceneDraw(pScene, psEye, puiPixels, TEST_SHAPE_SCENE_WIDTH, TEST_SHAPE_SCENE_HEIGHT));

	iDrawn = 0;
	for (i = 0; i < TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT; i++)
	{
		iDrawn += (puiPixels[i] != 0xFF000000) ? 1 : 0;
	}
	AssertTrue(iDrawn > (TEST_SHAPE_SCENE_WIDTH * TEST_SHAPE_SCENE_HEIGHT / 10));

	cFileUtil.TouchDir("Output");
	szFile.Init("Output" _FS_);
	szFile.Append(szName);
	szFile.Append(".raw");
	cFile.Init(DiskFile(szFile.Text()));
	if (cFile.Open(EFM_Write_Create))
	{
		cFile.WriteData(puiPixels, uiBytes);
		cFile.Close();
	}
	cFile.Kill();
	szFile.Kill();

	szFile.Init("Input" _FS_);
	szFile.Append(szName);
	szFile.Append(".raw");
	iDifferent = TestShapeSceneDifferences(szFile.Text(), puiPixels, iBlock, iTolerance);
	szFile.Kill();
	AssertTrue(iDifferent >= 0);
	AssertTrue(iDifferent <= ((TEST_SHAPE_SCENE_WIDTH / iBlock) * (TEST_SHAPE_SCENE_HEIGHT / iBlock) * iPercentDifferent / 100));

	free(puiPixels);
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
	Ptr<CScene>			pScene;
	Ptr<CShapeUnion>	pUnion;
	SFloat32Box			sBox;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		asAxes[3];
	SFloat32Vec3		sAxis;
	SFloat32Vec3		sStart;
	SFloat32Vec3		sEye;

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
	TestShapeSceneRender(pScene, &sEye, 4, "ShapeScene", 1, TEST_SHAPE_SCENE_CHANNEL, 1);
	pUnion = NULL;
	pScene = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A box and five spheres smoothly joined, unwrapped at 64 texels to the
// unit (grown charts by ARAP) and a checker of quarter unit cubes baked
// onto them at every mip level (Input/TexturedScene.raw), drawn from the
// level a pixel's texels make, within three hundredths of its four by
// four blocks: builds whose floats differ (FMA) grow and pack the charts
// differently, so the checker's texels are laid out differently and its
// mottle, where a level blends them, differs pixel for pixel.  The
// spheres' squares are two or three pixels across, so they're mostly
// mottle.  The box is off the checker's planes: a face on one is evenly
// both colours, as a solid texture is there.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeSceneTextured(void)
{
	Ptr<CScene>			pScene;
	Ptr<CSolidChecker>	pChecker;
	Ptr<CShape>			pSpheres;
	SFloat32Box			sBox;
	SFloat32Vec3		sCentre;
	SFloat32Vec3		asAxes[3];
	SFloat32Vec3		asCentres[5];
	SFloat32Vec3		sEye;
	int					i;

	pScene = OMalloc<CScene>();
	pChecker = OMalloc<CSolidChecker>(0.25f, Set32BitColour((uint8)230, (uint8)220, (uint8)200), Set32BitColour((uint8)60, (uint8)90, (uint8)150));

	sCentre.Init(-2.6f, 0.1f, 0.07f);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.0f, &asAxes[1], 1.0f, &asAxes[2], 1.0f);
	ShapeSceneAddTextured(pScene, OMalloc<CBox>(&sBox), pChecker, 64.0f);

	asCentres[0].Init(0.5f, 0.2f, 0.0f);
	asCentres[1].Init(1.4f, -0.4f, 0.3f);
	asCentres[2].Init(2.2f, 0.5f, -0.2f);
	asCentres[3].Init(3.0f, -0.3f, 0.2f);
	asCentres[4].Init(1.8f, 0.6f, 1.0f);
	pSpheres = OMalloc<CSphere>(&asCentres[0], 0.8f);
	for (i = 1; i < 5; i++)
	{
		pSpheres = OMalloc<CShapeSmoothUnion>(pSpheres, OMalloc<CSphere>(&asCentres[i], 0.7f), 0.4f);
	}
	ShapeSceneAddTextured(pScene, pSpheres, pChecker, 64.0f);

	sEye.Init(0.5f, -8.0f, 4.0f);
	TestShapeSceneRender(pScene, &sEye, 2, "TexturedScene", 4, TEST_SHAPE_SCENE_BLOCK, 3);
	pSpheres = NULL;
	pChecker = NULL;
	pScene = NULL;
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
	ObjectsFlush();
	TestShapeSceneTextured();
#endif
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

