#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Float4x4.h"
#include "BaseLib/Numbers.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Box.h"
#include "SupportLib/ConvexHull.h"
#include "TestLib/Assert.h"


void			AssertShapeGradient(CShape* pcShape, SFloat32Vec3* psPosition, float32 fX, float32 fY, float32 fZ);
void			AssertShapeIsBound(CShape* pcShape);
bool			TestShapeWrite(char* szDirectory, CShape* pcShape);
CBaseObject*	TestShapeRead(char* szDirectory, const char* szName);
float32			TestShapeRandom(uint32* puiState);


//////////////////////////////////////////////////////////////////////////
//
// The corners of a box from its centre along its axes, then points inside
// it on a grid.
//
//////////////////////////////////////////////////////////////////////////
static size TestConvexHullBoxPoints(SFloat32Vec3* asPoints, SFloat32Box* psBox)
{
	size	uiNumPoints;
	int		i;
	int		x;
	int		y;
	int		z;

	uiNumPoints = 0;
	for (i = 0; i < 8; i++)
	{
		psBox->GetCorner(&asPoints[uiNumPoints], i);
		uiNumPoints++;
	}
	for (z = 0; z < 3; z++)
	{
		for (y = 0; y < 3; y++)
		{
			for (x = 0; x < 3; x++)
			{
				asPoints[uiNumPoints] = psBox->sCenter + psBox->asAxes[0] * (psBox->afHalfLengths[0] * ((float32)x - 1.0f) * 0.5f)
														+ psBox->asAxes[1] * (psBox->afHalfLengths[1] * ((float32)y - 1.0f) * 0.5f)
														+ psBox->asAxes[2] * (psBox->afHalfLengths[2] * ((float32)z - 1.0f) * 0.5f);
				uiNumPoints++;
			}
		}
	}
	return uiNumPoints;
}


