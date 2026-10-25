/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "KicadSchemSubnetGraph.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnetGraph::Init(CKicadSchemSubnet* pcSubnet)
{
	mpcSubnet = pcSubnet;
	masPoints.Init();
	maiEdgeStarts.Init();
	maiEdgeEnds.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnetGraph::Kill(void)
{
	maiEdgeEnds.Kill();
	maiEdgeStarts.Kill();
	masPoints.Kill();
	mpcSubnet = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the point, added if it isn't already.  Add the points to be
// found before Build.
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemSubnetGraph::AddPoint(SKicadSchemPoint* psPoint)
{
	size	uiIndex;

	uiIndex = FindPoint(psPoint);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		masPoints.Add(psPoint);
		uiIndex = masPoints.NumElements() - 1;
	}
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemSubnetGraph::FindPoint(SKicadSchemPoint* psPoint)
{
	size	i;

	for (i = 0; i < masPoints.NumElements(); i++)
	{
		if (masPoints.Get(i)->Equals(psPoint))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnetGraph::Build(void)
{
	size				i;
	CKicadSchemWire*	pcWire;

	for (i = 0; i < mpcSubnet->NumWires(); i++)
	{
		pcWire = mpcSubnet->GetWire(i);
		AddPoint(pcWire->GetStart());
		AddPoint(pcWire->GetEnd());
	}
	for (i = 0; i < mpcSubnet->NumJunctions(); i++)
	{
		AddPoint(mpcSubnet->GetJunction(i)->GetPosition());
	}
	for (i = 0; i < mpcSubnet->NumWires(); i++)
	{
		AddWireEdges(mpcSubnet->GetWire(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The points on the wire in order from its start, each joined to the
// next.
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemSubnetGraph::AddWireEdges(CKicadSchemWire* pcWire)
{
	CArrayInt			aiOn;
	size				i;
	size				j;
	SKicadSchemPoint*	psStart;
	SKicadSchemPoint*	psPoint;
	float64				fDistance;
	float64				fOther;

	psStart = pcWire->GetStart();
	aiOn.Init();
	for (i = 0; i < masPoints.NumElements(); i++)
	{
		psPoint = masPoints.Get(i);
		if (!pcWire->Contains(psPoint))
		{
			continue;
		}
		fDistance = fabs(psPoint->fX - psStart->fX) + fabs(psPoint->fY - psStart->fY);
		for (j = 0; j < aiOn.NumElements(); j++)
		{
			psPoint = masPoints.Get(aiOn.GetValue(j));
			fOther = fabs(psPoint->fX - psStart->fX) + fabs(psPoint->fY - psStart->fY);
			if (fDistance < fOther)
			{
				break;
			}
		}
		aiOn.InsertAt((int)i, j);
	}
	for (i = 1; i < aiOn.NumElements(); i++)
	{
		maiEdgeStarts.Add(aiOn.GetValue(i - 1));
		maiEdgeEnds.Add(aiOn.GetValue(i));
	}
	aiOn.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The fewest wire pieces from the point to any of the targets, as the
// points along the way from uiFrom to the target (both included).  False
// if no target can be reached.
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemSubnetGraph::FindPath(size uiFrom, CArrayInt* paiTargets, CArrayInt* paiPath)
{
	CArrayInt	aiPrevious;
	CArrayInt	aiQueue;
	size		uiHead;
	size		i;
	int			iPoint;
	int			iNext;
	int			iFound;

	aiPrevious.Init();
	for (i = 0; i < masPoints.NumElements(); i++)
	{
		aiPrevious.Add(-2);
	}
	aiQueue.Init();
	aiQueue.Add((int)uiFrom);
	aiPrevious.SetValue(uiFrom, -1);

	iFound = -1;
	uiHead = 0;
	while (uiHead < aiQueue.NumElements())
	{
		iPoint = aiQueue.GetValue(uiHead);
		uiHead++;
		if (paiTargets->Find(iPoint) != ARRAY_ELEMENT_NOT_FOUND)
		{
			iFound = iPoint;
			break;
		}
		for (i = 0; i < maiEdgeStarts.NumElements(); i++)
		{
			if (maiEdgeStarts.GetValue(i) == iPoint)
			{
				iNext = maiEdgeEnds.GetValue(i);
			}
			else if (maiEdgeEnds.GetValue(i) == iPoint)
			{
				iNext = maiEdgeStarts.GetValue(i);
			}
			else
			{
				continue;
			}
			if (aiPrevious.GetValue(iNext) == -2)
			{
				aiPrevious.SetValue(iNext, iPoint);
				aiQueue.Add(iNext);
			}
		}
	}

	if (iFound != -1)
	{
		iPoint = iFound;
		while (iPoint != -1)
		{
			paiPath->InsertAt(iPoint, 0);
			iPoint = aiPrevious.GetValue(iPoint);
		}
	}
	aiQueue.Kill();
	aiPrevious.Kill();
	return iFound != -1;
}


//////////////////////////////////////////////////////////////////////////
//
// How far across the path's span its first turn is: the fraction of the
// way from its start to its end, along the way it starts, where it stops
// going that way.  A path that doesn't turn (or goes nowhere along the way
// it starts) gives a half.  pbHorizontal is whether it starts across.
//
//////////////////////////////////////////////////////////////////////////
float64 CKicadSchemSubnetGraph::GetBendFraction(CArrayKicadSchemPoint* pasPath, bool* pbHorizontal)
{
	size				i;
	SKicadSchemPoint*	psStart;
	SKicadSchemPoint*	psEnd;
	SKicadSchemPoint*	psFrom;
	SKicadSchemPoint*	psTo;
	SKicadSchemPoint*	psBend;
	float64				fDX;
	float64				fDY;
	bool				bStarted;
	bool				bHorizontal;
	bool				bSegmentHorizontal;
	float64				fSpan;
	float64				fFraction;

	*pbHorizontal = true;
	if (pasPath->NumElements() < 2)
	{
		return 0.5;
	}

	psStart = pasPath->Get(0);
	psEnd = pasPath->Tail();
	bStarted = false;
	bHorizontal = true;
	psBend = NULL;
	for (i = 1; i < pasPath->NumElements(); i++)
	{
		psFrom = pasPath->Get(i - 1);
		psTo = pasPath->Get(i);
		fDX = fabs(psTo->fX - psFrom->fX);
		fDY = fabs(psTo->fY - psFrom->fY);
		if ((fDX < KICAD_SCHEM_POINT_TOLERANCE) && (fDY < KICAD_SCHEM_POINT_TOLERANCE))
		{
			continue;
		}
		bSegmentHorizontal = fDX >= fDY;
		if (!bStarted)
		{
			bHorizontal = bSegmentHorizontal;
			bStarted = true;
		}
		else if (bSegmentHorizontal != bHorizontal)
		{
			psBend = psFrom;
			break;
		}
	}

	*pbHorizontal = bHorizontal;
	if (psBend == NULL)
	{
		return 0.5;
	}
	if (bHorizontal)
	{
		fSpan = psEnd->fX - psStart->fX;
		fFraction = (psBend->fX - psStart->fX);
	}
	else
	{
		fSpan = psEnd->fY - psStart->fY;
		fFraction = (psBend->fY - psStart->fY);
	}
	if (fabs(fSpan) < KICAD_SCHEM_POINT_TOLERANCE)
	{
		return 0.5;
	}
	fFraction /= fSpan;
	if (fFraction < 0)
	{
		return 0;
	}
	if (fFraction > 1)
	{
		return 1;
	}
	return fFraction;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CKicadSchemSubnetGraph::NumPoints(void) { return masPoints.NumElements(); }
SKicadSchemPoint* CKicadSchemSubnetGraph::GetPoint(size uiIndex) { return masPoints.SafeGet(uiIndex); }
size CKicadSchemSubnetGraph::NumEdges(void) { return maiEdgeStarts.NumElements(); }

