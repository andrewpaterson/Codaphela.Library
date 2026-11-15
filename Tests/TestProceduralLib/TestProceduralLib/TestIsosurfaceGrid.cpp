#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Sphere.h"
#include "SupportLib/Box.h"
#include "ProceduralLib/IsosurfaceGrid.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// The fewest cells of the size that cover the box, centred on it: a side
// of whole cells has no cell more.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceGridCovers(void)
{
	CIsosurfaceGrid	cGrid;
	SFloat32Bounds	sBounds;
	SFloat32Vec3	sMinimum;
	SFloat32Vec3	sMaximum;

	sMinimum.Init(0, 0, 0);
	sMaximum.Init(1, 2, 3);
	sBounds.Init(&sMinimum, &sMaximum);
	cGrid.Init(&sBounds, 0.4f);
	AssertInt(3, cGrid.miCellsX);
	AssertInt(5, cGrid.miCellsY);
	AssertInt(8, cGrid.miCellsZ);
	AssertFloat(-0.1f, cGrid.msOrigin.x, 4);
	AssertFloat(0.0f, cGrid.msOrigin.y, 4);
	AssertFloat(-0.1f, cGrid.msOrigin.z, 4);
	cGrid.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every corner has the sphere's distance, an edge has a crossing exactly
// when its ends differ inside and out, and each crossing is on its edge,
// on the sphere, with the sphere's normal.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceGridSphere(void)
{
	Ptr<CSphere>			pSphere;
	CIsosurfaceGrid			cGrid;
	SFloat32Bounds			sBounds;
	SFloat32Vec3			sCentre;
	SFloat32Vec3			sMinimum;
	SFloat32Vec3			sMaximum;
	SFloat32Vec3			sCorner;
	SFloat32Vec3			sNext;
	SFloat32Vec3			sExpected;
	SIsosurfaceCrossing*	psCrossing;
	int						x;
	int						y;
	int						z;
	int						iAxis;
	int						aiNext[3];
	int						iChanges;
	int						iFound;
	bool					bDistances;
	bool					bMatches;
	bool					bOnEdges;
	float32					fFurthest;
	float32					fWorstNormal;
	float32					fAlong;

	sCentre.Init(0.1f, 0.2f, 0.3f);
	pSphere = OMalloc<CSphere>(&sCentre, 2.0f);
	sMinimum.Init(-3, -3, -3);
	sMaximum.Init(3, 3, 3);
	sBounds.Init(&sMinimum, &sMaximum);
	cGrid.Init(&sBounds, 0.5f);
	cGrid.Sample(&pSphere);

	bDistances = true;
	bMatches = true;
	bOnEdges = true;
	iChanges = 0;
	iFound = 0;
	fFurthest = 0.0f;
	fWorstNormal = 0.0f;
	for (z = 0; z <= cGrid.miCellsZ; z++)
	{
		for (y = 0; y <= cGrid.miCellsY; y++)
		{
			for (x = 0; x <= cGrid.miCellsX; x++)
			{
				cGrid.GetCornerPosition(&sCorner, x, y, z);
				if (cGrid.GetCornerDistance(x, y, z) != pSphere->Distance(&sCorner))
				{
					bDistances = false;
				}

				for (iAxis = 0; iAxis < 3; iAxis++)
				{
					aiNext[0] = x + (iAxis == 0 ? 1 : 0);
					aiNext[1] = y + (iAxis == 1 ? 1 : 0);
					aiNext[2] = z + (iAxis == 2 ? 1 : 0);
					if ((aiNext[0] > cGrid.miCellsX) || (aiNext[1] > cGrid.miCellsY) || (aiNext[2] > cGrid.miCellsZ))
					{
						continue;
					}

					cGrid.GetCornerPosition(&sNext, aiNext[0], aiNext[1], aiNext[2]);
					psCrossing = cGrid.GetCrossing(x, y, z, iAxis);
					if (IsosurfaceInside(pSphere->Distance(&sCorner)) != IsosurfaceInside(pSphere->Distance(&sNext)))
					{
						iChanges++;
						if (psCrossing == NULL)
						{
							bMatches = false;
							continue;
						}
					}
					else
					{
						if (psCrossing != NULL)
						{
							bMatches = false;
						}
						continue;
					}

					iFound++;
					fAlong = (&psCrossing->sPosition.x)[iAxis] - (&sCorner.x)[iAxis];
					if ((fAlong < 0.0f) || (fAlong > cGrid.mfCellSize) || ((&psCrossing->sPosition.x)[(iAxis + 1) % 3] != (&sCorner.x)[(iAxis + 1) % 3]) || ((&psCrossing->sPosition.x)[(iAxis + 2) % 3] != (&sCorner.x)[(iAxis + 2) % 3]))
					{
						bOnEdges = false;
					}
					fFurthest = fmaxf(fFurthest, fabsf(pSphere->Distance(&psCrossing->sPosition)));
					sExpected = psCrossing->sPosition - sCentre;
					sExpected /= sExpected.Magnitude();
					sExpected -= psCrossing->sNormal;
					fWorstNormal = fmaxf(fWorstNormal, sExpected.Magnitude());
				}
			}
		}
	}
	AssertTrue(bDistances);
	AssertTrue(bMatches);
	AssertTrue(bOnEdges);
	AssertTrue(iChanges > 100);
	AssertInt(iChanges, iFound);
	AssertInt(iChanges, cGrid.NumCrossings());
	AssertTrue(fFurthest < 0.00001f);
	AssertTrue(fWorstNormal < 0.0001f);

	cGrid.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A box whose faces lie on the grid's planes: its 27 inside corners are
// those strictly inside, as zero is outside, so the 54 edges out of them
// cross where they meet its faces, on the corners there, with the faces'
// normals.
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceGridZeroIsOutside(void)
{
	Ptr<CBox>				pBox;
	CIsosurfaceGrid			cGrid;
	SFloat32Box				sBox;
	SFloat32Bounds			sBounds;
	SFloat32Vec3			sCentre;
	SFloat32Vec3			asAxes[3];
	SFloat32Vec3			sMinimum;
	SFloat32Vec3			sMaximum;
	SIsosurfaceCrossing*	psCrossing;
	int						i;
	int						iAxis;
	bool					bOnFaces;
	bool					bNormals;
	float32					fCoordinate;

	sCentre.Init(0, 0, 0);
	asAxes[0].Init(1, 0, 0);
	asAxes[1].Init(0, 1, 0);
	asAxes[2].Init(0, 0, 1);
	sBox.Init(&sCentre, &asAxes[0], 1.0f, &asAxes[1], 1.0f, &asAxes[2], 1.0f);
	pBox = OMalloc<CBox>(&sBox);

	sMinimum.Init(-2, -2, -2);
	sMaximum.Init(2, 2, 2);
	sBounds.Init(&sMinimum, &sMaximum);
	cGrid.Init(&sBounds, 0.5f);
	cGrid.Sample(&pBox);
	AssertFloat(0.0f, cGrid.GetCornerDistance(6, 4, 4), 6);
	AssertFalse(IsosurfaceInside(cGrid.GetCornerDistance(6, 4, 4)));
	AssertInt(54, cGrid.NumCrossings());

	bOnFaces = true;
	bNormals = true;
	for (i = 0; i < cGrid.NumCrossings(); i++)
	{
		psCrossing = cGrid.masCrossings.Get(i);
		for (iAxis = 0; iAxis < 3; iAxis++)
		{
			fCoordinate = (&psCrossing->sPosition.x)[iAxis];
			if (fabsf(fCoordinate) == 1.0f)
			{
				if (fabsf((&psCrossing->sNormal.x)[iAxis] - fCoordinate) > 0.000001f)
				{
					bNormals = false;
				}
				break;
			}
		}
		if (iAxis == 3)
		{
			bOnFaces = false;
		}
	}
	AssertTrue(bOnFaces);
	AssertTrue(bNormals);

	psCrossing = cGrid.GetCrossing(5, 4, 4, 0);
	AssertNotNull(psCrossing);
	AssertFloat3(SFloat32Vec3(1.0f, 0.0f, 0.0f), &psCrossing->sPosition, 6);
	AssertNull(cGrid.GetCrossing(6, 4, 4, 0));

	cGrid.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestIsosurfaceGrid(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();
	TestIsosurfaceGridCovers();
	TestIsosurfaceGridSphere();
	TestIsosurfaceGridZeroIsOutside();
	ObjectsDiscard();
	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}

