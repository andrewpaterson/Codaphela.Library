#include <math.h>
#include "ProceduralLib/IsosurfaceFit.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Three axes at right angles to each other, turned well away from x, y
// and z: a turned box's faces' normals.
//
//////////////////////////////////////////////////////////////////////////
static void TestIsosurfaceFitAxes(SFloat32Vec3* pasAxes)
{
	pasAxes[0].Init(2, 1, 2);
	pasAxes[0] /= 3.0f;
	pasAxes[1].Init(-2, 2, 1);
	pasAxes[1] /= 3.0f;
	Float32Vec3Cross(&pasAxes[2], &pasAxes[0], &pasAxes[1]);
}


//////////////////////////////////////////////////////////////////////////
//
// Crossings on the plane through psCorner along psNormal, spread over
// it along psAcross and psAlso.
//
//////////////////////////////////////////////////////////////////////////
static void TestIsosurfaceFitAddPlane(SIsosurfaceFit* psFit, SFloat32Vec3* psCorner, SFloat32Vec3* psNormal, SFloat32Vec3* psAcross, SFloat32Vec3* psAlso, float32 fFrom, float32 fTo)
{
	SFloat32Vec3	sPosition;

	sPosition = *psCorner + (*psAcross * fFrom) + (*psAlso * 0.1f);
	psFit->Add(&sPosition, psNormal);
	sPosition = *psCorner + (*psAcross * fTo) + (*psAlso * 0.3f);
	psFit->Add(&sPosition, psNormal);
}


