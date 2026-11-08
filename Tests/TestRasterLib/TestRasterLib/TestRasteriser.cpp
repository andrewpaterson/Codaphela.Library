#include <stdlib.h>
#include <string.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "RasterLib/Rasteriser.h"
#include "TestLib/Assert.h"


#define TEST_RASTER_SIZE	16
#define TEST_RED			0xFFFF0000
#define TEST_BLUE			0xFF0000FF
#define TEST_BLACK			0xFF000000


//////////////////////////////////////////////////////////////////////////
//
// x and y are subpixels.
//
//////////////////////////////////////////////////////////////////////////
void InitScreenVertex(SRasterScreenVertex* psVertex, int32 x, int32 y, int32 iDepth, int32 iShade)
{
	psVertex->sPosition.Init(x, y);
	psVertex->iDepth = iDepth;
	psVertex->iShade = iShade;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void DrawTestTriangle(CRasteriser* pcRasteriser, CRasterTarget* pcTarget, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3, int32 iDepth, uint32 uiColour, bool bTransparent = false)
{
	SRasterScreenVertex	sA;
	SRasterScreenVertex	sB;
	SRasterScreenVertex	sC;

	InitScreenVertex(&sA, x1, y1, iDepth, 256);
	InitScreenVertex(&sB, x2, y2, iDepth, 256);
	InitScreenVertex(&sC, x3, y3, iDepth, 256);
	pcRasteriser->DrawTriangle(pcTarget, &sA, &sB, &sC, uiColour, bTransparent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountPixels(CRasterTarget* pcTarget, uint32 uiColour)
{
	int		i;
	int		iCount;

	iCount = 0;
	for (i = 0; i < pcTarget->miWidth * pcTarget->miHeight; i++)
	{
		if (pcTarget->mpuiPixels[i] == uiColour)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Two triangles sharing an edge, wherever it is, must draw every pixel in the quad exactly once.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserFillRule(void)
{
	CRasteriser		cRasteriser;
	CRasterTarget	cTargetA;
	CRasterTarget	cTargetB;
	uint32			auiA[TEST_RASTER_SIZE * TEST_RASTER_SIZE];
	uint32			auiB[TEST_RASTER_SIZE * TEST_RASTER_SIZE];
	int				iCase;
	int				i;
	int				iBoth;
	int				iEither;
	int32			x0;
	int32			y0;
	int32			x1;
	int32			y1;
	int32			x2;
	int32			y2;
	int32			x3;
	int32			y3;

	cRasteriser.Init();
	cTargetA.Init(auiA, TEST_RASTER_SIZE, TEST_RASTER_SIZE, TEST_RASTER_SIZE);
	cTargetB.Init(auiB, TEST_RASTER_SIZE, TEST_RASTER_SIZE, TEST_RASTER_SIZE);

	//Arbitrary convex quads with corners on and off the samples, covering the row-major and shallow-edge cases.
	for (iCase = 0; iCase < 512; iCase++)
	{
		x0 = 8 + (iCase * 7) % 32;
		y0 = 8 + (iCase * 11) % 32;
		x1 = 216 - (iCase * 5) % 32;
		y1 = 8 + (iCase * 3) % 32;
		x2 = 216 - (iCase * 13) % 32;
		y2 = 216 - (iCase * 9) % 32;
		x3 = 8 + (iCase * 17) % 32;
		y3 = 216 - (iCase * 15) % 32;

		cTargetA.Clear(TEST_BLACK);
		cTargetB.Clear(TEST_BLACK);

		//Anticlockwise on the screen is a negative area with y down.
		DrawTestTriangle(&cRasteriser, &cTargetA, x0, y0, x2, y2, x1, y1, 1000, TEST_RED);
		DrawTestTriangle(&cRasteriser, &cTargetB, x0, y0, x3, y3, x2, y2, 1000, TEST_BLUE);

		iBoth = 0;
		iEither = 0;
		for (i = 0; i < TEST_RASTER_SIZE * TEST_RASTER_SIZE; i++)
		{
			if ((auiA[i] != TEST_BLACK) && (auiB[i] != TEST_BLACK))
			{
				iBoth++;
			}
			if ((auiA[i] != TEST_BLACK) || (auiB[i] != TEST_BLACK))
			{
				iEither++;
			}
		}
		AssertInt(0, iBoth);
		AssertTrue(iEither > 100);
	}

	//A pixel aligned square is exact: 8 x 8 samples, none on the edge.
	cTargetA.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTargetA, 32, 32, 160, 160, 160, 32, 1000, TEST_RED);
	DrawTestTriangle(&cRasteriser, &cTargetA, 32, 32, 32, 160, 160, 160, 1000, TEST_RED);
	AssertInt(64, CountPixels(&cTargetA, TEST_RED));
	AssertInt((int32)TEST_BLACK, (int32)auiA[1 * TEST_RASTER_SIZE + 1]);
	AssertInt((int32)TEST_RED, (int32)auiA[2 * TEST_RASTER_SIZE + 2]);
	AssertInt((int32)TEST_RED, (int32)auiA[9 * TEST_RASTER_SIZE + 9]);
	AssertInt((int32)TEST_BLACK, (int32)auiA[10 * TEST_RASTER_SIZE + 9]);

	cTargetB.Kill();
	cTargetA.Kill();
	cRasteriser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserDepthAndCulling(void)
{
	CRasteriser		cRasteriser;
	CRasterTarget	cTarget;
	uint32			aui[TEST_RASTER_SIZE * TEST_RASTER_SIZE];

	cRasteriser.Init();
	cTarget.Init(aui, TEST_RASTER_SIZE, TEST_RASTER_SIZE, TEST_RASTER_SIZE);

	//A larger depth is nearer, whichever order they are drawn in.
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 2000, TEST_RED);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 1000, TEST_BLUE);
	AssertInt(0, CountPixels(&cTarget, TEST_BLUE));
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 1000, TEST_BLUE);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 2000, TEST_RED);
	AssertInt(0, CountPixels(&cTarget, TEST_BLUE));
	AssertTrue(CountPixels(&cTarget, TEST_RED) > 100);

	//The other winding is a back face.
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 0, 256, 256, 1000, TEST_RED);
	AssertInt(0, CountPixels(&cTarget, TEST_RED));
	cRasteriser.SetCullBackFaces(false);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 0, 256, 256, 1000, TEST_RED);
	AssertTrue(CountPixels(&cTarget, TEST_RED) > 100);
	cRasteriser.SetCullBackFaces(true);

	//A vertex behind the near plane stops the triangle.
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 0, TEST_RED);
	AssertInt(0, CountPixels(&cTarget, TEST_RED));

	//Off the screen in every direction, and much larger than it, only touches the screen.
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, -4000, -4000, 4000, 4000, 4000, -4000, 1000, TEST_RED);
	DrawTestTriangle(&cRasteriser, &cTarget, -4000, -4000, -4000, 4000, 4000, 4000, 1000, TEST_RED);
	AssertInt(TEST_RASTER_SIZE * TEST_RASTER_SIZE, CountPixels(&cTarget, TEST_RED));
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, -400, -400, -100, -100, -100, -400, 1000, TEST_RED);
	DrawTestTriangle(&cRasteriser, &cTarget, 1000, 1000, 1300, 1300, 1300, 1000, 1000, TEST_RED);
	AssertInt(0, CountPixels(&cTarget, TEST_RED));

	cTarget.Kill();
	cRasteriser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserInterpolation(void)
{
	CRasteriser			cRasteriser;
	CRasterTarget		cTarget;
	uint32				aui[32 * 32];
	SRasterScreenVertex	sA;
	SRasterScreenVertex	sB;
	SRasterScreenVertex	sC;
	uint32				uiRed;

	cRasteriser.Init();
	cTarget.Init(aui, 32, 32, 32);

	//Shade 256 keeps the colour exactly and 128 halves it, for small and large triangles.
	cTarget.Clear(TEST_BLACK);
	InitScreenVertex(&sA, 0, 0, 1000, 128);
	InitScreenVertex(&sB, 48, 48, 1000, 128);
	InitScreenVertex(&sC, 48, 0, 1000, 128);
	cRasteriser.DrawTriangle(&cTarget, &sA, &sB, &sC, 0xFFFFFFFF, false);
	AssertInt((int32)0xFF7F7F7F, (int32)aui[1 * 32 + 2]);

	cTarget.Clear(TEST_BLACK);
	InitScreenVertex(&sB, 512, 512, 1000, 128);
	InitScreenVertex(&sC, 512, 0, 1000, 128);
	cRasteriser.DrawTriangle(&cTarget, &sA, &sB, &sC, 0xFFFFFFFF, false);
	AssertInt((int32)0xFF7F7F7F, (int32)aui[5 * 32 + 20]);

	//Shade is interpolated: a left to right ramp on a large triangle.
	cTarget.Clear(TEST_BLACK);
	InitScreenVertex(&sA, 0, 0, 1000, 0);
	InitScreenVertex(&sB, 512, 512, 1000, 256);
	InitScreenVertex(&sC, 512, 0, 1000, 256);
	cRasteriser.DrawTriangle(&cTarget, &sA, &sB, &sC, 0xFFFF0000, false);
	uiRed = (aui[0 * 32 + 16] >> 16) & 0xFF;
	AssertTrue((uiRed >= 130) && (uiRed <= 136));
	uiRed = (aui[0 * 32 + 30] >> 16) & 0xFF;
	AssertTrue(uiRed >= 240);

	//Depth is interpolated: this slopes through a flat triangle at the middle's depth.
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 512, 512, 512, 0, 5000, TEST_BLUE);
	InitScreenVertex(&sA, 0, 0, 1000, 256);
	InitScreenVertex(&sB, 512, 512, 9000, 256);
	InitScreenVertex(&sC, 512, 0, 9000, 256);
	cRasteriser.DrawTriangle(&cTarget, &sA, &sB, &sC, TEST_RED, false);
	AssertInt((int32)TEST_BLUE, (int32)aui[1 * 32 + 10]);
	AssertInt((int32)TEST_RED, (int32)aui[1 * 32 + 25]);

	//The same for a triangle small enough to be worked out per pixel.
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 128, 24, 128, 0, 5000, TEST_BLUE);
	InitScreenVertex(&sA, 0, 0, 1000, 256);
	InitScreenVertex(&sB, 128, 24, 9000, 256);
	InitScreenVertex(&sC, 128, 0, 9000, 256);
	cRasteriser.DrawTriangle(&cTarget, &sA, &sB, &sC, TEST_RED, false);
	AssertInt((int32)TEST_BLUE, (int32)aui[0 * 32 + 3]);
	AssertInt((int32)TEST_RED, (int32)aui[0 * 32 + 5]);

	cTarget.Kill();
	cRasteriser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserTransparency(void)
{
	CRasteriser		cRasteriser;
	CRasterTarget	cTarget;
	uint32			aui[TEST_RASTER_SIZE * TEST_RASTER_SIZE];

	cRasteriser.Init();
	cTarget.Init(aui, TEST_RASTER_SIZE, TEST_RASTER_SIZE, TEST_RASTER_SIZE);

	//Half of white over black.
	cTarget.Clear(TEST_BLACK);
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 1000, 0x80FFFFFF, true);
	AssertInt((int32)0xFF808080, (int32)aui[1 * TEST_RASTER_SIZE + 8]);

	//It doesn't write depth: something behind it drawn later still shows, as it would if they were sorted.
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 500, TEST_RED);
	AssertInt((int32)TEST_RED, (int32)aui[1 * TEST_RASTER_SIZE + 8]);

	//It is hidden by something opaque in front of it.
	DrawTestTriangle(&cRasteriser, &cTarget, 0, 0, 256, 256, 256, 0, 400, 0x80FFFFFF, true);
	AssertInt((int32)TEST_RED, (int32)aui[1 * TEST_RASTER_SIZE + 8]);

	cTarget.Kill();
	cRasteriser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A box from a mesh through to pixels.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserMesh(void)
{
	CMesh			cMesh;
	CRasterMesh		cRaster;
	CRasterCamera	cCamera;
	CRasteriser		cRasteriser;
	CRasterTarget	cTarget;
	uint32*			puiFirst;
	uint32*			puiSecond;
	int				i;
	int				iDrawn;
	bool			bSame;

	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;
	//Twice as deep as it is wide so that its front face only fills the middle of the screen.
	cMesh.AddCorner(-0.5f, -0.5f, 1.0f);
	cMesh.AddCorner(0.5f, -0.5f, 1.0f);
	cMesh.AddCorner(0.5f, 0.5f, 1.0f);
	cMesh.AddCorner(-0.5f, 0.5f, 1.0f);
	cMesh.AddCorner(-0.5f, -0.5f, -1.0f);
	cMesh.AddCorner(0.5f, -0.5f, -1.0f);
	cMesh.AddCorner(0.5f, 0.5f, -1.0f);
	cMesh.AddCorner(-0.5f, 0.5f, -1.0f);
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(0, 2, 3);
	cMesh.AddFace(5, 4, 7);
	cMesh.AddFace(5, 7, 6);

	cRaster.Init();
	cRaster.Build(&cMesh);

	puiFirst = (uint32*)malloc(64 * 64 * sizeof(uint32));
	puiSecond = (uint32*)malloc(64 * 64 * sizeof(uint32));

	cRasteriser.Init();
	cCamera.Init();

	for (i = 0; i < 2; i++)
	{
		cCamera.SetProjection((i == 0) ? RP_Perspective : RP_Orthographic);
		cCamera.Fit(64, 64);

		//Looking down Z only the front (lit, not black) face is visible.  The centre is drawn and a corner isn't.
		cTarget.Init(puiFirst, 64, 64, 64);
		cTarget.Clear(TEST_BLACK);
		cRasteriser.Draw(&cRaster, &cCamera, &cTarget);
		AssertTrue(puiFirst[32 * 64 + 32] != TEST_BLACK);
		AssertInt((int32)TEST_BLACK, (int32)puiFirst[0]);
		iDrawn = 64 * 64 - CountPixels(&cTarget, TEST_BLACK);
		AssertTrue(iDrawn > 64 * 64 / 8);
		cTarget.Kill();

		//The same camera draws the same pixels.
		cTarget.Init(puiSecond, 64, 64, 64);
		cTarget.Clear(TEST_BLACK);
		cRasteriser.Draw(&cRaster, &cCamera, &cTarget);
		bSame = memcmp(puiFirst, puiSecond, 64 * 64 * sizeof(uint32)) == 0;
		AssertTrue(bSame);
		cTarget.Kill();
	}

	cRasteriser.Kill();
	free(puiSecond);
	free(puiFirst);
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriser(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRasteriserFillRule();
	TestRasteriserDepthAndCulling();
	TestRasteriserInterpolation();
	TestRasteriserTransparency();
	TestRasteriserMesh();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

