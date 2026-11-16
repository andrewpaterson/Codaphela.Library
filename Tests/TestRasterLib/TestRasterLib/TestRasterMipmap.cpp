#include <math.h>
#include <stdlib.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Image.h"
#include "SupportLib/MaterialImage.h"
#include "RenderLib/TextureLevels.h"
#include "RasterLib/Rasteriser.h"
#include "RasterLib/RasterTexture.h"
#include "TestLib/Assert.h"


#define TEST_MIPMAP_SCREEN		128
#define TEST_MIPMAP_HALF		(3 << 18)
#define TEST_MIPMAP_BLACK		0xFF000000


//////////////////////////////////////////////////////////////////////////
//
// Each level's colour: its level in red, so a pixel says which level drew
// it.
//
//////////////////////////////////////////////////////////////////////////
static uint32 TestMipmapLevelColour(int32 iLevel)
{
	return 0xFF000000 | ((uint32)(iLevel * 20 + 10) << 16) | 0x0000FF00;
}


//////////////////////////////////////////////////////////////////////////
//
// A pixel's level, the nearest (a trilinear pixel is two levels blended),
// -1 for none drawn.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestMipmapLevelOf(uint32 uiPixel)
{
	if (uiPixel == TEST_MIPMAP_BLACK)
	{
		return -1;
	}
	return (int32)((uiPixel >> 16) & 0xFF) / 20;
}


//////////////////////////////////////////////////////////////////////////
//
// A texture iSize square, every level to 1 by 1, each level all its
// colour.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CRasterTexture> TestMipmapTexture(int32 iSize)
{
	Ptr<CRasterTexture>	pTexture;
	uint32*				puiTexels;
	int32				iLevel;
	int32				i;

	pTexture = OMalloc<CRasterTexture>(iSize, iSize, (uint64)iSize);
	puiTexels = pTexture->GetTexels(0);
	for (iLevel = 0; ; iLevel++)
	{
		for (i = 0; i < iSize * iSize; i++)
		{
			puiTexels[i] = TestMipmapLevelColour(iLevel);
		}
		if (iSize == 1)
		{
			break;
		}
		iSize = (iSize + 1) / 2;
		puiTexels = pTexture->AddLevel(iSize, iSize);
	}
	return pTexture;
}