//////////////////////////////////////////////////////////////////////////
//
// A turned box's corner from crossings on its three faces, exactly, far
// from the origin too, as the fit works relative to its reference.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceFitCorner(void)
{
	SIsosurfaceFit	sFit;
	SFloat32Vec3	asAxes[3];
	SFloat32Vec3	sCorner;
	SFloat32Vec3	sCentre;
	SFloat32Vec3	sResult;
	SFloat32Bounds	sCell;
	SFloat32Vec3	sHalf;
	int				i;
	int				iFar;

	TestIsosurfaceFitAxes(asAxes);
	for (iFar = 0; iFar < 2; iFar++)
	{
		sCentre.Init(0.5f, 0.5f, 0.5f);
		if (iFar == 1)
		{
			sCentre.Init(1000.5f, -2000.5f, 3000.5f);
		}
		sCorner = sCentre + SFloat32Vec3(0.1f, -0.15f, 0.2f);
		sHalf.Init(0.5f, 0.5f, 0.5f);
		sCell.Init();
		sCell.sMinimum = sCentre - sHalf;
		sCell.sMaximum = sCentre + sHalf;

		sFit.Init(&sCentre);
		for (i = 0; i < 3; i++)
		{
			TestIsosurfaceFitAddPlane(&sFit, &sCorner, &asAxes[i], &asAxes[(i + 1) % 3], &asAxes[(i + 2) % 3], -0.3f, -0.1f);
		}
		AssertInt(6, sFit.iNumPoints);
		sFit.Solve(&sResult, &sCell);
		AssertFloat3(sCorner, &sResult, iFar == 0 ? 5 : 3);
		AssertTrue(sFit.Error(&sResult) < 0.00001f);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Two faces give the point on their edge nearest the crossings' mass
// point, and one face gives the mass point itself.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceFitEdgeAndFace(void)
{
	SIsosurfaceFit	sFit;
	SFloat32Vec3	asAxes[3];
	SFloat32Vec3	sCorner;
	SFloat32Vec3	sResult;
	SFloat32Vec3	sMassPoint;
	SFloat32Vec3	sOffset;
	SFloat32Vec3	sOrigin;

	TestIsosurfaceFitAxes(asAxes);
	sCorner.Init(0.1f, -0.15f, 0.2f);
	sOrigin.Init(0, 0, 0);

	sFit.Init(&sOrigin);
	TestIsosurfaceFitAddPlane(&sFit, &sCorner, &asAxes[0], &asAxes[2], &asAxes[1], -0.2f, 0.3f);
	TestIsosurfaceFitAddPlane(&sFit, &sCorner, &asAxes[1], &asAxes[2], &asAxes[0], 0.1f, 0.4f);
	sFit.Solve(&sResult, NULL);
	sOffset = sResult - sCorner;
	AssertFloat(0.0f, Float32Vec3Dot(&sOffset, &asAxes[0]), 5);
	AssertFloat(0.0f, Float32Vec3Dot(&sOffset, &asAxes[1]), 5);
	sFit.GetMassPoint(&sMassPoint);
	sOffset = sResult - sMassPoint;
	AssertFloat(0.0f, Float32Vec3Dot(&sOffset, &asAxes[2]), 5);

	sFit.Init(&sOrigin);
	TestIsosurfaceFitAddPlane(&sFit, &sCorner, &asAxes[2], &asAxes[0], &asAxes[1], -0.2f, 0.3f);
	sFit.Solve(&sResult, NULL);
	sFit.GetMassPoint(&sMassPoint);
	AssertFloat3(sMassPoint, &sResult, 5);
}


//////////////////////////////////////////////////////////////////////////
//
// Crossings on two planes through the cell: x = 0.5, and one through
// (0.9, 0.9, 0.5) along psNormal2.
//
//////////////////////////////////////////////////////////////////////////
static void TestIsosurfaceFitTwoPlanes(SIsosurfaceFit* psFit, SFloat32Vec3* psNormal2)
{
	SFloat32Vec3	sNormal1;
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sAlong;
	SFloat32Vec3	sCentre;

	sCentre.Init(0.5f, 0.5f, 0.5f);
	psFit->Init(&sCentre);
	sNormal1.Init(1, 0, 0);
	sPosition.Init(0.5f, 0.2f, 0.5f);
	psFit->Add(&sPosition, &sNormal1);
	sPosition.Init(0.5f, 0.4f, 0.5f);
	psFit->Add(&sPosition, &sNormal1);
	sPosition.Init(0.9f, 0.9f, 0.5f);
	psFit->Add(&sPosition, psNormal2);
	sAlong.Init(-psNormal2->y, psNormal2->x, 0);
	sPosition += sAlong * 0.2f;
	psFit->Add(&sPosition, psNormal2);
}


//////////////////////////////////////////////////////////////////////////
//
// Planes at 60 degrees to each other meet just outside the cell, at y
// 1.13: the fit goes there, and is clamped back to the cell's side, where
// it fits better than the mass point.  Planes at 5 degrees meet far away,
// but the weaker of their directions is under the cut off (1 - cos 5 is
// a fiftieth of 1 + cos 5), so the fit stays at the mass point along it.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceFitClamped(void)
{
	SIsosurfaceFit	sFit;
	SFloat32Vec3	sNormal2;
	SFloat32Vec3	sResult;
	SFloat32Vec3	sMassPoint;
	SFloat32Vec3	sMinimum;
	SFloat32Vec3	sMaximum;
	SFloat32Bounds	sCell;

	sMinimum.Init(0, 0, 0);
	sMaximum.Init(1, 1, 1);
	sCell.Init(&sMinimum, &sMaximum);

	sNormal2.Init(0.5f, sqrtf(0.75f), 0);
	TestIsosurfaceFitTwoPlanes(&sFit, &sNormal2);
	sFit.Solve(&sResult, NULL);
	AssertFloat(0.5f, sResult.x, 4);
	AssertFloat(0.9f + (0.2f / sqrtf(0.75f)), sResult.y, 4);
	sFit.Solve(&sResult, &sCell);
	AssertFloat(0.5f, sResult.x, 4);
	AssertFloat(1.0f, sResult.y, 6);
	sFit.GetMassPoint(&sMassPoint);
	AssertTrue(sFit.Error(&sResult) < sFit.Error(&sMassPoint));

	sNormal2.Init(cosf(0.0872665f), sinf(0.0872665f), 0);
	TestIsosurfaceFitTwoPlanes(&sFit, &sNormal2);
	sFit.Solve(&sResult, &sCell);
	sFit.GetMassPoint(&sMassPoint);
	AssertTrue(sCell.Contains(&sResult));
	AssertFloat(sMassPoint.y, sResult.y, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// No crossings is the reference.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceFitEmpty(void)
{
	SIsosurfaceFit	sFit;
	SFloat32Vec3	sResult;
	SFloat32Vec3	sReference;

	sReference.Init(1, 2, 3);
	sFit.Init(&sReference);
	sFit.Solve(&sResult, NULL);
	AssertFloat3(SFloat32Vec3(1, 2, 3), &sResult, 6);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceFit(void)
{
	BeginTests();

	TestIsosurfaceFitCorner();
	TestIsosurfaceFitEdgeAndFace();
	TestIsosurfaceFitClamped();
	TestIsosurfaceFitEmpty();

	TestStatistics();
}