//////////////////////////////////////////////////////////////////////////
//
// The hull of a box's points is the box: its distance is the box's inside,
// exactly, and no more than the box's outside, where past an edge or a
// corner the box's is further.
//
//////////////////////////////////////////////////////////////////////////
static void AssertConvexHullIsBox(CConvexHull* pcHull, SFloat32Box* psBox)
{
	SFloat32Bounds	sBounds;
	SFloat32Vec3	sSize;
	SFloat32Vec3	sTest;
	uint32			uiState;
	int				i;
	float32			fHull;
	float32			fBox;

	psBox->GetBounds(&sBounds);
	sBounds.GetSize(&sSize);
	uiState = 4242;
	for (i = 0; i < 200; i++)
	{
		sTest.Init(sBounds.sMinimum.x - sSize.x + 3.0f * sSize.x * TestShapeRandom(&uiState),
				   sBounds.sMinimum.y - sSize.y + 3.0f * sSize.y * TestShapeRandom(&uiState),
				   sBounds.sMinimum.z - sSize.z + 3.0f * sSize.z * TestShapeRandom(&uiState));
		fHull = pcHull->Distance(&sTest);
		fBox = psBox->Distance(&sTest);
		if (fBox < 0.0f)
		{
			AssertFloat(fBox, fHull, 3);
		}
		else
		{
			AssertTrue(fHull <= fBox + 0.0001f);
			AssertTrue(fHull > 0.0f);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A cube from -1 to 1 from its corners and points inside: six planes and
// eight corners.  Beyond a face its distance is exact; past a corner it's
// the furthest plane's, a bound.
//
//////////////////////////////////////////////////////////////////////////
void TestConvexHullCube(void)
{
	Ptr<CConvexHull>	pHull;
	SFloat32Box			sBox;
	SFloat32Vec3		asCorners[2];
	SFloat32Vec3		asPoints[8 + 27];
	SFloat32Vec3		sTest;
	SFloat32Bounds		sBounds;
	size				uiNumPoints;

	asCorners[0].Init(-1, -1, -1);
	asCorners[1].Init(1, 1, 1);
	sBox.Init(asCorners, sizeof(SFloat32Vec3), 2);
	uiNumPoints = TestConvexHullBoxPoints(asPoints, &sBox);
	pHull = OMalloc<CConvexHull>(asPoints, sizeof(SFloat32Vec3), uiNumPoints);
	ORoot()->Add(pHull);

	AssertSize(6, pHull->NumPlanes());
	AssertSize(8, pHull->mcCorners.NumElements());
	AssertFalse(pHull->IsExact());

	sTest.Init(0, 0, 0);
	AssertFloat(-1.0f, pHull->Distance(&sTest), 4);
	AssertTrue(pHull->Contains(&sTest));
	sTest.Init(2, 0.2f, 0.1f);
	AssertFloat(1.0f, pHull->Distance(&sTest), 4);
	AssertShapeGradient(&pHull, &sTest, 1, 0, 0);
	sTest.Init(0.5f, -3, 0.5f);
	AssertFloat(2.0f, pHull->Distance(&sTest), 4);
	AssertShapeGradient(&pHull, &sTest, 0, -1, 0);
	sTest.Init(2, 2, 2);
	AssertFloat(1.0f, pHull->Distance(&sTest), 4);
	sTest.Init(1, 0, 0);
	AssertFalse(pHull->Contains(&sTest));

	pHull->GetBounds(&sBounds);
	AssertFloat(-1.0f, sBounds.sMinimum.x, 4);
	AssertFloat(1.0f, sBounds.sMaximum.z, 4);

	AssertConvexHullIsBox(&pHull, &sBox);
	AssertShapeIsBound(&pHull);

	ORoot()->Remove(pHull);
	pHull = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An 8 x 4 x 2 box turned 30 degrees about y and moved is its own hull.
//
//////////////////////////////////////////////////////////////////////////
void TestConvexHullTurnedBox(void)
{
	Ptr<CConvexHull>	pHull;
	SFloat32Box			sBox;
	SFloat4x4			sMatrix;
	SFloat32Vec3		sCenter;
	SFloat32Vec3		asAxes[3];
	SFloat32Vec3		asPoints[8 + 27];
	size				uiNumPoints;

	Float4x4RotationY(&sMatrix, Deg2Rad(30.0f));
	asAxes[0].Init(sMatrix.x.x, sMatrix.x.y, sMatrix.x.z);
	asAxes[1].Init(sMatrix.y.x, sMatrix.y.y, sMatrix.y.z);
	asAxes[2].Init(sMatrix.z.x, sMatrix.z.y, sMatrix.z.z);
	sCenter.Init(10, -5, 3);
	sBox.Init(&sCenter, &asAxes[0], 4.0f, &asAxes[1], 2.0f, &asAxes[2], 1.0f);
	uiNumPoints = TestConvexHullBoxPoints(asPoints, &sBox);
	pHull = OMalloc<CConvexHull>(asPoints, sizeof(SFloat32Vec3), uiNumPoints);
	ORoot()->Add(pHull);

	AssertSize(6, pHull->NumPlanes());
	AssertConvexHullIsBox(&pHull, &sBox);
	AssertShapeIsBound(&pHull);

	ORoot()->Remove(pHull);
	pHull = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Points in a plane make no hull: nothing, as far as can be from
// everywhere.
//
//////////////////////////////////////////////////////////////////////////
void TestConvexHullFlat(void)
{
	Ptr<CConvexHull>	pHull;
	SFloat32Vec3		asPoints[9];
	SFloat32Vec3		sTest;
	SFloat32Vec3		sGradient;
	int					i;

	for (i = 0; i < 9; i++)
	{
		asPoints[i].Init((float32)(i % 3), (float32)(i / 3), 1.0f);
	}
	pHull = gcObjects.Malloc<CConvexHull>();
	AssertFalse(pHull->Init(asPoints, sizeof(SFloat32Vec3), 9));
	AssertSize(0, pHull->NumPlanes());
	sTest.Init(1, 1, 1);
	AssertTrue(pHull->Distance(&sTest) >= FLOAT32_BOUNDS_LARGEST);
	AssertFalse(pHull->Contains(&sTest));
	pHull->Gradient(&sGradient, &sTest);
	AssertFloat(0.0f, sGradient.x, 4);
	pHull = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The same planes, corners and distances once saved and read back.
//
//////////////////////////////////////////////////////////////////////////
void TestConvexHullSaveLoad(void)
{
	char			szDirectory[] = "Output" _FS_ "TestConvexHull";
	CFileUtil		cFileUtil;
	SFloat32Vec3	asProbes[5];
	float32			afDistances[5];
	uint32			uiState;
	int				i;

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));
	uiState = 777;
	for (i = 0; i < 5; i++)
	{
		asProbes[i].Init(TestShapeRandom(&uiState) * 6.0f - 3.0f, TestShapeRandom(&uiState) * 6.0f - 3.0f, TestShapeRandom(&uiState) * 6.0f - 3.0f);
	}

	ObjectsInit();
	{
		Ptr<CConvexHull>	pHull;
		SFloat32Box			sBox;
		SFloat32Vec3		asCorners[2];
		SFloat32Vec3		asPoints[8 + 27];
		size				uiNumPoints;

		asCorners[0].Init(-1, -2, -1);
		asCorners[1].Init(1, 2, 0);
		sBox.Init(asCorners, sizeof(SFloat32Vec3), 2);
		uiNumPoints = TestConvexHullBoxPoints(asPoints, &sBox);
		pHull = gcObjects.Malloc<CConvexHull>("Hull");
		pHull->Init(asPoints, sizeof(SFloat32Vec3), uiNumPoints);
		for (i = 0; i < 5; i++)
		{
			afDistances[i] = pHull->Distance(&asProbes[i]);
		}
		AssertTrue(TestShapeWrite(szDirectory, &pHull));
	}
	ObjectsDiscard();
	ObjectsKill();

	ObjectsInit();
	{
		Ptr<CConvexHull>	pHull;

		gcObjects.AddConstructor<CConvexHull>();
		pHull = TestShapeRead(szDirectory, "Hull");
		AssertNotNull(&pHull);
		AssertSize(6, pHull->NumPlanes());
		AssertSize(8, pHull->mcCorners.NumElements());
		for (i = 0; i < 5; i++)
		{
			AssertFloat(afDistances[i], pHull->Distance(&asProbes[i]), 4);
		}
		pHull = NULL;
	}
	ObjectsDiscard();
	ObjectsKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestConvexHull(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestConvexHullCube();
	TestConvexHullTurnedBox();
	TestConvexHullFlat();
	ObjectsDiscard();
	ObjectsKill();

	TestConvexHullSaveLoad();
	DataIOKill();

	TestStatistics();
}
