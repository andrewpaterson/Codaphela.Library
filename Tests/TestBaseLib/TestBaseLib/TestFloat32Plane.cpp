#include "BaseLib/Float32Plane.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Through three points it faces the side they go anticlockwise from, and
// its inside is behind it.  On it is outside.  (TestHalfSpace's cases.)
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32PlaneThroughPoints(void)
{
	SFloat32Plane	sPlane;
	SFloat32Vec3	sPos1;
	SFloat32Vec3	sPos2;
	SFloat32Vec3	sPos3;
	SFloat32Vec3	sTest;

	sPos1.Init(25.0f, -12.5f, 50.0f);
	sPos2.Init(25.0f, -12.5f, -50.0f);
	sPos3.Init(-25.0f, -12.5f, 50.0f);
	AssertTrue(sPlane.Init(&sPos1, &sPos2, &sPos3));
	AssertFloat(0.0f, sPlane.sNormal.x, 3);
	AssertFloat(1.0f, sPlane.sNormal.y, 3);
	AssertFloat(0.0f, sPlane.sNormal.z, 3);
	AssertFloat(-12.5f, sPlane.fDistance, 3);

	sTest.Init(0.0f, -12.5f, 0.0f);
	AssertFloat(0.0f, sPlane.Distance(&sTest), 3);
	AssertFalse(sPlane.Contains(&sTest));
	sTest.Init(0.0f, -14.0f, 0.0f);
	AssertFloat(-1.5f, sPlane.Distance(&sTest), 3);
	AssertTrue(sPlane.Contains(&sTest));
	sTest.Init(0.0f, 12.5f, 0.0f);
	AssertFloat(25.0f, sPlane.Distance(&sTest), 3);
	AssertFalse(sPlane.Contains(&sTest));

	AssertTrue(sPlane.Init(&sPos1, &sPos3, &sPos2));
	AssertFloat(-1.0f, sPlane.sNormal.y, 3);
	sTest.Init(0.0f, -14.0f, 0.0f);
	AssertFalse(sPlane.Contains(&sTest));
	sTest.Init(0.0f, 12.5f, 0.0f);
	AssertTrue(sPlane.Contains(&sTest));

	//Three points in a line make no plane.
	sPos3.Init(25.0f, -12.5f, 0.0f);
	AssertFalse(sPlane.Init(&sPos1, &sPos2, &sPos3));
}


//////////////////////////////////////////////////////////////////////////
//
// A normal that isn't a unit is made one; the closest point is on the plane
// straight below; flipping swaps its sides.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32PlaneNormalAndPoint(void)
{
	SFloat32Plane	sPlane;
	SFloat32Vec3	sNormal;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sTest;
	SFloat32Vec3	sClosest;

	sNormal.Init(0, 0, 2);
	sPoint.Init(5, 6, 3);
	sPlane.Init(&sNormal, &sPoint);
	AssertFloat(1.0f, sPlane.sNormal.z, 3);
	AssertFloat(3.0f, sPlane.fDistance, 3);

	sTest.Init(1, 2, 7);
	AssertFloat(4.0f, sPlane.Distance(&sTest), 3);
	sPlane.ClosestPoint(&sClosest, &sTest);
	AssertFloat(1.0f, sClosest.x, 3);
	AssertFloat(2.0f, sClosest.y, 3);
	AssertFloat(3.0f, sClosest.z, 3);

	sPlane.Flip();
	AssertFloat(-4.0f, sPlane.Distance(&sTest), 3);
	AssertTrue(sPlane.Contains(&sTest));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Plane(void)
{
	BeginTests();

	TestFloat32PlaneThroughPoints();
	TestFloat32PlaneNormalAndPoint();

	TestStatistics();
}
