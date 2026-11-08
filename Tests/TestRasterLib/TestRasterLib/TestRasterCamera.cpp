#include "BaseLib/FixedPoint.h"
#include "BaseLib/GeometricTypes.h"
#include "RasterLib/RasterCamera.h"
#include "RasterLib/RasterDivide.h"
#include "RasterLib/RasterMesh.h"
#include "TestLib/Assert.h"


#define TEST_HALF_PI	1.57079632679f


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertProject(int32 iExpectedX, int32 iExpectedY, CRasterCamera* pcCamera, int32 x, int32 y, int32 z)
{
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sScreen;

	sPosition.Init(x, y, z);
	pcCamera->Project(&sScreen, &sPosition);
	AssertInt(iExpectedX, sScreen.sPosition.x.miRaw);
	AssertInt(iExpectedY, sScreen.sPosition.y.miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssertProjectNear(int32 iExpectedX, int32 iExpectedY, CRasterCamera* pcCamera, int32 x, int32 y, int32 z)
{
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sScreen;

	sPosition.Init(x, y, z);
	pcCamera->Project(&sScreen, &sPosition);
	if ((sScreen.sPosition.x.miRaw < iExpectedX - 1) || (sScreen.sPosition.x.miRaw > iExpectedX + 1))
	{
		AssertInt(iExpectedX, sScreen.sPosition.x.miRaw);
	}
	if ((sScreen.sPosition.y.miRaw < iExpectedY - 1) || (sScreen.sPosition.y.miRaw > iExpectedY + 1))
	{
		AssertInt(iExpectedY, sScreen.sPosition.y.miRaw);
	}
	AssertTrue(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraOrthographic(void)
{
	CRasterCamera		cCamera;
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sNear;
	SRasterScreenVertex	sFar;

	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(200, 100);
	cCamera.Prepare(200, 100);

	//A focal length of 120 pixels and an eye four half widths away make a half width 30 pixels, or 480 subpixels.
	//y goes down the screen.
	AssertProject(1600, 800, &cCamera, 0, 0, 0);
	AssertProject(1600 + 480, 800, &cCamera, RASTER_MESH_HALF_EXTENT, 0, 0);
	AssertProject(1600 - 480, 800, &cCamera, -RASTER_MESH_HALF_EXTENT, 0, 0);
	AssertProject(1600, 800 - 480, &cCamera, 0, RASTER_MESH_HALF_EXTENT, 0);

	sPosition.Init(0, 0, 100);
	cCamera.Project(&sNear, &sPosition);
	sPosition.Init(0, 0, -100);
	cCamera.Project(&sFar, &sPosition);
	AssertTrue(sNear.iDepth > sFar.iDepth);
	AssertTrue(sFar.iDepth > 0);

	//A quarter turn takes x to y.
	cCamera.SetOrbit(TEST_HALF_PI, 0.0f);
	cCamera.Prepare(200, 100);
	AssertProject(1600, 800 - 480, &cCamera, RASTER_MESH_HALF_EXTENT, 0, 0);

	//A quarter tilt takes y out of the screen and z down it.
	cCamera.SetOrbit(0.0f, TEST_HALF_PI);
	cCamera.Prepare(200, 100);
	AssertProject(1600, 800, &cCamera, 0, RASTER_MESH_HALF_EXTENT, 0);
	AssertProject(1600, 800 + 480, &cCamera, 0, 0, RASTER_MESH_HALF_EXTENT);

	cCamera.SetOrbit(0.0f, 0.0f);
	cCamera.SetPan(RASTER_MESH_HALF_EXTENT, 0);
	cCamera.Prepare(200, 100);
	AssertProject(1600 + 480, 800, &cCamera, 0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraPerspective(void)
{
	CRasterCamera		cCamera;
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sNear;
	SRasterScreenVertex	sFar;

	cCamera.Init();
	cCamera.SetPerspective(1 << 21, 100);
	cCamera.Prepare(200, 100);

	AssertProject(1600, 800, &cCamera, 0, 0, 0);

	//At the distance of the eye's focus x / distance is exact: half of 100 pixels.
	AssertProject(1600 + 800, 800, &cCamera, 1 << 20, 0, 0);
	AssertProject(1600 - 800, 800, &cCamera, -(1 << 20), 0, 0);
	AssertProject(1600, 0, &cCamera, 0, 1 << 20, 0);

	//Nearer things are further from the centre and have a larger depth.
	sPosition.Init(1 << 20, 0, 1 << 20);
	cCamera.Project(&sNear, &sPosition);
	sPosition.Init(1 << 20, 0, -(1 << 20));
	cCamera.Project(&sFar, &sPosition);
	AssertTrue(sNear.sPosition.x.miRaw > sFar.sPosition.x.miRaw);
	AssertTrue(sNear.iDepth > sFar.iDepth);
	AssertTrue(sFar.iDepth > 0);

	//Behind the near plane is not drawable.
	sPosition.Init(0, 0, 1 << 21);
	cCamera.Project(&sNear, &sPosition);
	AssertInt(0, sNear.iDepth);
	sPosition.Init(0, 0, (1 << 21) + 1000);
	cCamera.Project(&sNear, &sPosition);
	AssertInt(0, sNear.iDepth);

	//Fit keeps a mesh at its largest on the screen.
	cCamera.Fit(100, 100);
	cCamera.Prepare(100, 100);
	sPosition.Init(RASTER_MESH_HALF_EXTENT, 0, RASTER_MESH_HALF_EXTENT);
	cCamera.Project(&sNear, &sPosition);
	AssertTrue(sNear.sPosition.x.miRaw <= SRasterSubpixel::FromInt(100).miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraViewToMesh(void)
{
	CRasterCamera	cCamera;
	SRasterUnitVec3	sView;
	SRasterUnitVec3	sMesh;

	cCamera.Init();
	cCamera.SetOrbit(TEST_HALF_PI, 0.0f);
	cCamera.Prepare(100, 100);

	//The mesh's x axis is the screen's y, so the screen's y comes back as the mesh's x.
	sView.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(16384), SRasterUnit::FromRaw(0));
	cCamera.ViewToMesh(&sMesh, &sView);
	AssertInt(16384, sMesh.x.miRaw);
	AssertTrue((sMesh.y.miRaw >= -1) && (sMesh.y.miRaw <= 1));
	AssertInt(0, sMesh.z.miRaw);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraPan(void)
{
	CRasterCamera	cCamera;

	//The picture moves by pixels (16 subpixels), to within one subpixel as it is rounded to the mesh's units.
	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(100, 100);
	cCamera.Prepare(100, 100);
	AssertProject(800, 800, &cCamera, 0, 0, 0);
	cCamera.Pan(10, 5);
	cCamera.Prepare(100, 100);
	AssertProjectNear(800 + 160, 800 + 80, &cCamera, 0, 0, 0);

	cCamera.Init();
	cCamera.SetPerspective(1 << 21, 100);
	cCamera.Prepare(100, 100);
	cCamera.Pan(10, 5);
	cCamera.Prepare(100, 100);
	AssertProjectNear(800 + 160, 800 + 80, &cCamera, 0, 0, 0);

	//A step of the wheel is 7/160 of the distance: at 1 << 21 that is 91750 units or 70 subpixels at this focal length.
	cCamera.Init();
	cCamera.SetPerspective(1 << 21, 100);
	cCamera.PanStep(1, 0);
	cCamera.Prepare(100, 100);
	AssertProjectNear(800 + 70, 800, &cCamera, 0, 0, 0);
	cCamera.PanStep(-1, 1);
	cCamera.Prepare(100, 100);
	AssertProjectNear(800, 800 - 70, &cCamera, 0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraZoom(void)
{
	CRasterCamera	cCamera;
	int				i;

	//A closer eye scales what is at the pivot.
	cCamera.Init();
	cCamera.SetPerspective(1 << 21, 100);
	cCamera.Zoom(2.0f);
	cCamera.Prepare(100, 100);
	AssertProject(800 + 1600, 800, &cCamera, 1 << 20, 0, 0);
	cCamera.Zoom(0.5f);
	cCamera.Prepare(100, 100);
	AssertProject(800 + 800, 800, &cCamera, 1 << 20, 0, 0);

	//Orthographic zooms with it.
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(100, 100);
	cCamera.Zoom(2.0f);
	cCamera.Prepare(100, 100);
	AssertProject(800 + 960, 800, &cCamera, RASTER_MESH_HALF_EXTENT, 0, 0);

	//KiCad's limits: 0.02 and 2 times the initial distance.
	cCamera.Init();
	cCamera.SetPerspective(1 << 21, 100);
	for (i = 0; i < 400; i++)
	{
		cCamera.Zoom(1.1f);
	}
	cCamera.Prepare(100, 100);
	AssertProject(800 + 38, 800, &cCamera, 1000, 0, 0);
	for (i = 0; i < 400; i++)
	{
		cCamera.Zoom(1.0f / 1.1f);
	}
	cCamera.Prepare(100, 100);
	AssertProject(800 + 400, 800, &cCamera, 1 << 20, 0, 0);

	//Resizing keeps the zoom and the field of view.
	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(100, 100);
	cCamera.Zoom(2.0f);
	cCamera.Resize(200);
	cCamera.Prepare(100, 200);
	AssertProject(800 + 1928, 1600, &cCamera, RASTER_MESH_HALF_EXTENT, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraDrag(void)
{
	CRasterCamera		cCamera;
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sScreen;

	//Dragging right from the middle: chord 0.2016 of a sphere of 0.8 is a turn of 0.2527 radians around the screen's y,
	//which moves the front of the mesh towards the pointer: sin(0.2527) of 480 subpixels is 120.
	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(100, 100);
	cCamera.Drag(50, 50, 60, 50, 100, 100);
	cCamera.Prepare(100, 100);
	sPosition.Init(0, 0, RASTER_MESH_HALF_EXTENT);
	cCamera.Project(&sScreen, &sPosition);
	AssertTrue((sScreen.sPosition.x.miRaw >= 800 + 118) && (sScreen.sPosition.x.miRaw <= 800 + 122));
	AssertInt(800, sScreen.sPosition.y.miRaw);

	//Dragging up moves the front up the screen, and what is on the rotation's axis stays where it is.
	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(100, 100);
	cCamera.Drag(50, 50, 50, 40, 100, 100);
	cCamera.Prepare(100, 100);
	cCamera.Project(&sScreen, &sPosition);
	AssertTrue(sScreen.sPosition.y.miRaw < 800 - 100);
	AssertInt(800, sScreen.sPosition.x.miRaw);
	AssertProject(800 + 480, 800, &cCamera, RASTER_MESH_HALF_EXTENT, 0, 0);

	//Each drag turns what the last one left, around the screen's axes and not the mesh's.
	cCamera.Drag(50, 50, 50, 40, 100, 100);
	cCamera.Drag(50, 50, 50, 60, 100, 100);
	cCamera.Drag(50, 50, 50, 60, 100, 100);
	cCamera.Prepare(100, 100);
	AssertProjectNear(800, 800, &cCamera, 0, 0, RASTER_MESH_HALF_EXTENT);

	//Not moving does nothing, and a drag outside the sphere is still a small turn.
	cCamera.Drag(70, 70, 70, 70, 100, 100);
	cCamera.Prepare(100, 100);
	AssertProjectNear(800, 800, &cCamera, 0, 0, RASTER_MESH_HALF_EXTENT);
	cCamera.Drag(99, 50, 100, 50, 100, 100);
	cCamera.Prepare(100, 100);
	cCamera.Project(&sScreen, &sPosition);
	AssertTrue((sScreen.sPosition.x.miRaw > 800) && (sScreen.sPosition.x.miRaw < 800 + 60));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraLookAtScreen(void)
{
	CRasterCamera	cCamera;
	SInt32Vec3		sMin;
	SInt32Vec3		sMax;

	sMin.Init(-RASTER_MESH_HALF_EXTENT, -RASTER_MESH_HALF_EXTENT, -RASTER_MESH_HALF_EXTENT);
	sMax.Init(RASTER_MESH_HALF_EXTENT, RASTER_MESH_HALF_EXTENT, RASTER_MESH_HALF_EXTENT);

	//Pixel 59, 49's centre is 9.5 pixels right of the screen's and 0.5 up, and a half width is 30 pixels.  That point on
	//the box's top moves to the centre.
	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(100, 100);
	cCamera.Prepare(100, 100);
	AssertTrue(cCamera.LookAtScreen(59, 49, &sMin, &sMax));
	cCamera.Prepare(100, 100);
	AssertProjectNear(800, 800, &cCamera, 332050, 17476, RASTER_MESH_HALF_EXTENT);

	//And the mesh turns around it now.
	cCamera.SetOrbit(TEST_HALF_PI, 0.0f);
	cCamera.Prepare(100, 100);
	AssertProjectNear(800, 800, &cCamera, 332050, 17476, RASTER_MESH_HALF_EXTENT);

	//Off the box nothing changes.
	cCamera.Init();
	cCamera.SetProjection(RP_Orthographic);
	cCamera.Fit(100, 100);
	cCamera.Prepare(100, 100);
	AssertFalse(cCamera.LookAtScreen(1, 1, &sMin, &sMax));
	cCamera.Prepare(100, 100);
	AssertProject(800, 800, &cCamera, 0, 0, 0);

	//Perspective: from the eye through the same pixel with a focal length of 120 reaches the top, which is three half
	//widths from the eye, 9.5 / 120 and 0.5 / 120 of that along.
	cCamera.Init();
	cCamera.Fit(100, 100);
	cCamera.Prepare(100, 100);
	AssertTrue(cCamera.LookAtScreen(59, 49, &sMin, &sMax));
	cCamera.Prepare(100, 100);
	AssertProjectNear(800, 800, &cCamera, 249037, 13107, RASTER_MESH_HALF_EXTENT);
}


//////////////////////////////////////////////////////////////////////////
//
// A box away from the origin stays on the screen, and nearly reaches its
// sides, as it turns; and it turns around its own middle.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraFitOrbit(void)
{
	CRasterCamera		cCamera;
	SInt32Vec3			sMin;
	SInt32Vec3			sMax;
	SInt32Vec3			sCorner;
	SInt32Vec3			sMiddle;
	SRasterScreenVertex	sScreen;
	SRasterScreenVertex	sFirstMiddle;
	int32				iLeft;
	int32				iRight;
	int32				iTop;
	int32				iBottom;
	int					iYaw;
	int					i;

	sMin.Init(100000, 50000, 0);
	sMax.Init(900000, 650000, 300000);
	sMiddle.Init(500000, 350000, 150000);

	cCamera.Init();
	cCamera.FitOrbit(&sMin, &sMax, -1.04719755f, 640, 360, 10);

	iLeft = iTop = 0x7FFFFFFF;
	iRight = iBottom = -0x7FFFFFFF;
	cCamera.Prepare(640, 360);
	cCamera.Project(&sFirstMiddle, &sMiddle);
	for (iYaw = 0; iYaw < 360; iYaw += 7)
	{
		cCamera.SetOrbit((float32)iYaw * TEST_HALF_PI / 90.0f, -1.04719755f);
		cCamera.Prepare(640, 360);
		for (i = 0; i < 8; i++)
		{
			sCorner.Init((i & 1) ? sMax.x : sMin.x, (i & 2) ? sMax.y : sMin.y, (i & 4) ? sMax.z : sMin.z);
			cCamera.Project(&sScreen, &sCorner);
			iLeft = (sScreen.sPosition.x.miRaw < iLeft) ? sScreen.sPosition.x.miRaw : iLeft;
			iRight = (sScreen.sPosition.x.miRaw > iRight) ? sScreen.sPosition.x.miRaw : iRight;
			iTop = (sScreen.sPosition.y.miRaw < iTop) ? sScreen.sPosition.y.miRaw : iTop;
			iBottom = (sScreen.sPosition.y.miRaw > iBottom) ? sScreen.sPosition.y.miRaw : iBottom;
		}
		cCamera.Project(&sScreen, &sMiddle);
		AssertInt(sFirstMiddle.sPosition.x.miRaw, sScreen.sPosition.x.miRaw);
		AssertInt(sFirstMiddle.sPosition.y.miRaw, sScreen.sPosition.y.miRaw);
	}

	//The margins, within a pixel or two as the turns tried here aren't the fit's.
	AssertTrue(iLeft >= (10 - 2) * SRasterSubpixel::FromInt(1).miRaw);
	AssertTrue(iRight <= (630 + 2) * SRasterSubpixel::FromInt(1).miRaw);
	AssertTrue(iTop >= (10 - 2) * SRasterSubpixel::FromInt(1).miRaw);
	AssertTrue(iBottom <= (350 + 2) * SRasterSubpixel::FromInt(1).miRaw);

	//It fills one of the two ways, and is in the middle up and down.
	AssertTrue((iRight - iLeft >= (620 - 8) * SRasterSubpixel::FromInt(1).miRaw) || (iBottom - iTop >= (340 - 8) * SRasterSubpixel::FromInt(1).miRaw));
	AssertTrue((((iTop + iBottom) / 2) >= (180 - 2) * SRasterSubpixel::FromInt(1).miRaw) && (((iTop + iBottom) / 2) <= (180 + 2) * SRasterSubpixel::FromInt(1).miRaw));
}


//////////////////////////////////////////////////////////////////////////
//
// Perspective's divisions come out exactly as real divisions do, rounding
// down, for all sizes of position in front of the eye and either side of
// the middle of the screen.  Too near the eye there's no depth.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCameraPerspectiveDivides(void)
{
	CRasterCamera		cCamera;
	SInt32Vec3			sPosition;
	SRasterScreenVertex	sScreen;
	uint32				uiRandom;
	int32				aiPart[3];
	int64				iEye;
	int64				iExpectedX;
	int64				iExpectedY;
	int64				iClamp;
	int					iDifferent;
	int					iNegative;
	int					iClamped;
	int					iTooNear;
	int					i;
	int					k;

	iClamp = 1 << 19;
	cCamera.Init();
	cCamera.SetPerspective(1 << 21, 317);
	cCamera.Prepare(200, 100);

	uiRandom = 54321;
	iDifferent = 0;
	iNegative = 0;
	iClamped = 0;
	iTooNear = 0;
	for (i = 0; i < 100000; i++)
	{
		for (k = 0; k < 3; k++)
		{
			uiRandom = uiRandom * 1103515245 + 12345;
			aiPart[k] = (int32)((uiRandom >> 4) & ((1u << (4 + (i + k) % 20)) - 1));
			uiRandom = uiRandom * 1103515245 + 12345;
			aiPart[k] = ((uiRandom >> 16) & 1) ? -aiPart[k] : aiPart[k];
		}
		//Big in x, close to the eye or too close: off the side of the screen, or with no depth.
		if ((i % 20) == 19)
		{
			aiPart[2] = (1 << 21) - (i % 2048);
		}
		sPosition.Init(aiPart[0], aiPart[1], aiPart[2]);
		cCamera.Project(&sScreen, &sPosition);

		iEye = (int64)(1 << 21) - aiPart[2];
		if (iEye < (1 << 10))
		{
			iTooNear++;
			iDifferent += sScreen.iDepth != 0;
		}
		else
		{
			iExpectedX = 1600 + DivideFloor((int64)aiPart[0] * 317 * 16, iEye);
			iExpectedY = 800 - DivideFloor((int64)aiPart[1] * 317 * 16, iEye);
			iNegative += (aiPart[0] < 0) ? 1 : 0;
			iClamped += ((iExpectedX > iClamp) || (iExpectedX < -iClamp)) ? 1 : 0;
			iExpectedX = (iExpectedX > iClamp) ? iClamp : ((iExpectedX < -iClamp) ? -iClamp : iExpectedX);
			iExpectedY = (iExpectedY > iClamp) ? iClamp : ((iExpectedY < -iClamp) ? -iClamp : iExpectedY);
			iDifferent += sScreen.sPosition.x.miRaw != iExpectedX;
			iDifferent += sScreen.sPosition.y.miRaw != iExpectedY;
			iDifferent += sScreen.iDepth != (int32)(((int64)1 << 40) / iEye);
		}
	}
	AssertInt(0, iDifferent);
	AssertTrue(iNegative > 1000);
	AssertTrue(iClamped > 100);
	AssertTrue(iTooNear > 100);
}


//////////////////////////////////////////////////////////////////////////
//
// Dividing by an inverse gives what real division does, rounding down, for
// numbers of all sizes and signs and divisors from 1 up.
//
//////////////////////////////////////////////////////////////////////////
void TestRasterDivideFloor(void)
{
	uint64	uiRandom;
	uint64	uiDivisor;
	int64	iNumerator;
	int		iDifferent;
	int		i;

	uiRandom = 88172645463325252ULL;
	iDifferent = 0;
	for (i = 0; i < 300000; i++)
	{
		uiRandom ^= uiRandom << 13;
		uiRandom ^= uiRandom >> 7;
		uiRandom ^= uiRandom << 17;
		iNumerator = (int64)(uiRandom >> (2 + (i % 61)));
		iNumerator = ((uiRandom >> 1) & 1) ? -iNumerator : iNumerator;

		uiRandom ^= uiRandom << 13;
		uiRandom ^= uiRandom >> 7;
		uiRandom ^= uiRandom << 17;
		uiDivisor = (uiRandom >> (24 + (i % 40))) + 1;

		//Exact multiples, and one either side of them, are where rounding goes wrong.
		if ((i % 7) == 0)
		{
			iNumerator = (iNumerator / (int64)uiDivisor) * (int64)uiDivisor + ((i % 3) - 1);
		}
		iDifferent += RasterDivideFloor(iNumerator, uiDivisor, RasterDivideInverse(uiDivisor)) != DivideFloor(iNumerator, (int64)uiDivisor);
	}
	AssertInt(0, iDifferent);
	AssertLong(-1, RasterDivideFloor(-1, 2, RasterDivideInverse(2)));
	AssertLong(0, RasterDivideFloor(0, 1, RasterDivideInverse(1)));
	AssertLong(7, RasterDivideFloor(7, 1, RasterDivideInverse(1)));
	AssertLong(-3, RasterDivideFloor(-9, 3, RasterDivideInverse(3)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRasterCamera(void)
{
	BeginTests();

	TestRasterCameraOrthographic();
	TestRasterCameraPerspective();
	TestRasterCameraPerspectiveDivides();
	TestRasterDivideFloor();
	TestRasterCameraViewToMesh();
	TestRasterCameraPan();
	TestRasterCameraZoom();
	TestRasterCameraDrag();
	TestRasterCameraLookAtScreen();
	TestRasterCameraFitOrbit();

	TestStatistics();
}

