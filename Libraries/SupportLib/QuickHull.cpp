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

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/GeometricTypes.h"
#include "QuickHull.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::Init(void)
{
	mpsPoints = NULL;
	muiStride = 0;
	muiNumPoints = 0;
	mfTolerance = 0.0f;
	miVisit = 0;
	mcTriangles.Init();
	mcFaces.Init();
	maiCorners.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::Kill(void)
{
	KillTriangles();
	mcTriangles.Kill();
	mcFaces.Kill();
	maiCorners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The hull of the points, replacing what it had.  False, with no faces,
// for fewer than four points or points all in one plane.
//
//////////////////////////////////////////////////////////////////////////
bool CQuickHull::Generate(SFloat32Vec3* psPoints, size uiStride, size uiNumPoints)
{
	int		iTriangle;
	int		iEye;

	KillTriangles();
	KillFaces();
	mpsPoints = psPoints;
	muiStride = uiStride;
	muiNumPoints = uiNumPoints;
	if (uiNumPoints < 4)
	{
		return false;
	}

	mfTolerance = CalculateTolerance();
	if (!MakeTetrahedron())
	{
		KillTriangles();
		return false;
	}

	for (;;)
	{
		iTriangle = FindTriangleWithOutside();
		if (iTriangle == -1)
		{
			break;
		}
		iEye = FindFurthestOutside(iTriangle);
		if (!AddPoint(iTriangle, iEye))
		{
			KillTriangles();
			return false;
		}
	}

	MakeFaces();
	KillTriangles();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CQuickHull::NumFaces(void) const
{
	return mcFaces.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SQuickHullFace* CQuickHull::GetFace(size uiFace)
{
	return mcFaces.Get(uiFace);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CQuickHull::GetCorner(SQuickHullFace* psFace, size uiCorner)
{
	return maiCorners.GetValue(psFace->uiFirstCorner + uiCorner);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::GetVertices(CArrayInt* paiPoints)
{
	size	i;

	for (i = 0; i < maiCorners.NumElements(); i++)
	{
		paiPoints->InsertIntoSorted(maiCorners.GetValue(i), true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CQuickHull::GetTolerance(void) const
{
	return mfTolerance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat32Vec3* CQuickHull::GetPoint(int iPoint)
{
	return GetPosition(mpsPoints, muiStride, (size)iPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::KillTriangles(void)
{
	size	i;

	for (i = 0; i < mcTriangles.NumElements(); i++)
	{
		mcTriangles.Get(i)->aiOutside.Kill();
	}
	mcTriangles.Kill();
	mcTriangles.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::KillFaces(void)
{
	mcFaces.Kill();
	mcFaces.Init();
	maiCorners.Kill();
	maiCorners.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// How far in front of a plane a point must be to count, from how far the
// points reach along each axis: a float32's rounding grows with the size
// of what's rounded.
//
//////////////////////////////////////////////////////////////////////////
float32 CQuickHull::CalculateTolerance(void)
{
	size			i;
	SFloat32Vec3*	psPoint;
	float32			fX;
	float32			fY;
	float32			fZ;

	fX = 0.0f;
	fY = 0.0f;
	fZ = 0.0f;
	for (i = 0; i < muiNumPoints; i++)
	{
		psPoint = GetPoint((int)i);
		if (fabsf(psPoint->x) > fX)
		{
			fX = fabsf(psPoint->x);
		}
		if (fabsf(psPoint->y) > fY)
		{
			fY = fabsf(psPoint->y);
		}
		if (fabsf(psPoint->z) > fZ)
		{
			fZ = fabsf(psPoint->z);
		}
	}
	return (fX + fY + fZ) * QUICK_HULL_TOLERANCE;
}


//////////////////////////////////////////////////////////////////////////
//
// The two of the points furthest along each axis that are furthest apart,
// the point furthest from the line between them, and the point furthest
// from the plane of those three.  Each face is turned to have the corner it
// hasn't got behind it, and each other point goes to the first face it's
// in front of.  False if they're as good as all in one plane.
//
//////////////////////////////////////////////////////////////////////////
bool CQuickHull::MakeTetrahedron(void)
{
	int				aiExtremes[6];
	int				aiCorners[4];
	size			i;
	size			j;
	SFloat32Vec3*	psPoint;
	SFloat32Vec3	sOffset;
	SFloat32Vec3	sLine;
	SFloat32Vec3	sCross;
	float32			fLargest;
	float32			fDistance;
	float32			fLineLength;
	SFloat32Plane	sPlane;
	int				aiTriangles[4];
	CArrayInt		aiAll;

	for (i = 0; i < 6; i++)
	{
		aiExtremes[i] = 0;
	}
	for (i = 1; i < muiNumPoints; i++)
	{
		psPoint = GetPoint((int)i);
		if (psPoint->x < GetPoint(aiExtremes[0])->x) aiExtremes[0] = (int)i;
		if (psPoint->x > GetPoint(aiExtremes[1])->x) aiExtremes[1] = (int)i;
		if (psPoint->y < GetPoint(aiExtremes[2])->y) aiExtremes[2] = (int)i;
		if (psPoint->y > GetPoint(aiExtremes[3])->y) aiExtremes[3] = (int)i;
		if (psPoint->z < GetPoint(aiExtremes[4])->z) aiExtremes[4] = (int)i;
		if (psPoint->z > GetPoint(aiExtremes[5])->z) aiExtremes[5] = (int)i;
	}

	fLargest = -1.0f;
	aiCorners[0] = 0;
	aiCorners[1] = 0;
	for (i = 0; i < 6; i++)
	{
		for (j = i + 1; j < 6; j++)
		{
			Float32Vec3Subtract(&sOffset, GetPoint(aiExtremes[j]), GetPoint(aiExtremes[i]));
			fDistance = (float32)sOffset.SquareMagnitude();
			if (fDistance > fLargest)
			{
				fLargest = fDistance;
				aiCorners[0] = aiExtremes[i];
				aiCorners[1] = aiExtremes[j];
			}
		}
	}
	Float32Vec3Subtract(&sLine, GetPoint(aiCorners[1]), GetPoint(aiCorners[0]));
	fLineLength = sLine.Magnitude();
	if (fLineLength <= mfTolerance)
	{
		return false;
	}

	fLargest = -1.0f;
	aiCorners[2] = -1;
	for (i = 0; i < muiNumPoints; i++)
	{
		Float32Vec3Subtract(&sOffset, GetPoint((int)i), GetPoint(aiCorners[0]));
		Float32Vec3Cross(&sCross, &sLine, &sOffset);
		fDistance = sCross.Magnitude() / fLineLength;
		if (fDistance > fLargest)
		{
			fLargest = fDistance;
			aiCorners[2] = (int)i;
		}
	}
	if (fLargest <= mfTolerance)
	{
		return false;
	}

	sPlane.Init(GetPoint(aiCorners[0]), GetPoint(aiCorners[1]), GetPoint(aiCorners[2]));
	fLargest = -1.0f;
	aiCorners[3] = -1;
	for (i = 0; i < muiNumPoints; i++)
	{
		fDistance = fabsf(sPlane.Distance(GetPoint((int)i)));
		if (fDistance > fLargest)
		{
			fLargest = fDistance;
			aiCorners[3] = (int)i;
		}
	}
	if (fLargest <= mfTolerance)
	{
		return false;
	}

	//Face i is the corners other than i, turned so that corner i is behind it.
	for (i = 0; i < 4; i++)
	{
		int		aiFace[3];
		int		iNum;

		iNum = 0;
		for (j = 0; j < 4; j++)
		{
			if (j != i)
			{
				aiFace[iNum] = aiCorners[j];
				iNum++;
			}
		}
		sPlane.Init(GetPoint(aiFace[0]), GetPoint(aiFace[1]), GetPoint(aiFace[2]));
		if (sPlane.Distance(GetPoint(aiCorners[i])) > 0.0f)
		{
			aiTriangles[i] = AddTriangle(aiFace[0], aiFace[2], aiFace[1]);
		}
		else
		{
			aiTriangles[i] = AddTriangle(aiFace[0], aiFace[1], aiFace[2]);
		}
	}
	for (i = 0; i < 4; i++)
	{
		for (j = i + 1; j < 4; j++)
		{
			LinkNeighbours(aiTriangles[i], aiTriangles[j]);
		}
	}

	aiAll.Init();
	for (i = 0; i < 4; i++)
	{
		aiAll.Add(aiTriangles[i]);
	}
	for (i = 0; i < muiNumPoints; i++)
	{
		if (((int)i != aiCorners[0]) && ((int)i != aiCorners[1]) && ((int)i != aiCorners[2]) && ((int)i != aiCorners[3]))
		{
			GiveToTriangles((int)i, &aiAll);
		}
	}
	aiAll.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle with no neighbours and no points yet.  Its plane faces the
// side its corners go anticlockwise from.
//
//////////////////////////////////////////////////////////////////////////
int CQuickHull::AddTriangle(int i1, int i2, int i3)
{
	SQuickHullTriangle*	psTriangle;
	int					iTriangle;

	iTriangle = (int)mcTriangles.NumElements();
	psTriangle = mcTriangles.Add();
	psTriangle->aiCorners[0] = i1;
	psTriangle->aiCorners[1] = i2;
	psTriangle->aiCorners[2] = i3;
	psTriangle->aiNeighbours[0] = -1;
	psTriangle->aiNeighbours[1] = -1;
	psTriangle->aiNeighbours[2] = -1;
	psTriangle->sPlane.Init(GetPoint(i1), GetPoint(i2), GetPoint(i3));
	psTriangle->aiOutside.Init();
	psTriangle->iVisited = -1;
	psTriangle->iGroup = -1;
	psTriangle->bDeleted = false;
	return iTriangle;
}


//////////////////////////////////////////////////////////////////////////
//
// If the two share an edge (going opposite ways round them), each is the
// other's neighbour across it.
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::LinkNeighbours(int iTriangle1, int iTriangle2)
{
	SQuickHullTriangle*	psTriangle1;
	SQuickHullTriangle*	psTriangle2;
	int					i;
	int					j;

	psTriangle1 = mcTriangles.Get((size)iTriangle1);
	psTriangle2 = mcTriangles.Get((size)iTriangle2);
	for (i = 0; i < 3; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if ((psTriangle1->aiCorners[i] == psTriangle2->aiCorners[(j + 1) % 3]) && (psTriangle1->aiCorners[(i + 1) % 3] == psTriangle2->aiCorners[j]))
			{
				psTriangle1->aiNeighbours[i] = iTriangle2;
				psTriangle2->aiNeighbours[j] = iTriangle1;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CQuickHull::FindTriangleWithOutside(void)
{
	size				i;
	SQuickHullTriangle*	psTriangle;

	for (i = 0; i < mcTriangles.NumElements(); i++)
	{
		psTriangle = mcTriangles.Get(i);
		if (!psTriangle->bDeleted && (psTriangle->aiOutside.NumElements() > 0))
		{
			return (int)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CQuickHull::FindFurthestOutside(int iTriangle)
{
	SQuickHullTriangle*	psTriangle;
	size				i;
	int					iPoint;
	int					iFurthest;
	float32				fDistance;
	float32				fLargest;

	psTriangle = mcTriangles.Get((size)iTriangle);
	iFurthest = -1;
	fLargest = -1.0f;
	for (i = 0; i < psTriangle->aiOutside.NumElements(); i++)
	{
		iPoint = psTriangle->aiOutside.GetValue(i);
		fDistance = psTriangle->sPlane.Distance(GetPoint(iPoint));
		if (fDistance > fLargest)
		{
			fLargest = fDistance;
			iFurthest = iPoint;
		}
	}
	return iFurthest;
}


//////////////////////////////////////////////////////////////////////////
//
// The eye sees iTriangle and, across their edges, every triangle it's in
// front of that joins on: those go.  Their edges to triangles it can't see
// are the horizon, a loop, and a new triangle joins each to the eye.  The
// points the old triangles had go to the new ones they're in front of.
// False if the horizon isn't one loop, which rounding can only do to points
// far closer together than the tolerance.
//
//////////////////////////////////////////////////////////////////////////
bool CQuickHull::AddPoint(int iTriangle, int iEye)
{
	CArrayInt			aiVisible;
	CArrayInt			aiHorizonFrom;
	CArrayInt			aiHorizonTo;
	CArrayInt			aiHorizonOutside;
	CArrayInt			aiNew;
	SQuickHullTriangle*	psTriangle;
	SQuickHullTriangle*	psNeighbour;
	SFloat32Vec3*		psEye;
	size				uiNext;
	size				i;
	size				j;
	int					iEdge;
	int					iNeighbour;
	int					iNew;
	int					iVisible;
	bool				bResult;

	miVisit++;
	psEye = GetPoint(iEye);
	aiVisible.Init();
	aiVisible.Add(iTriangle);
	mcTriangles.Get((size)iTriangle)->iVisited = miVisit;
	for (uiNext = 0; uiNext < aiVisible.NumElements(); uiNext++)
	{
		psTriangle = mcTriangles.Get((size)aiVisible.GetValue(uiNext));
		for (iEdge = 0; iEdge < 3; iEdge++)
		{
			iNeighbour = psTriangle->aiNeighbours[iEdge];
			psNeighbour = mcTriangles.Get((size)iNeighbour);
			if ((psNeighbour->iVisited != miVisit) && (psNeighbour->sPlane.Distance(psEye) > mfTolerance))
			{
				psNeighbour->iVisited = miVisit;
				aiVisible.Add(iNeighbour);
			}
		}
	}

	aiHorizonFrom.Init();
	aiHorizonTo.Init();
	aiHorizonOutside.Init();
	for (i = 0; i < aiVisible.NumElements(); i++)
	{
		psTriangle = mcTriangles.Get((size)aiVisible.GetValue(i));
		for (iEdge = 0; iEdge < 3; iEdge++)
		{
			iNeighbour = psTriangle->aiNeighbours[iEdge];
			if (mcTriangles.Get((size)iNeighbour)->iVisited != miVisit)
			{
				aiHorizonFrom.Add(psTriangle->aiCorners[iEdge]);
				aiHorizonTo.Add(psTriangle->aiCorners[(iEdge + 1) % 3]);
				aiHorizonOutside.Add(iNeighbour);
			}
		}
	}

	//Each horizon edge's new triangle keeps the edge the way the visible triangle had it, so it faces out as that did.
	aiNew.Init();
	for (i = 0; i < aiHorizonFrom.NumElements(); i++)
	{
		iNew = AddTriangle(aiHorizonFrom.GetValue(i), aiHorizonTo.GetValue(i), iEye);
		aiNew.Add(iNew);
		LinkNeighbours(iNew, aiHorizonOutside.GetValue(i));
	}

	bResult = true;
	for (i = 0; i < aiNew.NumElements(); i++)
	{
		psTriangle = mcTriangles.Get((size)aiNew.GetValue(i));
		if (psTriangle->aiNeighbours[0] == -1)
		{
			bResult = false;
		}
		for (j = 0; j < aiNew.NumElements(); j++)
		{
			if (j != i)
			{
				LinkNeighbours(aiNew.GetValue(i), aiNew.GetValue(j));
			}
		}
		psTriangle = mcTriangles.Get((size)aiNew.GetValue(i));
		if ((psTriangle->aiNeighbours[1] == -1) || (psTriangle->aiNeighbours[2] == -1))
		{
			bResult = false;
		}
	}

	if (bResult)
	{
		for (i = 0; i < aiVisible.NumElements(); i++)
		{
			iVisible = aiVisible.GetValue(i);
			psTriangle = mcTriangles.Get((size)iVisible);
			for (j = 0; j < psTriangle->aiOutside.NumElements(); j++)
			{
				if (psTriangle->aiOutside.GetValue(j) != iEye)
				{
					GiveToTriangles(psTriangle->aiOutside.GetValue(j), &aiNew);
					psTriangle = mcTriangles.Get((size)iVisible);
				}
			}
			psTriangle->aiOutside.Kill();
			psTriangle->aiOutside.Init();
			psTriangle->bDeleted = true;
		}
	}

	aiNew.Kill();
	aiHorizonOutside.Kill();
	aiHorizonTo.Kill();
	aiHorizonFrom.Kill();
	aiVisible.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// To the first of the triangles it's in front of.  In front of none, it's
// inside the hull and is dropped.
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::GiveToTriangles(int iPoint, CArrayInt* paiTriangles)
{
	size				i;
	SQuickHullTriangle*	psTriangle;
	SFloat32Vec3*		psPoint;

	psPoint = GetPoint(iPoint);
	for (i = 0; i < paiTriangles->NumElements(); i++)
	{
		psTriangle = mcTriangles.Get((size)paiTriangles->GetValue(i));
		if (psTriangle->sPlane.Distance(psPoint) > mfTolerance)
		{
			psTriangle->aiOutside.Add(iPoint);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::MakeFaces(void)
{
	size				i;
	int					iGroup;
	SQuickHullTriangle*	psTriangle;

	iGroup = 0;
	for (i = 0; i < mcTriangles.NumElements(); i++)
	{
		psTriangle = mcTriangles.Get(i);
		if (!psTriangle->bDeleted && (psTriangle->iGroup == -1))
		{
			if (!MakeFace((int)i, iGroup))
			{
				//The triangles of a face that wouldn't go round in one loop are faces of their own.
				AddTriangleFace((int)i);
			}
			iGroup++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The triangles joined to iSeed whose corners are all in its plane, by the
// tolerance, are one face.  Its edges with other faces, each going the way
// its triangle goes round, make its outline: followed from corner to corner
// they're its corners in order.  Its plane is the outline's (Newell's
// normal, and the corners' average distance along it).
//
//////////////////////////////////////////////////////////////////////////
bool CQuickHull::MakeFace(int iSeed, int iGroup)
{
	CArrayInt			aiMembers;
	CArrayInt			aiFrom;
	CArrayInt			aiTo;
	SQuickHullTriangle*	psSeed;
	SQuickHullTriangle*	psTriangle;
	SQuickHullTriangle*	psNeighbour;
	SQuickHullFace*		psFace;
	size				uiNext;
	size				i;
	size				uiNumEdges;
	int					iEdge;
	int					iNeighbour;
	int					iCorner;
	int					iStart;
	int					iFound;
	SFloat32Vec3		sNormal;
	SFloat32Vec3*		psCorner;
	SFloat32Vec3*		psNextCorner;
	float32				fDistance;
	bool				bInPlane;

	psSeed = mcTriangles.Get((size)iSeed);
	psSeed->iGroup = iGroup;
	aiMembers.Init();
	aiMembers.Add(iSeed);
	for (uiNext = 0; uiNext < aiMembers.NumElements(); uiNext++)
	{
		psTriangle = mcTriangles.Get((size)aiMembers.GetValue(uiNext));
		for (iEdge = 0; iEdge < 3; iEdge++)
		{
			iNeighbour = psTriangle->aiNeighbours[iEdge];
			psNeighbour = mcTriangles.Get((size)iNeighbour);
			psSeed = mcTriangles.Get((size)iSeed);
			if (psNeighbour->iGroup == -1)
			{
				bInPlane = Float32Vec3Dot(&psNeighbour->sPlane.sNormal, &psSeed->sPlane.sNormal) > 0.0f;
				for (iCorner = 0; iCorner < 3; iCorner++)
				{
					bInPlane &= fabsf(psSeed->sPlane.Distance(GetPoint(psNeighbour->aiCorners[iCorner]))) <= mfTolerance;
				}
				if (bInPlane)
				{
					psNeighbour->iGroup = iGroup;
					aiMembers.Add(iNeighbour);
				}
			}
		}
	}

	aiFrom.Init();
	aiTo.Init();
	for (i = 0; i < aiMembers.NumElements(); i++)
	{
		psTriangle = mcTriangles.Get((size)aiMembers.GetValue(i));
		for (iEdge = 0; iEdge < 3; iEdge++)
		{
			if (mcTriangles.Get((size)psTriangle->aiNeighbours[iEdge])->iGroup != iGroup)
			{
				aiFrom.Add(psTriangle->aiCorners[iEdge]);
				aiTo.Add(psTriangle->aiCorners[(iEdge + 1) % 3]);
			}
		}
	}

	//Follow the outline round from its first edge.
	uiNumEdges = aiFrom.NumElements();
	psFace = mcFaces.Add();
	psFace->uiFirstCorner = maiCorners.NumElements();
	psFace->uiNumCorners = 0;
	iStart = aiFrom.GetValue(0);
	iCorner = iStart;
	do
	{
		maiCorners.Add(iCorner);
		psFace->uiNumCorners++;
		iFound = -1;
		for (i = 0; i < uiNumEdges; i++)
		{
			if (aiFrom.GetValue(i) == iCorner)
			{
				iFound = aiTo.GetValue(i);
				break;
			}
		}
		iCorner = iFound;
	}
	while ((iCorner != -1) && (iCorner != iStart) && (psFace->uiNumCorners <= uiNumEdges));

	aiTo.Kill();
	aiFrom.Kill();
	if ((iCorner != iStart) || (psFace->uiNumCorners != uiNumEdges))
	{
		maiCorners.SetUsedElements(psFace->uiFirstCorner);
		mcFaces.RemoveTail();
		for (i = 0; i < aiMembers.NumElements(); i++)
		{
			mcTriangles.Get((size)aiMembers.GetValue(i))->iGroup = -1;
		}
		aiMembers.Kill();
		return false;
	}
	aiMembers.Kill();

	sNormal.Init(0, 0, 0);
	for (i = 0; i < psFace->uiNumCorners; i++)
	{
		psCorner = GetPoint(GetCorner(psFace, i));
		psNextCorner = GetPoint(GetCorner(psFace, (size)((i + 1) % psFace->uiNumCorners)));
		sNormal.x += (psCorner->y - psNextCorner->y) * (psCorner->z + psNextCorner->z);
		sNormal.y += (psCorner->z - psNextCorner->z) * (psCorner->x + psNextCorner->x);
		sNormal.z += (psCorner->x - psNextCorner->x) * (psCorner->y + psNextCorner->y);
	}
	sNormal.Normalize();
	fDistance = 0.0f;
	for (i = 0; i < psFace->uiNumCorners; i++)
	{
		fDistance += Float32Vec3Dot(&sNormal, GetPoint(GetCorner(psFace, i)));
	}
	psFace->sPlane.Init(&sNormal, fDistance / (float32)psFace->uiNumCorners);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CQuickHull::AddTriangleFace(int iTriangle)
{
	SQuickHullTriangle*	psTriangle;
	SQuickHullFace*		psFace;
	int					i;

	psTriangle = mcTriangles.Get((size)iTriangle);
	psFace = mcFaces.Add();
	psFace->sPlane = psTriangle->sPlane;
	psFace->uiFirstCorner = maiCorners.NumElements();
	psFace->uiNumCorners = 3;
	for (i = 0; i < 3; i++)
	{
		maiCorners.Add(psTriangle->aiCorners[i]);
	}
	psTriangle->iGroup = -2;
}