//////////////////////////////////////////////////////////////////////////
//
// Levels added smaller, read back, and a level past the last (or before
// the first) is the last (or the first).
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMipmapLevels(void)
{
	Ptr<CRasterTexture>	pTexture;

	pTexture = OMalloc<CRasterTexture>(4, 3, (uint64)0);
	AssertInt(1, pTexture->NumLevels());
	pTexture->AddLevel(2, 2)[3] = 0x12345678;
	pTexture->AddLevel(1, 1)[0] = 0xFF00FF00;
	pTexture->GetTexels(0)[11] = 0xFF0000FF;
	AssertInt(3, pTexture->NumLevels());
	AssertInt(4, pTexture->GetWidth(0));
	AssertInt(3, pTexture->GetHeight(0));
	AssertInt(2, pTexture->GetWidth(1));
	AssertInt(1, pTexture->GetHeight(5));
	AssertIntHex((int32)0x12345678, (int32)pTexture->GetTexels(1)[3]);
	AssertIntHex((int32)0x12345678, (int32)pTexture->SampleNearest(RASTER_UV_ONE * 3 / 4, RASTER_UV_ONE * 3 / 4, 1));
	AssertIntHex((int32)0xFF00FF00, (int32)pTexture->SampleNearest(0, 0, 9));
	AssertIntHex((int32)0xFF0000FF, (int32)pTexture->SampleNearest(RASTER_UV_ONE - 1, RASTER_UV_ONE - 1, -2));
	AssertIntHex((int32)0xFF0000FF, (int32)pTexture->Sample(RASTER_UV_ONE - 1, RASTER_UV_ONE - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// Two texels, black and white: bilinear at the first's centre is it,
// half way between their centres is half each, and the UV wraps (at 0 it's
// half way between the second, wrapped round, and the first).  Trilinear
// between a black level and a white one is half each half way.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMipmapFilters(void)
{
	Ptr<CRasterTexture>	pTexture;
	uint32*				puiTexels;

	pTexture = OMalloc<CRasterTexture>(2, 1, (uint64)0);
	puiTexels = pTexture->GetTexels(0);
	puiTexels[0] = 0xFF000000;
	puiTexels[1] = 0xFFFFFFFF;
	AssertIntHex((int32)0xFF000000, (int32)pTexture->SampleBilinear(RASTER_UV_ONE / 4, 0, 0));
	AssertIntHex((int32)0xFFFFFFFF, (int32)pTexture->SampleBilinear(RASTER_UV_ONE * 3 / 4, 0, 0));
	AssertIntHex((int32)0xFF7F7F7F, (int32)pTexture->SampleBilinear(RASTER_UV_ONE / 2, 0, 0));
	AssertIntHex((int32)0xFF7F7F7F, (int32)pTexture->SampleBilinear(0, 0, 0));
	AssertIntHex((int32)0xFF3F3F3F, (int32)pTexture->SampleBilinear(RASTER_UV_ONE * 3 / 8, 0, 0));

	pTexture = OMalloc<CRasterTexture>(2, 2, (uint64)0);
	puiTexels = pTexture->GetTexels(0);
	puiTexels[0] = 0xFF000000;
	puiTexels[1] = 0xFF000000;
	puiTexels[2] = 0xFF000000;
	puiTexels[3] = 0xFF000000;
	pTexture->AddLevel(1, 1)[0] = 0xFFFFFFFF;
	AssertIntHex((int32)0xFF000000, (int32)pTexture->SampleTrilinear(RASTER_UV_ONE / 3, RASTER_UV_ONE / 3, -50));
	AssertIntHex((int32)0xFF000000, (int32)pTexture->SampleTrilinear(RASTER_UV_ONE / 3, RASTER_UV_ONE / 3, 0));
	AssertIntHex((int32)0xFF7F7F7F, (int32)pTexture->SampleTrilinear(RASTER_UV_ONE / 3, RASTER_UV_ONE / 3, RASTER_LOD_ONE / 2));
	AssertIntHex((int32)0xFFFFFFFF, (int32)pTexture->SampleTrilinear(RASTER_UV_ONE / 3, RASTER_UV_ONE / 3, RASTER_LOD_ONE));
	AssertIntHex((int32)0xFFFFFFFF, (int32)pTexture->SampleTrilinear(RASTER_UV_ONE / 3, RASTER_UV_ONE / 3, RASTER_LOD_ONE * 7));
}


//////////////////////////////////////////////////////////////////////////
//
// A square, iCells by iCells cells of two triangles, UV 0 to once across.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CRasterMesh> TestMipmapSquare(SInt32Vec3* psCorner, SInt32Vec3* psAcross, SInt32Vec3* psUp, int32 iCells, Ptr<CRasterTexture> pTexture)
{
	Ptr<CRasterMesh>	pMesh;
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	int32				i;
	int32				j;
	int32				k;
	int32				aiCorner[4];

	pMesh = OMalloc<CRasterMesh>();
	pMesh->AddTexture(pTexture);
	for (j = 0; j <= iCells; j++)
	{
		for (i = 0; i <= iCells; i++)
		{
			psVertex = pMesh->masVertices.Add();
			psVertex->sPosition.Init((int32)(psCorner->x + ((int64)psAcross->x * i + (int64)psUp->x * j) / iCells),
									 (int32)(psCorner->y + ((int64)psAcross->y * i + (int64)psUp->y * j) / iCells),
									 (int32)(psCorner->z + ((int64)psAcross->z * i + (int64)psUp->z * j) / iCells));
			psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromInt(1));
			pMesh->masUVs.Add()->Init((int32)(((int64)RASTER_UV_ONE * i) / iCells), (int32)(((int64)RASTER_UV_ONE * j) / iCells));
		}
	}
	for (j = 0; j < iCells; j++)
	{
		for (i = 0; i < iCells; i++)
		{
			aiCorner[0] = j * (iCells + 1) + i;
			aiCorner[1] = aiCorner[0] + 1;
			aiCorner[2] = aiCorner[1] + iCells + 1;
			aiCorner[3] = aiCorner[0] + iCells + 1;
			for (k = 0; k < 2; k++)
			{
				psTriangle = pMesh->masOpaque.Add();
				psTriangle->aiVertex[0] = aiCorner[0];
				psTriangle->aiVertex[1] = aiCorner[k + 1];
				psTriangle->aiVertex[2] = aiCorner[k + 2];
				psTriangle->uiColour = 0xFFFFFFFF;
				pMesh->AddRun(&pMesh->masOpaqueRuns, (uint32)(pMesh->masOpaque.NumElements() - 1), 0);
			}
		}
	}
	pMesh->UpdateBounds();
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// pcMesh drawn into puiPixels, every shade 256, by eFilter.
//
//////////////////////////////////////////////////////////////////////////
static void TestMipmapDraw(CRasterMesh* pcMesh, ERasterFilter eFilter, uint32* puiPixels)
{
	CRasterCamera	cCamera;
	CRasteriser		cRasteriser;
	CRasterTarget	cTarget;

	cTarget.Init(puiPixels, TEST_MIPMAP_SCREEN, TEST_MIPMAP_SCREEN, TEST_MIPMAP_SCREEN);
	cRasteriser.Init();
	cRasteriser.GetLights()->Clear();
	cRasteriser.GetLights()->SetAmbient(256);
	cRasteriser.SetFilter(eFilter);
	cCamera.Init();
	cCamera.Fit(TEST_MIPMAP_SCREEN, TEST_MIPMAP_SCREEN);
	cTarget.Clear(TEST_MIPMAP_BLACK);
	cRasteriser.Draw(pcMesh, &cCamera, &cTarget);
	cRasteriser.Kill();
	cTarget.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The levels of the pixels drawn: the least and the most.
//
//////////////////////////////////////////////////////////////////////////
static void TestMipmapLevelsDrawn(uint32* puiPixels, int32* piLeast, int32* piMost)
{
	int32	i;
	int32	iLevel;

	*piLeast = -1;
	*piMost = -1;
	for (i = 0; i < TEST_MIPMAP_SCREEN * TEST_MIPMAP_SCREEN; i++)
	{
		iLevel = TestMipmapLevelOf(puiPixels[i]);
		if (iLevel >= 0)
		{
			*piLeast = ((*piLeast < 0) || (iLevel < *piLeast)) ? iLevel : *piLeast;
			*piMost = (iLevel > *piMost) ? iLevel : *piMost;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The levels down the middle column: at its bottom (nearest) and its top
// (furthest) pixels drawn.
//
//////////////////////////////////////////////////////////////////////////
static void TestMipmapColumnLevels(uint32* puiPixels, int32* piNear, int32* piFar)
{
	int32	x;
	int32	y;
	int32	iLevel;

	*piNear = -1;
	*piFar = -1;
	x = TEST_MIPMAP_SCREEN / 2;
	for (y = 0; y < TEST_MIPMAP_SCREEN; y++)
	{
		iLevel = TestMipmapLevelOf(puiPixels[(y * TEST_MIPMAP_SCREEN) + x]);
		if (iLevel >= 0)
		{
			*piFar = (*piFar < 0) ? iLevel : *piFar;
			*piNear = iLevel;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A square facing the eye, so many pixels across: textured by a texture as
// many texels across it's drawn from level 0, by one four times as wide
// from level 2, by every filter.  Tilted back, its far rows are drawn from
// coarser levels than its near ones, within a triangle too (by each
// pixel's distance).
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMipmapChoice(void)
{
	Ptr<CRasterMesh>	pMesh;
	CRasterCamera		cCamera;
	SRasterScreenVertex	sLowLeft;
	SRasterScreenVertex	sHighRight;
	SInt32Vec3			sCorner;
	SInt32Vec3			sAcross;
	SInt32Vec3			sUp;
	uint32*				puiPixels;
	int32				iPixels;
	int32				iTexels;
	int32				iFilter;
	int32				iLeast;
	int32				iMost;
	int32				iNear;
	int32				iFar;
	int32				iCells;

	puiPixels = (uint32*)malloc(TEST_MIPMAP_SCREEN * TEST_MIPMAP_SCREEN * sizeof(uint32));
	sCorner.Init(-TEST_MIPMAP_HALF, -TEST_MIPMAP_HALF, 0);
	sAcross.Init(TEST_MIPMAP_HALF * 2, 0, 0);
	sUp.Init(0, TEST_MIPMAP_HALF * 2, 0);

	pMesh = TestMipmapSquare(&sCorner, &sAcross, &sUp, 1, TestMipmapTexture(4));
	cCamera.Init();
	cCamera.Fit(TEST_MIPMAP_SCREEN, TEST_MIPMAP_SCREEN);
	cCamera.Project(&sLowLeft, &pMesh->masVertices.Get(0)->sPosition);
	cCamera.Project(&sHighRight, &pMesh->masVertices.Get(3)->sPosition);
	iPixels = (int32)(sHighRight.sPosition.x.ToFloat() - sLowLeft.sPosition.x.ToFloat());
	AssertInt(57, iPixels);

	for (iFilter = RF_Nearest; iFilter <= RF_Trilinear; iFilter++)
	{
		for (iTexels = iPixels; iTexels <= iPixels * 4; iTexels *= 4)
		{
			pMesh = TestMipmapSquare(&sCorner, &sAcross, &sUp, 1, TestMipmapTexture(iTexels));
			TestMipmapDraw(&pMesh, (ERasterFilter)iFilter, puiPixels);
			TestMipmapLevelsDrawn(puiPixels, &iLeast, &iMost);
			AssertInt((iTexels == iPixels) ? 0 : 2, iLeast);
			AssertInt(iLeast, iMost);
		}
	}

	//Tilted, in many triangles and in two.
	sCorner.Init(-TEST_MIPMAP_HALF, -TEST_MIPMAP_HALF, TEST_MIPMAP_HALF * 3 / 2);
	sUp.Init(0, TEST_MIPMAP_HALF * 2, -TEST_MIPMAP_HALF * 3);
	for (iCells = 8; iCells >= 1; iCells -= 7)
	{
		pMesh = TestMipmapSquare(&sCorner, &sAcross, &sUp, iCells, TestMipmapTexture(256));
		TestMipmapDraw(&pMesh, RF_Nearest, puiPixels);
		TestMipmapColumnLevels(puiPixels, &iNear, &iFar);
		AssertTrue(iNear >= 0);
		AssertTrue(iFar > iNear);
	}

	free(puiPixels);
}


//////////////////////////////////////////////////////////////////////////
//
// Three by three halved: each texel the mean of the two by two it covers
// (each channel, rounded), the last row and column of fewer.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMipmapHalve(void)
{
	uint32	auiSource[9] = { 0x00000000, 0x04080C10, 0xFF000000, 0x00000000, 0x04080C10, 0xFF000000, 0x01020304, 0x03020100, 0x80808080 };
	uint32	auiDest[4];

	AssertInt(2, HalfTextureSize(3));
	AssertInt(1, HalfTextureSize(1));
	HalveTexels(auiDest, auiSource, 3, 3);
	AssertIntHex((int32)0x02040608, (int32)auiDest[0]);
	AssertIntHex((int32)0xFF000000, (int32)auiDest[1]);
	AssertIntHex((int32)0x02020202, (int32)auiDest[2]);
	AssertIntHex((int32)0x80808080, (int32)auiDest[3]);
}


//////////////////////////////////////////////////////////////////////////
//
// A material image 4 by 2 with its own level 1 (2 by 1) but not level 2:
// levels 4 by 2, 2 by 1 (its own) and 1 by 1 (halved from that).  An image
// whose own level is the wrong size has it made instead.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMipmapMaterialLevels(void)
{
	Ptr<CMaterialImage>	pMaterialImage;
	Ptr<CImage>			pLevel;
	CTextureLevels		cLevels;
	uint8*				puiData;
	int32				i;

	pMaterialImage = OMalloc<CMaterialImage>();
	pMaterialImage->mpImage = OMalloc<CImage>((uint32)4, (uint32)2, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	puiData = (uint8*)pMaterialImage->mpImage->GetData();
	for (i = 0; i < 4 * 2 * 3; i++)
	{
		puiData[i] = 0;
	}
	pLevel = OMalloc<CImage>((uint32)2, (uint32)1, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	puiData = (uint8*)pLevel->GetData();
	puiData[0] = 200;
	puiData[1] = 0;
	puiData[2] = 0;
	puiData[3] = 100;
	puiData[4] = 0;
	puiData[5] = 0;
	pMaterialImage->mapLevels.Add(pLevel);

	AssertTrue(cLevels.Init(&pMaterialImage));
	AssertInt(3, cLevels.NumLevels());
	AssertInt(4, cLevels.GetWidth(0));
	AssertInt(2, cLevels.GetHeight(0));
	AssertInt(2, cLevels.GetWidth(1));
	AssertInt(1, cLevels.GetHeight(1));
	AssertInt(1, cLevels.GetWidth(2));
	AssertIntHex((int32)0xFF000000, (int32)cLevels.GetTexels(0)[5]);
	AssertIntHex((int32)0xFFC80000, (int32)cLevels.GetTexels(1)[0]);
	AssertIntHex((int32)0xFF960000, (int32)cLevels.GetTexels(2)[0]);
	cLevels.Kill();

	pMaterialImage->mapLevels.RemoveAll();
	pMaterialImage->mapLevels.Add(OMalloc<CImage>((uint32)3, (uint32)1, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP));
	AssertTrue(cLevels.Init(&pMaterialImage));
	AssertIntHex((int32)0xFF000000, (int32)cLevels.GetTexels(1)[0]);
	cLevels.Kill();

	AssertFalse(cLevels.Init(NULL));
	cLevels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterMipmap(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRasterMipmapLevels();
	TestRasterMipmapFilters();
	TestRasterMipmapChoice();
	TestRasterMipmapHalve();
	TestRasterMipmapMaterialLevels();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

