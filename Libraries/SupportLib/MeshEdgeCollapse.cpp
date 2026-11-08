/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <stdlib.h>
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Float32Vec3.h"
#include "BaseLib/Float64Vec3.h"
#include "MeshEdgeCollapse.h"


//How much a corner is held to where it started, as a part of how much it's held to a plane.  It is the standard deviation
//of the normal in a probabilistic quadric, and it's what makes A in Ax = b always solvable.  It is small so that it decides
//where a corner goes on a flat area, and doesn't count against the corner on a curved one.
#define MESH_EDGE_COLLAPSE_NORMAL_SPREAD	0.01

//How much more a corner on an open edge is held to the plane through that edge.  Faces that meet at an open edge with
//different corners (CAD's patches do) can't be more than fMaxError apart with this, so cracks can't be seen.
#define MESH_EDGE_COLLAPSE_BOUNDARY_WEIGHT	4.0

//A face can't turn further than this, about 78 degrees, in one collapse.
#define MESH_EDGE_COLLAPSE_MIN_COSINE		0.2


//The sum over a corner's planes of the square of the distance from them, x'Ax - 2b'x + c, and the sum over the points it
//started from of the square of the distance from them.  A corner's error comes from the planes alone, and they are summed
//with no regard to the size of their faces, so a tiny feature can't be lost in a large face.  As it's a sum it comes to
//more than the distance from any one of them, and what's measured on real boards is well under fMaxError.
struct SMeshEdgeCollapseQuadric
{
	float64	fA00;
	float64	fA01;
	float64	fA02;
	float64	fA11;
	float64	fA12;
	float64	fA22;
	float64	fB0;
	float64	fB1;
	float64	fB2;
	float64	fC;

	float64	fNumPoints;
	float64	fSumX;
	float64	fSumY;
	float64	fSumZ;
	float64	fSumSquares;
};


struct SMeshEdgeCollapseVertex
{
	SFloat64Vec3				sPosition;
	SMeshEdgeCollapseQuadric	sQuadric;
	int							iFirstFaceCorner;	//A list through aiNextFaceCorner, -1 at its end.
	int							iStamp;				//Changes when it does, so that candidates for what it was are ignored.
	int							iMark;
};


//A face corner is the face's index * NUM_FACE_CORNERS + which corner of it.
struct SMeshEdgeCollapseFace
{
	int	aiVertex[NUM_FACE_CORNERS];  //The first is -1 if it has gone.
	int	aiNextFaceCorner[NUM_FACE_CORNERS];
};


//An edge between two faces is on both of them, and there is one of these for each.
struct SMeshEdgeCollapseEdge
{
	int	iVertex1;
	int	iVertex2;
	int	iFace;
};


struct SMeshEdgeCollapseCandidate
{
	float64	fCost;
	int		iVertex1;
	int		iVertex2;
	int		iStamp1;
	int		iStamp2;
};


typedef CArrayTemplate<SMeshEdgeCollapseVertex>		CArrayMeshEdgeCollapseVertex;
typedef CArrayTemplate<SMeshEdgeCollapseFace>		CArrayMeshEdgeCollapseFace;
typedef CArrayTemplate<SMeshEdgeCollapseEdge>		CArrayMeshEdgeCollapseEdge;
typedef CArrayTemplate<SMeshEdgeCollapseCandidate>	CArrayMeshEdgeCollapseCandidate;


//What's known about a mesh as its edges collapse.
struct SMeshEdgeCollapse
{
	CMesh*							pcMesh;
	float64							fMaxErrorSquared;
	SFloat64Vec3					sCentre;

	CArrayMeshEdgeCollapseVertex	asVertices;
	CArrayMeshEdgeCollapseFace		asFaces;

	CArrayMeshEdgeCollapseCandidate	asHeap;
	size							uiHeapSize;

	int								iMark;
	int								iNumCollapses;
};


