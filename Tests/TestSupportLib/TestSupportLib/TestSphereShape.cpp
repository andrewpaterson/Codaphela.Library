#include "SupportLib/Sphere.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestShereIntersectionVolume(void)
{
	CSphere		cSphere1;
	CSphere		cSphere2;
	SFloat32Vec3	sPos1;
	SFloat32Vec3	sPos2;

	cSphere1.Init(&sPos1);
	cSphere1.Set(0,0,0, 1.0f);
	cSphere2.Init(&sPos2);
	cSphere2.Set(0.69459271f,0,0, 1.0f);

	AssertFloat(4.18879f, cSphere1.Volume(), 5);
	AssertFloat(4.18879f, cSphere2.Volume(), 5);
	AssertFloat(2.09440f, cSphere1.IntersectSphereVolume(&cSphere2), 5);

	cSphere2.Set(0,0,0, 1.0f);
	AssertFloat(4.18879f, cSphere1.IntersectSphereVolume(&cSphere2), 5);

	cSphere2.Set(1,0,0, 1.0f);
	AssertFloat(1.30900f, cSphere1.IntersectSphereVolume(&cSphere2), 5);

	cSphere2.Set(2,0,0, 1.0f);
	AssertFloat(0.00000f, cSphere1.IntersectSphereVolume(&cSphere2), 5);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSphereFromPoints(void)
{
	SFloat32Vec3	asPosition[3];
	SFloat32Vec3	sCenter;
	CSphere		cSphere;

	asPosition[0].Init(0,0,0);
	asPosition[1].Init(1,0,0);
	asPosition[2].Init(-1,0,0);

	cSphere.Init(&sCenter);
	cSphere.SetFromPointsUsingAveragePosition(asPosition, sizeof(SFloat32Vec3), 3);
	AssertFloat(0.0f, cSphere.mpsPosition->x, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->y, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->z, 3);
	AssertFloat(1.0f, cSphere.mfRadius, 3);

	cSphere.SetFromPointsUsingBestFit(asPosition, sizeof(SFloat32Vec3), 3);
	AssertFloat(0.0f, cSphere.mpsPosition->x, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->y, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->z, 3);
	AssertFloat(1.0f, cSphere.mfRadius, 3);

	asPosition[0].Init(1,0,0);
	asPosition[1].Init(1,0,0);
	asPosition[2].Init(-1,0,0);

	cSphere.SetFromPointsUsingAveragePosition(asPosition, sizeof(SFloat32Vec3), 3);
	AssertFloat(0.333f, cSphere.mpsPosition->x, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->y, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->z, 3);
	AssertFloat(1.333f, cSphere.mfRadius, 3);

	cSphere.SetFromPointsUsingBestFit(asPosition, sizeof(SFloat32Vec3), 3);
	AssertFloat(0.0f, cSphere.mpsPosition->x, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->y, 3);
	AssertFloat(0.0f, cSphere.mpsPosition->z, 3);
	AssertFloat(1.0f, cSphere.mfRadius, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestSphereShape(void)
{
	BeginTests();

	TestSphereFromPoints();
	TestShereIntersectionVolume();
	
	TestStatistics();
}

