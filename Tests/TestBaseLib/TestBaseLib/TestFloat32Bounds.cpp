#include "BaseLib/Float32Bounds.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Empty until a point is added, then the box around what's added.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32BoundsAdd(void)
{
	SFloat32Bounds	sBounds;
	SFloat32Bounds	sOther;
	SFloat32Vec3	sPoint;
	SFloat32Vec3	sCenter;
	SFloat32Vec3	sSize;
	SFloat32Vec3	asPoints[3];

	sBounds.Init();
	AssertTrue(sBounds.IsEmpty());

	sPoint.Init(1, 2, 3);
	sBounds.Add(&sPoint);
	AssertFalse(sBounds.IsEmpty());
	AssertFloat(1.0f, sBounds.sMinimum.x, 3);
	AssertFloat(1.0f, sBounds.sMaximum.x, 3);

	sPoint.Init(-1, 6, 4);
	sBounds.Add(&sPoint);
	sBounds.GetCenter(&sCenter);
	sBounds.GetSize(&sSize);
	AssertFloat(0.0f, sCenter.x, 3);
	AssertFloat(4.0f, sCenter.y, 3);
	AssertFloat(3.5f, sCenter.z, 3);
	AssertFloat(2.0f, sSize.x, 3);
	AssertFloat(4.0f, sSize.y, 3);
	AssertFloat(1.0f, sSize.z, 3);

	//An empty one adds nothing.
	sOther.Init();
	sBounds.Add(&sOther);
	AssertFloat(-1.0f, sBounds.sMinimum.x, 3);
	AssertFloat(6.0f, sBounds.sMaximum.y, 3);

	asPoints[0].Init(10, 0, 0);
	asPoints[1].Init(0, -10, 0);
	asPoints[2].Init(0, 0, 10);
	sOther.Init(asPoints, sizeof(SFloat32Vec3), 3);
	sBounds.Add(&sOther);
	AssertFloat(-1.0f, sBounds.sMinimum.x, 3);
	AssertFloat(-10.0f, sBounds.sMinimum.y, 3);
	AssertFloat(10.0f, sBounds.sMaximum.x, 3);
	AssertFloat(10.0f, sBounds.sMaximum.z, 3);
}


//////////////////////////////////////////////////////////////////////////
//
// Exact: outside a face, past an edge, past a corner, inside, and zero on
// a side, which is outside.
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32BoundsDistance(void)
{
	SFloat32Bounds	sBounds;
	SFloat32Vec3	sMinimum;
	SFloat32Vec3	sMaximum;
	SFloat32Vec3	sTest;

	sMinimum.Init(0, 0, 0);
	sMaximum.Init(4, 2, 2);
	sBounds.Init(&sMinimum, &sMaximum);

	sTest.Init(6, 1, 1);
	AssertFloat(2.0f, sBounds.Distance(&sTest), 3);
	sTest.Init(7, 6, 1);
	AssertFloat(5.0f, sBounds.Distance(&sTest), 3);
	sTest.Init(-1, -2, -2);
	AssertFloat(3.0f, sBounds.Distance(&sTest), 3);
	sTest.Init(1, 1, 1);
	AssertFloat(-1.0f, sBounds.Distance(&sTest), 3);
	AssertTrue(sBounds.Contains(&sTest));
	sTest.Init(3.5f, 1, 1);
	AssertFloat(-0.5f, sBounds.Distance(&sTest), 3);
	sTest.Init(4, 1, 1);
	AssertFloat(0.0f, sBounds.Distance(&sTest), 3);
	AssertFalse(sBounds.Contains(&sTest));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFloat32Bounds(void)
{
	BeginTests();

	TestFloat32BoundsAdd();
	TestFloat32BoundsDistance();

	TestStatistics();
}
