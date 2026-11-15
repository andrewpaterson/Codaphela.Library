#include "BaseLib/Float32Sphere.h"
#include "BaseLib/Float32Triangle.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Volumes, and how much of two overlap.  (TestSphereShape's cases.)
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32SphereIntersectionVolume(void)
{
	SFloat32Sphere	sSphere1;
	SFloat32Sphere	sSphere2;

	sSphere1.Init(0, 0, 0, 1.0f);
	sSphere2.Init(0.69459271f, 0, 0, 1.0f);
	AssertFloat(4.18879f, sSphere1.Volume(), 5);
	AssertFloat(4.18879f, sSphere2.Volume(), 5);
	AssertFloat(2.09440f, sSphere1.IntersectionVolume(&sSphere2), 5);

	sSphere2.Init(0, 0, 0, 1.0f);
	AssertFloat(4.18879f, sSphere1.IntersectionVolume(&sSphere2), 5);
	sSphere2.Init(1, 0, 0, 1.0f);
	AssertFloat(1.30900f, sSphere1.IntersectionVolume(&sSphere2), 5);
	sSphere2.Init(2, 0, 0, 1.0f);
	AssertFloat(0.00000f, sSphere1.IntersectionVolume(&sSphere2), 5);

	//A smaller one inside is all overlap.
	sSphere2.Init(0.25f, 0, 0, 0.5f);
	AssertFloat(sSphere2.Volume(), sSphere1.IntersectionVolume(&sSphere2), 5);
}


//////////////////////////////////////////////////////////////////////////
//
// Around points: centred on their average, out to the furthest, so every
// point is inside or on it.  (TestSphereShape's cases.)
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32SphereFromPoints(void)
{
	SFloat32Sphere	sSphere;
	SFloat32Vec3	asPoints[3];

	asPoints[0].Init(0, 0, 0);
	asPoints[1].Init(1, 0, 0);
	asPoints[2].Init(-1, 0, 0);
	sSphere.Init(asPoints, sizeof(SFloat32Vec3), 3);
	AssertFloat(0.0f, sSphere.sCenter.x, 3);
	AssertFloat(0.0f, sSphere.sCenter.y, 3);
	AssertFloat(0.0f, sSphere.sCenter.z, 3);
	AssertFloat(1.0f, sSphere.fRadius, 3);

	asPoints[0].Init(1, 0, 0);
	sSphere.Init(asPoints, sizeof(SFloat32Vec3), 3);
	AssertFloat(0.333f, sSphere.sCenter.x, 3);
	AssertFloat(1.333f, sSphere.fRadius, 3);

	sSphere.Init(asPoints, sizeof(SFloat32Vec3), 0);
	AssertFloat(0.0f, sSphere.fRadius, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// Distance from its surface: negative inside, zero on it, which is outside.
// A triangle is inside if its corners are.  (The 2009 CSphere compared
// squared distances with the square root of its radius.)
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32SphereDistance(void)
{
	SFloat32Sphere		sSphere;
	SFloat32Vec3		sTest;
	SFloat32Triangle	sTriangle;
	SFloat32Vec3		asCorners[3];
	SFloat32Bounds		sBounds;

	sSphere.Init(1, 0, 0, 2.0f);
	sTest.Init(1, 0, 3);
	AssertFloat(1.0f, sSphere.Distance(&sTest), 3);
	sTest.Init(2.5f, 0, 0);
	AssertFloat(-0.5f, sSphere.Distance(&sTest), 3);
	AssertTrue(sSphere.Contains(&sTest));
	sTest.Init(3, 0, 0);
	AssertFloat(0.0f, sSphere.Distance(&sTest), 3);
	AssertFalse(sSphere.Contains(&sTest));

	sSphere.Init(0, 0, 0, 2.0f);
	asCorners[0].Init(1.5f, 0, 0);
	asCorners[1].Init(0, 1.5f, 0);
	asCorners[2].Init(0, 0, 1.5f);
	sTriangle.Init(&asCorners[0], &asCorners[1], &asCorners[2]);
	AssertTrue(sSphere.Contains(&sTriangle));
	asCorners[2].Init(0, 0, 2.5f);
	sTriangle.Init(&asCorners[0], &asCorners[1], &asCorners[2]);
	AssertFalse(sSphere.Contains(&sTriangle));

	sSphere.GetBounds(&sBounds);
	AssertFloat(-2.0f, sBounds.sMinimum.y, 3);
	AssertFloat(2.0f, sBounds.sMaximum.z, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Sphere(void)
{
	BeginTests();

	TestFloat32SphereIntersectionVolume();
	TestFloat32SphereFromPoints();
	TestFloat32SphereDistance();

	TestStatistics();
}
