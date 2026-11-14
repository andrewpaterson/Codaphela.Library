#include <stdlib.h>
#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "RasterLib/Rasteriser.h"
#include "RasterLib/RasterTexture.h"
#include "TestLib/Assert.h"


#define TEST_TEXTURE_BLACK		0xFF000000
#define TEST_TEXTURE_SIZE		256
#define TEST_TEXTURE_SCREEN		128
#define TEST_TEXTURE_HALF		(3 << 18)  //With the camera's distance of 1 << 22 a tilted square is near twice as far at the top.


int CountPixels(CRasterTarget* pcTarget, uint32 uiColour);


//////////////////////////////////////////////////////////////////////////
//
// A texture whose texel at x, y is red x and green y, so that a pixel says
// which texel drew it.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CRasterTexture> TestTextureOfCoordinates(void)
{
	Ptr<CRasterTexture>	pTexture;
	int32				x;
	int32				y;

	pTexture = OMalloc<CRasterTexture>(TEST_TEXTURE_SIZE, TEST_TEXTURE_SIZE, (uint64)1);
	for (y = 0; y < TEST_TEXTURE_SIZE; y++)
	{
		for (x = 0; x < TEST_TEXTURE_SIZE; x++)
		{
			pTexture->GetTexels()[y * TEST_TEXTURE_SIZE + x] = 0xFF000000 | ((uint32)x << 16) | ((uint32)y << 8);
		}
	}
	return pTexture;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CRasterTexture> TestTextureOfOne(uint32 uiTexel)
{
	Ptr<CRasterTexture>	pTexture;

	pTexture = OMalloc<CRasterTexture>(1, 1, (uint64)uiTexel);
	pTexture->GetTexels()[0] = uiTexel;
	return pTexture;
}


//////////////////////////////////////////////////////////////////////////
//
// A square, from psCorner along psAcross and psUp, in iCells by iCells
// cells of two triangles each, facing the way psAcross cross psUp does.  Its
// UV is 0 at psCorner and RASTER_UV_ONE across and up.  All of it is in one
// run of iTexture.
//
//////////////////////////////////////////////////////////////////////////
static void TestTextureAddSquare(CRasterMesh* pcMesh, SInt32Vec3* psCorner, SInt32Vec3* psAcross, SInt32Vec3* psUp, int32 iCells, int32 iTexture, uint32 uiColour, bool bTransparent)
{
	CArrayRasterTriangle*	pasTriangles;
	CArrayRasterTextureRun*	pasRuns;
	SRasterVertex*			psVertex;
	SRasterTriangle*		psTriangle;
	int32					iFirst;
	int32					i;
	int32					j;
	int32					k;
	int32					aiCorner[4];

	pasTriangles = bTransparent ? &pcMesh->masTransparent : &pcMesh->masOpaque;
	pasRuns = bTransparent ? &pcMesh->masTransparentRuns : &pcMesh->masOpaqueRuns;
	iFirst = (int32)pcMesh->masVertices.NumElements();
	for (j = 0; j <= iCells; j++)
	{
		for (i = 0; i <= iCells; i++)
		{
			psVertex = pcMesh->masVertices.Add();
			psVertex->sPosition.Init((int32)(psCorner->x + ((int64)psAcross->x * i + (int64)psUp->x * j) / iCells),
									 (int32)(psCorner->y + ((int64)psAcross->y * i + (int64)psUp->y * j) / iCells),
									 (int32)(psCorner->z + ((int64)psAcross->z * i + (int64)psUp->z * j) / iCells));
			psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromInt(1));
			pcMesh->masUVs.Add()->Init((int32)(((int64)RASTER_UV_ONE * i) / iCells), (int32)(((int64)RASTER_UV_ONE * j) / iCells));
		}
	}
	for (j = 0; j < iCells; j++)
	{
		for (i = 0; i < iCells; i++)
		{
			aiCorner[0] = iFirst + j * (iCells + 1) + i;
			aiCorner[1] = aiCorner[0] + 1;
			aiCorner[2] = aiCorner[1] + iCells + 1;
			aiCorner[3] = aiCorner[0] + iCells + 1;
			for (k = 0; k < 2; k++)
			{
				psTriangle = pasTriangles->Add();
				psTriangle->aiVertex[0] = aiCorner[0];
				psTriangle->aiVertex[1] = aiCorner[k + 1];
				psTriangle->aiVertex[2] = aiCorner[k + 2];
				psTriangle->uiColour = uiColour;
				pcMesh->AddRun(pasRuns, (uint32)(pasTriangles->NumElements() - 1), iTexture);
			}
		}
	}
	pcMesh->UpdateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// A square tilted back, its top further from the eye than its bottom.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CRasterMesh> TestTextureTiltedSquare(int32 iCells)
{
	Ptr<CRasterMesh>	pMesh;
	SInt32Vec3			sCorner;
	SInt32Vec3			sAcross;
	SInt32Vec3			sUp;

	pMesh = OMalloc<CRasterMesh>();
	pMesh->AddTexture(TestTextureOfCoordinates());
	sCorner.Init(-TEST_TEXTURE_HALF, -TEST_TEXTURE_HALF, TEST_TEXTURE_HALF * 3 / 2);
	sAcross.Init(TEST_TEXTURE_HALF * 2, 0, 0);
	sUp.Init(0, TEST_TEXTURE_HALF * 2, -TEST_TEXTURE_HALF * 3);
	TestTextureAddSquare(&pMesh, &sCorner, &sAcross, &sUp, iCells, 0, 0xFFFFFFFF, false);
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
// Shades of exactly 256, so that a pixel is its colour times its texel.
//
//////////////////////////////////////////////////////////////////////////
static void TestTextureFullLight(CRasteriser* pcRasteriser)
{
	pcRasteriser->GetLights()->Clear();
	pcRasteriser->GetLights()->SetAmbient(256);
}


//////////////////////////////////////////////////////////////////////////
//
// Texels repeat, by masking a size that's a power of two and by floor
// modulo one that isn't.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterTextureSample(void)
{
	Ptr<CRasterTexture>	pTexture;
	int32				i;

	pTexture = OMalloc<CRasterTexture>(4, 4, (uint64)7);
	AssertInt(4, pTexture->GetWidth());
	AssertLong(7, pTexture->GetSource());
	AssertIntHex((int32)0xFFFFFFFF, (int32)pTexture->GetTexels()[5]);
	for (i = 0; i < 16; i++)
	{
		pTexture->GetTexels()[i] = (uint32)i;
	}
	AssertInt(0, (int32)pTexture->Sample(0, 0));
	AssertInt(15, (int32)pTexture->Sample(RASTER_UV_ONE - 1, RASTER_UV_ONE - 1));
	AssertInt(1, (int32)pTexture->Sample(RASTER_UV_ONE / 4, 0));
	AssertInt(0, (int32)pTexture->Sample(RASTER_UV_ONE, 0));
	AssertInt(3, (int32)pTexture->Sample(-1, 0));
	AssertInt(12, (int32)pTexture->Sample(0, -1));
	AssertInt(6, (int32)pTexture->Sample(RASTER_UV_ONE * 5 + RASTER_UV_ONE / 2, -RASTER_UV_ONE * 3 + RASTER_UV_ONE / 4));

	pTexture = OMalloc<CRasterTexture>(3, 5, (uint64)0);
	for (i = 0; i < 15; i++)
	{
		pTexture->GetTexels()[i] = (uint32)i;
	}
	AssertInt(14, (int32)pTexture->Sample(-1, -1));
	AssertInt(0, (int32)pTexture->Sample(RASTER_UV_ONE * 2, RASTER_UV_ONE * 7));
	AssertInt(4, (int32)pTexture->Sample(RASTER_UV_ONE / 3 + 1, RASTER_UV_ONE / 5 + 1));
}


//////////////////////////////////////////////////////////////////////////
//
// The texel each pixel of puiPixels drew, or -1 for none.
//
//////////////////////////////////////////////////////////////////////////
static void TestTextureTexelAt(uint32* puiPixels, int32 i, int32* piU, int32* piV)
{
	if (puiPixels[i] == TEST_TEXTURE_BLACK)
	{
		*piU = -1;
		*piV = -1;
		return;
	}
	*piU = (int32)((puiPixels[i] >> 16) & 0xFF);
	*piV = (int32)((puiPixels[i] >> 8) & 0xFF);
}


//////////////////////////////////////////////////////////////////////////
//
// A square facing the eye is textured straight across: each pixel draws
// the texel that its middle is over, from the square's corners on the
// screen, give or take one for rounding.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterTextureFlat(void)
{
	Ptr<CRasterMesh>	pMesh;
	CRasterCamera		cCamera;
	CRasteriser			cRasteriser;
	CRasterTarget		cTarget;
	SRasterScreenVertex	sLowLeft;
	SRasterScreenVertex	sHighRight;
	SInt32Vec3			sCorner;
	SInt32Vec3			sAcross;
	SInt32Vec3			sUp;
	uint32*				puiPixels;
	float64				fU;
	float64				fV;
	int32				iU;
	int32				iV;
	int32				x;
	int32				y;
	int32				iDrawn;
	int32				iWorst;

	pMesh = OMalloc<CRasterMesh>();
	pMesh->AddTexture(TestTextureOfCoordinates());
	sCorner.Init(-TEST_TEXTURE_HALF, -TEST_TEXTURE_HALF, 0);
	sAcross.Init(TEST_TEXTURE_HALF * 2, 0, 0);
	sUp.Init(0, TEST_TEXTURE_HALF * 2, 0);
	TestTextureAddSquare(&pMesh, &sCorner, &sAcross, &sUp, 1, 0, 0xFFFFFFFF, false);
	AssertTrue(pMesh->IsTextured());

	puiPixels = (uint32*)malloc(TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN * sizeof(uint32));
	cTarget.Init(puiPixels, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);
	cRasteriser.Init();
	TestTextureFullLight(&cRasteriser);
	cCamera.Init();
	cCamera.Fit(TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);
	cTarget.Clear(TEST_TEXTURE_BLACK);
	cRasteriser.Draw(&pMesh, &cCamera, &cTarget);

	//UV 0, 0 is the low left corner, and the screen's y is down.
	cCamera.Project(&sLowLeft, &pMesh->masVertices.Get(0)->sPosition);
	cCamera.Project(&sHighRight, &pMesh->masVertices.Get(3)->sPosition);
	iDrawn = 0;
	iWorst = 0;
	for (y = 0; y < TEST_TEXTURE_SCREEN; y++)
	{
		for (x = 0; x < TEST_TEXTURE_SCREEN; x++)
		{
			TestTextureTexelAt(puiPixels, y * TEST_TEXTURE_SCREEN + x, &iU, &iV);
			if (iU >= 0)
			{
				iDrawn++;
				fU = ((x + 0.5) - sLowLeft.sPosition.x.ToFloat()) / (sHighRight.sPosition.x.ToFloat() - sLowLeft.sPosition.x.ToFloat());
				fV = ((y + 0.5) - sLowLeft.sPosition.y.ToFloat()) / (sHighRight.sPosition.y.ToFloat() - sLowLeft.sPosition.y.ToFloat());
				iWorst = (abs(iU - (int32)(fU * TEST_TEXTURE_SIZE)) > iWorst) ? abs(iU - (int32)(fU * TEST_TEXTURE_SIZE)) : iWorst;
				iWorst = (abs(iV - (int32)(fV * TEST_TEXTURE_SIZE)) > iWorst) ? abs(iV - (int32)(fV * TEST_TEXTURE_SIZE)) : iWorst;
			}
		}
	}
	AssertTrue(iDrawn > 3000);
	AssertTrue(iWorst <= 1);

	cRasteriser.Kill();
	cTarget.Kill();
	free(puiPixels);
}


//////////////////////////////////////////////////////////////////////////
//
// The tilted square's pixels, its depths taken off.
//
//////////////////////////////////////////////////////////////////////////
static void TestTextureDraw(CRasterMesh* pcMesh, CRasterCamera* pcCamera, CRasteriser* pcRasteriser, uint32* puiPixels)
{
	CRasterTarget	cTarget;

	cTarget.Init(puiPixels, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);
	cTarget.Clear(TEST_TEXTURE_BLACK);
	pcRasteriser->Draw(pcMesh, pcCamera, &cTarget);
	cTarget.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The furthest apart two drawings of the tilted square's texels are where
// both drew.  Where one did, *piOneOnly: the many small triangles' corners
// along its edges are rounded off them.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestTextureWorst(uint32* puiFirst, uint32* puiSecond, int32* piOneOnly)
{
	int32	i;
	int32	iU1;
	int32	iV1;
	int32	iU2;
	int32	iV2;
	int32	iWorst;

	iWorst = 0;
	*piOneOnly = 0;
	for (i = 0; i < TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN; i++)
	{
		TestTextureTexelAt(puiFirst, i, &iU1, &iV1);
		TestTextureTexelAt(puiSecond, i, &iU2, &iV2);
		if ((iU1 < 0) != (iU2 < 0))
		{
			(*piOneOnly)++;
			continue;
		}
		iWorst = (abs(iU1 - iU2) > iWorst) ? abs(iU1 - iU2) : iWorst;
		iWorst = (abs(iV1 - iV2) > iWorst) ? abs(iV1 - iV2) : iWorst;
	}
	return iWorst;
}


//////////////////////////////////////////////////////////////////////////
//
// A square tilted back, drawn as two triangles, is textured as it is when
// drawn as many small ones, which are near enough straight across each that
// they show where its texels go.  Its middle row is well away from halfway
// down its texture, which is what drawing straight across the screen would
// put there.  With an orthographic camera it is textured straight across.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterTexturePerspective(void)
{
	Ptr<CRasterMesh>	pTwo;
	Ptr<CRasterMesh>	pMany;
	CRasterCamera		cCamera;
	CRasteriser			cRasteriser;
	uint32*				puiTwo;
	uint32*				puiMany;
	int32				iTop;
	int32				iBottom;
	int32				iMiddle;
	int32				iU;
	int32				iV;
	int32				iTopV;
	int32				iBottomV;
	int32				y;
	int32				x;
	int32				iWorst;
	int32				iOneOnly;

	pTwo = TestTextureTiltedSquare(1);
	pMany = TestTextureTiltedSquare(32);
	puiTwo = (uint32*)malloc(TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN * sizeof(uint32));
	puiMany = (uint32*)malloc(TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN * sizeof(uint32));
	cRasteriser.Init();
	TestTextureFullLight(&cRasteriser);
	cCamera.Init();
	cCamera.Fit(TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);

	TestTextureDraw(&pTwo, &cCamera, &cRasteriser, puiTwo);
	TestTextureDraw(&pMany, &cCamera, &cRasteriser, puiMany);
	iWorst = TestTextureWorst(puiTwo, puiMany, &iOneOnly);
	AssertTrue(iWorst <= 2);
	AssertTrue(iOneOnly <= 8);

	//The rows it covers down the middle column.
	x = TEST_TEXTURE_SCREEN / 2;
	iTop = -1;
	iBottom = -1;
	for (y = 0; y < TEST_TEXTURE_SCREEN; y++)
	{
		TestTextureTexelAt(puiMany, y * TEST_TEXTURE_SCREEN + x, &iU, &iV);
		if (iU >= 0)
		{
			iTop = (iTop < 0) ? y : iTop;
			iBottom = y;
		}
	}
	AssertTrue(iBottom - iTop > 40);
	//A row in from each end, as one exactly on the far edge can be at a UV of one, which repeats to 0.
	iMiddle = (iTop + iBottom) / 2;
	TestTextureTexelAt(puiMany, (iTop + 1) * TEST_TEXTURE_SCREEN + x, &iU, &iTopV);
	TestTextureTexelAt(puiMany, (iBottom - 1) * TEST_TEXTURE_SCREEN + x, &iU, &iBottomV);
	TestTextureTexelAt(puiMany, iMiddle * TEST_TEXTURE_SCREEN + x, &iU, &iV);
	AssertTrue(abs(iTopV - iBottomV) > 230);
	AssertTrue(abs(iV - (iTopV + iBottomV) / 2) > 20);

	cCamera.SetProjection(RP_Orthographic);
	TestTextureDraw(&pTwo, &cCamera, &cRasteriser, puiTwo);
	TestTextureDraw(&pMany, &cCamera, &cRasteriser, puiMany);
	iWorst = TestTextureWorst(puiTwo, puiMany, &iOneOnly);
	AssertTrue(iWorst <= 1);
	AssertTrue(iOneOnly <= 8);
	TestTextureTexelAt(puiMany, (TEST_TEXTURE_SCREEN / 2) * TEST_TEXTURE_SCREEN + x, &iU, &iV);
	AssertTrue(abs(iV - TEST_TEXTURE_SIZE / 2) <= 8);  //The pixel's middle is half a pixel, some texels, from the screen's.

	cRasteriser.Kill();
	free(puiMany);
	free(puiTwo);
}


//////////////////////////////////////////////////////////////////////////
//
// The pixel in the middle of a square of one texel.
//
//////////////////////////////////////////////////////////////////////////
static uint32 TestTextureMiddle(uint32 uiTexel, uint32 uiColour, bool bTransparent, int32 iTexture)
{
	Ptr<CRasterMesh>	pMesh;
	CRasterCamera		cCamera;
	CRasteriser			cRasteriser;
	CRasterTarget		cTarget;
	SInt32Vec3			sCorner;
	SInt32Vec3			sAcross;
	SInt32Vec3			sUp;
	uint32*				puiPixels;
	uint32				uiMiddle;

	pMesh = OMalloc<CRasterMesh>();
	pMesh->AddTexture(TestTextureOfOne(uiTexel));
	sCorner.Init(-TEST_TEXTURE_HALF, -TEST_TEXTURE_HALF, 0);
	sAcross.Init(TEST_TEXTURE_HALF * 2, 0, 0);
	sUp.Init(0, TEST_TEXTURE_HALF * 2, 0);
	TestTextureAddSquare(&pMesh, &sCorner, &sAcross, &sUp, 1, iTexture, uiColour, bTransparent);

	puiPixels = (uint32*)malloc(TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN * sizeof(uint32));
	cTarget.Init(puiPixels, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);
	cRasteriser.Init();
	TestTextureFullLight(&cRasteriser);
	cCamera.Init();
	cCamera.Fit(TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);
	cTarget.Clear(TEST_TEXTURE_BLACK);
	cRasteriser.Draw(&pMesh, &cCamera, &cTarget);
	uiMiddle = puiPixels[(TEST_TEXTURE_SCREEN / 2) * TEST_TEXTURE_SCREEN + TEST_TEXTURE_SCREEN / 2];

	cRasteriser.Kill();
	cTarget.Kill();
	free(puiPixels);
	return uiMiddle;
}


//////////////////////////////////////////////////////////////////////////
//
// A pixel is its colour times its texel, each channel rounded, as the
// Vulkan renderer draws it, and a transparent one's alpha is both of
// theirs.  A run of no texture, or of a texture the mesh hasn't got, is its
// colour alone.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterTextureColour(void)
{
	AssertIntHex((int32)0xFF4080C0, (int32)TestTextureMiddle(0xFF4080C0, 0xFFFFFFFF, false, 0));
	AssertIntHex((int32)0xFF204060, (int32)TestTextureMiddle(0xFF4080C0, 0xFF808080, false, 0));
	AssertIntHex((int32)0xFF204060, (int32)TestTextureMiddle(0x004080C0, 0xFF808080, false, 0));
	AssertIntHex((int32)0xFF204060, (int32)TestTextureMiddle(0xFF4080C0, 0x80FFFFFF, true, 0));
	AssertIntHex((int32)0xFF204060, (int32)TestTextureMiddle(0x804080C0, 0xFFFFFFFF, true, 0));
	AssertIntHex((int32)0xFF808080, (int32)TestTextureMiddle(0xFF4080C0, 0xFF808080, false, RASTER_NO_TEXTURE));
	AssertIntHex((int32)0xFF808080, (int32)TestTextureMiddle(0xFF4080C0, 0xFF808080, false, 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertTestTextureRun(CArrayRasterTextureRun* pasRuns, size uiRun, uint32 uiFirst, uint32 uiNumTriangles, int32 iTexture)
{
	SRasterTextureRun*	psRun;

	psRun = pasRuns->Get(uiRun);
	AssertInt((int32)uiFirst, (int32)psRun->uiFirst);
	AssertInt((int32)uiNumTriangles, (int32)psRun->uiNumTriangles);
	AssertInt(iTexture, psRun->iTexture);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs grow while their texture is the same, and a texture is added once.
// Taking out triangles with no area keeps the runs over what's left, and a
// run left with nothing goes.  A mesh's file keeps its UVs and runs but not
// its textures.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterTextureRuns(void)
{
	Ptr<CRasterMesh>	pMesh;
	Ptr<CRasterMesh>	pRead;
	Ptr<CRasterTexture>	pFirst;
	Ptr<CRasterTexture>	pSecond;
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	CFileUtil			cFileUtil;
	int32				i;
	int32				aiTexture[6];

	pMesh = OMalloc<CRasterMesh>();
	AssertFalse(pMesh->IsTextured());
	pFirst = TestTextureOfOne(1);
	pSecond = TestTextureOfOne(2);
	AssertInt(0, pMesh->AddTexture(pFirst));
	AssertInt(1, pMesh->AddTexture(pSecond));
	AssertInt(0, pMesh->AddTexture(pFirst));
	AssertInt(2, (int32)pMesh->mapTextures.NumElements());
	AssertTrue(pMesh->GetTexture(1) == &pSecond);
	AssertNull(pMesh->GetTexture(RASTER_NO_TEXTURE));
	AssertNull(pMesh->GetTexture(2));

	for (i = 0; i < 4; i++)
	{
		psVertex = pMesh->masVertices.Add();
		memset(psVertex, 0, sizeof(SRasterVertex));
		psVertex->sPosition.Init(((i == 1) || (i == 2)) ? 100 : 0, (i >= 2) ? 100 : 0, 0);
		pMesh->masUVs.Add()->Init(i * 1000, -i * 1000);
	}

	//Six triangles, runs of the first texture, none and the second.  The third, fourth and fifth have no area.
	aiTexture[0] = 0;
	aiTexture[1] = 0;
	aiTexture[2] = RASTER_NO_TEXTURE;
	aiTexture[3] = RASTER_NO_TEXTURE;
	aiTexture[4] = 1;
	aiTexture[5] = 1;
	for (i = 0; i < 6; i++)
	{
		psTriangle = pMesh->masOpaque.Add();
		psTriangle->aiVertex[0] = 0;
		psTriangle->aiVertex[1] = ((i >= 2) && (i <= 4)) ? 0 : 1;
		psTriangle->aiVertex[2] = 2;
		psTriangle->uiColour = 0xFF000000 | (uint32)i;
		pMesh->AddRun(&pMesh->masOpaqueRuns, (uint32)i, aiTexture[i]);
	}
	AssertSize(3, pMesh->masOpaqueRuns.NumElements());
	AssertTestTextureRun(&pMesh->masOpaqueRuns, 1, 2, 2, RASTER_NO_TEXTURE);
	AssertTrue(pMesh->IsTextured());

	AssertSize(3, pMesh->RemoveDegenerate());
	AssertSize(3, pMesh->masOpaque.NumElements());
	AssertSize(2, pMesh->masOpaqueRuns.NumElements());
	AssertTestTextureRun(&pMesh->masOpaqueRuns, 0, 0, 2, 0);
	AssertTestTextureRun(&pMesh->masOpaqueRuns, 1, 2, 1, 1);
	AssertIntHex((int32)0xFF000005, (int32)pMesh->masOpaque.Get(2)->uiColour);

	cFileUtil.RemoveDir("Output" _FS_ "RasterTexture");
	cFileUtil.MakeDir("Output" _FS_ "RasterTexture");
	AssertTrue(pMesh->WriteFile("Output" _FS_ "RasterTexture" _FS_ "Runs.rastermesh", 99));
	pRead = OMalloc<CRasterMesh>();
	AssertTrue(pRead->ReadFile("Output" _FS_ "RasterTexture" _FS_ "Runs.rastermesh", 99));
	AssertSize(4, pRead->masUVs.NumElements());
	AssertInt(-3000, pRead->masUVs.Get(3)->y);
	AssertSize(2, pRead->masOpaqueRuns.NumElements());
	AssertTestTextureRun(&pRead->masOpaqueRuns, 1, 2, 1, 1);
	AssertSize(0, pRead->mapTextures.NumElements());
	AssertNull(pRead->GetTexture(0));
	cFileUtil.RemoveDir("Output" _FS_ "RasterTexture");
}


//////////////////////////////////////////////////////////////////////////
//
// A textured instance that mirrors draws what a textured mesh made mirrored
// does: its UVs go with its corners when its triangles are wound the other
// way.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterTextureMirrored(void)
{
	Ptr<CRasterMesh>	pMesh;
	Ptr<CRasterMesh>	pMade;
	CRasterCamera		cCamera;
	CRasteriser			cRasteriser;
	CRasterTarget		cInstance;
	CRasterTarget		cMade;
	SRasterInstance		sInstance;
	SRasterInstance		sMade;
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	uint32*				puiInstance;
	uint32*				puiMade;
	size				i;
	bool				bSame;

	pMesh = TestTextureTiltedSquare(3);
	pMade = OMalloc<CRasterMesh>();
	pMade->AddTexture(pMesh->mapTextures.Get(0));
	for (i = 0; i < pMesh->masVertices.NumElements(); i++)
	{
		psVertex = pMade->masVertices.Add();
		*psVertex = *pMesh->masVertices.Get(i);
		psVertex->sPosition.x = -psVertex->sPosition.x;
		*pMade->masUVs.Add() = *pMesh->masUVs.Get(i);
	}
	for (i = 0; i < pMesh->masOpaque.NumElements(); i++)
	{
		psTriangle = pMade->masOpaque.Add();
		*psTriangle = *pMesh->masOpaque.Get(i);
		psTriangle->aiVertex[1] = pMesh->masOpaque.Get(i)->aiVertex[2];
		psTriangle->aiVertex[2] = pMesh->masOpaque.Get(i)->aiVertex[1];
		pMade->AddRun(&pMade->masOpaqueRuns, (uint32)i, 0);
	}
	pMade->UpdateBounds();

	sInstance.Init(&pMesh);
	sInstance.asAxes[0].Init(-(1 << RASTER_AXIS_BITS), 0, 0);
	sInstance.asLightAxes[0] = sInstance.asAxes[0];
	sInstance.bMirrored = true;
	sMade.Init(&pMade);

	puiInstance = (uint32*)malloc(TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN * sizeof(uint32));
	puiMade = (uint32*)malloc(TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN * sizeof(uint32));
	cInstance.Init(puiInstance, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);
	cMade.Init(puiMade, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);
	cRasteriser.Init();
	TestTextureFullLight(&cRasteriser);
	cCamera.Init();
	cCamera.Fit(TEST_TEXTURE_SCREEN, TEST_TEXTURE_SCREEN);

	cInstance.Clear(TEST_TEXTURE_BLACK);
	cRasteriser.Draw(&sInstance, 1, &cCamera, &cInstance);
	cMade.Clear(TEST_TEXTURE_BLACK);
	cRasteriser.Draw(&sMade, 1, &cCamera, &cMade);
	AssertTrue(CountPixels(&cInstance, TEST_TEXTURE_BLACK) < TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN - 2000);
	bSame = memcmp(puiInstance, puiMade, TEST_TEXTURE_SCREEN * TEST_TEXTURE_SCREEN * sizeof(uint32)) == 0;
	AssertTrue(bSame);

	cRasteriser.Kill();
	cMade.Kill();
	cInstance.Kill();
	free(puiMade);
	free(puiInstance);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterTexture(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRasterTextureSample();
	TestRasterTextureFlat();
	TestRasterTexturePerspective();
	TestRasterTextureColour();
	TestRasterTextureRuns();
	TestRasterTextureMirrored();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

