#include <math.h>
#include "BaseLib/Float4x4.h"
#include "BaseLib/FloatHelper.h"
#include "BaseLib/Numbers.h"
#include "SupportLib/QuickHull.h"
#include "TestLib/Assert.h"


float32	TestShapeRandom(uint32* puiState);


//////////////////////////////////////////////////////////////////////////
//
// Every point is behind or on every face, every face's corners are on its
// plane and go round it anticlockwise from outside, and the faces close up:
// corners - edges + faces is 2.
//
//////////////////////////////////////////////////////////////////////////
static void AssertQuickHullValid(CQuickHull* pcHull, SFloat32Vec3* asPoints, size uiNumPoints)
{
	size			i;
	size			j;
	SQuickHullFace*	psFace;
	float32			fTolerance;
	float32			fLargest;
	float32			fDistance;
	size			uiNumCornersOfFaces;
	CArrayInt		aiVertices;
	SFloat32Vec3	sSide1;
	SFloat32Vec3	sSide2;
	SFloat32Vec3	sCross;
	SFloat32Vec3*	ps0;
	SFloat32Vec3*	ps1;
	SFloat32Vec3*	ps2;

	fTolerance = pcHull->GetTolerance();
	uiNumCornersOfFaces = 0;
	for (i = 0; i < pcHull->NumFaces(); i++)
	{
		psFace = pcHull->GetFace(i);
		AssertTrue(psFace->uiNumCorners >= 3);
		uiNumCornersOfFaces += psFace->uiNumCorners;

		fLargest = -1.0f;
		for (j = 0; j < uiNumPoints; j++)
		{
			fDistance = psFace->sPlane.Distance(&asPoints[j]);
			if (fDistance > fLargest)
			{
				fLargest = fDistance;
			}
		}
		AssertTrue(fLargest <= fTolerance * 2.0f);

		for (j = 0; j < psFace->uiNumCorners; j++)
		{
			AssertTrue(fabsf(psFace->sPlane.Distance(&asPoints[pcHull->GetCorner(psFace, j)])) <= fTolerance * 2.0f);
		}

		ps0 = &asPoints[pcHull->GetCorner(psFace, 0)];
		ps1 = &asPoints[pcHull->GetCorner(psFace, 1)];
		ps2 = &asPoints[pcHull->GetCorner(psFace, 2)];
		Float32Vec3Subtract(&sSide1, ps1, ps0);
		Float32Vec3Subtract(&sSide2, ps2, ps0);
		Float32Vec3Cross(&sCross, &sSide1, &sSide2);
		AssertTrue(Float32Vec3Dot(&sCross, &psFace->sPlane.sNormal) > 0.0f);
	}

	aiVertices.Init();
	pcHull->GetVertices(&aiVertices);
	AssertSize(0, uiNumCornersOfFaces % 2);
	AssertInt(2, (int)aiVertices.NumElements() - (int)(uiNumCornersOfFaces / 2) + (int)pcHull->NumFaces());
	aiVertices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The corners of a cube from -1 to 1 first, then points inside it, the
// middles of its faces and the middles of its edges.
//
//////////////////////////////////////////////////////////////////////////
static size TestQuickHullCubePoints(SFloat32Vec3* asPoints)
{
	size	uiNumPoints;
	int		x;
	int		y;
	int		z;

	uiNumPoints = 0;
	for (z = -1; z <= 1; z += 2)
	{
		for (y = -1; y <= 1; y += 2)
		{
			for (x = -1; x <= 1; x += 2)
			{
				asPoints[uiNumPoints].Init((float32)x, (float32)y, (float32)z);
				uiNumPoints++;
			}
		}
	}
	for (z = -1; z <= 1; z++)
	{
		for (y = -1; y <= 1; y++)
		{
			for (x = -1; x <= 1; x++)
			{
				if ((x != 0) && (y != 0) && (z != 0))
				{
					continue;
				}
				asPoints[uiNumPoints].Init((float32)x, (float32)y, (float32)z);
				uiNumPoints++;
				asPoints[uiNumPoints].Init((float32)x * 0.5f, (float32)y * 0.25f, (float32)z * 0.75f);
				uiNumPoints++;
			}
		}
	}
	return uiNumPoints;
}


//////////////////////////////////////////////////////////////////////////
//
// A cube is six square faces, facing along the axes, and only its eight
// corners are corners: the points on its faces and edges, and inside it,
// aren't.
//
//////////////////////////////////////////////////////////////////////////
void TestQuickHullCube(void)
{
	CQuickHull		cHull;
	SFloat32Vec3	asPoints[8 + 2 * 19];
	size			uiNumPoints;
	CArrayInt		aiVertices;
	SQuickHullFace*	psFace;
	size			i;
	float32			fLargest;

	uiNumPoints = TestQuickHullCubePoints(asPoints);
	AssertSize(46, uiNumPoints);

	cHull.Init();
	AssertTrue(cHull.Generate(asPoints, sizeof(SFloat32Vec3), uiNumPoints));
	AssertSize(6, cHull.NumFaces());
	for (i = 0; i < 6; i++)
	{
		psFace = cHull.GetFace(i);
		AssertSize(4, psFace->uiNumCorners);
		fLargest = maxf(fabsf(psFace->sPlane.sNormal.x), maxf(fabsf(psFace->sPlane.sNormal.y), fabsf(psFace->sPlane.sNormal.z)));
		AssertFloat(1.0f, fLargest, 4);
		AssertFloat(1.0f, psFace->sPlane.fDistance, 4);
	}

	aiVertices.Init();
	cHull.GetVertices(&aiVertices);
	AssertSize(8, aiVertices.NumElements());
	for (i = 0; i < 8; i++)
	{
		AssertInt((int)i, aiVertices.GetValue(i));
	}
	aiVertices.Kill();

	AssertQuickHullValid(&cHull, asPoints, uiNumPoints);
	cHull.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An 8 x 4 x 2 box turned 30 degrees about y and moved, with points inside:
// six faces of four corners.  Generating again replaces the first hull.
//
//////////////////////////////////////////////////////////////////////////
void TestQuickHullTurnedBox(void)
{
	CQuickHull		cHull;
	SFloat32Vec3	asPoints[8 + 5 * 5 * 5];
	SFloat32Vec3	sLocal;
	SFloat4x4		sMatrix;
	size			uiNumPoints;
	CArrayInt		aiVertices;
	int				x;
	int				y;
	int				z;
	size			i;

	Float4x4RotationY(&sMatrix, Deg2Rad(30.0f));
	uiNumPoints = 0;
	for (z = -1; z <= 1; z += 2)
	{
		for (y = -1; y <= 1; y += 2)
		{
			for (x = -1; x <= 1; x += 2)
			{
				sLocal.Init(4.0f * (float32)x, 2.0f * (float32)y, 1.0f * (float32)z);
				Float32Vec3TransformCoord(&asPoints[uiNumPoints], &sLocal, &sMatrix);
				asPoints[uiNumPoints].x += 10.0f;
				asPoints[uiNumPoints].y -= 5.0f;
				uiNumPoints++;
			}
		}
	}
	for (z = 0; z < 5; z++)
	{
		for (y = 0; y < 5; y++)
		{
			for (x = 0; x < 5; x++)
			{
				sLocal.Init(4.0f * ((float32)x * 0.4f - 0.8f), 2.0f * ((float32)y * 0.4f - 0.8f), 1.0f * ((float32)z * 0.4f - 0.8f));
				Float32Vec3TransformCoord(&asPoints[uiNumPoints], &sLocal, &sMatrix);
				asPoints[uiNumPoints].x += 10.0f;
				asPoints[uiNumPoints].y -= 5.0f;
				uiNumPoints++;
			}
		}
	}

	cHull.Init();
	AssertTrue(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 8));
	AssertSize(6, cHull.NumFaces());
	AssertTrue(cHull.Generate(asPoints, sizeof(SFloat32Vec3), uiNumPoints));
	AssertSize(6, cHull.NumFaces());
	for (i = 0; i < 6; i++)
	{
		AssertSize(4, cHull.GetFace(i)->uiNumCorners);
	}
	aiVertices.Init();
	cHull.GetVertices(&aiVertices);
	AssertSize(8, aiVertices.NumElements());
	aiVertices.Kill();
	AssertQuickHullValid(&cHull, asPoints, uiNumPoints);
	cHull.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Points scattered through a box, checked by brute force; and points on a
// sphere, which are all corners.
//
//////////////////////////////////////////////////////////////////////////
void TestQuickHullClouds(void)
{
	CQuickHull		cHull;
	SFloat32Vec3	asPoints[200];
	uint32			uiState;
	size			i;
	CArrayInt		aiVertices;

	uiState = 12345;
	for (i = 0; i < 200; i++)
	{
		asPoints[i].Init(TestShapeRandom(&uiState) * 20.0f - 7.0f, TestShapeRandom(&uiState) * 6.0f + 100.0f, TestShapeRandom(&uiState) * 2.0f - 1.0f);
	}
	cHull.Init();
	AssertTrue(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 200));
	AssertTrue(cHull.NumFaces() >= 4);
	AssertQuickHullValid(&cHull, asPoints, 200);

	for (i = 0; i < 120; i++)
	{
		do
		{
			asPoints[i].Init(TestShapeRandom(&uiState) * 2.0f - 1.0f, TestShapeRandom(&uiState) * 2.0f - 1.0f, TestShapeRandom(&uiState) * 2.0f - 1.0f);
		}
		while (asPoints[i].SquareMagnitude() < 0.01);
		asPoints[i].Normalize();
		asPoints[i] *= 5.0f;
	}
	AssertTrue(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 120));
	AssertQuickHullValid(&cHull, asPoints, 120);
	aiVertices.Init();
	cHull.GetVertices(&aiVertices);
	AssertSize(120, aiVertices.NumElements());
	aiVertices.Kill();
	cHull.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Fewer than four points, or points in a plane, a line or one place, have
// no hull.  Four points not in a plane are four triangles.
//
//////////////////////////////////////////////////////////////////////////
void TestQuickHullDegenerate(void)
{
	CQuickHull		cHull;
	SFloat32Vec3	asPoints[16];
	size			i;

	cHull.Init();
	for (i = 0; i < 16; i++)
	{
		asPoints[i].Init((float32)(i % 4), (float32)(i / 4), 3.0f);
	}
	AssertFalse(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 3));
	AssertSize(0, cHull.NumFaces());
	AssertFalse(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 16));
	AssertSize(0, cHull.NumFaces());

	for (i = 0; i < 16; i++)
	{
		asPoints[i].Init((float32)i, (float32)i * 2.0f, 1.0f);
	}
	AssertFalse(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 16));

	for (i = 0; i < 16; i++)
	{
		asPoints[i].Init(1.0f, 2.0f, 3.0f);
	}
	AssertFalse(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 16));

	asPoints[0].Init(0, 0, 0);
	asPoints[1].Init(1, 0, 0);
	asPoints[2].Init(0, 1, 0);
	asPoints[3].Init(0, 0, 1);
	AssertTrue(cHull.Generate(asPoints, sizeof(SFloat32Vec3), 4));
	AssertSize(4, cHull.NumFaces());
	for (i = 0; i < 4; i++)
	{
		AssertSize(3, cHull.GetFace(i)->uiNumCorners);
	}
	AssertQuickHullValid(&cHull, asPoints, 4);
	cHull.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestQuickHull(void)
{
	BeginTests();

	TestQuickHullCube();
	TestQuickHullTurnedBox();
	TestQuickHullClouds();
	TestQuickHullDegenerate();

	TestStatistics();
}
