/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "TraceLines.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STraceLine::Init(int32 x0, int32 y0, int32 x1, int32 y1)
{
	sStart.Init(x0, y0);
	sEnd.Init(x1, y1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STraceLine::Init(SInt32Vec2 sStart, SInt32Vec2 sEnd)
{
	this->sStart = sStart;
	this->sEnd = sEnd;
}


//////////////////////////////////////////////////////////////////////////
//
// A point is neither horizontal nor vertical.
//
//////////////////////////////////////////////////////////////////////////
bool STraceLine::IsHorizontal(void) { return (sStart.y == sEnd.y) && (sStart.x != sEnd.x); }
bool STraceLine::IsVertical(void) { return (sStart.x == sEnd.x) && (sStart.y != sEnd.y); }
bool STraceLine::IsPoint(void) { return sStart == sEnd; }
int32 STraceLine::GetMinimumX(void) { return (sStart.x < sEnd.x) ? sStart.x : sEnd.x; }
int32 STraceLine::GetMaximumX(void) { return (sStart.x > sEnd.x) ? sStart.x : sEnd.x; }
int32 STraceLine::GetMinimumY(void) { return (sStart.y < sEnd.y) ? sStart.y : sEnd.y; }
int32 STraceLine::GetMaximumY(void) { return (sStart.y > sEnd.y) ? sStart.y : sEnd.y; }


//////////////////////////////////////////////////////////////////////////
//
// Ends included, as Logicim's isPositionOn.
//
//////////////////////////////////////////////////////////////////////////
bool STraceLine::Contains(SInt32Vec2 sPoint)
{
	if (sStart.x == sEnd.x)
	{
		return (sPoint.x == sStart.x) && (sPoint.y >= GetMinimumY()) && (sPoint.y <= GetMaximumY());
	}
	if (sStart.y == sEnd.y)
	{
		return (sPoint.y == sStart.y) && (sPoint.x >= GetMinimumX()) && (sPoint.x <= GetMaximumX());
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// On the line but not one of its ends.
//
//////////////////////////////////////////////////////////////////////////
bool STraceLine::IsInside(SInt32Vec2 sPoint)
{
	return Contains(sPoint) && (sPoint != sStart) && (sPoint != sEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Lines join where an end of one is on the other, as Logicim's touches.
//
//////////////////////////////////////////////////////////////////////////
bool STraceLine::Touches(STraceLine* psOther)
{
	return Contains(psOther->sStart) || Contains(psOther->sEnd) || psOther->Contains(sStart) || psOther->Contains(sEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Both horizontal (or both vertical) on the same row (or column) and
// sharing more than a point, or if bEndsCount, at least a point, as
// Logicim's getOverlap.
//
//////////////////////////////////////////////////////////////////////////
bool STraceLine::Overlaps(STraceLine* psOther, bool bEndsCount)
{
	int32	iMinimum;
	int32	iMaximum;
	int32	iOtherMinimum;
	int32	iOtherMaximum;

	if (IsHorizontal() && psOther->IsHorizontal() && (sStart.y == psOther->sStart.y))
	{
		iMinimum = GetMinimumX();
		iMaximum = GetMaximumX();
		iOtherMinimum = psOther->GetMinimumX();
		iOtherMaximum = psOther->GetMaximumX();
	}
	else if (IsVertical() && psOther->IsVertical() && (sStart.x == psOther->sStart.x))
	{
		iMinimum = GetMinimumY();
		iMaximum = GetMaximumY();
		iOtherMinimum = psOther->GetMinimumY();
		iOtherMaximum = psOther->GetMaximumY();
	}
	else
	{
		return false;
	}

	if (bEndsCount)
	{
		return (iOtherMaximum >= iMinimum) && (iOtherMinimum <= iMaximum);
	}
	return (iOtherMaximum > iMinimum) && (iOtherMinimum < iMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
// Grows the line to cover the other, which runs along it, as Logicim's
// extend.  The line runs from its least to its greatest end after.
//
//////////////////////////////////////////////////////////////////////////
void STraceLine::Extend(STraceLine* psOther)
{
	int32	iMinimum;
	int32	iMaximum;

	if (IsHorizontal())
	{
		iMinimum = (GetMinimumX() < psOther->GetMinimumX()) ? GetMinimumX() : psOther->GetMinimumX();
		iMaximum = (GetMaximumX() > psOther->GetMaximumX()) ? GetMaximumX() : psOther->GetMaximumX();
		sStart.x = iMinimum;
		sEnd.x = iMaximum;
	}
	else if (IsVertical())
	{
		iMinimum = (GetMinimumY() < psOther->GetMinimumY()) ? GetMinimumY() : psOther->GetMinimumY();
		iMaximum = (GetMaximumY() > psOther->GetMaximumY()) ? GetMaximumY() : psOther->GetMaximumY();
		sStart.y = iMinimum;
		sEnd.y = iMaximum;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's LineMinimiser: lines along each other that overlap or meet
// end to end become one.  Points are dropped.
//
//////////////////////////////////////////////////////////////////////////
void MergeTraceLines(CArrayTraceLine* pasLines)
{
	size		i;
	size		j;
	bool		bMerged;
	STraceLine*	psLine;

	for (i = pasLines->NumElements(); i > 0; i--)
	{
		if (pasLines->Get(i - 1)->IsPoint())
		{
			pasLines->RemoveAt(i - 1, true);
		}
	}

	do
	{
		bMerged = false;
		for (i = 0; (i < pasLines->NumElements()) && !bMerged; i++)
		{
			psLine = pasLines->Get(i);
			for (j = i + 1; j < pasLines->NumElements(); j++)
			{
				if (psLine->Overlaps(pasLines->Get(j), true))
				{
					psLine->Extend(pasLines->Get(j));
					pasLines->RemoveAt(j, true);
					bMerged = true;
					break;
				}
			}
		}
	}
	while (bMerged);
}


//////////////////////////////////////////////////////////////////////////
//
// Splits each line the point is inside of into two at the point.
//
//////////////////////////////////////////////////////////////////////////
void SplitTraceLinesAt(CArrayTraceLine* pasLines, SInt32Vec2 sPoint)
{
	size		i;
	size		uiNumLines;
	STraceLine	sLine;
	STraceLine*	psSecond;

	uiNumLines = pasLines->NumElements();
	for (i = 0; i < uiNumLines; i++)
	{
		sLine = *pasLines->Get(i);
		if (sLine.IsInside(sPoint))
		{
			pasLines->Get(i)->sEnd = sPoint;
			psSecond = pasLines->Add();
			psSecond->Init(sPoint, sLine.sEnd);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's LineSplitter: at each of the ends (of the lines before they
// were merged) where a horizontal and a vertical line meet and it's inside
// one of them, and at each port that is on a line, the lines through it
// are split there.
//
//////////////////////////////////////////////////////////////////////////
void SplitTraceLines(CArrayTraceLine* pasLines, CArrayInt32Vec2* pasEnds, CArrayInt32Vec2* pasPorts)
{
	size			i;
	size			j;
	SInt32Vec2		sPoint;
	STraceLine*		psHorizontal;
	STraceLine*		psVertical;
	STraceLine*		psLine;
	CArrayInt32Vec2	asJunctions;

	asJunctions.Init();
	for (i = 0; i < pasEnds->NumElements(); i++)
	{
		sPoint = *pasEnds->Get(i);
		psHorizontal = NULL;
		psVertical = NULL;
		for (j = 0; j < pasLines->NumElements(); j++)
		{
			psLine = pasLines->Get(j);
			if (psLine->Contains(sPoint))
			{
				if (psLine->IsHorizontal())
				{
					psHorizontal = psLine;
				}
				else if (psLine->IsVertical())
				{
					psVertical = psLine;
				}
			}
		}
		if ((psHorizontal != NULL) && (psVertical != NULL) && (psHorizontal->IsInside(sPoint) || psVertical->IsInside(sPoint)))
		{
			asJunctions.Add(&sPoint);
		}
	}

	for (i = 0; i < asJunctions.NumElements(); i++)
	{
		SplitTraceLinesAt(pasLines, *asJunctions.Get(i));
	}
	for (i = 0; i < pasPorts->NumElements(); i++)
	{
		SplitTraceLinesAt(pasLines, *pasPorts->Get(i));
	}
	asJunctions.Kill();
}

