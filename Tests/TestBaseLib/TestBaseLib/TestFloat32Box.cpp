#include "BaseLib/Float4x4.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/Float32Box.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Around points along the axes, the longest first, centred between the
// extremes.  Every point is looked at.  (The 2009 CBox read the point after
// the last on every pass and never set its centre.)
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32BoxFromPoints(void)
{
	SFloat32Box		sBox;
	SFloat32Vec3	asPoints[3];

	asPoints[0].Init(-1, 2, 3);
	asPoints[1].Init(5, 4, 4);
	asPoints[2].Init(0, 3, 3.5f);
	sBox.Init(asPoints, sizeof(SFloat32Vec3), 3);

	AssertFloat(3.0f, sBox.afHalfLengths[0], 3);
	AssertFloat(1.0f, sBox.afHalfLengths[1], 3);
	AssertFloat(0.5f, sBox.afHalfLengths[2], 3);
	AssertFloat(1.0f, sBox.asAxes[0].x, 3);
	AssertFloat(1.0f, sBox.asAxes[1].y, 3);
	AssertFloat(1.0f, sBox.asAxes[2].z, 3);
	AssertFloat(2.0f, sBox.sCenter.x, 3);
	AssertFloat(3.0f, sBox.sCenter.y, 3);
	AssertFloat(3.5f, sBox.sCenter.z, 3);
	AssertFloat(12.0f, sBox.Volume(), 3);

	//Longest along z, then x, then y.
	asPoints[0].Init(0, 0, 0);
	asPoints[1].Init(2, 1, 8);
	sBox.Init(asPoints, sizeof(SFloat32Vec3), 2);
	AssertFloat(4.0f, sBox.afHalfLengths[0], 3);
	AssertFloat(1.0f, sBox.afHalfLengths[1], 3);
	AssertFloat(0.5f, sBox.afHalfLengths[2], 3);
	AssertFloat(1.0f, sBox.asAxes[0].z, 3);
	AssertFloat(1.0f, sBox.asAxes[1].x, 3);
	AssertFloat(1.0f, sBox.asAxes[2].y, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// An 8 x 4 x 2 box turned 30 degrees about y and moved to (10, -5, 3):
// distances along its own axes are exact, its corners are where they
// should be, and its bounds hold them.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32BoxTurned(void)
{
	SFloat32Box		sBox;
	SFloat4x4		sMatrix;
	SFloat32Vec3	sCenter;
	SFloat32Vec3	asAxes[3];
	SFloat32Vec3	sTest;
	SFloat32Vec3	sCorner;
	SFloat32Bounds	sBounds;
	int				i;

	Float4x4RotationY(&sMatrix, Deg2Rad(30.0f));
	asAxes[0].Init(sMatrix.x.x, sMatrix.x.y, sMatrix.x.z);
	asAxes[1].Init(sMatrix.y.x, sMatrix.y.y, sMatrix.y.z);
	asAxes[2].Init(sMatrix.z.x, sMatrix.z.y, sMatrix.z.z);
	sCenter.Init(10, -5, 3);
	sBox.Init(&sCenter, &asAxes[0], 4.0f, &asAxes[1], 2.0f, &asAxes[2], 1.0f);
	AssertFloat(64.0f, sBox.Volume(), 3);

	//3 beyond the long face.
	sTest = sCenter + asAxes[0] * 7.0f;
	AssertFloat(3.0f, sBox.Distance(&sTest), 3);
	//Past an edge: 3 beyond the long face and 4 beyond the middle one.
	sTest = sCenter + asAxes[0] * 7.0f + asAxes[1] * 6.0f;
	AssertFloat(5.0f, sBox.Distance(&sTest), 3);
	//Inside, nearest the short faces.
	sTest = sCenter + asAxes[2] * 0.25f;
	AssertFloat(-0.75f, sBox.Distance(&sTest), 3);
	AssertTrue(sBox.Contains(&sTest));
	sTest = sCenter + asAxes[2] * 1.0f;
	AssertFloat(0.0f, sBox.Distance(&sTest), 3);
	AssertFalse(sBox.Contains(&sTest));

	sBox.GetCorner(&sCorner, 7);
	sTest = sCenter + asAxes[0] * 4.0f + asAxes[1] * 2.0f + asAxes[2] * 1.0f;
	AssertFloat(sTest.x, sCorner.x, 3);
	AssertFloat(sTest.y, sCorner.y, 3);
	AssertFloat(sTest.z, sCorner.z, 3);
	sBox.GetCorner(&sCorner, 0);
	sTest = sCenter - asAxes[0] * 4.0f - asAxes[1] * 2.0f - asAxes[2] * 1.0f;
	AssertFloat(sTest.x, sCorner.x, 3);
	AssertFloat(sTest.z, sCorner.z, 3);

	sBox.GetBounds(&sBounds);
	for (i = 0; i < 8; i++)
	{
		sBox.GetCorner(&sCorner, i);
		AssertTrue(sBounds.Distance(&sCorner) <= 0.0001f);
	}
	AssertFloat(-7.0f, sBounds.sMinimum.y, 3);
	AssertFloat(-3.0f, sBounds.sMaximum.y, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Box(void)
{
	BeginTests();

	TestFloat32BoxFromPoints();
	TestFloat32BoxTurned();

	TestStatistics();
}
