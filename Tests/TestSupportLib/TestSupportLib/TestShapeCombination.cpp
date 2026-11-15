#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Float4x4.h"
#include "BaseLib/Numbers.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Box.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/ShapeUnion.h"
#include "SupportLib/ShapeIntersection.h"
#include "SupportLib/ShapeSubtraction.h"
#include "SupportLib/ShapeSmoothUnion.h"
#include "SupportLib/ShapeTransform.h"
#include "TestLib/Assert.h"


void			AssertShapeGradient(CShape* pcShape, SFloat32Vec3* psPosition, float32 fX, float32 fY, float32 fZ);
bool			TestShapeWrite(char* szDirectory, CShape* pcShape);
CBaseObject*	TestShapeRead(char* szDirectory, const char* szName);
size			CountMeshMemoryObjects(const char* szClassName);
float32			TestShapeRandom(uint32* puiState);


#define SHAPE_BOUND_CELLS	12
#define SHAPE_BOUND_PROBES	30


//////////////////////////////////////////////////////////////////////////
//
// The shape's distance is never more than the true distance to its surface.
// The surface is sampled where Contains changes along the edges of a grid
// over its bounds, each crossing bisected; a probe's distance to the
// nearest sample is no less than the true distance, and more by at most
// about a cell, so the shape's distance mustn't be more than that plus a
// cell.  Inside, its distance mustn't be further below zero.
//
//////////////////////////////////////////////////////////////////////////
void AssertShapeIsBound(CShape* pcShape)
{
	SFloat32Bounds		sBounds;
	SFloat32Vec3		sSize;
	SFloat32Vec3		sCell;
	SFloat32Vec3		sCorner;
	SFloat32Vec3		sNext;
	SFloat32Vec3		sInside;
	SFloat32Vec3		sOutside;
	SFloat32Vec3		sMiddle;
	SFloat32Vec3		sProbe;
	SFloat32Vec3		sOffset;
	CArrayFloat32Vec3	asSurface;
	bool				abInside[(SHAPE_BOUND_CELLS + 1) * (SHAPE_BOUND_CELLS + 1) * (SHAPE_BOUND_CELLS + 1)];
	int					x;
	int					y;
	int					z;
	int					iAxis;
	int					iStep;
	int					iIndex;
	int					iNext;
	size				i;
	uint32				uiState;
	float32				fCell;
	float32				fNearest;
	float32				fDistance;

	pcShape->GetBounds(&sBounds);
	sBounds.sMinimum.Init(sBounds.sMinimum.x - 1.0f, sBounds.sMinimum.y - 1.0f, sBounds.sMinimum.z - 1.0f);
	sBounds.sMaximum.Init(sBounds.sMaximum.x + 1.0f, sBounds.sMaximum.y + 1.0f, sBounds.sMaximum.z + 1.0f);
	sBounds.GetSize(&sSize);
	sCell = sSize / (float32)SHAPE_BOUND_CELLS;
	fCell = sCell.Magnitude();

	for (z = 0; z <= SHAPE_BOUND_CELLS; z++)
	{
		for (y = 0; y <= SHAPE_BOUND_CELLS; y++)
		{
			for (x = 0; x <= SHAPE_BOUND_CELLS; x++)
			{
				sCorner.Init(sBounds.sMinimum.x + sCell.x * (float32)x, sBounds.sMinimum.y + sCell.y * (float32)y, sBounds.sMinimum.z + sCell.z * (float32)z);
				abInside[(z * (SHAPE_BOUND_CELLS + 1) + y) * (SHAPE_BOUND_CELLS + 1) + x] = pcShape->Contains(&sCorner);
			}
		}
	}

	asSurface.Init();
	for (z = 0; z <= SHAPE_BOUND_CELLS; z++)
	{
		for (y = 0; y <= SHAPE_BOUND_CELLS; y++)
		{
			for (x = 0; x <= SHAPE_BOUND_CELLS; x++)
			{
				iIndex = (z * (SHAPE_BOUND_CELLS + 1) + y) * (SHAPE_BOUND_CELLS + 1) + x;
				for (iAxis = 0; iAxis < 3; iAxis++)
				{
					if (((iAxis == 0) && (x == SHAPE_BOUND_CELLS)) || ((iAxis == 1) && (y == SHAPE_BOUND_CELLS)) || ((iAxis == 2) && (z == SHAPE_BOUND_CELLS)))
					{
						continue;
					}
					iNext = iIndex + ((iAxis == 0) ? 1 : ((iAxis == 1) ? (SHAPE_BOUND_CELLS + 1) : (SHAPE_BOUND_CELLS + 1) * (SHAPE_BOUND_CELLS + 1)));
					if (abInside[iIndex] == abInside[iNext])
					{
						continue;
					}

					sCorner.Init(sBounds.sMinimum.x + sCell.x * (float32)x, sBounds.sMinimum.y + sCell.y * (float32)y, sBounds.sMinimum.z + sCell.z * (float32)z);
					sNext = sCorner;
					if (iAxis == 0)
					{
						sNext.x += sCell.x;
					}
					else if (iAxis == 1)
					{
						sNext.y += sCell.y;
					}
					else
					{
						sNext.z += sCell.z;
					}
					sInside = abInside[iIndex] ? sCorner : sNext;
					sOutside = abInside[iIndex] ? sNext : sCorner;
					for (iStep = 0; iStep < 12; iStep++)
					{
						sMiddle = (sInside + sOutside) / 2.0f;
						if (pcShape->Contains(&sMiddle))
						{
							sInside = sMiddle;
						}
						else
						{
							sOutside = sMiddle;
						}
					}
					*asSurface.Add() = sOutside;
				}
			}
		}
	}
	AssertTrue(asSurface.NumElements() > 0);

	uiState = 271828;
	for (iIndex = 0; iIndex < SHAPE_BOUND_PROBES; iIndex++)
	{
		sProbe.Init(sBounds.sMinimum.x + sSize.x * TestShapeRandom(&uiState),
					sBounds.sMinimum.y + sSize.y * TestShapeRandom(&uiState),
					sBounds.sMinimum.z + sSize.z * TestShapeRandom(&uiState));
		fNearest = FLOAT32_BOUNDS_LARGEST;
		for (i = 0; i < asSurface.NumElements(); i++)
		{
			Float32Vec3Subtract(&sOffset, &sProbe, asSurface.Get(i));
			fDistance = sOffset.Magnitude();
			if (fDistance < fNearest)
			{
				fNearest = fDistance;
			}
		}
		fDistance = pcShape->Distance(&sProbe);
		AssertTrue(fabsf(fDistance) <= fNearest + fCell);
	}
	asSurface.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Its gradient agrees with central differences at a point away from its
// creases.
//
//////////////////////////////////////////////////////////////////////////
static void AssertGradientIsDifferences(CShape* pcShape, SFloat32Vec3* psPosition)
{
	SFloat32Vec3	sGradient;
	SFloat32Vec3	sDifferences;

	pcShape->Gradient(&sGradient, psPosition);
	pcShape->CShape::Gradient(&sDifferences, psPosition);
	AssertFloat(sGradient.x, sDifferences.x, 2);
	AssertFloat(sGradient.y, sDifferences.y, 2);
	AssertFloat(sGradient.z, sDifferences.z, 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Two spheres of radius 1, at the origin and at (1.5, 0, 0).
//
//////////////////////////////////////////////////////////////////////////
static void TestShapeCombinationSpheres(Ptr<CSphere>& pSphere1, Ptr<CSphere>& pSphere2)
{
	SFloat32Vec3	sCenter;

	sCenter.Init(0, 0, 0);
	pSphere1 = OMalloc<CSphere>(&sCenter, 1.0f);
	sCenter.Init(1.5f, 0, 0);
	pSphere2 = OMalloc<CSphere>(&sCenter, 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// Two overlapping spheres: the nearer's distance and gradient, and inside
// where they overlap only a bound.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeCombinationUnion(void)
{
	Ptr<CShapeUnion>	pUnion;
	Ptr<CSphere>		pSphere1;
	Ptr<CSphere>		pSphere2;
	SFloat32Vec3		sTest;
	SFloat32Bounds		sBounds;

	TestShapeCombinationSpheres(pSphere1, pSphere2);
	pUnion = OMalloc<CShapeUnion>();
	ORoot()->Add(pUnion);
	pUnion->Add(pSphere1);
	pUnion->Add(pSphere2);
	AssertSize(2, pUnion->NumShapes());
	AssertFalse(pUnion->IsExact());

	sTest.Init(-2, 0, 0);
	AssertFloat(1.0f, pUnion->Distance(&sTest), 4);
	AssertShapeGradient(&pUnion, &sTest, -1, 0, 0);
	sTest.Init(3.5f, 0, 0);
	AssertFloat(1.0f, pUnion->Distance(&sTest), 4);
	AssertShapeGradient(&pUnion, &sTest, 1, 0, 0);
	sTest.Init(0.75f, 0, 0);
	AssertFloat(-0.25f, pUnion->Distance(&sTest), 4);
	AssertTrue(pUnion->Contains(&sTest));

	pUnion->GetBounds(&sBounds);
	AssertFloat(-1.0f, sBounds.sMinimum.x, 4);
	AssertFloat(2.5f, sBounds.sMaximum.x, 4);
	AssertShapeIsBound(&pUnion);

	ORoot()->Remove(pUnion);
	pUnion = NULL;
	pSphere1 = NULL;
	pSphere2 = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The lens where the two spheres overlap: the further's distance, and the
// bounds where theirs overlap.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeCombinationIntersection(void)
{
	Ptr<CShapeIntersection>	pIntersection;
	Ptr<CSphere>			pSphere1;
	Ptr<CSphere>			pSphere2;
	SFloat32Vec3			sTest;
	SFloat32Bounds			sBounds;

	TestShapeCombinationSpheres(pSphere1, pSphere2);
	pIntersection = OMalloc<CShapeIntersection>();
	ORoot()->Add(pIntersection);
	pIntersection->Add(pSphere1);
	pIntersection->Add(pSphere2);

	sTest.Init(-2, 0, 0);
	AssertFloat(2.5f, pIntersection->Distance(&sTest), 4);
	AssertShapeGradient(&pIntersection, &sTest, -1, 0, 0);
	sTest.Init(0.75f, 0, 0);
	AssertFloat(-0.25f, pIntersection->Distance(&sTest), 4);
	sTest.Init(0, 0, 0);
	AssertFloat(0.5f, pIntersection->Distance(&sTest), 4);
	AssertFalse(pIntersection->Contains(&sTest));

	pIntersection->GetBounds(&sBounds);
	AssertFloat(0.5f, sBounds.sMinimum.x, 4);
	AssertFloat(1.0f, sBounds.sMaximum.x, 4);
	AssertFloat(-1.0f, sBounds.sMinimum.y, 4);
	AssertShapeIsBound(&pIntersection);

	ORoot()->Remove(pIntersection);
	pIntersection = NULL;
	pSphere1 = NULL;
	pSphere2 = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A box 4 across with a sphere of radius 1 cut out of its middle: the hole
// is outside, and its gradient there points into the hole, away from the
// box's material.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeCombinationSubtraction(void)
{
	Ptr<CShapeSubtraction>	pSubtraction;
	Ptr<CBox>				pBox;
	Ptr<CSphere>			pSphere;
	SFloat32Box				sBox;
	SFloat32Vec3			asCorners[2];
	SFloat32Vec3			sCenter;
	SFloat32Vec3			sTest;
	SFloat32Bounds			sBounds;

	asCorners[0].Init(-2, -2, -2);
	asCorners[1].Init(2, 2, 2);
	sBox.Init(asCorners, sizeof(SFloat32Vec3), 2);
	pBox = OMalloc<CBox>(&sBox);
	sCenter.Init(0, 0, 0);
	pSphere = OMalloc<CSphere>(&sCenter, 1.0f);
	pSubtraction = OMalloc<CShapeSubtraction>(pBox, pSphere);
	ORoot()->Add(pSubtraction);
	AssertFalse(pSubtraction->IsExact());

	AssertFloat(1.0f, pSubtraction->Distance(&sCenter), 4);
	AssertFalse(pSubtraction->Contains(&sCenter));
	sTest.Init(0.5f, 0, 0);
	AssertFloat(0.5f, pSubtraction->Distance(&sTest), 4);
	AssertShapeGradient(&pSubtraction, &sTest, -1, 0, 0);
	sTest.Init(1.5f, 0, 0);
	AssertFloat(-0.5f, pSubtraction->Distance(&sTest), 4);
	AssertTrue(pSubtraction->Contains(&sTest));
	sTest.Init(3, 0.5f, 0);
	AssertFloat(1.0f, pSubtraction->Distance(&sTest), 4);
	AssertShapeGradient(&pSubtraction, &sTest, 1, 0, 0);

	pSubtraction->GetBounds(&sBounds);
	AssertFloat(-2.0f, sBounds.sMinimum.x, 4);
	AssertFloat(2.0f, sBounds.sMaximum.z, 4);
	AssertShapeIsBound(&pSubtraction);

	ORoot()->Remove(pSubtraction);
	pSubtraction = NULL;
	pBox = NULL;
	pSphere = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Two spheres of radius 1 touching at (1, 0, 0), joined over 0.5: where
// they touch it comes in by a quarter of that, far away it's the nearer's,
// and its gradient is exact through the blend.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeCombinationSmoothUnion(void)
{
	Ptr<CShapeSmoothUnion>	pSmooth;
	Ptr<CSphere>			pSphere1;
	Ptr<CSphere>			pSphere2;
	SFloat32Vec3			sCenter;
	SFloat32Vec3			sTest;
	SFloat32Bounds			sBounds;

	sCenter.Init(0, 0, 0);
	pSphere1 = OMalloc<CSphere>(&sCenter, 1.0f);
	sCenter.Init(2, 0, 0);
	pSphere2 = OMalloc<CSphere>(&sCenter, 1.0f);
	pSmooth = OMalloc<CShapeSmoothUnion>(pSphere1, pSphere2, 0.5f);
	ORoot()->Add(pSmooth);
	AssertFalse(pSmooth->IsExact());

	sTest.Init(1, 0, 0);
	AssertFloat(-0.125f, pSmooth->Distance(&sTest), 4);
	sTest.Init(-3, 0, 0);
	AssertFloat(2.0f, pSmooth->Distance(&sTest), 4);
	AssertShapeGradient(&pSmooth, &sTest, -1, 0, 0);
	sTest.Init(1, 0.6f, 0);
	AssertGradientIsDifferences(&pSmooth, &sTest);
	sTest.Init(0.9f, 0.8f, 0.3f);
	AssertGradientIsDifferences(&pSmooth, &sTest);

	pSmooth->GetBounds(&sBounds);
	AssertFloat(-1.125f, sBounds.sMinimum.x, 4);
	AssertFloat(3.125f, sBounds.sMaximum.x, 4);
	AssertShapeIsBound(&pSmooth);

	ORoot()->Remove(pSmooth);
	pSmooth = NULL;
	pSphere1 = NULL;
	pSphere2 = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A sphere of radius 1 scaled by 2, turned and moved to (5, 0, 0) is a
// sphere of radius 2 there, exactly.  Scaled by 2 along x only it's an
// ellipsoid, and its distance only a bound.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeCombinationTransform(void)
{
	Ptr<CShapeTransform>	pTransform;
	Ptr<CSphere>			pSphere;
	SFloat4x4				sScale;
	SFloat4x4				sTurn;
	SFloat4x4				sMove;
	SFloat4x4				sMatrix;
	SFloat32Vec3			sCenter;
	SFloat32Vec3			sTest;
	SFloat32Bounds			sBounds;

	sCenter.Init(0, 0, 0);
	pSphere = OMalloc<CSphere>(&sCenter, 1.0f);

	Float4x4Identity(&sScale);
	sScale.x.x = 2.0f;
	sScale.y.y = 2.0f;
	sScale.z.z = 2.0f;
	Float4x4RotationY(&sTurn, Deg2Rad(30.0f));
	Float4x4Translation(&sMove, 5, 0, 0);
	Float4x4Multiply(&sMatrix, &sScale, &sTurn);
	Float4x4Multiply(&sMatrix, &sMatrix, &sMove);
	pTransform = OMalloc<CShapeTransform>(pSphere, &sMatrix);
	ORoot()->Add(pTransform);
	AssertTrue(pTransform->IsExact());
	AssertFloat(2.0f, pTransform->mfScale, 4);

	sTest.Init(8, 0, 0);
	AssertFloat(1.0f, pTransform->Distance(&sTest), 4);
	AssertShapeGradient(&pTransform, &sTest, 1, 0, 0);
	sTest.Init(5, 0.5f, 0);
	AssertFloat(-1.5f, pTransform->Distance(&sTest), 4);
	sTest.Init(6, 1, 3);
	AssertGradientIsDifferences(&pTransform, &sTest);
	pTransform->GetBounds(&sBounds);
	AssertTrue(sBounds.Distance(&sTest) < 2.0f);
	sTest.Init(7, 0, 0);
	AssertTrue(sBounds.Distance(&sTest) <= 0.0001f);
	AssertShapeIsBound(&pTransform);
	ORoot()->Remove(pTransform);
	pTransform = NULL;

	Float4x4Identity(&sScale);
	sScale.x.x = 2.0f;
	Float4x4Multiply(&sMatrix, &sScale, &sMove);
	pTransform = OMalloc<CShapeTransform>(pSphere, &sMatrix);
	ORoot()->Add(pTransform);
	AssertFalse(pTransform->IsExact());
	AssertFloat(1.0f, pTransform->mfScale, 4);
	sTest.Init(5, 3, 0);
	AssertFloat(2.0f, pTransform->Distance(&sTest), 4);
	sTest.Init(9, 0, 0);
	AssertFloat(1.0f, pTransform->Distance(&sTest), 4);
	AssertFalse(pTransform->Contains(&sCenter));
	sTest.Init(6.5f, 0, 0);
	AssertTrue(pTransform->Contains(&sTest));
	AssertShapeIsBound(&pTransform);
	ORoot()->Remove(pTransform);
	pTransform = NULL;
	pSphere = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A union of a subtraction and a moved sphere is one graph: saved from its
// root, it reads back with the same distances, and dropping its root frees
// all of it.
//
//////////////////////////////////////////////////////////////////////////
void TestShapeCombinationGraph(void)
{
	char			szDirectory[] = "Output" _FS_ "TestShapeCombination";
	CFileUtil		cFileUtil;
	float32			afDistances[4];
	int				i;
	uint32			uiState;
	SFloat32Vec3	asProbes[4];

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
	AssertTrue(cFileUtil.TouchDir(szDirectory));
	uiState = 31415;
	for (i = 0; i < 4; i++)
	{
		asProbes[i].Init(TestShapeRandom(&uiState) * 10.0f - 3.0f, TestShapeRandom(&uiState) * 6.0f - 3.0f, TestShapeRandom(&uiState) * 6.0f - 3.0f);
	}

	ObjectsInit();
	{
		Ptr<CShapeUnion>		pUnion;
		Ptr<CShapeSubtraction>	pSubtraction;
		Ptr<CShapeTransform>	pTransform;
		Ptr<CBox>				pBox;
		Ptr<CSphere>			pSphere;
		SFloat32Box				sBox;
		SFloat32Vec3			asCorners[2];
		SFloat32Vec3			sCenter;
		SFloat4x4				sMove;

		asCorners[0].Init(-2, -2, -2);
		asCorners[1].Init(2, 2, 2);
		sBox.Init(asCorners, sizeof(SFloat32Vec3), 2);
		pBox = OMalloc<CBox>(&sBox);
		sCenter.Init(0, 0, 0);
		pSphere = OMalloc<CSphere>(&sCenter, 1.0f);
		pSubtraction = OMalloc<CShapeSubtraction>(pBox, pSphere);
		Float4x4Translation(&sMove, 5, 0, 0);
		pTransform = OMalloc<CShapeTransform>(pSphere, &sMove);
		pUnion = gcObjects.Malloc<CShapeUnion>("Graph");
		pUnion->Init();
		pUnion->Add(pSubtraction);
		pUnion->Add(pTransform);
		for (i = 0; i < 4; i++)
		{
			afDistances[i] = pUnion->Distance(&asProbes[i]);
		}
		AssertTrue(TestShapeWrite(szDirectory, &pUnion));

		ORoot()->Add(pUnion);
		pBox = NULL;
		pSphere = NULL;
		pSubtraction = NULL;
		pTransform = NULL;
		AssertSize(1, CountMeshMemoryObjects("CSphere"));
		ORoot()->Remove(pUnion);
		pUnion = NULL;
		AssertSize(0, CountMeshMemoryObjects("CSphere"));
		AssertSize(0, CountMeshMemoryObjects("CBox"));
		AssertSize(0, CountMeshMemoryObjects("CShapeSubtraction"));
		AssertSize(0, CountMeshMemoryObjects("CShapeTransform"));
	}
	ObjectsDiscard();
	ObjectsKill();

	ObjectsInit();
	{
		Ptr<CShapeUnion>	pUnion;

		gcObjects.AddConstructor<CShapeUnion>();
		gcObjects.AddConstructor<CShapeSubtraction>();
		gcObjects.AddConstructor<CShapeTransform>();
		gcObjects.AddConstructor<CBox>();
		gcObjects.AddConstructor<CSphere>();
		gcObjects.AddConstructor<CArray<CShape> >();

		pUnion = TestShapeRead(szDirectory, "Graph");
		AssertNotNull(&pUnion);
		AssertSize(2, pUnion->NumShapes());
		for (i = 0; i < 4; i++)
		{
			AssertFloat(afDistances[i], pUnion->Distance(&asProbes[i]), 4);
		}
		pUnion = NULL;
	}
	ObjectsDiscard();
	ObjectsKill();

	AssertTrue(cFileUtil.RemoveDir(szDirectory));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestShapeCombination(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestShapeCombinationUnion();
	TestShapeCombinationIntersection();
	TestShapeCombinationSubtraction();
	TestShapeCombinationSmoothUnion();
	TestShapeCombinationTransform();
	ObjectsDiscard();
	ObjectsKill();

	TestShapeCombinationGraph();
	DataIOKill();

	TestStatistics();
}
