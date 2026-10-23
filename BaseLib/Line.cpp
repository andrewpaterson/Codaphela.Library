/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "Chars.h"
#include "Logger.h"
#include "Line.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLine::Init(SInt32Vec2* psStart, SInt32Vec2* psEnd)
{
	Init(psStart->x, psStart->y, psEnd->x, psEnd->y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLine::Init(int32 iStartX, int32 iStartY, int32 iEndX, int32 iEndY)
{
	msStart.Init(iStartX, iStartY);
	msEnd.Init(iEndX, iEndY);
	meDirection = CalculateLineDirection(iStartX, iStartY, iEndX, iEndY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLine::Init(CLine* pcLine)
{
	Init(&pcLine->msStart, &pcLine->msEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's Line.createLine: only a horizontal or vertical line with a
// length.  Returns false, leaving the line untouched, for any other.
//
//////////////////////////////////////////////////////////////////////////
bool CLine::InitOrthogonal(SInt32Vec2* psStart, SInt32Vec2* psEnd)
{
	if ((psStart == NULL) || (psEnd == NULL))
	{
		return false;
	}
	if (CalculateLineDirection(psStart->x, psStart->y, psEnd->x, psEnd->y) == LD_Cannot)
	{
		return false;
	}
	Init(psStart, psEnd);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CLine::GetMinimumX(void)
{
	return (msStart.x < msEnd.x) ? msStart.x : msEnd.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CLine::GetMinimumY(void)
{
	return (msStart.y < msEnd.y) ? msStart.y : msEnd.y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CLine::GetMaximumX(void)
{
	return (msStart.x > msEnd.x) ? msStart.x : msEnd.x;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CLine::GetMaximumY(void)
{
	return (msStart.y > msEnd.y) ? msStart.y : msEnd.y;
}


//////////////////////////////////////////////////////////////////////////
//
// Halfway between the ends, rounded towards zero.
//
//////////////////////////////////////////////////////////////////////////
void CLine::GetCenter(SInt32Vec2* psCenter)
{
	psCenter->Init((msStart.x + msEnd.x) / 2, (msStart.y + msEnd.y) / 2);
}


//////////////////////////////////////////////////////////////////////////
//
// The top left and the size, in grid units.
//
//////////////////////////////////////////////////////////////////////////
void CLine::GetBoundingBox(SInt32Vec2* psPosition, SInt32Vec2* psDimension)
{
	psPosition->Init(GetMinimumX(), GetMinimumY());
	psDimension->Init(GetMaximumX() - GetMinimumX(), GetMaximumY() - GetMinimumY());
}


//////////////////////////////////////////////////////////////////////////
//
// The length of a line that is neither horizontal nor vertical is rounded
// to the nearest whole number (Logicim used the x length for both axes).
//
//////////////////////////////////////////////////////////////////////////
int32 CLine::GetLength(void)
{
	float64	x;
	float64	y;

	if (IsEastWest())
	{
		return GetMaximumX() - GetMinimumX();
	}
	else if (IsNorthSouth())
	{
		return GetMaximumY() - GetMinimumY();
	}
	else
	{
		x = (float64)GetMaximumX() - GetMinimumX();
		y = (float64)GetMaximumY() - GetMinimumY();
		return (int32)floor(sqrt(x * x + y * y) + 0.5);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLine::IsPositionOn(SInt32Vec2* ps)
{
	return IsPositionOn(ps->x, ps->y);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the point is on the line, ends included.  Always false for a
// line that is neither horizontal nor vertical.
//
//////////////////////////////////////////////////////////////////////////
bool CLine::IsPositionOn(int32 x, int32 y)
{
	switch (meDirection)
	{
	case LD_North:
		return (x == msStart.x) && (y >= msStart.y) && (y <= msEnd.y);
	case LD_South:
		return (x == msStart.x) && (y >= msEnd.y) && (y <= msStart.y);
	case LD_West:
		return (y == msStart.y) && (x >= msStart.x) && (x <= msEnd.x);
	case LD_East:
		return (y == msStart.y) && (x >= msEnd.x) && (x <= msStart.x);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLine::IsEastWest(void)
{
	return (meDirection == LD_West) || (meDirection == LD_East);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLine::IsNorthSouth(void)
{
	return (meDirection == LD_North) || (meDirection == LD_South);
}


//////////////////////////////////////////////////////////////////////////
//
// The x of a vertical line.
//
//////////////////////////////////////////////////////////////////////////
int32 CLine::GetX(void)
{
	if (msStart.x != msEnd.x)
	{
		gcLogger.Error2(__METHOD__, " Cannot get a single X for an East / West line.", NULL);
	}
	return msStart.x;
}


//////////////////////////////////////////////////////////////////////////
//
// The y of a horizontal line.
//
//////////////////////////////////////////////////////////////////////////
int32 CLine::GetY(void)
{
	if (msStart.y != msEnd.y)
	{
		gcLogger.Error2(__METHOD__, " Cannot get a single Y for a North / South line.", NULL);
	}
	return msStart.y;
}


//////////////////////////////////////////////////////////////////////////
//
// How pcOther overlaps this line when both are on the same horizontal or
// the same vertical.  Lines that only meet at an end overlap if
// bEndInclusive.
//
//////////////////////////////////////////////////////////////////////////
ELineOverlap CLine::GetOverlap(CLine* pcOther, bool bEndInclusive)
{
	if (IsNorthSouth() && pcOther->IsNorthSouth())
	{
		if (msStart.x == pcOther->msStart.x)
		{
			return CalculateOverlap(pcOther->GetMinimumY(), pcOther->GetMaximumY(), GetMinimumY(), GetMaximumY(), msStart.y, msEnd.y, bEndInclusive);
		}
	}
	else if (IsEastWest() && pcOther->IsEastWest())
	{
		if (msStart.y == pcOther->msStart.y)
		{
			return CalculateOverlap(pcOther->GetMinimumX(), pcOther->GetMaximumX(), GetMinimumX(), GetMaximumX(), msStart.x, msEnd.x, bEndInclusive);
		}
	}
	return LO_None;
}


//////////////////////////////////////////////////////////////////////////
//
// How the other line, iLineMinimum to iLineMaximum, overlaps this one
// along their shared axis.
//
//////////////////////////////////////////////////////////////////////////
ELineOverlap CLine::CalculateOverlap(int32 iLineMinimum, int32 iLineMaximum, int32 iMinimum, int32 iMaximum, int32 iStart, int32 iEnd, bool bEndInclusive)
{
	if (bEndInclusive)
	{
		if ((iLineMaximum < iMinimum) || (iLineMinimum > iMaximum))
		{
			return LO_None;
		}
	}
	else
	{
		if ((iLineMaximum <= iMinimum) || (iLineMinimum >= iMaximum))
		{
			return LO_None;
		}
	}

	if ((iLineMinimum <= iMinimum) && (iLineMaximum >= iMaximum))
	{
		return LO_Fully;
	}
	if ((iLineMinimum > iMinimum) && (iLineMaximum < iMaximum))
	{
		return LO_Center;
	}
	if ((iStart >= iLineMinimum) && (iStart <= iLineMaximum))
	{
		return LO_Start;
	}
	if ((iEnd >= iLineMinimum) && (iEnd <= iLineMaximum))
	{
		return LO_End;
	}

	gcLogger.Error2(__METHOD__, " Could not determine line over trace overlap.", NULL);
	return LO_None;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the lines meet: overlapping along the same axis (ends included)
// or crossing at right angles where an end of one is on the other.
//
//////////////////////////////////////////////////////////////////////////
ELineOverlap CLine::Touches(CLine* pcOther)
{
	ELineOverlap	eOverlap;

	eOverlap = GetOverlap(pcOther, true);
	if (eOverlap != LO_None)
	{
		return eOverlap;
	}

	eOverlap = TouchesOrthogonal(pcOther);
	if (eOverlap != LO_None)
	{
		return eOverlap;
	}

	return pcOther->TouchesOrthogonal(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ELineOverlap CLine::TouchesOrthogonal(CLine* pcOther)
{
	if (IsPositionOn(&pcOther->msStart) || IsPositionOn(&pcOther->msEnd))
	{
		return LO_Orthogonal;
	}
	return LO_None;
}


//////////////////////////////////////////////////////////////////////////
//
// Grows this line to cover pcOther if they are on the same horizontal or
// vertical.  The extended line runs from its minimum to its maximum.
//
//////////////////////////////////////////////////////////////////////////
void CLine::Extend(CLine* pcOther)
{
	int32	iMinimum;
	int32	iMaximum;

	if (IsNorthSouth() && pcOther->IsNorthSouth())
	{
		if (msStart.x == pcOther->msStart.x)
		{
			iMinimum = (GetMinimumY() < pcOther->GetMinimumY()) ? GetMinimumY() : pcOther->GetMinimumY();
			iMaximum = (GetMaximumY() > pcOther->GetMaximumY()) ? GetMaximumY() : pcOther->GetMaximumY();
			Init(msStart.x, iMinimum, msEnd.x, iMaximum);
		}
	}
	else if (IsEastWest() && pcOther->IsEastWest())
	{
		if (msStart.y == pcOther->msStart.y)
		{
			iMinimum = (GetMinimumX() < pcOther->GetMinimumX()) ? GetMinimumX() : pcOther->GetMinimumX();
			iMaximum = (GetMaximumX() > pcOther->GetMaximumX()) ? GetMaximumX() : pcOther->GetMaximumX();
			Init(iMinimum, msStart.y, iMaximum, msEnd.y);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The same start and end.
//
//////////////////////////////////////////////////////////////////////////
bool CLine::Equals(CLine* pcOther)
{
	return (msStart == pcOther->msStart) && (msEnd == pcOther->msEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// (x, y) -> (x, y)
//
//////////////////////////////////////////////////////////////////////////
void CLine::Print(CChars* psz)
{
	psz->Append("(");
	psz->Append(msStart.x);
	psz->Append(", ");
	psz->Append(msStart.y);
	psz->Append(") -> (");
	psz->Append(msEnd.x);
	psz->Append(", ");
	psz->Append(msEnd.y);
	psz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLine::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


SInt32Vec2* CLine::GetStart(void) { return &msStart; }
SInt32Vec2* CLine::GetEnd(void) { return &msEnd; }
ELineDirection CLine::GetDirection(void) { return meDirection; }

