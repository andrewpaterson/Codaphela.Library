#include <math.h>
#include "BaseLib/GlobalDataTypesIO.h"
#include "BaseLib/Logger.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "SupportLib/MeshEdgeCollapse.h"
#include "SupportLib/MeshModifierStack.h"
#include "SupportLib/MeshWeldCorners.h"
#include "TestLib/Assert.h"


#define TEST_MESH_EDGE_COLLAPSE_GREY	0
#define TEST_MESH_EDGE_COLLAPSE_DARK	1


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshEdgeCollapseNewMesh(void)
{
	Ptr<CMesh>	pMesh;

	pMesh = OMalloc<CMesh>();
	pMesh->mcNormals.mbInUse = true;
	pMesh->mcColours.mbInUse = true;
	pMesh->mcColours.mcColours.Add(0xFF808080);
	pMesh->mcColours.mcColours.Add(0xFF404040);
	return pMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshEdgeCollapseAddFace(CMesh* pcMesh, int iCorner1, int iCorner2, int iCorner3, int iColour)
{
	CMFRet	r;
	int		i;

	r = pcMesh->AddFace(iCorner1, iCorner2, iCorner3);
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		pcMesh->mcColours.mcFaces.Get(r.miFirstFace)->aiCornerColours[i] = iColour;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A flat grid facing up at z, of iCells x iCells squares fCell wide, of one
// colour or of iColour2 from iColourColumn along.
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshEdgeCollapseAddGrid(CMesh* pcMesh, int iCells, float32 fCell, float32 fZ, int iColour, int iColour2, int iColourColumn)
{
	int	iFirst;
	int	x;
	int	y;
	int	iRow;
	int	iFaceColour;

	iFirst = pcMesh->NumCorners();
	iRow = iCells + 1;
	for (y = 0; y <= iCells; y++)
	{
		for (x = 0; x <= iCells; x++)
		{
			pcMesh->AddCorner(x * fCell, y * fCell, fZ);
		}
	}
	for (y = 0; y < iCells; y++)
	{
		for (x = 0; x < iCells; x++)
		{
			iFaceColour = (x >= iColourColumn) ? iColour2 : iColour;
			TestMeshEdgeCollapseAddFace(pcMesh, iFirst + y * iRow + x, iFirst + y * iRow + x + 1, iFirst + y * iRow + x + iRow + 1, iFaceColour);
			TestMeshEdgeCollapseAddFace(pcMesh, iFirst + y * iRow + x, iFirst + y * iRow + x + iRow + 1, iFirst + y * iRow + x + iRow, iFaceColour);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> TestMeshEdgeCollapseOutput(CMeshModifierStack* pcStack, Ptr<CMesh> pSource, float32 fMaxError)
{
	pcStack->Init(pSource);
	pcStack->AddModifier<CMeshEdgeCollapse>()->Init(fMaxError);
	return pcStack->GetOutput();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float64 TestMeshEdgeCollapseFaceArea(CMesh* pcMesh, int iFace)
{
	SFloat32Vec3*	psA;
	SFloat32Vec3*	psB;
	SFloat32Vec3*	psC;
	int*			piCorners;
	float64			fX;
	float64			fY;
	float64			fZ;

	piCorners = pcMesh->GetFaceCorners(iFace);
	psA = pcMesh->GetPosition(piCorners[0]);
	psB = pcMesh->GetPosition(piCorners[1]);
	psC = pcMesh->GetPosition(piCorners[2]);
	fX = ((float64)psB->y - psA->y) * ((float64)psC->z - psA->z) - ((float64)psB->z - psA->z) * ((float64)psC->y - psA->y);
	fY = ((float64)psB->z - psA->z) * ((float64)psC->x - psA->x) - ((float64)psB->x - psA->x) * ((float64)psC->z - psA->z);
	fZ = ((float64)psB->x - psA->x) * ((float64)psC->y - psA->y) - ((float64)psB->y - psA->y) * ((float64)psC->x - psA->x);
	return sqrt(fX * fX + fY * fY + fZ * fZ) * 0.5;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float64 TestMeshEdgeCollapseArea(CMesh* pcMesh)
{
	float64	fArea;
	int		i;

	fArea = 0.0;
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		fArea += TestMeshEdgeCollapseFaceArea(pcMesh, i);
	}
	return fArea;
}


//////////////////////////////////////////////////////////////////////////
//
// The volume inside a closed mesh whose faces face outwards.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestMeshEdgeCollapseVolume(CMesh* pcMesh)
{
	SFloat32Vec3*	psA;
	SFloat32Vec3*	psB;
	SFloat32Vec3*	psC;
	int*			piCorners;
	float64			fVolume;
	int				i;

	fVolume = 0.0;
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		piCorners = pcMesh->GetFaceCorners(i);
		psA = pcMesh->GetPosition(piCorners[0]);
		psB = pcMesh->GetPosition(piCorners[1]);
		psC = pcMesh->GetPosition(piCorners[2]);
		fVolume += (float64)psA->x * ((float64)psB->y * psC->z - (float64)psB->z * psC->y);
		fVolume += (float64)psA->y * ((float64)psB->z * psC->x - (float64)psB->x * psC->z);
		fVolume += (float64)psA->z * ((float64)psB->x * psC->y - (float64)psB->y * psC->x);
	}
	return fVolume / 6.0;
}


//////////////////////////////////////////////////////////////////////////
//
// The distance from a point to a triangle.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestMeshEdgeCollapseDistance(SFloat64Vec3* psP, SFloat64Vec3* psA, SFloat64Vec3* psB, SFloat64Vec3* psC)
{
	SFloat64Vec3	sAB;
	SFloat64Vec3	sAC;
	SFloat64Vec3	sAP;
	SFloat64Vec3	sClosest;
	SFloat64Vec3	sDifference;
	float64			fD1;
	float64			fD2;
	float64			fD3;
	float64			fD4;
	float64			fD5;
	float64			fD6;
	float64			fVA;
	float64			fVB;
	float64			fVC;
	float64			fV;
	float64			fW;
	float64			fDenominator;

	Float64Vec3Subtract(&sAB, psB, psA);
	Float64Vec3Subtract(&sAC, psC, psA);
	Float64Vec3Subtract(&sAP, psP, psA);
	fD1 = Float64Vec3Dot(&sAB, &sAP);
	fD2 = Float64Vec3Dot(&sAC, &sAP);
	Float64Vec3Subtract(&sAP, psP, psB);
	fD3 = Float64Vec3Dot(&sAB, &sAP);
	fD4 = Float64Vec3Dot(&sAC, &sAP);
	Float64Vec3Subtract(&sAP, psP, psC);
	fD5 = Float64Vec3Dot(&sAB, &sAP);
	fD6 = Float64Vec3Dot(&sAC, &sAP);

	fVC = fD1 * fD4 - fD3 * fD2;
	fVB = fD5 * fD2 - fD1 * fD6;
	fVA = fD3 * fD6 - fD5 * fD4;

	if ((fD1 <= 0.0) && (fD2 <= 0.0))
	{
		sClosest = *psA;
	}
	else if ((fD3 >= 0.0) && (fD4 <= fD3))
	{
		sClosest = *psB;
	}
	else if ((fD6 >= 0.0) && (fD5 <= fD6))
	{
		sClosest = *psC;
	}
	else if ((fVC <= 0.0) && (fD1 >= 0.0) && (fD3 <= 0.0))
	{
		fV = fD1 / (fD1 - fD3);
		sClosest.Init(psA->x + sAB.x * fV, psA->y + sAB.y * fV, psA->z + sAB.z * fV);
	}
	else if ((fVB <= 0.0) && (fD2 >= 0.0) && (fD6 <= 0.0))
	{
		fW = fD2 / (fD2 - fD6);
		sClosest.Init(psA->x + sAC.x * fW, psA->y + sAC.y * fW, psA->z + sAC.z * fW);
	}
	else if ((fVA <= 0.0) && ((fD4 - fD3) >= 0.0) && ((fD5 - fD6) >= 0.0))
	{
		fW = (fD4 - fD3) / ((fD4 - fD3) + (fD5 - fD6));
		sClosest.Init(psB->x + (psC->x - psB->x) * fW, psB->y + (psC->y - psB->y) * fW, psB->z + (psC->z - psB->z) * fW);
	}
	else
	{
		fDenominator = 1.0 / (fVA + fVB + fVC);
		fV = fVB * fDenominator;
		fW = fVC * fDenominator;
		sClosest.Init(psA->x + sAB.x * fV + sAC.x * fW, psA->y + sAB.y * fV + sAC.y * fW, psA->z + sAB.z * fV + sAC.z * fW);
	}
	Float64Vec3Subtract(&sDifference, psP, &sClosest);
	return sqrt(Float64Vec3Dot(&sDifference, &sDifference));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshEdgeCollapseCorner(CMesh* pcMesh, int iCorner, SFloat64Vec3* ps)
{
	SFloat32Vec3*	psPosition;

	psPosition = pcMesh->GetPosition(iCorner);
	ps->Init(psPosition->x, psPosition->y, psPosition->z);
}


//////////////////////////////////////////////////////////////////////////
//
// How far f is outside the range of fA, fB and fC on one axis, or 0 if it
// is inside it.  It's never more than the distance to their triangle.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestMeshEdgeCollapseAxisGap(float64 f, float64 fA, float64 fB, float64 fC)
{
	float64	fMin;
	float64	fMax;

	fMin = (fA < fB) ? fA : fB;
	fMin = (fC < fMin) ? fC : fMin;
	fMax = (fA > fB) ? fA : fB;
	fMax = (fC > fMax) ? fC : fMax;
	if (f < fMin)
	{
		return fMin - f;
	}
	else if (f > fMax)
	{
		return f - fMax;
	}
	return 0.0;
}


//////////////////////////////////////////////////////////////////////////
//
// How far a point is from a mesh's nearest face.  A face further away on
// one axis than the nearest so far is passed over, as working out its
// distance is slow in soft float.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestMeshEdgeCollapseDistanceToMesh(SFloat64Vec3* psPoint, CMesh* pcMesh)
{
	SFloat64Vec3	sA;
	SFloat64Vec3	sB;
	SFloat64Vec3	sC;
	int*			piCorners;
	float64			fBest;
	float64			fDistance;
	int				i;

	fBest = 1e30;
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		piCorners = pcMesh->GetFaceCorners(i);
		TestMeshEdgeCollapseCorner(pcMesh, piCorners[0], &sA);
		TestMeshEdgeCollapseCorner(pcMesh, piCorners[1], &sB);
		TestMeshEdgeCollapseCorner(pcMesh, piCorners[2], &sC);
		if ((TestMeshEdgeCollapseAxisGap(psPoint->x, sA.x, sB.x, sC.x) >= fBest) ||
			(TestMeshEdgeCollapseAxisGap(psPoint->y, sA.y, sB.y, sC.y) >= fBest) ||
			(TestMeshEdgeCollapseAxisGap(psPoint->z, sA.z, sB.z, sC.z) >= fBest))
		{
			continue;
		}
		fDistance = TestMeshEdgeCollapseDistance(psPoint, &sA, &sB, &sC);
		fBest = (fDistance < fBest) ? fDistance : fBest;
	}
	return fBest;
}


//////////////////////////////////////////////////////////////////////////
//
// How far any corner or the middle of any face of one mesh is from the other.
//
//////////////////////////////////////////////////////////////////////////
static float64 TestMeshEdgeCollapseFurthest(CMesh* pcFrom, CMesh* pcTo)
{
	SFloat64Vec3	sA;
	SFloat64Vec3	sB;
	SFloat64Vec3	sC;
	SFloat64Vec3	sMiddle;
	int*			piCorners;
	float64			fFurthest;
	float64			fDistance;
	int				i;

	fFurthest = 0.0;
	for (i = 0; i < pcFrom->NumFaces(); i++)
	{
		piCorners = pcFrom->GetFaceCorners(i);
		TestMeshEdgeCollapseCorner(pcFrom, piCorners[0], &sA);
		TestMeshEdgeCollapseCorner(pcFrom, piCorners[1], &sB);
		TestMeshEdgeCollapseCorner(pcFrom, piCorners[2], &sC);
		sMiddle.Init((sA.x + sB.x + sC.x) / 3.0, (sA.y + sB.y + sC.y) / 3.0, (sA.z + sB.z + sC.z) / 3.0);

		fDistance = TestMeshEdgeCollapseDistanceToMesh(&sA, pcTo);
		fFurthest = (fDistance > fFurthest) ? fDistance : fFurthest;
		fDistance = TestMeshEdgeCollapseDistanceToMesh(&sMiddle, pcTo);
		fFurthest = (fDistance > fFurthest) ? fDistance : fFurthest;
	}
	return fFurthest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestMeshEdgeCollapseHasCorner(CMesh* pcMesh, float32 x, float32 y, float32 z, float32 fTolerance)
{
	SFloat32Vec3*	psPosition;
	int				i;

	for (i = 0; i < pcMesh->NumCorners(); i++)
	{
		psPosition = pcMesh->GetPosition(i);
		if ((fabsf(psPosition->x - x) <= fTolerance) && (fabsf(psPosition->y - y) <= fTolerance) && (fabsf(psPosition->z - z) <= fTolerance))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A flat grid has only its outline to keep.  It keeps that and covers what it
// did, and nothing changes in the source.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseFlat(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	int					i;

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 12, 10, 5, TEST_MESH_EDGE_COLLAPSE_GREY, TEST_MESH_EDGE_COLLAPSE_GREY, 12);

	pOutput = TestMeshEdgeCollapseOutput(&cStack, pSource, 0.1f);

	AssertInt(169, pSource->NumCorners());
	AssertInt(288, pSource->NumFaces());
	AssertFalse(pSource->GetConnectivity()->IsGenerated());
	AssertFloat(10, pSource->GetPosition(1)->x, 3);

	AssertTrue(pOutput->NumFaces() <= 30);  //How close it gets depends on rounding, so it isn't the same on every CPU.
	AssertFalse(pOutput->GetConnectivity()->IsGenerated());
	AssertTrue(fabs(TestMeshEdgeCollapseArea(&pOutput) - 14400.0) <= 0.1 * 480.0);  //It can move that far around its outline.
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 0, 0, 5, 0.1f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 120, 0, 5, 0.1f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 120, 120, 5, 0.1f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 0, 120, 5, 0.1f));
	for (i = 0; i < pOutput->NumCorners(); i++)
	{
		AssertFloat(5, pOutput->GetPosition(i)->z, 3);
	}
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// One side of a box 10 wide, of 5 x 5 squares with corners of its own.  U and
// V are its two directions, which cross to point out of the box, and it starts
// at O.  All are in tens.
//
//////////////////////////////////////////////////////////////////////////
static void TestMeshEdgeCollapseAddSide(CMesh* pcMesh, int iUX, int iUY, int iUZ, int iVX, int iVY, int iVZ, int iOX, int iOY, int iOZ)
{
	int	iFirst;
	int	x;
	int	y;

	iFirst = pcMesh->NumCorners();
	for (y = 0; y <= 5; y++)
	{
		for (x = 0; x <= 5; x++)
		{
			pcMesh->AddCorner((float32)(iUX * x * 2 + iVX * y * 2 + iOX * 10), (float32)(iUY * x * 2 + iVY * y * 2 + iOY * 10), (float32)(iUZ * x * 2 + iVZ * y * 2 + iOZ * 10));
		}
	}
	for (y = 0; y < 5; y++)
	{
		for (x = 0; x < 5; x++)
		{
			TestMeshEdgeCollapseAddFace(pcMesh, iFirst + y * 6 + x, iFirst + y * 6 + x + 1, iFirst + y * 6 + x + 7, TEST_MESH_EDGE_COLLAPSE_GREY);
			TestMeshEdgeCollapseAddFace(pcMesh, iFirst + y * 6 + x, iFirst + y * 6 + x + 7, iFirst + y * 6 + x + 6, TEST_MESH_EDGE_COLLAPSE_GREY);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A box exported as separate sides: welded, it comes down to about a box, with
// sharp corners and the same volume.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseBox(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	float64				fVolume;

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddSide(&pSource, 0, 1, 0, 1, 0, 0, 0, 0, 0);
	TestMeshEdgeCollapseAddSide(&pSource, 1, 0, 0, 0, 1, 0, 0, 0, 1);
	TestMeshEdgeCollapseAddSide(&pSource, 0, 0, 1, 0, 1, 0, 0, 0, 0);
	TestMeshEdgeCollapseAddSide(&pSource, 0, 1, 0, 0, 0, 1, 1, 0, 0);
	TestMeshEdgeCollapseAddSide(&pSource, 1, 0, 0, 0, 0, 1, 0, 0, 0);
	TestMeshEdgeCollapseAddSide(&pSource, 0, 0, 1, 1, 0, 0, 0, 1, 0);

	cStack.Init(pSource);
	cStack.AddModifier<CMeshWeldCorners>()->Init();
	cStack.AddModifier<CMeshEdgeCollapse>()->Init(0.01f);
	pOutput = cStack.GetOutput();

	AssertInt(300, pSource->NumFaces());
	AssertTrue(pOutput->NumFaces() >= 12);
	AssertTrue(pOutput->NumFaces() <= 60);
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 0, 0, 0, 0.01f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 10, 0, 0, 0.01f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 0, 10, 0, 0.01f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 0, 0, 10, 0.01f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 10, 10, 10, 0.01f));

	fVolume = TestMeshEdgeCollapseVolume(&pOutput);
	AssertTrue(fVolume > 999.0);
	AssertTrue(fVolume < 1001.0);
	AssertTrue(TestMeshEdgeCollapseFurthest(&pSource, &pOutput) <= 0.01);
	AssertTrue(TestMeshEdgeCollapseFurthest(&pOutput, &pSource) <= 0.01);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A tube made of far more faces than its curve needs loses most of them and
// stays within about the error of the tube it was.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseCurve(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	int					iAround;
	int					iAlong;
	int					i;
	int					j;
	int					iNext;
	float32				fAngle;
	SFloat32Vec3*		psPosition;
	float32				fRadius;

	pSource = TestMeshEdgeCollapseNewMesh();
	iAround = 48;
#ifdef __W65C816__
	//Fewer rings on the W65C816, where checking every face against every face in soft float would take more than the
	//4G instructions a 65 test program gets.
	iAlong = 3;
#else
	iAlong = 6;
#endif
	for (j = 0; j <= iAlong; j++)
	{
		for (i = 0; i < iAround; i++)
		{
			fAngle = (float32)i * 6.28318530718f / iAround;
			pSource->AddCorner(10.0f * cosf(fAngle), 10.0f * sinf(fAngle), j * 4.0f);
		}
	}
	for (j = 0; j < iAlong; j++)
	{
		for (i = 0; i < iAround; i++)
		{
			iNext = (i + 1) % iAround;
			TestMeshEdgeCollapseAddFace(&pSource, j * iAround + i, j * iAround + iNext, (j + 1) * iAround + iNext, TEST_MESH_EDGE_COLLAPSE_GREY);
			TestMeshEdgeCollapseAddFace(&pSource, j * iAround + i, (j + 1) * iAround + iNext, (j + 1) * iAround + i, TEST_MESH_EDGE_COLLAPSE_GREY);
		}
	}

	pOutput = TestMeshEdgeCollapseOutput(&cStack, pSource, 0.1f);

	AssertTrue(pOutput->NumFaces() < pSource->NumFaces() / 2);
	AssertTrue(pOutput->NumFaces() > 12);
	for (i = 0; i < pOutput->NumCorners(); i++)
	{
		psPosition = pOutput->GetPosition(i);
		fRadius = sqrtf(psPosition->x * psPosition->x + psPosition->y * psPosition->y);
		AssertTrue(fabsf(fRadius - 10.0f) <= 0.1f);
		AssertTrue((psPosition->z >= -0.1f) && (psPosition->z <= iAlong * 4.0f + 0.1f));
	}
	AssertTrue(TestMeshEdgeCollapseFurthest(&pSource, &pOutput) <= 0.2);
	AssertTrue(TestMeshEdgeCollapseFurthest(&pOutput, &pSource) <= 0.2);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A tiny spike has almost no area of a large plate's, but it's still there, as
// a corner's error doesn't depend on how big its faces are.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseKeepsSmallFeature(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	int					iCentre;
	int					iApex;
	int					iRow;
	int					iFirstOfSquare;

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 10, 10, 0, TEST_MESH_EDGE_COLLAPSE_GREY, TEST_MESH_EDGE_COLLAPSE_GREY, 10);

	//The two faces of the grid's middle square become four, around a corner raised over it.
	iRow = 11;
	iCentre = 5 * iRow + 5;
	iApex = pSource->NumCorners();
	pSource->AddCorner(55.0f, 55.0f, 1.0f);
	iFirstOfSquare = (5 * 10 + 5) * 2;
	pSource->GetFaceCorners(iFirstOfSquare)[2] = iApex;
	pSource->GetFaceCorners(iFirstOfSquare + 1)[1] = iApex;
	TestMeshEdgeCollapseAddFace(&pSource, iCentre + 1, iCentre + iRow + 1, iApex, TEST_MESH_EDGE_COLLAPSE_GREY);
	TestMeshEdgeCollapseAddFace(&pSource, iCentre + iRow + 1, iCentre + iRow, iApex, TEST_MESH_EDGE_COLLAPSE_GREY);

	pOutput = TestMeshEdgeCollapseOutput(&cStack, pSource, 0.1f);

	AssertTrue(pOutput->NumFaces() < pSource->NumFaces() / 4);
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 55, 55, 1, 0.1f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 0, 0, 0, 0.1f));
	AssertTrue(TestMeshEdgeCollapseHasCorner(&pOutput, 100, 100, 0, 0.1f));
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Two layers a hair apart, as a board's copper and solder mask are, don't touch
// so they stay that hair apart, however much error is allowed.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseKeepsLayers(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SFloat32Vec3*		psPosition;
	int					i;
	int					iLow;
	int					iHigh;

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 10, 10, 0, TEST_MESH_EDGE_COLLAPSE_GREY, TEST_MESH_EDGE_COLLAPSE_GREY, 10);
	TestMeshEdgeCollapseAddGrid(&pSource, 10, 10, 0.05f, TEST_MESH_EDGE_COLLAPSE_DARK, TEST_MESH_EDGE_COLLAPSE_DARK, 10);

	pOutput = TestMeshEdgeCollapseOutput(&cStack, pSource, 1.0f);

	AssertTrue(pOutput->NumFaces() < 16);
	iLow = 0;
	iHigh = 0;
	for (i = 0; i < pOutput->NumCorners(); i++)
	{
		psPosition = pOutput->GetPosition(i);
		if (fabsf(psPosition->z) <= 0.0001f)
		{
			iLow++;
		}
		else if (fabsf(psPosition->z - 0.05f) <= 0.0001f)
		{
			iHigh++;
		}
	}
	AssertInt(pOutput->NumCorners(), iLow + iHigh);
	AssertTrue(iLow >= 3);
	AssertTrue(iHigh >= 3);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Faces of two colours on the same corners keep the line between the colours.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseKeepsColourEdge(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	float64				afArea[2];
	int					i;
	int					iColour;

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 10, 10, 0, TEST_MESH_EDGE_COLLAPSE_GREY, TEST_MESH_EDGE_COLLAPSE_DARK, 3);

	pOutput = TestMeshEdgeCollapseOutput(&cStack, pSource, 0.1f);

	AssertTrue(pOutput->NumFaces() < pSource->NumFaces() / 4);
	afArea[0] = 0.0;
	afArea[1] = 0.0;
	for (i = 0; i < pOutput->NumFaces(); i++)
	{
		iColour = pOutput->mcColours.mcFaces.Get(i)->aiCornerColours[0];
		afArea[iColour] += TestMeshEdgeCollapseFaceArea(&pOutput, i);
	}
	AssertTrue(fabs(afArea[TEST_MESH_EDGE_COLLAPSE_GREY] - 30.0 * 100.0) < 40.0);
	AssertTrue(fabs(afArea[TEST_MESH_EDGE_COLLAPSE_DARK] - 70.0 * 100.0) < 40.0);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A closed part can't collapse to nothing however much error is allowed, and an
// edge on three faces doesn't upset it.  Corners too far apart to join come
// back as the mesh itself.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseStops(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;

	pSource = TestMeshEdgeCollapseNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(0, 10, 0);
	pSource->AddCorner(0, 0, 10);
	TestMeshEdgeCollapseAddFace(&pSource, 0, 2, 1, TEST_MESH_EDGE_COLLAPSE_GREY);
	TestMeshEdgeCollapseAddFace(&pSource, 0, 1, 3, TEST_MESH_EDGE_COLLAPSE_GREY);
	TestMeshEdgeCollapseAddFace(&pSource, 0, 3, 2, TEST_MESH_EDGE_COLLAPSE_GREY);
	TestMeshEdgeCollapseAddFace(&pSource, 1, 2, 3, TEST_MESH_EDGE_COLLAPSE_GREY);
	AssertTrue(TestMeshEdgeCollapseOutput(&cStack, pSource, 1000.0f) == pSource);
	cStack.Kill();

	pSource = TestMeshEdgeCollapseNewMesh();
	pSource->AddCorner(0, 0, 0);
	pSource->AddCorner(10, 0, 0);
	pSource->AddCorner(5, 10, 0);
	pSource->AddCorner(5, -10, 0);
	pSource->AddCorner(5, 0, 10);
	TestMeshEdgeCollapseAddFace(&pSource, 0, 1, 2, TEST_MESH_EDGE_COLLAPSE_GREY);
	TestMeshEdgeCollapseAddFace(&pSource, 1, 0, 3, TEST_MESH_EDGE_COLLAPSE_GREY);
	TestMeshEdgeCollapseAddFace(&pSource, 0, 1, 4, TEST_MESH_EDGE_COLLAPSE_GREY);
	AssertTrue(TestMeshEdgeCollapseOutput(&cStack, pSource, 0.01f) == pSource);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The faces that stay keep their names and the normals of their corners, which
// are numbered after the flat normals of the faces that stayed.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseKeepsDetails(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pOutput;
	SMeshNormalFace*	psNormalFace;
	SFloat32Vec3*		psNormal;
	int					i;
	int					k;
	int					iNormal;

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 10, 10, 0, TEST_MESH_EDGE_COLLAPSE_DARK, TEST_MESH_EDGE_COLLAPSE_DARK, 10);
	iNormal = (int)pSource->mcNormals.mcNormals.NumElements();
	pSource->AddNormal(0, 0.6f, 0.8f);
	for (i = 0; i < pSource->NumFaces(); i++)
	{
		pSource->SetFaceName(i, 7);
		psNormalFace = pSource->mcNormals.mcFaces.Get(i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psNormalFace->aiCornerNormals[k] = iNormal;
		}
	}

	pOutput = TestMeshEdgeCollapseOutput(&cStack, pSource, 0.1f);

	AssertTrue(pOutput->NumFaces() < pSource->NumFaces());
	AssertInt(pOutput->NumFaces() + 1, (int)pOutput->mcNormals.mcNormals.NumElements());
	for (i = 0; i < pOutput->NumFaces(); i++)
	{
		AssertInt(7, pOutput->GetFaceName(i));
		AssertInt(TEST_MESH_EDGE_COLLAPSE_DARK, pOutput->mcColours.mcFaces.Get(i)->aiCornerColours[0]);
		psNormalFace = pOutput->mcNormals.mcFaces.Get(i);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			AssertInt(pOutput->NumFaces(), psNormalFace->aiCornerNormals[k]);
		}
		psNormal = pOutput->mcNormals.mcNormals.Get(pOutput->NumFaces());
		AssertFloat(0.6f, psNormal->y, 3);
	}
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each time starts from the source and comes to exactly the same mesh.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseAgain(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	Ptr<CMesh>			pFirst;
	Ptr<CMesh>			pSecond;
	SFloat32Vec3*		psFirst;
	SFloat32Vec3*		psSecond;
	int					i;
	int					k;

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 8, 10, 0, TEST_MESH_EDGE_COLLAPSE_GREY, TEST_MESH_EDGE_COLLAPSE_DARK, 3);

	pFirst = TestMeshEdgeCollapseOutput(&cStack, pSource, 0.1f);
	cStack.Touch();
	pSecond = cStack.GetOutput();

	AssertFalse(pFirst == pSecond);
	AssertInt(pFirst->NumCorners(), pSecond->NumCorners());
	AssertInt(pFirst->NumFaces(), pSecond->NumFaces());
	for (i = 0; i < pFirst->NumCorners(); i++)
	{
		psFirst = pFirst->GetPosition(i);
		psSecond = pSecond->GetPosition(i);
		AssertTrue((psFirst->x == psSecond->x) && (psFirst->y == psSecond->y) && (psFirst->z == psSecond->z));
	}
	for (i = 0; i < pFirst->NumFaces(); i++)
	{
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			AssertInt(pFirst->GetFaceCorners(i)[k], pSecond->GetFaceCorners(i)[k]);
		}
	}
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With no faces, no error allowed, or what a new mesh wouldn't keep, the mesh
// itself comes back.
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapseNothingToDo(void)
{
	CMeshModifierStack	cStack;
	Ptr<CMesh>			pSource;
	SLogConfig			sLogConfig;

	pSource = TestMeshEdgeCollapseNewMesh();
	AssertTrue(TestMeshEdgeCollapseOutput(&cStack, pSource, 1.0f) == pSource);
	cStack.Kill();

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 2, 10, 0, TEST_MESH_EDGE_COLLAPSE_GREY, TEST_MESH_EDGE_COLLAPSE_GREY, 2);
	AssertTrue(TestMeshEdgeCollapseOutput(&cStack, pSource, 0.0f) == pSource);
	cStack.Kill();

	pSource = TestMeshEdgeCollapseNewMesh();
	TestMeshEdgeCollapseAddGrid(&pSource, 2, 10, 0, TEST_MESH_EDGE_COLLAPSE_GREY, TEST_MESH_EDGE_COLLAPSE_GREY, 2);
	pSource->mcSkin.mbInUse = true;
	sLogConfig = gcLogger.SetSilent();
	AssertTrue(TestMeshEdgeCollapseOutput(&cStack, pSource, 1.0f) == pSource);
	gcLogger.SetConfig(&sLogConfig);
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestMeshEdgeCollapse(void)
{
	BeginTests();

	DataIOInit();
	ObjectsInit();

	TestMeshEdgeCollapseFlat();
	TestMeshEdgeCollapseBox();
	TestMeshEdgeCollapseCurve();
	TestMeshEdgeCollapseKeepsSmallFeature();
	TestMeshEdgeCollapseKeepsLayers();
	TestMeshEdgeCollapseKeepsColourEdge();
	TestMeshEdgeCollapseStops();
	TestMeshEdgeCollapseKeepsDetails();
	TestMeshEdgeCollapseAgain();
	TestMeshEdgeCollapseNothingToDo();

	ObjectsKill(false);
	DataIOKill();

	TestStatistics();
}
