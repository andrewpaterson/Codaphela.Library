#include "BaseLib/Float32Triangle.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Its normal faces the side its corners go anticlockwise from, and each
// corner keeps its own coordinates.  (The 2009 CTriangle took the first
// corner's z for the second's.)
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32TriangleNormal(void)
{
	SFloat32Triangle	sTriangle;
	SFloat32Vec3		sPos1;
	SFloat32Vec3		sPos2;
	SFloat32Vec3		sPos3;
	SFloat32Vec3		sNormal;
	SFloat32Plane		sPlane;
	SFloat32Vec3		sCenter;
	SFloat32Bounds		sBounds;

	sPos1.Init(0, 0, 0);
	sPos2.Init(1, 0, 1);
	sPos3.Init(0, 1, 0);
	sTriangle.Init(&sPos1, &sPos2, &sPos3);
	AssertFloat(1.0f, sTriangle.asPositions[1].z, 3);

	AssertTrue(sTriangle.GetNormal(&sNormal));
	AssertFloat(-0.7071f, sNormal.x, 4);
	AssertFloat(0.0f, sNormal.y, 4);
	AssertFloat(0.7071f, sNormal.z, 4);

	AssertTrue(sTriangle.GetPlane(&sPlane));
	AssertFloat(0.0f, sPlane.fDistance, 4);
	AssertFloat(0.0f, sPlane.Distance(&sPos2), 4);
	AssertFloat(0.7071f, sTriangle.Area(), 4);

	sTriangle.GetCenter(&sCenter);
	AssertFloat(0.3333f, sCenter.x, 4);
	AssertFloat(0.3333f, sCenter.y, 4);
	AssertFloat(0.3333f, sCenter.z, 4);

	sTriangle.GetBounds(&sBounds);
	AssertFloat(1.0f, sBounds.sMaximum.x, 3);
	AssertFloat(1.0f, sBounds.sMaximum.z, 3);

	//No area, no normal.
	sPos3.Init(2, 0, 2);
	sTriangle.Init(&sPos1, &sPos2, &sPos3);
	AssertFalse(sTriangle.GetNormal(&sNormal));
	AssertFloat(0.0f, sTriangle.Area(), 4);
}


//////////////////////////////////////////////////////////////////////////
//
// The closest point is a corner, a point on an edge or a point inside, as
// psPosition lies; the distance is unsigned, the same in front and behind.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32TriangleClosestPoint(void)
{
	SFloat32Triangle	sTriangle;
	SFloat32Vec3		sPos1;
	SFloat32Vec3		sPos2;
	SFloat32Vec3		sPos3;
	SFloat32Vec3		sTest;
	SFloat32Vec3		sClosest;

	sPos1.Init(0, 0, 0);
	sPos2.Init(4, 0, 0);
	sPos3.Init(0, 4, 0);
	sTriangle.Init(&sPos1, &sPos2, &sPos3);

	sTest.Init(-1, -1, 0);
	sTriangle.ClosestPoint(&sClosest, &sTest);
	AssertFloat(0.0f, sClosest.x, 3);
	AssertFloat(0.0f, sClosest.y, 3);

	sTest.Init(6, -1, 0);
	sTriangle.ClosestPoint(&sClosest, &sTest);
	AssertFloat(4.0f, sClosest.x, 3);
	AssertFloat(0.0f, sClosest.y, 3);

	sTest.Init(-1, 7, 0);
	sTriangle.ClosestPoint(&sClosest, &sTest);
	AssertFloat(0.0f, sClosest.x, 3);
	AssertFloat(4.0f, sClosest.y, 3);

	sTest.Init(2, -3, 0);
	sTriangle.ClosestPoint(&sClosest, &sTest);
	AssertFloat(2.0f, sClosest.x, 3);
	AssertFloat(0.0f, sClosest.y, 3);
	AssertFloat(3.0f, sTriangle.Distance(&sTest), 3);

	sTest.Init(-2, 1, 0);
	sTriangle.ClosestPoint(&sClosest, &sTest);
	AssertFloat(0.0f, sClosest.x, 3);
	AssertFloat(1.0f, sClosest.y, 3);

	sTest.Init(3, 3, 0);
	sTriangle.ClosestPoint(&sClosest, &sTest);
	AssertFloat(2.0f, sClosest.x, 3);
	AssertFloat(2.0f, sClosest.y, 3);
	AssertFloat(1.4142f, sTriangle.Distance(&sTest), 4);

	sTest.Init(1, 1, 5);
	sTriangle.ClosestPoint(&sClosest, &sTest);
	AssertFloat(1.0f, sClosest.x, 3);
	AssertFloat(1.0f, sClosest.y, 3);
	AssertFloat(0.0f, sClosest.z, 3);
	AssertFloat(5.0f, sTriangle.Distance(&sTest), 3);
	sTest.Init(1, 1, -5);
	AssertFloat(5.0f, sTriangle.Distance(&sTest), 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Triangle(void)
{
	BeginTests();

	TestFloat32TriangleNormal();
	TestFloat32TriangleClosestPoint();

	TestStatistics();
}