//////////////////////////////////////////////////////////////////////////
//
// The plane through psPoint with the unit normal psNormal.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseQuadricAddPlane(SMeshEdgeCollapseQuadric* psQuadric, SFloat64Vec3* psPoint, SFloat64Vec3* psNormal, float64 fWeight)
{
	float64	fDistance;

	fDistance = Float64Vec3Dot(psNormal, psPoint);

	psQuadric->fA00 += fWeight * psNormal->x * psNormal->x;
	psQuadric->fA01 += fWeight * psNormal->x * psNormal->y;
	psQuadric->fA02 += fWeight * psNormal->x * psNormal->z;
	psQuadric->fA11 += fWeight * psNormal->y * psNormal->y;
	psQuadric->fA12 += fWeight * psNormal->y * psNormal->z;
	psQuadric->fA22 += fWeight * psNormal->z * psNormal->z;
	psQuadric->fB0 += fWeight * fDistance * psNormal->x;
	psQuadric->fB1 += fWeight * fDistance * psNormal->y;
	psQuadric->fB2 += fWeight * fDistance * psNormal->z;
	psQuadric->fC += fWeight * fDistance * fDistance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseQuadricAddPoint(SMeshEdgeCollapseQuadric* psQuadric, SFloat64Vec3* psPoint)
{
	psQuadric->fNumPoints += 1.0;
	psQuadric->fSumX += psPoint->x;
	psQuadric->fSumY += psPoint->y;
	psQuadric->fSumZ += psPoint->z;
	psQuadric->fSumSquares += Float64Vec3Dot(psPoint, psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseQuadricSum(SMeshEdgeCollapseQuadric* psOut, SMeshEdgeCollapseQuadric* ps1, SMeshEdgeCollapseQuadric* ps2)
{
	psOut->fA00 = ps1->fA00 + ps2->fA00;
	psOut->fA01 = ps1->fA01 + ps2->fA01;
	psOut->fA02 = ps1->fA02 + ps2->fA02;
	psOut->fA11 = ps1->fA11 + ps2->fA11;
	psOut->fA12 = ps1->fA12 + ps2->fA12;
	psOut->fA22 = ps1->fA22 + ps2->fA22;
	psOut->fB0 = ps1->fB0 + ps2->fB0;
	psOut->fB1 = ps1->fB1 + ps2->fB1;
	psOut->fB2 = ps1->fB2 + ps2->fB2;
	psOut->fC = ps1->fC + ps2->fC;
	psOut->fNumPoints = ps1->fNumPoints + ps2->fNumPoints;
	psOut->fSumX = ps1->fSumX + ps2->fSumX;
	psOut->fSumY = ps1->fSumY + ps2->fSumY;
	psOut->fSumZ = ps1->fSumZ + ps2->fSumZ;
	psOut->fSumSquares = ps1->fSumSquares + ps2->fSumSquares;
}


//////////////////////////////////////////////////////////////////////////
//
// The sum of the squares of the distances from a quadric's planes, which can
// only be a little below zero and then only by rounding.
//
//////////////////////////////////////////////////////////////////////////
static float64 MeshEdgeCollapseQuadricPlaneError(SMeshEdgeCollapseQuadric* ps, SFloat64Vec3* psPoint)
{
	float64	fError;
	float64	x;
	float64	y;
	float64	z;

	x = psPoint->x;
	y = psPoint->y;
	z = psPoint->z;
	fError = ps->fA00 * x * x + ps->fA11 * y * y + ps->fA22 * z * z;
	fError += 2.0 * (ps->fA01 * x * y + ps->fA02 * x * z + ps->fA12 * y * z);
	fError -= 2.0 * (ps->fB0 * x + ps->fB1 * y + ps->fB2 * z);
	fError += ps->fC;
	return (fError > 0.0) ? fError : 0.0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float64 MeshEdgeCollapseQuadricPointError(SMeshEdgeCollapseQuadric* ps, SFloat64Vec3* psPoint)
{
	float64	fError;

	fError = ps->fNumPoints * Float64Vec3Dot(psPoint, psPoint);
	fError -= 2.0 * (ps->fSumX * psPoint->x + ps->fSumY * psPoint->y + ps->fSumZ * psPoint->z);
	fError += ps->fSumSquares;
	return (fError > 0.0) ? fError : 0.0;
}


//////////////////////////////////////////////////////////////////////////
//
// The best position, from solving Ax = b by Cholesky.  It can't fail if there
// are any points to be held to, but a corner with only faces of no area has none.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshEdgeCollapseQuadricMinimum(SMeshEdgeCollapseQuadric* ps, SFloat64Vec3* psPoint)
{
	float64	fSpread;
	float64	fA00;
	float64	fA11;
	float64	fA22;
	float64	fB0;
	float64	fB1;
	float64	fB2;
	float64	fD0;
	float64	fD1;
	float64	fD2;
	float64	fL10;
	float64	fL20;
	float64	fL21;
	float64	fY1;
	float64	fY2;
	float64	fA12MinusL20A01;

	if (ps->fNumPoints == 0.0)
	{
		return false;
	}

	fSpread = MESH_EDGE_COLLAPSE_NORMAL_SPREAD * MESH_EDGE_COLLAPSE_NORMAL_SPREAD;
	fA00 = ps->fA00 + fSpread * ps->fNumPoints;
	fA11 = ps->fA11 + fSpread * ps->fNumPoints;
	fA22 = ps->fA22 + fSpread * ps->fNumPoints;
	fB0 = ps->fB0 + fSpread * ps->fSumX;
	fB1 = ps->fB1 + fSpread * ps->fSumY;
	fB2 = ps->fB2 + fSpread * ps->fSumZ;

	fD0 = fA00;
	fL10 = ps->fA01 / fD0;
	fL20 = ps->fA02 / fD0;
	fD1 = fA11 - fL10 * ps->fA01;
	if (fD1 <= 0.0)
	{
		return false;
	}
	fA12MinusL20A01 = ps->fA12 - fL20 * ps->fA01;
	fL21 = fA12MinusL20A01 / fD1;
	fD2 = fA22 - fL20 * ps->fA02 - fL21 * fA12MinusL20A01;
	if (fD2 <= 0.0)
	{
		return false;
	}

	fY1 = fB1 - fL10 * fB0;
	fY2 = fB2 - fL20 * fB0 - fL21 * fY1;
	psPoint->z = fY2 / fD2;
	psPoint->y = fY1 / fD1 - fL21 * psPoint->z;
	psPoint->x = fB0 / fD0 - fL10 * psPoint->y - fL20 * psPoint->z;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the two ends of an edge would go and what's left when they have.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseBestPosition(SMeshEdgeCollapse* ps, int iVertex1, int iVertex2, SMeshEdgeCollapseQuadric* psQuadric, SFloat64Vec3* psPoint)
{
	SMeshEdgeCollapseVertex*	psVertex1;
	SMeshEdgeCollapseVertex*	psVertex2;

	psVertex1 = ps->asVertices.Get(iVertex1);
	psVertex2 = ps->asVertices.Get(iVertex2);
	MeshEdgeCollapseQuadricSum(psQuadric, &psVertex1->sQuadric, &psVertex2->sQuadric);
	if (!MeshEdgeCollapseQuadricMinimum(psQuadric, psPoint))
	{
		psPoint->x = (psVertex1->sPosition.x + psVertex2->sPosition.x) * 0.5;
		psPoint->y = (psVertex1->sPosition.y + psVertex2->sPosition.y) * 0.5;
		psPoint->z = (psVertex1->sPosition.z + psVertex2->sPosition.z) * 0.5;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cheapest first, and the lowest numbered first of those the same.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshEdgeCollapseIsBefore(SMeshEdgeCollapseCandidate* ps1, SMeshEdgeCollapseCandidate* ps2)
{
	if (ps1->fCost != ps2->fCost)
	{
		return ps1->fCost < ps2->fCost;
	}
	if (ps1->iVertex1 != ps2->iVertex1)
	{
		return ps1->iVertex1 < ps2->iVertex1;
	}
	return ps1->iVertex2 < ps2->iVertex2;
}


//////////////////////////////////////////////////////////////////////////
//
// An edge that would move a corner too far isn't a candidate, until one of its
// ends changes.  The heap's array isn't shrunk as it empties, as it fills again.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapsePush(SMeshEdgeCollapse* ps, int iVertex1, int iVertex2)
{
	SMeshEdgeCollapseQuadric	sQuadric;
	SFloat64Vec3				sPoint;
	SMeshEdgeCollapseCandidate	sCandidate;
	SMeshEdgeCollapseCandidate*	psParent;
	float64						fPlaneError;
	size						uiIndex;
	size						uiParent;

	MeshEdgeCollapseBestPosition(ps, iVertex1, iVertex2, &sQuadric, &sPoint);
	fPlaneError = MeshEdgeCollapseQuadricPlaneError(&sQuadric, &sPoint);
	if (fPlaneError > ps->fMaxErrorSquared)
	{
		return;
	}

	sCandidate.fCost = fPlaneError + MESH_EDGE_COLLAPSE_NORMAL_SPREAD * MESH_EDGE_COLLAPSE_NORMAL_SPREAD * MeshEdgeCollapseQuadricPointError(&sQuadric, &sPoint);
	sCandidate.iVertex1 = (iVertex1 < iVertex2) ? iVertex1 : iVertex2;
	sCandidate.iVertex2 = (iVertex1 < iVertex2) ? iVertex2 : iVertex1;
	sCandidate.iStamp1 = ps->asVertices.Get(sCandidate.iVertex1)->iStamp;
	sCandidate.iStamp2 = ps->asVertices.Get(sCandidate.iVertex2)->iStamp;

	if (ps->uiHeapSize == ps->asHeap.NumElements())
	{
		ps->asHeap.Add();
	}
	uiIndex = ps->uiHeapSize;
	ps->uiHeapSize++;
	while (uiIndex > 0)
	{
		uiParent = (uiIndex - 1) / 2;
		psParent = ps->asHeap.Get(uiParent);
		if (!MeshEdgeCollapseIsBefore(&sCandidate, psParent))
		{
			break;
		}
		*ps->asHeap.Get(uiIndex) = *psParent;
		uiIndex = uiParent;
	}
	*ps->asHeap.Get(uiIndex) = sCandidate;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapsePop(SMeshEdgeCollapse* ps, SMeshEdgeCollapseCandidate* psCandidate)
{
	SMeshEdgeCollapseCandidate	sLast;
	SMeshEdgeCollapseCandidate*	psChild;
	size						uiIndex;
	size						uiChild;

	*psCandidate = *ps->asHeap.Get(0);
	ps->uiHeapSize--;
	sLast = *ps->asHeap.Get(ps->uiHeapSize);

	uiIndex = 0;
	for (;;)
	{
		uiChild = uiIndex * 2 + 1;
		if (uiChild >= ps->uiHeapSize)
		{
			break;
		}
		if ((uiChild + 1 < ps->uiHeapSize) && MeshEdgeCollapseIsBefore(ps->asHeap.Get(uiChild + 1), ps->asHeap.Get(uiChild)))
		{
			uiChild++;
		}
		psChild = ps->asHeap.Get(uiChild);
		if (!MeshEdgeCollapseIsBefore(psChild, &sLast))
		{
			break;
		}
		*ps->asHeap.Get(uiIndex) = *psChild;
		uiIndex = uiChild;
	}
	if (ps->uiHeapSize > 0)
	{
		*ps->asHeap.Get(uiIndex) = sLast;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A face's unnormalised normal, with iVertex at psPoint rather than where it is.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseFaceNormal(SMeshEdgeCollapse* ps, SMeshEdgeCollapseFace* psFace, int iVertex, SFloat64Vec3* psPoint, SFloat64Vec3* psNormal)
{
	SFloat64Vec3*	apsCorners[NUM_FACE_CORNERS];
	SFloat64Vec3	sB;
	SFloat64Vec3	sC;
	int				i;

	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		apsCorners[i] = (psFace->aiVertex[i] == iVertex) ? psPoint : &ps->asVertices.Get(psFace->aiVertex[i])->sPosition;
	}
	Float64Vec3Subtract(&sB, apsCorners[1], apsCorners[0]);
	Float64Vec3Subtract(&sC, apsCorners[2], apsCorners[0]);
	Float64Vec3Cross(psNormal, &sB, &sC);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool MeshEdgeCollapseFaceContains(SMeshEdgeCollapseFace* psFace, int iVertex)
{
	return (psFace->aiVertex[0] == iVertex) || (psFace->aiVertex[1] == iVertex) || (psFace->aiVertex[2] == iVertex);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a face that has iVertex in it still faces about the same way with
// iVertex at psPoint, and still has some area.  One with no area to start with
// faces no way.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshEdgeCollapseKeepsFacing(SMeshEdgeCollapse* ps, SMeshEdgeCollapseFace* psFace, int iVertex, SFloat64Vec3* psPoint)
{
	SFloat64Vec3	sOld;
	SFloat64Vec3	sNew;
	float64			fOld;
	float64			fNew;
	float64			fDot;

	MeshEdgeCollapseFaceNormal(ps, psFace, -1, NULL, &sOld);
	MeshEdgeCollapseFaceNormal(ps, psFace, iVertex, psPoint, &sNew);
	fOld = Float64Vec3Dot(&sOld, &sOld);
	fNew = Float64Vec3Dot(&sNew, &sNew);
	if (fOld == 0.0)
	{
		return true;
	}
	if (fNew == 0.0)
	{
		return false;
	}
	fDot = Float64Vec3Dot(&sOld, &sNew);
	return (fDot > 0.0) && (fDot * fDot >= MESH_EDGE_COLLAPSE_MIN_COSINE * MESH_EDGE_COLLAPSE_MIN_COSINE * fOld * fNew);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether every face at iVertex but those on the edge keeps facing the same way.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshEdgeCollapseFacesKeepFacing(SMeshEdgeCollapse* ps, int iVertex, int iOtherVertex, SFloat64Vec3* psPoint)
{
	SMeshEdgeCollapseFace*	psFace;
	int						iFaceCorner;

	iFaceCorner = ps->asVertices.Get(iVertex)->iFirstFaceCorner;
	while (iFaceCorner >= 0)
	{
		psFace = ps->asFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
		if ((psFace->aiVertex[0] >= 0) && !MeshEdgeCollapseFaceContains(psFace, iOtherVertex))
		{
			if (!MeshEdgeCollapseKeepsFacing(ps, psFace, iVertex, psPoint))
			{
				return false;
			}
		}
		iFaceCorner = psFace->aiNextFaceCorner[iFaceCorner % NUM_FACE_CORNERS];
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether joining an edge's ends leaves the surface connected as it was: the
// only corners both are next to must be those across the edge's own faces, or
// the surface would pinch there.  A face at one end that is across from the other
// would become a repeat of another, which is also what stops a closed part
// collapsing to nothing.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshEdgeCollapseKeepsTopology(SMeshEdgeCollapse* ps, int iVertex1, int iVertex2)
{
	SMeshEdgeCollapseVertex*	psOther;
	SMeshEdgeCollapseFace*		psFace;
	int							iFaceCorner;
	int							iNearVertex1;
	int							iCountedBoth;
	int							iNumEdgeFaces;
	int							iNumShared;
	int							iNumOthersNear;
	int							i;

	ps->iMark += 2;
	iNearVertex1 = ps->iMark;
	iCountedBoth = ps->iMark + 1;

	iFaceCorner = ps->asVertices.Get(iVertex1)->iFirstFaceCorner;
	while (iFaceCorner >= 0)
	{
		psFace = ps->asFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
		if (psFace->aiVertex[0] >= 0)
		{
			for (i = 0; i < NUM_FACE_CORNERS; i++)
			{
				ps->asVertices.Get(psFace->aiVertex[i])->iMark = iNearVertex1;
			}
		}
		iFaceCorner = psFace->aiNextFaceCorner[iFaceCorner % NUM_FACE_CORNERS];
	}

	iNumEdgeFaces = 0;
	iNumShared = 0;
	iFaceCorner = ps->asVertices.Get(iVertex2)->iFirstFaceCorner;
	while (iFaceCorner >= 0)
	{
		psFace = ps->asFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
		if (psFace->aiVertex[0] >= 0)
		{
			iNumOthersNear = 0;
			for (i = 0; i < NUM_FACE_CORNERS; i++)
			{
				if ((psFace->aiVertex[i] != iVertex1) && (psFace->aiVertex[i] != iVertex2))
				{
					psOther = ps->asVertices.Get(psFace->aiVertex[i]);
					if (psOther->iMark == iNearVertex1)
					{
						iNumShared++;
						psOther->iMark = iCountedBoth;
						iNumOthersNear++;
					}
					else if (psOther->iMark == iCountedBoth)
					{
						iNumOthersNear++;
					}
				}
			}

			if (MeshEdgeCollapseFaceContains(psFace, iVertex1))
			{
				iNumEdgeFaces++;
			}
			else if (iNumOthersNear == 2)
			{
				return false;
			}
		}
		iFaceCorner = psFace->aiNextFaceCorner[iFaceCorner % NUM_FACE_CORNERS];
	}
	return iNumShared == iNumEdgeFaces;
}


//////////////////////////////////////////////////////////////////////////
//
// Joins iVertex2 to iVertex1 at psPoint.  The faces on the edge go, and the
// list of the faces that stay is rebuilt without those that have gone.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseJoin(SMeshEdgeCollapse* ps, int iVertex1, int iVertex2, SFloat64Vec3* psPoint, SMeshEdgeCollapseQuadric* psQuadric)
{
	SMeshEdgeCollapseVertex*	psVertex1;
	SMeshEdgeCollapseVertex*	psVertex2;
	SMeshEdgeCollapseVertex*	psOther;
	SMeshEdgeCollapseFace*		psFace;
	int							iFaceCorner;
	int							iNext;
	int							iFirst;
	int							iCorner;
	int							i;

	psVertex1 = ps->asVertices.Get(iVertex1);
	psVertex2 = ps->asVertices.Get(iVertex2);

	iFirst = -1;
	iFaceCorner = psVertex1->iFirstFaceCorner;
	while (iFaceCorner >= 0)
	{
		iCorner = iFaceCorner % NUM_FACE_CORNERS;
		psFace = ps->asFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
		iNext = psFace->aiNextFaceCorner[iCorner];
		if ((psFace->aiVertex[0] >= 0) && MeshEdgeCollapseFaceContains(psFace, iVertex2))
		{
			psFace->aiVertex[0] = -1;
		}
		if (psFace->aiVertex[0] >= 0)
		{
			psFace->aiNextFaceCorner[iCorner] = iFirst;
			iFirst = iFaceCorner;
		}
		iFaceCorner = iNext;
	}

	iFaceCorner = psVertex2->iFirstFaceCorner;
	while (iFaceCorner >= 0)
	{
		iCorner = iFaceCorner % NUM_FACE_CORNERS;
		psFace = ps->asFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
		iNext = psFace->aiNextFaceCorner[iCorner];
		if (psFace->aiVertex[0] >= 0)
		{
			psFace->aiVertex[iCorner] = iVertex1;
			psFace->aiNextFaceCorner[iCorner] = iFirst;
			iFirst = iFaceCorner;
		}
		iFaceCorner = iNext;
	}

	psVertex1->sPosition = *psPoint;
	psVertex1->sQuadric = *psQuadric;
	psVertex1->iFirstFaceCorner = iFirst;
	psVertex1->iStamp++;
	psVertex2->iFirstFaceCorner = -1;
	psVertex2->iStamp = -1;
	ps->iNumCollapses++;

	//Each of its edges is now a different one.
	ps->iMark++;
	psVertex1->iMark = ps->iMark;
	iFaceCorner = iFirst;
	while (iFaceCorner >= 0)
	{
		psFace = ps->asFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
		for (i = 0; i < NUM_FACE_CORNERS; i++)
		{
			psOther = ps->asVertices.Get(psFace->aiVertex[i]);
			if (psOther->iMark != ps->iMark)
			{
				psOther->iMark = ps->iMark;
				MeshEdgeCollapsePush(ps, iVertex1, psFace->aiVertex[i]);
			}
		}
		iFaceCorner = psFace->aiNextFaceCorner[iFaceCorner % NUM_FACE_CORNERS];
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool MeshEdgeCollapseCanJoinAt(SMeshEdgeCollapse* ps, int iVertex1, int iVertex2, SFloat64Vec3* psPoint, SMeshEdgeCollapseQuadric* psQuadric)
{
	return (MeshEdgeCollapseQuadricPlaneError(psQuadric, psPoint) <= ps->fMaxErrorSquared) &&
		MeshEdgeCollapseFacesKeepFacing(ps, iVertex1, iVertex2, psPoint) &&
		MeshEdgeCollapseFacesKeepFacing(ps, iVertex2, iVertex1, psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
// The best position first.  When that would turn a face over one of the edge's
// ends may not, and on a flat area there's no cost to using it.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseTryCandidate(SMeshEdgeCollapse* ps, SMeshEdgeCollapseCandidate* psCandidate)
{
	SMeshEdgeCollapseQuadric	sQuadric;
	SFloat64Vec3				sPoint;

	if (!MeshEdgeCollapseKeepsTopology(ps, psCandidate->iVertex1, psCandidate->iVertex2))
	{
		return;
	}

	MeshEdgeCollapseBestPosition(ps, psCandidate->iVertex1, psCandidate->iVertex2, &sQuadric, &sPoint);
	if (!MeshEdgeCollapseCanJoinAt(ps, psCandidate->iVertex1, psCandidate->iVertex2, &sPoint, &sQuadric))
	{
		sPoint = ps->asVertices.Get(psCandidate->iVertex1)->sPosition;
		if (!MeshEdgeCollapseCanJoinAt(ps, psCandidate->iVertex1, psCandidate->iVertex2, &sPoint, &sQuadric))
		{
			sPoint = ps->asVertices.Get(psCandidate->iVertex2)->sPosition;
			if (!MeshEdgeCollapseCanJoinAt(ps, psCandidate->iVertex1, psCandidate->iVertex2, &sPoint, &sQuadric))
			{
				return;
			}
		}
	}
	MeshEdgeCollapseJoin(ps, psCandidate->iVertex1, psCandidate->iVertex2, &sPoint, &sQuadric);
}


//////////////////////////////////////////////////////////////////////////
//
// Every edge of the mesh as it is.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapsePushAll(SMeshEdgeCollapse* ps)
{
	SMeshEdgeCollapseVertex*	psOther;
	SMeshEdgeCollapseFace*		psFace;
	int							iVertex;
	int							iNumVertices;
	int							iFaceCorner;
	int							i;

	iNumVertices = (int)ps->asVertices.NumElements();
	for (iVertex = 0; iVertex < iNumVertices; iVertex++)
	{
		ps->iMark++;
		ps->asVertices.Get(iVertex)->iMark = ps->iMark;
		iFaceCorner = ps->asVertices.Get(iVertex)->iFirstFaceCorner;
		while (iFaceCorner >= 0)
		{
			psFace = ps->asFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
			if (psFace->aiVertex[0] >= 0)
			{
				for (i = 0; i < NUM_FACE_CORNERS; i++)
				{
					psOther = ps->asVertices.Get(psFace->aiVertex[i]);
					if (psOther->iMark != ps->iMark)
					{
						psOther->iMark = ps->iMark;
						if (iVertex < psFace->aiVertex[i])
						{
							MeshEdgeCollapsePush(ps, iVertex, psFace->aiVertex[i]);
						}
					}
				}
			}
			iFaceCorner = psFace->aiNextFaceCorner[iFaceCorner % NUM_FACE_CORNERS];
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An edge turned down isn't tried again when the faces around it change, so
// it goes round again until there's nothing more that can go.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseCollapse(SMeshEdgeCollapse* ps)
{
	SMeshEdgeCollapseCandidate	sCandidate;
	int							iLastNumCollapses;

	do
	{
		iLastNumCollapses = ps->iNumCollapses;
		MeshEdgeCollapsePushAll(ps);
		while (ps->uiHeapSize > 0)
		{
			MeshEdgeCollapsePop(ps, &sCandidate);
			if ((ps->asVertices.Get(sCandidate.iVertex1)->iStamp == sCandidate.iStamp1) && (ps->asVertices.Get(sCandidate.iVertex2)->iStamp == sCandidate.iStamp2))
			{
				MeshEdgeCollapseTryCandidate(ps, &sCandidate);
			}
		}
	}
	while (ps->iNumCollapses != iLastNumCollapses);
}


//////////////////////////////////////////////////////////////////////////
//
// The vertices about the middle of the mesh, as a quadric is more accurate near
// zero, and the faces with their lists.  A face that repeats a corner goes.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseLoad(SMeshEdgeCollapse* ps)
{
	SMeshEdgeCollapseVertex*	psVertex;
	SMeshEdgeCollapseFace*		psFace;
	SFloat32Vec3*				psPosition;
	SFloat64Vec3				sMin;
	SFloat64Vec3				sMax;
	int*						piCorners;
	int							iNumCorners;
	int							iNumFaces;
	int							i;
	int							k;

	iNumCorners = ps->pcMesh->NumCorners();
	iNumFaces = ps->pcMesh->NumFaces();
	ps->asVertices.Resize(iNumCorners);
	ps->asFaces.Resize(iNumFaces);

	sMin.Init(0.0, 0.0, 0.0);
	sMax.Init(0.0, 0.0, 0.0);
	for (i = 0; i < iNumCorners; i++)
	{
		psPosition = ps->pcMesh->GetPosition(i);
		if (i == 0)
		{
			sMin.Init(psPosition->x, psPosition->y, psPosition->z);
			sMax = sMin;
		}
		sMin.x = (psPosition->x < sMin.x) ? psPosition->x : sMin.x;
		sMin.y = (psPosition->y < sMin.y) ? psPosition->y : sMin.y;
		sMin.z = (psPosition->z < sMin.z) ? psPosition->z : sMin.z;
		sMax.x = (psPosition->x > sMax.x) ? psPosition->x : sMax.x;
		sMax.y = (psPosition->y > sMax.y) ? psPosition->y : sMax.y;
		sMax.z = (psPosition->z > sMax.z) ? psPosition->z : sMax.z;
	}
	ps->sCentre.Init((sMin.x + sMax.x) * 0.5, (sMin.y + sMax.y) * 0.5, (sMin.z + sMax.z) * 0.5);

	for (i = 0; i < iNumCorners; i++)
	{
		psPosition = ps->pcMesh->GetPosition(i);
		psVertex = ps->asVertices.Get(i);
		psVertex->sPosition.Init(psPosition->x - ps->sCentre.x, psPosition->y - ps->sCentre.y, psPosition->z - ps->sCentre.z);
		memset(&psVertex->sQuadric, 0, sizeof(SMeshEdgeCollapseQuadric));
		psVertex->iFirstFaceCorner = -1;
		psVertex->iStamp = 0;
		psVertex->iMark = 0;
	}

	for (i = 0; i < iNumFaces; i++)
	{
		piCorners = ps->pcMesh->GetFaceCorners(i);
		psFace = ps->asFaces.Get(i);
		if ((piCorners[0] == piCorners[1]) || (piCorners[1] == piCorners[2]) || (piCorners[2] == piCorners[0]))
		{
			psFace->aiVertex[0] = -1;
			psFace->aiVertex[1] = -1;
			psFace->aiVertex[2] = -1;
		}
		else
		{
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				psFace->aiVertex[k] = piCorners[k];
				psVertex = ps->asVertices.Get(piCorners[k]);
				psFace->aiNextFaceCorner[k] = psVertex->iFirstFaceCorner;
				psVertex->iFirstFaceCorner = i * NUM_FACE_CORNERS + k;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each corner of a face is held to the face's plane and to where it started.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseAddFaces(SMeshEdgeCollapse* ps)
{
	SMeshEdgeCollapseVertex*	psVertex;
	SMeshEdgeCollapseFace*		psFace;
	SFloat64Vec3				sNormal;
	float64						fLength;
	size						u;
	size						uiNumElements;
	int							k;

	uiNumElements = ps->asFaces.NumElements();
	for (u = 0; u < uiNumElements; u++)
	{
		psFace = ps->asFaces.Get(u);
		if (psFace->aiVertex[0] >= 0)
		{
			MeshEdgeCollapseFaceNormal(ps, psFace, -1, NULL, &sNormal);
			fLength = sqrt(Float64Vec3Dot(&sNormal, &sNormal));
			if (fLength > 0.0)
			{
				sNormal.x /= fLength;
				sNormal.y /= fLength;
				sNormal.z /= fLength;
				for (k = 0; k < NUM_FACE_CORNERS; k++)
				{
					psVertex = ps->asVertices.Get(psFace->aiVertex[k]);
					MeshEdgeCollapseQuadricAddPlane(&psVertex->sQuadric, &psVertex->sPosition, &sNormal, 1.0);
					MeshEdgeCollapseQuadricAddPoint(&psVertex->sQuadric, &psVertex->sPosition);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int __cdecl MeshEdgeCollapseCompareEdges(const void* pv1, const void* pv2)
{
	SMeshEdgeCollapseEdge*	ps1;
	SMeshEdgeCollapseEdge*	ps2;

	ps1 = (SMeshEdgeCollapseEdge*)pv1;
	ps2 = (SMeshEdgeCollapseEdge*)pv2;
	if (ps1->iVertex1 != ps2->iVertex1)
	{
		return (ps1->iVertex1 < ps2->iVertex1) ? -1 : 1;
	}
	if (ps1->iVertex2 != ps2->iVertex2)
	{
		return (ps1->iVertex2 < ps2->iVertex2) ? -1 : 1;
	}
	return (ps1->iFace < ps2->iFace) ? -1 : ((ps1->iFace > ps2->iFace) ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Every edge of every face, with those the same together.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseSortEdges(SMeshEdgeCollapse* ps, CArrayMeshEdgeCollapseEdge* pasEdges)
{
	SMeshEdgeCollapseFace*	psFace;
	SMeshEdgeCollapseEdge*	psEdge;
	size					u;
	size					uiNumElements;
	int						k;
	int						iVertex1;
	int						iVertex2;

	uiNumElements = ps->asFaces.NumElements();
	for (u = 0; u < uiNumElements; u++)
	{
		psFace = ps->asFaces.Get(u);
		if (psFace->aiVertex[0] >= 0)
		{
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				iVertex1 = psFace->aiVertex[k];
				iVertex2 = psFace->aiVertex[(k + 1) % NUM_FACE_CORNERS];
				psEdge = pasEdges->Add();
				psEdge->iVertex1 = (iVertex1 < iVertex2) ? iVertex1 : iVertex2;
				psEdge->iVertex2 = (iVertex1 < iVertex2) ? iVertex2 : iVertex1;
				psEdge->iFace = (int)u;
			}
		}
	}
	if (pasEdges->NumElements() > 0)
	{
		qsort(pasEdges->GetData(), pasEdges->NumElements(), sizeof(SMeshEdgeCollapseEdge), MeshEdgeCollapseCompareEdges);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A face with no colour isn't the colour of any value.
//
//////////////////////////////////////////////////////////////////////////
static int64 MeshEdgeCollapseFaceColour(SMeshEdgeCollapse* ps, int iFace)
{
	int	iColour;

	if (!ps->pcMesh->mcColours.InUse())
	{
		return -1;
	}
	iColour = ps->pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours[0];
	if (iColour < 0)
	{
		return -1;
	}
	return (int64)(uint32)ps->pcMesh->mcColours.mcColours.GetValue(iColour);
}


//////////////////////////////////////////////////////////////////////////
//
// The ends of an edge are held to the plane through it at a right angle to
// the face, so the edge can't move across that plane and only corners along
// the edge can go.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseHoldEdge(SMeshEdgeCollapse* ps, SMeshEdgeCollapseEdge* psEdge)
{
	SMeshEdgeCollapseVertex*	psVertex1;
	SMeshEdgeCollapseVertex*	psVertex2;
	SFloat64Vec3				sNormal;
	SFloat64Vec3				sAlong;
	SFloat64Vec3				sPlane;
	float64						fLength;

	MeshEdgeCollapseFaceNormal(ps, ps->asFaces.Get(psEdge->iFace), -1, NULL, &sNormal);
	psVertex1 = ps->asVertices.Get(psEdge->iVertex1);
	psVertex2 = ps->asVertices.Get(psEdge->iVertex2);
	Float64Vec3Subtract(&sAlong, &psVertex2->sPosition, &psVertex1->sPosition);
	Float64Vec3Cross(&sPlane, &sAlong, &sNormal);
	fLength = sqrt(Float64Vec3Dot(&sPlane, &sPlane));
	if (fLength > 0.0)
	{
		sPlane.x /= fLength;
		sPlane.y /= fLength;
		sPlane.z /= fLength;
		MeshEdgeCollapseQuadricAddPlane(&psVertex1->sQuadric, &psVertex1->sPosition, &sPlane, MESH_EDGE_COLLAPSE_BOUNDARY_WEIGHT);
		MeshEdgeCollapseQuadricAddPlane(&psVertex2->sQuadric, &psVertex2->sPosition, &sPlane, MESH_EDGE_COLLAPSE_BOUNDARY_WEIGHT);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Holds the edges that aren't the join of two faces of a colour.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseAddEdges(SMeshEdgeCollapse* ps)
{
	CArrayMeshEdgeCollapseEdge	asEdges;
	SMeshEdgeCollapseEdge*		psEdge;
	SMeshEdgeCollapseEdge*		psNext;
	size						u;
	size						v;
	size						uiNumElements;

	asEdges.Init();
	MeshEdgeCollapseSortEdges(ps, &asEdges);

	uiNumElements = asEdges.NumElements();
	for (u = 0; u < uiNumElements; u = v)
	{
		psEdge = asEdges.Get(u);
		for (v = u + 1; v < uiNumElements; v++)
		{
			psNext = asEdges.Get(v);
			if ((psNext->iVertex1 != psEdge->iVertex1) || (psNext->iVertex2 != psEdge->iVertex2))
			{
				break;
			}
		}

		if ((v - u != 2) || (MeshEdgeCollapseFaceColour(ps, psEdge->iFace) != MeshEdgeCollapseFaceColour(ps, asEdges.Get(u + 1)->iFace)))
		{
			for (; u < v; u++)
			{
				MeshEdgeCollapseHoldEdge(ps, asEdges.Get(u));
			}
		}
	}

	asEdges.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The corners back where the mesh has them, and the corners of each face.
//
//////////////////////////////////////////////////////////////////////////
static void MeshEdgeCollapseResults(SMeshEdgeCollapse* ps, CArrayFloat32Vec3* pasCorners, CArrayInt* paiFaceCorners)
{
	SMeshEdgeCollapseVertex*	psVertex;
	SMeshEdgeCollapseFace*		psFace;
	SFloat32Vec3*				psCorner;
	size						u;
	size						uiNumElements;
	int							k;

	uiNumElements = ps->asVertices.NumElements();
	pasCorners->Resize(uiNumElements);
	for (u = 0; u < uiNumElements; u++)
	{
		psVertex = ps->asVertices.Get(u);
		psCorner = pasCorners->Get(u);
		psCorner->x = (float32)(psVertex->sPosition.x + ps->sCentre.x);
		psCorner->y = (float32)(psVertex->sPosition.y + ps->sCentre.y);
		psCorner->z = (float32)(psVertex->sPosition.z + ps->sCentre.z);
	}

	uiNumElements = ps->asFaces.NumElements();
	paiFaceCorners->Resize(uiNumElements * NUM_FACE_CORNERS);
	for (u = 0; u < uiNumElements; u++)
	{
		psFace = ps->asFaces.Get(u);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			paiFaceCorners->SetValue(u * NUM_FACE_CORNERS + k, psFace->aiVertex[k]);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshEdgeCollapse::Init(float32 fMaxError)
{
	mfMaxError = fMaxError;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshEdgeCollapse::Modify(Ptr<CMesh> pMesh)
{
	SMeshEdgeCollapse	sCollapse;
	CArrayFloat32Vec3	asCorners;
	CArrayInt			aiFaceCorners;
	Ptr<CMesh>			pOutput;

	if ((pMesh->NumFaces() == 0) || (mfMaxError <= 0.0f) || !CanRebuild(&pMesh))
	{
		return pMesh;
	}

	sCollapse.pcMesh = &pMesh;
	sCollapse.fMaxErrorSquared = (float64)mfMaxError * mfMaxError;
	sCollapse.asVertices.Init();
	sCollapse.asFaces.Init();
	sCollapse.asHeap.Init();
	sCollapse.uiHeapSize = 0;
	sCollapse.iMark = 0;
	sCollapse.iNumCollapses = 0;

	MeshEdgeCollapseLoad(&sCollapse);
	MeshEdgeCollapseAddFaces(&sCollapse);
	MeshEdgeCollapseAddEdges(&sCollapse);
	MeshEdgeCollapseCollapse(&sCollapse);

	pOutput = pMesh;
	if (sCollapse.iNumCollapses > 0)
	{
		asCorners.Init();
		aiFaceCorners.Init();
		MeshEdgeCollapseResults(&sCollapse, &asCorners, &aiFaceCorners);
		pOutput = BuildFromFaces(&pMesh, asCorners.GetData(), (int)asCorners.NumElements(), aiFaceCorners.GetData());
		aiFaceCorners.Kill();
		asCorners.Kill();
	}

	sCollapse.asHeap.Kill();
	sCollapse.asFaces.Kill();
	sCollapse.asVertices.Kill();
	return pOutput;
}
