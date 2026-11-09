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
#include "BaseLib/ArrayInt.h"
#include "BaseLib/GeometricTypes.h"
#include "MeshDefines.h"
#include "MeshEditor.h"
#include "MeshPolygonShape.h"


//////////////////////////////////////////////////////////////////////////
//
// Whether two of a polygon's corners are next to each other around it, so
// that the edge between them is on its outside.
//
//////////////////////////////////////////////////////////////////////////
bool IsMeshPolygonEdge(int iCorner1, int iCorner2, int iNumCorners)
{
	return (((iCorner1 + 1) % iNumCorners) == iCorner2) || (((iCorner2 + 1) % iNumCorners) == iCorner1);
}


//////////////////////////////////////////////////////////////////////////
//
// Flattens a polygon by dropping the axis its normal is most along, so that
// its corners go anticlockwise.  A polygon need not be exactly flat.
//
//////////////////////////////////////////////////////////////////////////
static void ProjectMeshPolygon(CMesh* pcMesh, const int32* paiPositions, int32 iNumCorners, CArrayFloat64Vec2* pasPoints)
{
	SFloat32Vec3*	psPosition;
	SFloat32Vec3*	psNext;
	SFloat64Vec2*	psPoint;
	float64			fNormalX;
	float64			fNormalY;
	float64			fNormalZ;
	float64			fSizeX;
	float64			fSizeY;
	float64			fSizeZ;
	float64			fU;
	float64			fV;
	bool			bSwap;
	int32			i;

	fNormalX = 0.0;
	fNormalY = 0.0;
	fNormalZ = 0.0;
	for (i = 0; i < iNumCorners; i++)
	{
		psPosition = pcMesh->GetPosition(paiPositions[i]);
		psNext = pcMesh->GetPosition(paiPositions[(i + 1) % iNumCorners]);
		fNormalX += ((float64)psPosition->y - (float64)psNext->y) * ((float64)psPosition->z + (float64)psNext->z);
		fNormalY += ((float64)psPosition->z - (float64)psNext->z) * ((float64)psPosition->x + (float64)psNext->x);
		fNormalZ += ((float64)psPosition->x - (float64)psNext->x) * ((float64)psPosition->y + (float64)psNext->y);
	}

	fSizeX = fabs(fNormalX);
	fSizeY = fabs(fNormalY);
	fSizeZ = fabs(fNormalZ);
	for (i = 0; i < iNumCorners; i++)
	{
		psPosition = pcMesh->GetPosition(paiPositions[i]);
		if ((fSizeZ >= fSizeX) && (fSizeZ >= fSizeY))
		{
			fU = psPosition->x;
			fV = psPosition->y;
			bSwap = fNormalZ < 0.0;
		}
		else if (fSizeX >= fSizeY)
		{
			fU = psPosition->y;
			fV = psPosition->z;
			bSwap = fNormalX < 0.0;
		}
		else
		{
			fU = psPosition->z;
			fV = psPosition->x;
			bSwap = fNormalY < 0.0;
		}

		psPoint = pasPoints->Add();
		psPoint->x = bSwap ? fV : fU;
		psPoint->y = bSwap ? fU : fV;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Above zero if the three points go anticlockwise.
//
//////////////////////////////////////////////////////////////////////////
static float64 MeshPolygonPointsTurn(SFloat64Vec2* psA, SFloat64Vec2* psB, SFloat64Vec2* psC)
{
	return (psB->x - psA->x) * (psC->y - psA->y) - (psB->y - psA->y) * (psC->x - psA->x);
}


//////////////////////////////////////////////////////////////////////////
//
// A corner can be cut off if it turns the way the polygon does and no other
// corner is in the triangle it would cut off.  A corner in the same place as
// one of the triangle's doesn't count, or a polygon that touches itself
// would have no ears.
//
//////////////////////////////////////////////////////////////////////////
static bool IsMeshPolygonEar(CArrayFloat64Vec2* pasPoints, CArrayInt* paiRemaining, int iNumRemaining, int iCorner)
{
	SFloat64Vec2*	psA;
	SFloat64Vec2*	psB;
	SFloat64Vec2*	psC;
	SFloat64Vec2*	psOther;
	int				iPrevious;
	int				iNext;
	int				i;

	iPrevious = (iCorner + iNumRemaining - 1) % iNumRemaining;
	iNext = (iCorner + 1) % iNumRemaining;
	psA = pasPoints->Get(paiRemaining->GetValue(iPrevious));
	psB = pasPoints->Get(paiRemaining->GetValue(iCorner));
	psC = pasPoints->Get(paiRemaining->GetValue(iNext));

	if (MeshPolygonPointsTurn(psA, psB, psC) <= 0.0)
	{
		return false;
	}

	for (i = 0; i < iNumRemaining; i++)
	{
		if ((i != iPrevious) && (i != iCorner) && (i != iNext))
		{
			psOther = pasPoints->Get(paiRemaining->GetValue(i));
			if (((psOther->x != psA->x) || (psOther->y != psA->y)) &&
				((psOther->x != psB->x) || (psOther->y != psB->y)) &&
				((psOther->x != psC->x) || (psOther->y != psC->y)) &&
				(MeshPolygonPointsTurn(psA, psB, psOther) >= 0.0) &&
				(MeshPolygonPointsTurn(psB, psC, psOther) >= 0.0) &&
				(MeshPolygonPointsTurn(psC, psA, psOther) >= 0.0))
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A polygon with no ears, as it has no area or crosses itself, is still cut
// up, from its first corner.
//
//////////////////////////////////////////////////////////////////////////
static int FindMeshPolygonEar(CArrayFloat64Vec2* pasPoints, CArrayInt* paiRemaining, int iNumRemaining)
{
	int		i;

	for (i = 0; i < iNumRemaining; i++)
	{
		if (IsMeshPolygonEar(pasPoints, paiRemaining, iNumRemaining, i))
		{
			return i;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Cuts a polygon into faces by taking the first ear each time.  A convex quad
// comes out as CMesh::AddQuad cuts it: corners 0, 1, 3 and then 1, 2, 3.
//
//////////////////////////////////////////////////////////////////////////
void CutMeshPolygon(CMesh* pcMesh, const int32* paiPositions, int32 iNumCorners, CArrayMeshPolygonCorners* paiFaceCorners)
{
	CArrayFloat64Vec2	asPoints;
	CArrayInt			aiRemaining;
	int					iNumRemaining;
	int					iEar;
	int					i;

	asPoints.Init();
	ProjectMeshPolygon(pcMesh, paiPositions, iNumCorners, &asPoints);

	iNumRemaining = iNumCorners;
	aiRemaining.Init();
	aiRemaining.Resize(iNumRemaining);
	for (i = 0; i < iNumRemaining; i++)
	{
		aiRemaining.SetValue(i, i);
	}

	while (iNumRemaining >= NUM_FACE_CORNERS)
	{
		iEar = FindMeshPolygonEar(&asPoints, &aiRemaining, iNumRemaining);
		paiFaceCorners->Add(aiRemaining.GetValue(iEar));
		paiFaceCorners->Add(aiRemaining.GetValue((iEar + 1) % iNumRemaining));
		paiFaceCorners->Add(aiRemaining.GetValue((iEar + iNumRemaining - 1) % iNumRemaining));

		iNumRemaining--;
		for (i = iEar; i < iNumRemaining; i++)
		{
			aiRemaining.SetValue(i, aiRemaining.GetValue(i + 1));
		}
	}

	aiRemaining.Kill();
	asPoints.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Gets the edges of a polygon's faces that are on its outside, those with no
// face going back along them.  False if an edge is used twice the same way.
//
//////////////////////////////////////////////////////////////////////////
static bool GetMeshPolygonOutsideEdges(CMesh* pcMesh, CMeshPolygon* pcPolygon, CArrayMeshPolygonEdge* pasEdges, CArrayMeshPolygonEdge* pasOutside)
{
	SMeshPolygonEdge*	psEdge;
	SMeshPolygonEdge*	psOther;
	int*		piCorners;
	int			iFace;
	bool		bReversed;
	size		i;
	size		j;
	size		uiNumEdges;
	int			k;

	for (i = 0; i < pcPolygon->maiFaces.NumElements(); i++)
	{
		iFace = pcPolygon->maiFaces.GetValue(i);
		piCorners = pcMesh->GetFaceCorners(iFace);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			psEdge = pasEdges->Add();
			psEdge->iFrom = piCorners[k];
			psEdge->iTo = piCorners[(k + 1) % NUM_FACE_CORNERS];
			psEdge->iFace = iFace;
			psEdge->iFaceCorner = k;
		}
	}

	uiNumEdges = pasEdges->NumElements();
	for (i = 0; i < uiNumEdges; i++)
	{
		psEdge = pasEdges->Get(i);
		bReversed = false;
		for (j = 0; j < uiNumEdges; j++)
		{
			psOther = pasEdges->Get(j);
			if ((j != i) && (psOther->iFrom == psEdge->iFrom) && (psOther->iTo == psEdge->iTo))
			{
				return false;
			}
			bReversed |= (psOther->iFrom == psEdge->iTo) && (psOther->iTo == psEdge->iFrom);
		}
		if (!bReversed)
		{
			*pasOutside->Add() = *psEdge;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts a polygon's outside edges in order around it.  False unless they are
// one loop with every corner of the polygon on it: n faces with no corners
// inside them have n + 2 edges around them.
//
//////////////////////////////////////////////////////////////////////////
static bool OrderMeshPolygonLoop(CMeshPolygon* pcPolygon, CArrayMeshPolygonEdge* pasOutside)
{
	SMeshPolygonEdge*	psEdge;
	SMeshPolygonEdge*	psNext;
	SMeshPolygonEdge	sSwap;
	size		uiNumEdges;
	size		i;
	size		j;
	size		uiFound;

	uiNumEdges = pasOutside->NumElements();
	if (uiNumEdges != (pcPolygon->maiFaces.NumElements() + 2))
	{
		return false;
	}

	for (i = 0; i + 1 < uiNumEdges; i++)
	{
		psEdge = pasOutside->Get(i);
		uiFound = uiNumEdges;
		for (j = 0; j < uiNumEdges; j++)
		{
			psNext = pasOutside->Get(j);
			if (psNext->iFrom == psEdge->iTo)
			{
				if ((j <= i) || (uiFound != uiNumEdges))
				{
					//It came back too soon, or it can go two ways from here.
					return false;
				}
				uiFound = j;
			}
		}
		if (uiFound == uiNumEdges)
		{
			return false;
		}

		sSwap = *pasOutside->Get(i + 1);
		*pasOutside->Get(i + 1) = *pasOutside->Get(uiFound);
		*pasOutside->Get(uiFound) = sSwap;
	}
	return pasOutside->Get(uiNumEdges - 1)->iTo == pasOutside->Get(0)->iFrom;
}


//////////////////////////////////////////////////////////////////////////
//
// The loop starts from the polygon's lowest numbered corner, so that it
// doesn't depend on how the polygon was cut into faces.
//
//////////////////////////////////////////////////////////////////////////
bool GetMeshPolygonLoop(CMesh* pcMesh, CMeshPolygon* pcPolygon, CArrayMeshPolygonEdge* pasEdges, CArrayMeshPolygonEdge* pasLoop)
{
	CArrayMeshPolygonEdge	asOrdered;
	size					uiNumLoop;
	size					uiStart;
	size					i;

	asOrdered.Init();
	if (!GetMeshPolygonOutsideEdges(pcMesh, pcPolygon, pasEdges, &asOrdered) || !OrderMeshPolygonLoop(pcPolygon, &asOrdered))
	{
		asOrdered.Kill();
		return false;
	}

	uiNumLoop = asOrdered.NumElements();
	uiStart = 0;
	for (i = 1; i < uiNumLoop; i++)
	{
		if (asOrdered.Get(i)->iFrom < asOrdered.Get(uiStart)->iFrom)
		{
			uiStart = i;
		}
	}
	for (i = 0; i < uiNumLoop; i++)
	{
		*pasLoop->Add() = *asOrdered.Get((uiStart + i) % uiNumLoop);
	}
	asOrdered.Kill();
	return true;
}

