#include <stdlib.h>
#include <string.h>
#include "BaseLib/FixedPoint.h"
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "RasterLib/Rasteriser.h"
#include "RasterRenderLib/RasterMeshBuild.h"
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
	psVertex->sPosition.Init(SRasterSubpixel::FromRaw(x), SRasterSubpixel::FromRaw(y));
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
	BuildRasterMesh(&cRaster, &cMesh);

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
		AssertTrue(iDrawn > 64 * 64 / 16);
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
// Triangles wide enough to have each row's covered pixels worked out must
// cover the same pixels as ones whose pixels are looked for.  Each triangle
// is drawn across the whole of a wide target, then moved and drawn again in
// tiles too narrow to work anything out.  All sorts of triangles, long thin
// ones and ones off the screen too, wound both ways.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserWideTriangles(void)
{
	CRasteriser		cRasteriser;
	CRasterTarget	cWide;
	CRasterTarget	cTile;
	uint32*			puiWide;
	uint32*			puiTile;
	uint32			uiRandom;
	int32			aiCorner[6];
	int32			iShift;
	int				iTriangle;
	int				iTile;
	int				x;
	int				y;
	int				i;
	int				iDifferent;
	int				iDrawn;

	puiWide = (uint32*)malloc(128 * 64 * sizeof(uint32));
	puiTile = (uint32*)malloc(4 * 64 * sizeof(uint32));
	cRasteriser.Init();
	cRasteriser.SetCullBackFaces(false);
	cWide.Init(puiWide, 128, 64, 128);
	cTile.Init(puiTile, 4, 64, 4);

	uiRandom = 12345;
	iDifferent = 0;
	iDrawn = 0;
	for (iTriangle = 0; iTriangle < 500; iTriangle++)
	{
		for (i = 0; i < 6; i++)
		{
			uiRandom = uiRandom * 1103515245 + 12345;
			iShift = (i & 1) ? (64 * SRasterSubpixel::FromInt(1).miRaw) : (128 * SRasterSubpixel::FromInt(1).miRaw);
			aiCorner[i] = (int32)((uiRandom >> 8) % (uint32)(iShift * 2)) - iShift / 2;
		}

		//Some start as long thin ones: the third corner close to the second.
		if (iTriangle % 3 == 0)
		{
			aiCorner[4] = aiCorner[2] + (aiCorner[5] % 32);
			aiCorner[5] = aiCorner[3] + (aiCorner[4] % 32);
		}

		cWide.Clear(TEST_BLACK);
		DrawTestTriangle(&cRasteriser, &cWide, aiCorner[0], aiCorner[1], aiCorner[2], aiCorner[3], aiCorner[4], aiCorner[5], 100, TEST_RED);
		iDrawn += 128 * 64 - CountPixels(&cWide, TEST_BLACK);

		for (iTile = 0; iTile < 32; iTile++)
		{
			iShift = iTile * 4 * SRasterSubpixel::FromInt(1).miRaw;
			cTile.Clear(TEST_BLACK);
			DrawTestTriangle(&cRasteriser, &cTile, aiCorner[0] - iShift, aiCorner[1], aiCorner[2] - iShift, aiCorner[3], aiCorner[4] - iShift, aiCorner[5], 100, TEST_RED);
			for (y = 0; y < 64; y++)
			{
				for (x = 0; x < 4; x++)
				{
					iDifferent += puiTile[y * 4 + x] != puiWide[y * 128 + iTile * 4 + x];
				}
			}
		}
	}
	AssertInt(0, iDifferent);
	AssertTrue(iDrawn > 100000);

	cTile.Kill();
	cWide.Kill();
	cRasteriser.Kill();
	free(puiTile);
	free(puiWide);
}


