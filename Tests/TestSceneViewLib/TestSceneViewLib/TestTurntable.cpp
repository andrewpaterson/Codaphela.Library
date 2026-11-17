#include <math.h>
#include "SceneViewLib/Turntable.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// A matrix's x, y, z or pos as a vector.
//
//////////////////////////////////////////////////////////////////////////
static SFloat32Vec3 TurntableRow(SFloat32Vec4* psRow)
{
	SFloat32Vec3	sRow;

	sRow.Init(psRow->x, psRow->y, psRow->z);
	return sRow;
}


//////////////////////////////////////////////////////////////////////////
//
// As made, the view is a unit back along z from the origin, level.
//
//////////////////////////////////////////////////////////////////////////
void TestTurntableInit(void)
{
	STurntable		sView;
	SFloat4x4		sWorld;
	SFloat32Vec3	sRow;

	sView.Init();
	AssertFloat(TURNTABLE_FOV, sView.fFOV, 4);
	sView.World(&sWorld);

	sRow = TurntableRow(&sWorld.x);
	AssertFloat3(SFloat32Vec3(1, 0, 0), &sRow, 4);
	sRow = TurntableRow(&sWorld.y);
	AssertFloat3(SFloat32Vec3(0, 1, 0), &sRow, 4);
	sRow = TurntableRow(&sWorld.z);
	AssertFloat3(SFloat32Vec3(0, 0, 1), &sRow, 4);
	sRow = TurntableRow(&sWorld.pos);
	AssertFloat3(SFloat32Vec3(0, 0, 1), &sRow, 4);
	AssertFloat(1.0f, sWorld.pos.w, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// Framed, a sphere just fits the shorter way of a 4 by 3 view: its edge
// is at the half angle that way from the view's middle.
//
//////////////////////////////////////////////////////////////////////////
void TestTurntableFrame(void)
{
	STurntable		sView;
	SFloat32Vec3	sCentre;
	float32			fHalfDown;

	sCentre.Init(1, 2, 3);
	sView.Init();
	sView.Orbit(40, -30);
	sView.Frame(&sCentre, 2.0f);

	AssertFloat3(SFloat32Vec3(1, 2, 3), &sView.sTarget, 4);
	AssertFloat(0.0f, sView.fYaw, 4);
	AssertFloat(0.0f, sView.fPitch, 4);
	fHalfDown = atanf(tanf(sView.fFOV * 0.5f) * 0.75f);
	AssertFloat(2.0f, sView.fDistance * sinf(fHalfDown), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// A view taken from another's matrix is that view, when what it's
// centred on is what the other looked at.  A matrix with no z isn't a
// camera's, and a centre behind the camera puts the target a radius in
// front.
//
//////////////////////////////////////////////////////////////////////////
void TestTurntableFrom(void)
{
	STurntable		sView;
	STurntable		sFrom;
	SFloat4x4		sWorld;
	SFloat32Vec3	sCentre;
	SFloat32Vec3	sExpected;

	sView.Init();
	sView.sTarget.Init(3, -1, 2);
	sView.fDistance = 5.0f;
	sView.fFOV = 1.2f;
	sView.Orbit(-70, 45);
	sView.World(&sWorld);

	sFrom.Init();
	AssertTrue(sFrom.From(&sWorld, 1.2f, &sView.sTarget, 1.0f));
	AssertFloat(sView.fYaw, sFrom.fYaw, 4);
	AssertFloat(sView.fPitch, sFrom.fPitch, 4);
	AssertFloat(5.0f, sFrom.fDistance, 4);
	AssertFloat(1.2f, sFrom.fFOV, 4);
	AssertFloat3(SFloat32Vec3(3, -1, 2), &sFrom.sTarget, 3);

	//The centre 20 behind the camera along its z.
	sCentre.Init(sWorld.pos.x + sWorld.z.x * 20, sWorld.pos.y + sWorld.z.y * 20, sWorld.pos.z + sWorld.z.z * 20);
	AssertTrue(sFrom.From(&sWorld, 1.2f, &sCentre, 1.5f));
	AssertFloat(1.5f, sFrom.fDistance, 4);
	sExpected.Init(sWorld.pos.x - sWorld.z.x * 1.5f, sWorld.pos.y - sWorld.z.y * 1.5f, sWorld.pos.z - sWorld.z.z * 1.5f);
	AssertFloat3(sExpected, &sFrom.sTarget, 3);

	sWorld.z.Init(0, 0, 0, 0);
	AssertFalse(sFrom.From(&sWorld, 1.2f, &sCentre, 1.5f));
}


//////////////////////////////////////////////////////////////////////////
//
// A pixel's drag is TURNTABLE_ORBIT_RADIANS; the pitch stops short of
// straight up and down; the eye stays the distance from the target.
//
//////////////////////////////////////////////////////////////////////////
void TestTurntableOrbit(void)
{
	STurntable		sView;
	SFloat4x4		sWorld;
	SFloat32Vec3	sEye;
	SFloat32Vec3	sOut;

	sView.Init();
	sView.sTarget.Init(1, 1, 1);
	sView.fDistance = 4.0f;
	sView.Orbit(100, 50);
	AssertFloat(-1.0f, sView.fYaw, 4);
	AssertFloat(0.5f, sView.fPitch, 4);

	sView.World(&sWorld);
	sEye = TurntableRow(&sWorld.pos);
	sOut = sEye - sView.sTarget;
	AssertFloat(4.0f, sOut.Magnitude(), 4);
	AssertTrue(sEye.y > 1.0f);  //Tipped towards the view, it's seen from above.

	sView.Orbit(0, 1000);
	AssertFloat(TURNTABLE_PITCH_LIMIT, sView.fPitch, 4);
	sView.Orbit(0, -5000);
	AssertFloat(-TURNTABLE_PITCH_LIMIT, sView.fPitch, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// Panning moves the target the other way across the view by what the
// pixels cover there, and zooming by 2 halves the distance.
//
//////////////////////////////////////////////////////////////////////////
void TestTurntablePanAndZoom(void)
{
	STurntable	sView;
	float32		fPerPixel;

	sView.Init();
	sView.fDistance = 10.0f;
	fPerPixel = 2.0f * 10.0f * tanf(sView.fFOV * 0.5f) / 200;

	sView.Pan(10, 0, 200);
	AssertFloat3(SFloat32Vec3(-10 * fPerPixel, 0, 0), &sView.sTarget, 4);
	sView.Pan(-10, 4, 200);
	AssertFloat3(SFloat32Vec3(0, 4 * fPerPixel, 0), &sView.sTarget, 4);

	sView.Pan(10, 10, 0);  //A view with no width.
	AssertFloat3(SFloat32Vec3(0, 4 * fPerPixel, 0), &sView.sTarget, 4);

	sView.Zoom(2.0f);
	AssertFloat(5.0f, sView.fDistance, 4);
	sView.Zoom(0.5f);
	AssertFloat(10.0f, sView.fDistance, 4);
}


//////////////////////////////////////////////////////////////////////////
//
// The middle of the view is the target (but for half a pixel), a point
// is on the plane through the target facing the view however the view is
// turned, and a pixel across is what Pan moves by.
//
//////////////////////////////////////////////////////////////////////////
void TestTurntableScreenToPlane(void)
{
	STurntable		sView;
	SFloat4x4		sWorld;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sNext;
	SFloat32Vec3	sBack;
	SFloat32Vec3	sOut;
	float32			fPerPixel;

	sView.Init();
	sView.sTarget.Init(2, 0, -3);
	sView.fDistance = 8.0f;
	fPerPixel = 2.0f * 8.0f * tanf(sView.fFOV * 0.5f) / 200;

	AssertTrue(sView.ScreenToPlane(100, 50, 200, 100, &sPoint));
	AssertFloat3(SFloat32Vec3(2 + 0.5f * fPerPixel, -0.5f * fPerPixel, -3), &sPoint, 4);
	AssertTrue(sView.ScreenToPlane(0, 0, 200, 100, &sPoint));
	AssertFloat3(SFloat32Vec3(2 - 99.5f * fPerPixel, 49.5f * fPerPixel, -3), &sPoint, 3);

	sView.Orbit(130, -60);
	sView.World(&sWorld);
	sBack = TurntableRow(&sWorld.z);
	AssertTrue(sView.ScreenToPlane(17, 80, 200, 100, &sPoint));
	sOut = sPoint - sView.sTarget;
	AssertFloat(0.0f, Float32Vec3Dot(&sOut, &sBack), 3);
	AssertTrue(sView.ScreenToPlane(18, 80, 200, 100, &sNext));
	sOut = sNext - sPoint;
	AssertFloat(fPerPixel, sOut.Magnitude(), 4);

	AssertFalse(sView.ScreenToPlane(0, 0, 0, 100, &sPoint));
	AssertFalse(sView.ScreenToPlane(0, 0, 200, 0, &sPoint));
	sView.fFOV = 0;
	AssertFalse(sView.ScreenToPlane(0, 0, 200, 100, &sPoint));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTurntable(void)
{
	BeginTests();

	TestTurntableInit();
	TestTurntableFrame();
	TestTurntableFrom();
	TestTurntableOrbit();
	TestTurntablePanAndZoom();
	TestTurntableScreenToPlane();

	TestStatistics();
}

