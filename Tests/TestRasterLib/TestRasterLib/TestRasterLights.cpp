#include <stdlib.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/NumberControl.h"
#include "StandardLib/Objects.h"
#include "RasterLib/Rasteriser.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterLightsSquareRoot(void)
{
	AssertInt(0, RasterSquareRoot(0));
	AssertInt(1, RasterSquareRoot(1));
	AssertInt(1, RasterSquareRoot(3));
	AssertInt(2, RasterSquareRoot(4));
	AssertInt(3, RasterSquareRoot(15));
	AssertInt(4, RasterSquareRoot(16));
	AssertInt(1 << 30, RasterSquareRoot((uint64)1 << 60));
	AssertInt((1 << 30) - 1, RasterSquareRoot(((uint64)1 << 60) - 1));
	AssertIntHex((int32)0xFFFFFFFF, (int32)RasterSquareRoot(0xFFFFFFFFFFFFFFFFULL));
}


//////////////////////////////////////////////////////////////////////////
//
// A point light reaches as far as what it adds is at least a shade, and no
// further than its range.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterLightsReach(void)
{
	CRasterLights	cLights;
	SInt32Vec3		sPosition;

	cLights.Init();
	cLights.Clear();
	AssertInt(0, cLights.miAmbient);
	AssertSize(0, cLights.masDirectional.NumElements());

	sPosition.Init(0, 0, 0);
	AssertTrue(cLights.AddPoint(&sPosition, 100, RLD_Linear, 10, 0));
	AssertTrue(cLights.AddPoint(&sPosition, 100, RLD_Square, 10, 0));
	AssertTrue(cLights.AddPoint(&sPosition, 100, RLD_None, 10, 500));
	AssertTrue(cLights.AddPoint(&sPosition, 100, RLD_None, 10, 0));
	AssertTrue(cLights.AddPoint(&sPosition, 100, RLD_Linear, 10, 300));
	AssertTrue(cLights.AddPoint(&sPosition, 0, RLD_None, 10, 0));
	AssertInt(1001, cLights.masPoint.Get(0)->iReach);
	AssertInt(110, cLights.masPoint.Get(1)->iReach);
	AssertInt(500, cLights.masPoint.Get(2)->iReach);
	AssertInt(RASTER_MAX_REACH, cLights.masPoint.Get(3)->iReach);
	AssertInt(300, cLights.masPoint.Get(4)->iReach);
	AssertInt(0, cLights.masPoint.Get(5)->iReach);
	AssertTrue(1001ULL * 1001ULL == cLights.masPoint.Get(0)->uiReachSquared);
	cLights.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Facing a point light, within where it starts to fall off, is all of it.
// Past that it falls off as it says, and from its reach on it adds nothing,
// as it does facing away.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterLightsPointShade(void)
{
	CRasterLights	cLights;
	SInt32Vec3		sPosition;
	SInt32Vec3		sLight;
	SInt64Vec3		sTowards;
	SInt64Vec3		sAway;

	cLights.Init();
	cLights.Clear();
	sLight.Init(0, 0, 0);
	cLights.AddPoint(&sLight, 100, RLD_Linear, 10, 0);
	cLights.AddPoint(&sLight, 100, RLD_Square, 10, 0);
	sTowards.Init(0, 0, -(1 << SRasterUnit::Bits));
	sAway.Init(0, 0, 1 << SRasterUnit::Bits);

	sPosition.Init(0, 0, 5);
	AssertInt(100, RasterPointShade(cLights.masPoint.Get(0), &sTowards, &sPosition));
	AssertInt(0, RasterPointShade(cLights.masPoint.Get(0), &sAway, &sPosition));
	sPosition.Init(0, 0, 40);
	AssertInt(25, RasterPointShade(cLights.masPoint.Get(0), &sTowards, &sPosition));
	AssertInt(6, RasterPointShade(cLights.masPoint.Get(1), &sTowards, &sPosition));
	sPosition.Init(0, 0, 1000);
	AssertInt(1, RasterPointShade(cLights.masPoint.Get(0), &sTowards, &sPosition));
	sPosition.Init(0, 0, 1001);
	AssertInt(0, RasterPointShade(cLights.masPoint.Get(0), &sTowards, &sPosition));
	sPosition.Init(0, 0, 110);
	AssertInt(0, RasterPointShade(cLights.masPoint.Get(1), &sTowards, &sPosition));
	sPosition.Init(0, 0, -(1 << 30));
	AssertInt(0, RasterPointShade(cLights.masPoint.Get(0), &sTowards, &sPosition));
	cLights.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A white square facing the eye, its middle pixel's shade, through the mesh
// and through an instance, which must agree.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestRasterLightsShade(CRasteriser* pcRasteriser, int32 iOffsetZ)
{
	Ptr<CRasterMesh>	pRaster;
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	SRasterInstance		sInstance;
	CRasterCamera		cCamera;
	CRasterTarget		cTarget;
	uint32*				puiPixels;
	uint32				uiMesh;
	uint32				uiInstance;
	int					i;

	pRaster = OMalloc<CRasterMesh>();
	for (i = 0; i < 4; i++)
	{
		psVertex = pRaster->masVertices.Add();
		psVertex->sPosition.Init(((i == 1) || (i == 2)) ? (1 << 18) : -(1 << 18), (i >= 2) ? (1 << 18) : -(1 << 18), iOffsetZ);
		psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromInt(1));
	}
	for (i = 0; i < 2; i++)
	{
		psTriangle = pRaster->masOpaque.Add();
		psTriangle->aiVertex[0] = 0;
		psTriangle->aiVertex[1] = i + 1;
		psTriangle->aiVertex[2] = i + 2;
		psTriangle->uiColour = 0xFFFFFFFF;
	}
	pRaster->UpdateBounds();

	puiPixels = (uint32*)malloc(64 * 64 * sizeof(uint32));
	cTarget.Init(puiPixels, 64, 64, 64);
	cCamera.Init();
	cCamera.Fit(64, 64);

	cTarget.Clear(0xFF000000);
	pcRasteriser->Draw(&pRaster, &cCamera, &cTarget);
	uiMesh = puiPixels[32 * 64 + 32];

	sInstance.pcMesh = &pRaster;
	sInstance.asAxes[0].Init(1 << RASTER_AXIS_BITS, 0, 0);
	sInstance.asAxes[1].Init(0, 1 << RASTER_AXIS_BITS, 0);
	sInstance.asAxes[2].Init(0, 0, 1 << RASTER_AXIS_BITS);
	sInstance.OnlyTurns();
	sInstance.sOffset.Init(0, 0, 0);
	cTarget.Clear(0xFF000000);
	pcRasteriser->Draw(&sInstance, 1, &cCamera, &cTarget);
	uiInstance = puiPixels[32 * 64 + 32];
	AssertIntHex((int32)uiMesh, (int32)uiInstance);

	cTarget.Kill();
	free(puiPixels);
	return (int32)(uiMesh & 0xFF);
}


//////////////////////////////////////////////////////////////////////////
//
// What white is shaded to.
//
//////////////////////////////////////////////////////////////////////////
static int32 TestRasterLightsWhite(int32 iShade)
{
	return (255 * iShade) >> 8;
}


//////////////////////////////////////////////////////////////////////////
//
// The ambient alone, lights far away adding up, and a point light that only
// lights what it reaches.  A point light is worked out at the square's
// corners, which all get the same.  With lights that add up to more than the
// most, the shade is the most.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterLightsShading(void)
{
	CRasteriser		cRasteriser;
	CRasterLights*	pcLights;
	SRasterUnitVec3	sTowards;
	SInt32Vec3		sLight;
	SInt32Vec3		sCorner;
	SInt64Vec3		sNormal;
	int32			iCorner;

	cRasteriser.Init();
	pcLights = cRasteriser.GetLights();

	pcLights->Clear();
	pcLights->SetAmbient(64);
	AssertInt(TestRasterLightsWhite(64), TestRasterLightsShade(&cRasteriser, 0));

	sTowards.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromInt(1));
	pcLights->Clear();
	AssertTrue(pcLights->AddDirectional(&sTowards, 64, false));
	AssertTrue(pcLights->AddDirectional(&sTowards, 32, false));
	AssertInt(TestRasterLightsWhite(96), TestRasterLightsShade(&cRasteriser, 0));

	//On the eye's side of the square and reaching all of it, then too far to reach it at all.
	pcLights->Clear();
	pcLights->SetAmbient(16);
	sLight.Init(0, 0, 1 << 16);
	pcLights->AddPoint(&sLight, 128, RLD_None, 0, 1 << 20);
	sCorner.Init(1 << 18, 1 << 18, 0);
	sNormal.Init(0, 0, 1 << SRasterUnit::Bits);
	iCorner = RasterPointShade(pcLights->masPoint.Get(0), &sNormal, &sCorner);
	AssertTrue(iCorner > 0);
	AssertInt(TestRasterLightsWhite(16 + iCorner), TestRasterLightsShade(&cRasteriser, 0));
	AssertInt(TestRasterLightsWhite(16), TestRasterLightsShade(&cRasteriser, -(1 << 21)));

	pcLights->SetAmbient(250);
	AssertInt(TestRasterLightsWhite(256), TestRasterLightsShade(&cRasteriser, 0));

	cRasteriser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterLights(void)
{
	BeginTests();

	DataIOInit();
	NumberInit();
	ObjectsInit();

	TestRasterLightsSquareRoot();
	TestRasterLightsReach();
	TestRasterLightsPointShade();
	TestRasterLightsShading();

	ObjectsKill(false);
	NumberKill();
	DataIOKill();

	TestStatistics();
}