//////////////////////////////////////////////////////////////////////////
//
// One mesh, held once, drawn twice.  Left where it is an instance draws what
// the mesh does.  Moved it appears there too, in the same picture whichever
// is drawn first as the depths sort them out.  Turned, it faces the way it
// was turned.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserInstances(void)
{
	CMesh			cMesh;
	CRasterMesh		cRaster;
	CRasterCamera	cCamera;
	CRasteriser		cRasteriser;
	CRasterTarget	cTarget;
	SRasterInstance	asInstances[2];
	SRasterInstance	sSwap;
	uint32*			puiFirst;
	uint32*			puiSecond;
	int				iOne;
	int				iTwo;
	bool			bSame;

	cMesh.Init();
	cMesh.mcNormals.mbInUse = true;

	//A square facing +Z at the origin.
	cMesh.AddCorner(-0.25f, -0.25f, 0.0f);
	cMesh.AddCorner(0.25f, -0.25f, 0.0f);
	cMesh.AddCorner(0.25f, 0.25f, 0.0f);
	cMesh.AddCorner(-0.25f, 0.25f, 0.0f);
	cMesh.AddFace(0, 1, 2);
	cMesh.AddFace(0, 2, 3);

	cRaster.Init();
	BuildRasterMesh(&cRaster, &cMesh, 1 << 20);

	puiFirst = (uint32*)malloc(64 * 64 * sizeof(uint32));
	puiSecond = (uint32*)malloc(64 * 64 * sizeof(uint32));
	cRasteriser.Init();
	cCamera.Init();
	cCamera.Fit(64, 64);

	//An instance that hasn't moved is the mesh.
	cTarget.Init(puiFirst, 64, 64, 64);
	cTarget.Clear(TEST_BLACK);
	cRasteriser.Draw(&cRaster, &cCamera, &cTarget);
	iOne = 64 * 64 - CountPixels(&cTarget, TEST_BLACK);
	AssertTrue(iOne > 0);
	cTarget.Kill();

	asInstances[0].pcMesh = &cRaster;
	asInstances[0].asAxes[0].Init(1 << RASTER_AXIS_BITS, 0, 0);
	asInstances[0].asAxes[1].Init(0, 1 << RASTER_AXIS_BITS, 0);
	asInstances[0].asAxes[2].Init(0, 0, 1 << RASTER_AXIS_BITS);
	asInstances[0].sOffset.Init(0, 0, 0);

	cTarget.Init(puiSecond, 64, 64, 64);
	cTarget.Clear(TEST_BLACK);
	cRasteriser.Draw(asInstances, 1, &cCamera, &cTarget);
	bSame = memcmp(puiFirst, puiSecond, 64 * 64 * sizeof(uint32)) == 0;
	AssertTrue(bSame);
	cTarget.Kill();

	//The same mesh again, overlapping it, moved right and nearer the eye.  They cover more than one does, and it makes no
	//difference which is drawn first.
	asInstances[1] = asInstances[0];
	asInstances[1].sOffset.Init(1 << 18, 0, 1 << 20);
	cTarget.Init(puiFirst, 64, 64, 64);
	cTarget.Clear(TEST_BLACK);
	cRasteriser.Draw(asInstances, 2, &cCamera, &cTarget);
	iTwo = 64 * 64 - CountPixels(&cTarget, TEST_BLACK);
	AssertTrue(iTwo > iOne);
	cTarget.Kill();

	sSwap = asInstances[0];
	asInstances[0] = asInstances[1];
	asInstances[1] = sSwap;
	cTarget.Init(puiSecond, 64, 64, 64);
	cTarget.Clear(TEST_BLACK);
	cRasteriser.Draw(asInstances, 2, &cCamera, &cTarget);
	bSame = memcmp(puiFirst, puiSecond, 64 * 64 * sizeof(uint32)) == 0;
	AssertTrue(bSame);

	//A quarter turn around X makes the square edge on: nothing is drawn.
	asInstances[0] = sSwap;
	asInstances[0].asAxes[1].Init(0, 0, -(1 << RASTER_AXIS_BITS));
	asInstances[0].asAxes[2].Init(0, 1 << RASTER_AXIS_BITS, 0);
	cTarget.Clear(TEST_BLACK);
	cRasteriser.Draw(asInstances, 1, &cCamera, &cTarget);
	AssertInt(64 * 64, CountPixels(&cTarget, TEST_BLACK));

	//Half a turn around Y faces it away from the eye, and it is culled.
	asInstances[0].asAxes[0].Init(-(1 << RASTER_AXIS_BITS), 0, 0);
	asInstances[0].asAxes[1].Init(0, 1 << RASTER_AXIS_BITS, 0);
	asInstances[0].asAxes[2].Init(0, 0, -(1 << RASTER_AXIS_BITS));
	cTarget.Clear(TEST_BLACK);
	cRasteriser.Draw(asInstances, 1, &cCamera, &cTarget);
	AssertInt(64 * 64, CountPixels(&cTarget, TEST_BLACK));
	cTarget.Kill();

	cRasteriser.Kill();
	free(puiSecond);
	free(puiFirst);
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How far a sample is inside an edge, with what the top-left rule takes off.
//
//////////////////////////////////////////////////////////////////////////
static int64 TestReferenceEdge(SRasterScreenVertex* psFrom, SRasterScreenVertex* psTo, int64 iX, int64 iY, int64* piBias)
{
	int64	iDeltaX;
	int64	iDeltaY;

	iDeltaX = (int64)psTo->sPosition.x.miRaw - psFrom->sPosition.x.miRaw;
	iDeltaY = (int64)psTo->sPosition.y.miRaw - psFrom->sPosition.y.miRaw;
	*piBias = ((iDeltaY < 0) || ((iDeltaY == 0) && (iDeltaX > 0))) ? 0 : -1;
	return iDeltaX * (iY - psFrom->sPosition.y.miRaw) - iDeltaY * (iX - psFrom->sPosition.x.miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
// A front facing triangle of under 16 square pixels of doubled area, tested
// and weighted a pixel at a time by the rules of DrawTriangle.  False if the
// triangle isn't one of those.
//
//////////////////////////////////////////////////////////////////////////
static bool TestReferenceSmallTriangle(CRasterTarget* pcTarget, SRasterScreenVertex* psA, SRasterScreenVertex* psB, SRasterScreenVertex* psC, uint32 uiColour, bool bTransparent)
{
	SRasterScreenVertex*	psSwap;
	int64					iArea;
	int64					iBias0;
	int64					iBias1;
	int64					iBias2;
	int64					i0;
	int64					i1;
	int64					i2;
	int32					iDepth;
	int32					iShade;
	uint32					uiRB;
	uint32					uiG;
	uint32					uiAlpha;
	int						x;
	int						y;

	iArea = TestReferenceEdge(psA, psB, psC->sPosition.x.miRaw, psC->sPosition.y.miRaw, &iBias0);
	if ((iArea >= 0) || (-iArea >= (16 << 8)))
	{
		return false;
	}
	psSwap = psB;
	psB = psC;
	psC = psSwap;
	iArea = -iArea;

	for (y = 0; y < pcTarget->miHeight; y++)
	{
		for (x = 0; x < pcTarget->miWidth; x++)
		{
			i0 = TestReferenceEdge(psB, psC, x * 16 + 8, y * 16 + 8, &iBias0);
			i1 = TestReferenceEdge(psC, psA, x * 16 + 8, y * 16 + 8, &iBias1);
			i2 = TestReferenceEdge(psA, psB, x * 16 + 8, y * 16 + 8, &iBias2);
			if ((i0 + iBias0 >= 0) && (i1 + iBias1 >= 0) && (i2 + iBias2 >= 0))
			{
				iDepth = (int32)DivideFloor(i0 * psA->iDepth + i1 * psB->iDepth + i2 * psC->iDepth, iArea);
				if (iDepth > pcTarget->mpiDepths[y * pcTarget->miWidth + x])
				{
					iShade = (int32)DivideFloor(i0 * psA->iShade + i1 * psB->iShade + i2 * psC->iShade, iArea);
					uiRB = (((uiColour & 0x00FF00FF) * (uint32)iShade) >> 8) & 0x00FF00FF;
					uiG = (((uiColour & 0x0000FF00) * (uint32)iShade) >> 8) & 0x0000FF00;
					if (bTransparent)
					{
						uiAlpha = uiColour >> 24;
						uiAlpha += uiAlpha >> 7;
						uiRB = ((uiRB * uiAlpha + (pcTarget->mpuiPixels[y * pcTarget->miStride + x] & 0x00FF00FF) * (256 - uiAlpha)) >> 8) & 0x00FF00FF;
						uiG = ((uiG * uiAlpha + (pcTarget->mpuiPixels[y * pcTarget->miStride + x] & 0x0000FF00) * (256 - uiAlpha)) >> 8) & 0x0000FF00;
					}
					else
					{
						pcTarget->mpiDepths[y * pcTarget->miWidth + x] = iDepth;
					}
					pcTarget->mpuiPixels[y * pcTarget->miStride + x] = 0xFF000000 | uiRB | uiG;
				}
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Small triangles, which AVX2 builds test a row at a time, draw what each
// pixel tested on its own does.  Up against the target's sides too, and long
// thin ones whose boxes are too big for a row at a time.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserSmallTriangles(void)
{
	static const int	aiSpreads[5] = { 32, 64, 128, 256, 640 };

	CRasteriser			cRasteriser;
	CRasterTarget		cActual;
	CRasterTarget		cExpected;
	SRasterScreenVertex	asCorners[3];
	uint32*				puiActual;
	uint32*				puiExpected;
	uint32				uiRandom;
	uint32				uiColour;
	bool				bTransparent;
	int32				iBaseX;
	int32				iBaseY;
	int32				iSpread;
	int					iTriangle;
	int					iCorner;
	int					iDifferent;
	int					iDrawn;

	puiActual = (uint32*)malloc(32 * 24 * sizeof(uint32));
	puiExpected = (uint32*)malloc(32 * 24 * sizeof(uint32));
	cRasteriser.Init();
	cActual.Init(puiActual, 32, 24, 32);
	cExpected.Init(puiExpected, 32, 24, 32);

	uiRandom = 2024;
	iDifferent = 0;
	iDrawn = 0;
	for (iTriangle = 0; iTriangle < 20000; iTriangle++)
	{
		uiRandom = uiRandom * 1103515245 + 12345;
		iSpread = aiSpreads[(uiRandom >> 16) % 5];
		uiRandom = uiRandom * 1103515245 + 12345;
		iBaseX = (int32)((uiRandom >> 8) % (34 * 16)) - 16;
		uiRandom = uiRandom * 1103515245 + 12345;
		iBaseY = (int32)((uiRandom >> 8) % (26 * 16)) - 16;
		for (iCorner = 0; iCorner < 3; iCorner++)
		{
			int32	iX;
			int32	iY;

			uiRandom = uiRandom * 1103515245 + 12345;
			iX = iBaseX + (int32)((uiRandom >> 8) % (uint32)iSpread);
			uiRandom = uiRandom * 1103515245 + 12345;
			iY = iBaseY + (int32)((uiRandom >> 8) % (uint32)iSpread);
			uiRandom = uiRandom * 1103515245 + 12345;
			InitScreenVertex(&asCorners[iCorner], iX, iY, 1 + (int32)((uiRandom >> 4) % 100000), (int32)((uiRandom >> 20) % 257));
		}
		uiRandom = uiRandom * 1103515245 + 12345;
		uiColour = 0xFF000000 | (uiRandom >> 8);
		bTransparent = ((uiRandom >> 3) & 3) == 0;
		if (bTransparent)
		{
			uiColour = 0x80000000 | (uiColour & 0x00FFFFFF);
		}

		cActual.Clear(TEST_BLACK);
		cExpected.Clear(TEST_BLACK);
		if (TestReferenceSmallTriangle(&cExpected, &asCorners[0], &asCorners[1], &asCorners[2], uiColour, bTransparent))
		{
			cRasteriser.DrawTriangle(&cActual, &asCorners[0], &asCorners[1], &asCorners[2], uiColour, bTransparent);
			iDifferent += memcmp(puiActual, puiExpected, 32 * 24 * sizeof(uint32)) != 0;
			iDifferent += memcmp(cActual.mpiDepths, cExpected.mpiDepths, 32 * 24 * sizeof(int32)) != 0;
			iDrawn += 32 * 24 - CountPixels(&cActual, TEST_BLACK);
		}
	}
	AssertInt(0, iDifferent);
	AssertTrue(iDrawn > 1000);

	cExpected.Kill();
	cActual.Kill();
	cRasteriser.Kill();
	free(puiExpected);
	free(puiActual);
}


//////////////////////////////////////////////////////////////////////////
//
// A square facing +Z in a mesh, in a list of its triangles.
//
//////////////////////////////////////////////////////////////////////////
static void TestRasterAddSquare(CRasterMesh* pcRaster, CArrayRasterTriangle* pasTriangles, int32 x, int32 y, int32 z, int32 iHalf, uint32 uiColour)
{
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	int32				iFirst;
	int					i;

	iFirst = (int32)pcRaster->masVertices.NumElements();
	for (i = 0; i < 4; i++)
	{
		psVertex = pcRaster->masVertices.Add();
		psVertex->sPosition.Init(x + (((i == 1) || (i == 2)) ? iHalf : -iHalf), y + ((i >= 2) ? iHalf : -iHalf), z);
		psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromInt(1));
	}
	for (i = 0; i < 2; i++)
	{
		psTriangle = pasTriangles->Add();
		psTriangle->aiVertex[0] = iFirst;
		psTriangle->aiVertex[1] = iFirst + i + 1;
		psTriangle->aiVertex[2] = iFirst + i + 2;
		psTriangle->uiColour = uiColour;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Instances with opaque and transparent triangles draw the same as those
// triangles a list at a time, all the opaque before any transparent.  Each
// instance's transparent triangles have to find its own vertices, whichever
// meshes came before it.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserInstancesTransparent(void)
{
	CRasterMesh		acWhole[2];
	CRasterMesh		acOpaque[2];
	CRasterMesh		acTransparent[2];
	CRasterCamera	cCamera;
	CRasteriser		cRasteriser;
	CRasterTarget	cTogether;
	CRasterTarget	cApart;
	SRasterInstance	asInstances[2];
	SRasterInstance	sInstance;
	uint32*			puiTogether;
	uint32*			puiApart;
	int				i;
	int				iFrame;
	bool			bSame;

	for (i = 0; i < 2; i++)
	{
		acWhole[i].Init();
		acOpaque[i].Init();
		acTransparent[i].Init();
	}

	//A mesh with more vertices first, so that the second's vertices aren't where the first's are.
	TestRasterAddSquare(&acWhole[0], &acWhole[0].masOpaque, -(1 << 18), 0, 0, 1 << 18, 0xFF00FF00);
	TestRasterAddSquare(&acWhole[0], &acWhole[0].masOpaque, -(1 << 18), 1 << 19, 0, 1 << 17, 0xFFFFFF00);
	TestRasterAddSquare(&acWhole[0], &acWhole[0].masTransparent, -(1 << 17), 1 << 16, 1 << 19, 1 << 18, 0x8000FFFF);
	TestRasterAddSquare(&acOpaque[0], &acOpaque[0].masOpaque, -(1 << 18), 0, 0, 1 << 18, 0xFF00FF00);
	TestRasterAddSquare(&acOpaque[0], &acOpaque[0].masOpaque, -(1 << 18), 1 << 19, 0, 1 << 17, 0xFFFFFF00);
	TestRasterAddSquare(&acTransparent[0], &acTransparent[0].masTransparent, -(1 << 17), 1 << 16, 1 << 19, 1 << 18, 0x8000FFFF);

	TestRasterAddSquare(&acWhole[1], &acWhole[1].masTransparent, 1 << 18, 0, 1 << 20, 1 << 18, 0x80FF00FF);
	TestRasterAddSquare(&acWhole[1], &acWhole[1].masOpaque, 1 << 17, -(1 << 18), 1 << 18, 1 << 18, 0xFFFF0000);
	TestRasterAddSquare(&acOpaque[1], &acOpaque[1].masOpaque, 1 << 17, -(1 << 18), 1 << 18, 1 << 18, 0xFFFF0000);
	TestRasterAddSquare(&acTransparent[1], &acTransparent[1].masTransparent, 1 << 18, 0, 1 << 20, 1 << 18, 0x80FF00FF);

	puiTogether = (uint32*)malloc(64 * 64 * sizeof(uint32));
	puiApart = (uint32*)malloc(64 * 64 * sizeof(uint32));
	cTogether.Init(puiTogether, 64, 64, 64);
	cApart.Init(puiApart, 64, 64, 64);
	cRasteriser.Init();
	cCamera.Init();
	cCamera.Fit(64, 64);

	sInstance.pcMesh = NULL;
	sInstance.asAxes[0].Init(1 << RASTER_AXIS_BITS, 0, 0);
	sInstance.asAxes[1].Init(0, 1 << RASTER_AXIS_BITS, 0);
	sInstance.asAxes[2].Init(0, 0, 1 << RASTER_AXIS_BITS);
	sInstance.sOffset.Init(1 << 16, 0, 0);

	//The second frame draws over what the first left in the screen, and with fewer instances the third.
	for (iFrame = 0; iFrame < 3; iFrame++)
	{
		int	iNumInstances = (iFrame == 2) ? 1 : 2;

		for (i = 0; i < 2; i++)
		{
			asInstances[i] = sInstance;
			asInstances[i].pcMesh = &acWhole[i];
		}
		cTogether.Clear(TEST_BLACK);
		cRasteriser.Draw(asInstances, iNumInstances, &cCamera, &cTogether);

		cApart.Clear(TEST_BLACK);
		for (i = 0; i < iNumInstances; i++)
		{
			asInstances[0].pcMesh = &acOpaque[i];
			cRasteriser.Draw(asInstances, 1, &cCamera, &cApart);
		}
		for (i = 0; i < iNumInstances; i++)
		{
			asInstances[0].pcMesh = &acTransparent[i];
			cRasteriser.Draw(asInstances, 1, &cCamera, &cApart);
		}

		AssertTrue(CountPixels(&cTogether, TEST_BLACK) < 64 * 64);
		bSame = memcmp(puiTogether, puiApart, 64 * 64 * sizeof(uint32)) == 0;
		AssertTrue(bSame);
		bSame = memcmp(cTogether.mpiDepths, cApart.mpiDepths, 64 * 64 * sizeof(int32)) == 0;
		AssertTrue(bSame);
	}

	cRasteriser.Kill();
	cApart.Kill();
	cTogether.Kill();
	free(puiApart);
	free(puiTogether);
	for (i = 0; i < 2; i++)
	{
		acTransparent[i].Kill();
		acOpaque[i].Kill();
		acWhole[i].Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh draws what its triangles do one at a time through DrawTriangle:
// whichever way they face, with none of their area, or off the target.  With
// no normals every vertex has just the ambient shade.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserTurnedAway(void)
{
	CRasterMesh				cRaster;
	CRasterCamera			cCamera;
	CRasteriser				cRasteriser;
	CRasterTarget			cMesh;
	CRasterTarget			cOneByOne;
	SRasterVertex*			psVertex;
	SRasterTriangle*		psTriangle;
	SRasterScreenVertex*	psScreen;
	uint32*					puiMesh;
	uint32*					puiOneByOne;
	uint32					uiRandom;
	int						i;
	int						iCorner;
	int						iCull;
	bool					bSame;

	cRaster.Init();
	uiRandom = 777;
	for (i = 0; i < 400; i++)
	{
		psVertex = cRaster.masVertices.Add();
		uiRandom = uiRandom * 1103515245 + 12345;
		psVertex->sPosition.x = (int32)((uiRandom >> 8) % (RASTER_MESH_HALF_EXTENT * 4)) - RASTER_MESH_HALF_EXTENT * 2;
		uiRandom = uiRandom * 1103515245 + 12345;
		psVertex->sPosition.y = (int32)((uiRandom >> 8) % (RASTER_MESH_HALF_EXTENT * 4)) - RASTER_MESH_HALF_EXTENT * 2;
		uiRandom = uiRandom * 1103515245 + 12345;
		psVertex->sPosition.z = (int32)((uiRandom >> 8) % RASTER_MESH_HALF_EXTENT) - RASTER_MESH_HALF_EXTENT / 2;
		psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0));
	}
	for (i = 0; i < 1500; i++)
	{
		psTriangle = (i % 5 == 0) ? cRaster.masTransparent.Add() : cRaster.masOpaque.Add();
		uiRandom = uiRandom * 1103515245 + 12345;
		psTriangle->uiColour = (i % 5 == 0) ? (0x80000000 | (uiRandom >> 8)) : (0xFF000000 | (uiRandom >> 8));
		for (iCorner = 0; iCorner < 3; iCorner++)
		{
			uiRandom = uiRandom * 1103515245 + 12345;
			psTriangle->aiVertex[iCorner] = (int32)((uiRandom >> 8) % 400);
		}

		//Some with the same vertex twice, for no area.
		if (i % 50 == 0)
		{
			psTriangle->aiVertex[1] = psTriangle->aiVertex[0];
		}
	}

	puiMesh = (uint32*)malloc(64 * 48 * sizeof(uint32));
	puiOneByOne = (uint32*)malloc(64 * 48 * sizeof(uint32));
	psScreen = (SRasterScreenVertex*)malloc(400 * sizeof(SRasterScreenVertex));
	cMesh.Init(puiMesh, 64, 48, 64);
	cOneByOne.Init(puiOneByOne, 64, 48, 64);
	cRasteriser.Init();
	cCamera.Init();
	cCamera.Fit(64, 48);

	for (iCull = 0; iCull < 2; iCull++)
	{
		cRasteriser.SetCullBackFaces(iCull == 0);
		cMesh.Clear(TEST_BLACK);
		cRasteriser.Draw(&cRaster, &cCamera, &cMesh);

		cOneByOne.Clear(TEST_BLACK);
		for (i = 0; i < 400; i++)
		{
			cCamera.Project(&psScreen[i], &cRaster.masVertices.Get(i)->sPosition);
			psScreen[i].iShade = 64;
		}
		for (i = 0; i < (int)cRaster.masOpaque.NumElements(); i++)
		{
			psTriangle = cRaster.masOpaque.Get(i);
			cRasteriser.DrawTriangle(&cOneByOne, &psScreen[psTriangle->aiVertex[0]], &psScreen[psTriangle->aiVertex[1]], &psScreen[psTriangle->aiVertex[2]], psTriangle->uiColour, false);
		}
		for (i = 0; i < (int)cRaster.masTransparent.NumElements(); i++)
		{
			psTriangle = cRaster.masTransparent.Get(i);
			cRasteriser.DrawTriangle(&cOneByOne, &psScreen[psTriangle->aiVertex[0]], &psScreen[psTriangle->aiVertex[1]], &psScreen[psTriangle->aiVertex[2]], psTriangle->uiColour, true);
		}

		AssertTrue(CountPixels(&cMesh, TEST_BLACK) < 64 * 48);
		bSame = memcmp(puiMesh, puiOneByOne, 64 * 48 * sizeof(uint32)) == 0;
		AssertTrue(bSame);
		bSame = memcmp(cMesh.mpiDepths, cOneByOne.mpiDepths, 64 * 48 * sizeof(int32)) == 0;
		AssertTrue(bSame);
	}

	cRasteriser.Kill();
	cOneByOne.Kill();
	cMesh.Kill();
	free(psScreen);
	free(puiOneByOne);
	free(puiMesh);
	cRaster.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestRasterAddFace(CRasterMesh* pcRaster, int32 i0, int32 i1, int32 i2, uint32 uiColour)
{
	SRasterTriangle*	psTriangle;

	psTriangle = pcRaster->masOpaque.Add();
	psTriangle->aiVertex[0] = i0;
	psTriangle->aiVertex[1] = i1;
	psTriangle->aiVertex[2] = i2;
	psTriangle->uiColour = uiColour;
}


//////////////////////////////////////////////////////////////////////////
//
// An instance turned by quarter turns draws what the mesh does with its
// positions and normals turned by hand, for a mesh with runs of the same
// normal and changes between them.  An instance at any other angle moves its
// vertices as its axes say, rounding down: with no normals only the positions
// can differ.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserInstanceTurns(void)
{
	static const int32	aaiTurns[6][9] =
	{
		{ 1, 0, 0,   0, 1, 0,   0, 0, 1 },
		{ 0, 0, 1,   0, 1, 0,  -1, 0, 0 },
		{ 1, 0, 0,   0, 0, -1,  0, 1, 0 },
		{ 0, -1, 0,  1, 0, 0,   0, 0, 1 },
		{ -1, 0, 0,  0, -1, 0,  0, 0, 1 },
		{ 0, 1, 0,   0, 0, 1,   1, 0, 0 },
	};

	CRasterMesh		cRaster;
	CRasterMesh		cTurned;
	CRasterCamera	cCamera;
	CRasteriser		cRasteriser;
	CRasterTarget	cActual;
	CRasterTarget	cExpected;
	SRasterInstance	sInstance;
	SRasterVertex*	psFrom;
	SRasterVertex*	psTo;
	uint32*			puiActual;
	uint32*			puiExpected;
	uint32			uiRandom;
	int64			aiAxis[3];
	int32			iNormal[3];
	int				iTurn;
	int				iAxis;
	int				i;
	bool			bSame;

	cRaster.Init();
	uiRandom = 4242;
	for (i = 0; i < 300; i++)
	{
		psFrom = cRaster.masVertices.Add();
		uiRandom = uiRandom * 1103515245 + 12345;
		psFrom->sPosition.x = (int32)((uiRandom >> 8) % (1 << 20)) - (1 << 19);
		uiRandom = uiRandom * 1103515245 + 12345;
		psFrom->sPosition.y = (int32)((uiRandom >> 8) % (1 << 20)) - (1 << 19);
		uiRandom = uiRandom * 1103515245 + 12345;
		psFrom->sPosition.z = (int32)((uiRandom >> 8) % (1 << 19)) - (1 << 18);

		//Five vertices a normal, and each part of it a different size.
		iNormal[0] = (((i / 5) % 7) - 3) * 1000;
		iNormal[1] = (((i / 5) % 3) - 1) * 3000;
		iNormal[2] = 9000 - ((i / 5) % 4) * 1000;
		psFrom->sNormal.Init(SRasterUnit::FromRaw((int16)iNormal[0]), SRasterUnit::FromRaw((int16)iNormal[1]), SRasterUnit::FromRaw((int16)iNormal[2]));
	}
	for (i = 0; i < 300; i += 3)
	{
		TestRasterAddFace(&cRaster, i, i + 1, i + 2, 0xFF80C040);
	}

	puiActual = (uint32*)malloc(64 * 48 * sizeof(uint32));
	puiExpected = (uint32*)malloc(64 * 48 * sizeof(uint32));
	cActual.Init(puiActual, 64, 48, 64);
	cExpected.Init(puiExpected, 64, 48, 64);
	cRasteriser.Init();
	cRasteriser.SetCullBackFaces(false);
	cCamera.Init();
	cCamera.Fit(64, 48);

	for (iTurn = 0; iTurn < 6; iTurn++)
	{
		cTurned.Init();
		sInstance.pcMesh = &cRaster;
		sInstance.sOffset.Init(1 << 15, -(1 << 16), 1 << 14);
		for (iAxis = 0; iAxis < 3; iAxis++)
		{
			sInstance.asAxes[iAxis].Init(aaiTurns[iTurn][iAxis * 3] << RASTER_AXIS_BITS, aaiTurns[iTurn][iAxis * 3 + 1] << RASTER_AXIS_BITS, aaiTurns[iTurn][iAxis * 3 + 2] << RASTER_AXIS_BITS);
		}
		for (i = 0; i < 300; i++)
		{
			psFrom = cRaster.masVertices.Get(i);
			psTo = cTurned.masVertices.Add();
			for (iAxis = 0; iAxis < 3; iAxis++)
			{
				aiAxis[iAxis] = aaiTurns[iTurn][iAxis * 3] * psFrom->sPosition.x + aaiTurns[iTurn][iAxis * 3 + 1] * psFrom->sPosition.y + aaiTurns[iTurn][iAxis * 3 + 2] * psFrom->sPosition.z;
				iNormal[iAxis] = aaiTurns[iTurn][iAxis * 3] * psFrom->sNormal.x.miRaw + aaiTurns[iTurn][iAxis * 3 + 1] * psFrom->sNormal.y.miRaw + aaiTurns[iTurn][iAxis * 3 + 2] * psFrom->sNormal.z.miRaw;
			}
			psTo->sPosition.Init((int32)aiAxis[0] + sInstance.sOffset.x, (int32)aiAxis[1] + sInstance.sOffset.y, (int32)aiAxis[2] + sInstance.sOffset.z);
			psTo->sNormal.Init(SRasterUnit::FromRaw((int16)iNormal[0]), SRasterUnit::FromRaw((int16)iNormal[1]), SRasterUnit::FromRaw((int16)iNormal[2]));
		}
		for (i = 0; i < 300; i += 3)
		{
			TestRasterAddFace(&cTurned, i, i + 1, i + 2, 0xFF80C040);
		}

		cActual.Clear(TEST_BLACK);
		cRasteriser.Draw(&sInstance, 1, &cCamera, &cActual);
		cExpected.Clear(TEST_BLACK);
		cRasteriser.Draw(&cTurned, &cCamera, &cExpected);
		AssertTrue(CountPixels(&cActual, TEST_BLACK) < 64 * 48);
		bSame = memcmp(puiActual, puiExpected, 64 * 48 * sizeof(uint32)) == 0;
		AssertTrue(bSame);
		bSame = memcmp(cActual.mpiDepths, cExpected.mpiDepths, 64 * 48 * sizeof(int32)) == 0;
		AssertTrue(bSame);
		cTurned.Kill();
	}

	//About 30 degrees around z, as no quarter turn is.
	for (i = 0; i < 300; i++)
	{
		cRaster.masVertices.Get(i)->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0));
	}
	sInstance.asAxes[0].Init(56756, -32768, 0);
	sInstance.asAxes[1].Init(32768, 56756, 0);
	sInstance.asAxes[2].Init(0, 0, 65536);
	cTurned.Init();
	for (i = 0; i < 300; i++)
	{
		psFrom = cRaster.masVertices.Get(i);
		psTo = cTurned.masVertices.Add();
		psTo->sPosition.Init((int32)DivideFloor((int64)psFrom->sPosition.x * 56756 - (int64)psFrom->sPosition.y * 32768, 65536) + sInstance.sOffset.x,
							 (int32)DivideFloor((int64)psFrom->sPosition.x * 32768 + (int64)psFrom->sPosition.y * 56756, 65536) + sInstance.sOffset.y,
							 psFrom->sPosition.z + sInstance.sOffset.z);
		psTo->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0));
	}
	for (i = 0; i < 300; i += 3)
	{
		TestRasterAddFace(&cTurned, i, i + 1, i + 2, 0xFF80C040);
	}
	cActual.Clear(TEST_BLACK);
	cRasteriser.Draw(&sInstance, 1, &cCamera, &cActual);
	cExpected.Clear(TEST_BLACK);
	cRasteriser.Draw(&cTurned, &cCamera, &cExpected);
	AssertTrue(CountPixels(&cActual, TEST_BLACK) < 64 * 48);
	bSame = memcmp(puiActual, puiExpected, 64 * 48 * sizeof(uint32)) == 0;
	AssertTrue(bSame);
	cTurned.Kill();

	cRasteriser.Kill();
	cExpected.Kill();
	cActual.Kill();
	free(puiExpected);
	free(puiActual);
	cRaster.Kill();
}


//Jobs run last first, when asked to wait, to show that nothing depends on the order they run in.
class CTestBackwardsJobs : public CJobs
{
public:
	JobFunction	mafJobs[1024];
	void*		mapvData[1024];
	int			miNumJobs;
	int			miRun;

	void	Init(void) { miNumJobs = 0; miRun = 0; }
	void	Run(JobFunction fJob, void* pvData) override { mafJobs[miNumJobs] = fJob; mapvData[miNumJobs] = pvData; miNumJobs++; }
	void	Wait(void) override
	{
		while (miNumJobs > 0)
		{
			miNumJobs--;
			miRun++;
			mafJobs[miNumJobs](mapvData[miNumJobs]);
		}
	}
	int		NumThreads(void) override { return 8; }
};


//////////////////////////////////////////////////////////////////////////
//
// Instances draw the same bytes with jobs as without, in a mesh big enough
// for several jobs of each kind, with triangles across many bands and depths
// fighting, and transparent ones over them.  Jobs that can't run at the same
// time draw as they do without jobs.
//
//////////////////////////////////////////////////////////////////////////
void TestRasteriserJobs(void)
{
	CRasterMesh			acRaster[2];
	CRasterCamera		cCamera;
	CRasteriser			cRasteriser;
	CRasterTarget		cWithout;
	CRasterTarget		cWith;
	SRasterInstance		asInstances[3];
	CTestBackwardsJobs	cJobs;
	CSerialJobs			cSerial;
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	uint32*				puiWithout;
	uint32*				puiWith;
	uint32				uiRandom;
	int					iMesh;
	int					iNumVertices;
	int					iNumTriangles;
	int					i;
	int					iCorner;
	int					iCull;
	bool				bSame;

	uiRandom = 31337;
	for (iMesh = 0; iMesh < 2; iMesh++)
	{
		iNumVertices = (iMesh == 0) ? 70000 : 300;
		iNumTriangles = (iMesh == 0) ? 80000 : 500;
		acRaster[iMesh].Init();
		for (i = 0; i < iNumVertices; i++)
		{
			psVertex = acRaster[iMesh].masVertices.Add();
			uiRandom = uiRandom * 1103515245 + 12345;
			psVertex->sPosition.x = (int32)((uiRandom >> 8) % (RASTER_MESH_HALF_EXTENT * 2)) - RASTER_MESH_HALF_EXTENT;
			uiRandom = uiRandom * 1103515245 + 12345;
			psVertex->sPosition.y = (int32)((uiRandom >> 8) % (RASTER_MESH_HALF_EXTENT * 2)) - RASTER_MESH_HALF_EXTENT;
			uiRandom = uiRandom * 1103515245 + 12345;
			psVertex->sPosition.z = (int32)((uiRandom >> 8) % RASTER_MESH_HALF_EXTENT) - RASTER_MESH_HALF_EXTENT / 2;

			//Each three vertices close together, for small triangles.
			if ((i % 3) != 0)
			{
				psVertex->sPosition.x = acRaster[iMesh].masVertices.Get(i - 1)->sPosition.x + (psVertex->sPosition.x >> 5);
				psVertex->sPosition.y = acRaster[iMesh].masVertices.Get(i - 1)->sPosition.y + (psVertex->sPosition.y >> 5);
				psVertex->sPosition.z = acRaster[iMesh].masVertices.Get(i - 1)->sPosition.z;
			}
			psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromInt(1));
		}
		for (i = 0; i < iNumTriangles; i++)
		{
			psTriangle = (i % 4 == 0) ? acRaster[iMesh].masTransparent.Add() : acRaster[iMesh].masOpaque.Add();
			uiRandom = uiRandom * 1103515245 + 12345;
			psTriangle->uiColour = (i % 4 == 0) ? (0x80000000 | (uiRandom >> 8)) : (0xFF000000 | (uiRandom >> 8));
			uiRandom = uiRandom * 1103515245 + 12345;
			if ((i % 8) != 0)
			{
				psTriangle->aiVertex[0] = (int32)((uiRandom >> 8) % (uint32)(iNumVertices / 3)) * 3;
				psTriangle->aiVertex[1] = psTriangle->aiVertex[0] + 1;
				psTriangle->aiVertex[2] = psTriangle->aiVertex[0] + 2;
			}
			else
			{
				//Big ones, across many bands.
				for (iCorner = 0; iCorner < 3; iCorner++)
				{
					uiRandom = uiRandom * 1103515245 + 12345;
					psTriangle->aiVertex[iCorner] = (int32)((uiRandom >> 8) % (uint32)iNumVertices);
				}
			}
		}
	}

	puiWithout = (uint32*)malloc(200 * 150 * sizeof(uint32));
	puiWith = (uint32*)malloc(200 * 150 * sizeof(uint32));
	cWithout.Init(puiWithout, 200, 150, 200);
	cWith.Init(puiWith, 200, 150, 200);
	cRasteriser.Init();
	cCamera.Init();
	cCamera.Fit(200, 150);

	asInstances[0].pcMesh = &acRaster[1];
	asInstances[0].asAxes[0].Init(1 << RASTER_AXIS_BITS, 0, 0);
	asInstances[0].asAxes[1].Init(0, 1 << RASTER_AXIS_BITS, 0);
	asInstances[0].asAxes[2].Init(0, 0, 1 << RASTER_AXIS_BITS);
	asInstances[0].sOffset.Init(0, 0, 1 << 16);
	asInstances[1] = asInstances[0];
	asInstances[1].pcMesh = &acRaster[0];
	asInstances[1].sOffset.Init(1 << 15, -(1 << 15), 0);
	asInstances[2] = asInstances[0];
	asInstances[2].sOffset.Init(-(1 << 17), 1 << 16, 1 << 17);

	for (iCull = 0; iCull < 2; iCull++)
	{
		cRasteriser.SetCullBackFaces(iCull == 0);

		cRasteriser.SetJobs(NULL);
		cWithout.Clear(TEST_BLACK);
		cRasteriser.Draw(asInstances, 3, &cCamera, &cWithout);
		AssertTrue(CountPixels(&cWithout, TEST_BLACK) < 200 * 150 - 1000);

		cJobs.Init();
		cRasteriser.SetJobs(&cJobs);
		cWith.Clear(TEST_BLACK);
		cRasteriser.Draw(asInstances, 3, &cCamera, &cWith);
		AssertTrue(cJobs.miRun > 10);
		bSame = memcmp(puiWithout, puiWith, 200 * 150 * sizeof(uint32)) == 0;
		AssertTrue(bSame);
		bSame = memcmp(cWithout.mpiDepths, cWith.mpiDepths, 200 * 150 * sizeof(int32)) == 0;
		AssertTrue(bSame);

		//One job at a time is no reason to use them.
		cRasteriser.SetJobs(&cSerial);
		cWith.Clear(TEST_BLACK);
		cRasteriser.Draw(asInstances, 3, &cCamera, &cWith);
		bSame = memcmp(puiWithout, puiWith, 200 * 150 * sizeof(uint32)) == 0;
		AssertTrue(bSame);
	}

	cRasteriser.Kill();
	cWith.Kill();
	cWithout.Kill();
	free(puiWith);
	free(puiWithout);
	acRaster[1].Kill();
	acRaster[0].Kill();
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
	TestRasteriserWideTriangles();
	TestRasteriserSmallTriangles();
	TestRasteriserInstances();
	TestRasteriserInstancesTransparent();
	TestRasteriserJobs();
	TestRasteriserInstanceTurns();
	TestRasteriserTurnedAway();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

