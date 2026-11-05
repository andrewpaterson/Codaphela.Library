#include <math.h>
#include "BaseLib/Numbers.h"
#include "SupportLib/VectorPath.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The point at fT along a cubic Bezier curve.
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2 TestCubicPoint(SFloat64Vec2* psStart, SFloat64Vec2* psControl1, SFloat64Vec2* psControl2, SFloat64Vec2* psEnd, float64 fT)
{
	SFloat64Vec2	sPoint;
	float64			fU;

	fU = 1 - fT;
	sPoint.x = fU * fU * fU * psStart->x + 3 * fU * fU * fT * psControl1->x + 3 * fU * fT * fT * psControl2->x + fT * fT * fT * psEnd->x;
	sPoint.y = fU * fU * fU * psStart->y + 3 * fU * fU * fT * psControl1->y + 3 * fU * fT * fT * psControl2->y + fT * fT * fT * psEnd->y;
	return sPoint;
}


//////////////////////////////////////////////////////////////////////////
//
// A cubic is straight edges on its curve, as many as Wang's formula says,
// each no further from the curve than the tolerance.  A straight cubic is
// one edge, and one with nothing before it is a move.
//
//////////////////////////////////////////////////////////////////////////
void TestVectorPathCubic(void)
{
	CVectorPath		cPath;
	SFloat64Vec2	sStart;
	SFloat64Vec2	sControl1;
	SFloat64Vec2	sControl2;
	SFloat64Vec2	sEnd;
	SFloat64Vec2	sCurve;
	SFloat64Vec2*	psFrom;
	SFloat64Vec2*	psTo;
	float64			fDistance;
	float64			fFurthest;
	size			i;
	size			uiLines;

	//An arch: the larger second difference is 100 * sqrt(2), so sqrt(0.75 * 141.42 / 0.1) = 32.6 rounds up to 33 edges.
	sStart.Init(0, 0);
	sControl1.Init(0, 100);
	sControl2.Init(100, 100);
	sEnd.Init(100, 0);
	cPath.Init();
	cPath.MoveTo(&sStart);
	cPath.CubicTo(&sControl1, &sControl2, &sEnd);
	uiLines = cPath.NumSegments() - 1;
	AssertSize(33, uiLines);
	AssertInt(VS_Line, cPath.GetSegment(1)->eType);
	AssertDouble(100, cPath.GetSegment(33)->sPoint.x, 9);
	AssertDouble(0, cPath.GetSegment(33)->sPoint.y, 9);

	//Each corner is on the curve and each edge's middle is within the tolerance of the curve's middle between them.
	fFurthest = 0;
	for (i = 1; i <= uiLines; i++)
	{
		psFrom = &cPath.GetSegment(i - 1)->sPoint;
		psTo = &cPath.GetSegment(i)->sPoint;
		sCurve = TestCubicPoint(&sStart, &sControl1, &sControl2, &sEnd, (float64)i / uiLines);
		AssertDouble(sCurve.x, psTo->x, 9);
		AssertDouble(sCurve.y, psTo->y, 9);
		sCurve = TestCubicPoint(&sStart, &sControl1, &sControl2, &sEnd, (i - 0.5) / uiLines);
		fDistance = sqrt(((psFrom->x + psTo->x) / 2 - sCurve.x) * ((psFrom->x + psTo->x) / 2 - sCurve.x) + ((psFrom->y + psTo->y) / 2 - sCurve.y) * ((psFrom->y + psTo->y) / 2 - sCurve.y));
		if (fDistance > fFurthest)
		{
			fFurthest = fDistance;
		}
	}
	AssertTrue(fFurthest <= VECTOR_PATH_CUBIC_TOLERANCE);
	AssertTrue(fFurthest > 0);

	//A looser tolerance needs fewer edges: sqrt(0.75 * 141.42 / 1) = 10.3, 11.
	cPath.Kill();
	cPath.Init();
	cPath.MoveTo(&sStart);
	cPath.CubicTo(&sControl1, &sControl2, &sEnd, 1);
	AssertSize(12, cPath.NumSegments());

	//Controls a third and two thirds along the line make a straight edge.  (Elsewhere on the line the curve is straight
	//but goes along it unevenly, and the formula still asks for edges.)
	cPath.Kill();
	cPath.Init();
	cPath.MoveTo(&sStart);
	sControl1.Init(100.0 / 3, 0);
	sControl2.Init(200.0 / 3, 0);
	cPath.CubicTo(&sControl1, &sControl2, &sEnd);
	AssertSize(2, cPath.NumSegments());
	cPath.Kill();

	//Nothing before it: a move.
	cPath.Init();
	cPath.CubicTo(&sControl1, &sControl2, &sEnd);
	AssertSize(1, cPath.NumSegments());
	AssertInt(VS_Move, cPath.GetSegment(0)->eType);
	cPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A quarter circle drawn as a cubic (controls 0.5523 of the radius along
// the tangents) has nearly the quarter circle's area, a little less as
// its straight edges cut inside the curve.
//
//////////////////////////////////////////////////////////////////////////
void TestVectorPathCubicArea(void)
{
	CVectorPath		cPath;
	SFloat64Vec2	sPoint;
	SFloat64Vec2	sControl1;
	SFloat64Vec2	sControl2;
	float64			fArea;

	cPath.Init();
	sPoint.Init(0, 0);
	cPath.MoveTo(&sPoint);
	sPoint.Init(100, 0);
	cPath.LineTo(&sPoint);
	sControl1.Init(100, 55.228475);
	sControl2.Init(55.228475, 100);
	sPoint.Init(0, 100);
	cPath.CubicTo(&sControl1, &sControl2, &sPoint);
	fArea = cPath.Area();
	//The edges cut inside the curve by no more than the tolerance along its 157 or so length: under 16 short.
	AssertTrue(fArea < N_PI_64 * 10000 / 4);
	AssertTrue(fArea > N_PI_64 * 10000 / 4 - 157 * VECTOR_PATH_CUBIC_TOLERANCE);
	cPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestVectorPath(void)
{
	BeginTests();

	TestVectorPathCubic();
	TestVectorPathCubicArea();

	TestStatistics();
}
