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
#ifndef __LINE_H__
#define __LINE_H__
#include "ArrayTemplate.h"
#include "Int32Vec2.h"
#include "LineDirection.h"
#include "LineOverlap.h"


class CChars;


//A line between two points on an integer grid, ported from Logicim's Line.  Traces are horizontal or vertical lines;
//a line that is neither has direction LD_Cannot and never overlaps or touches another.
class CLine
{
protected:
	SInt32Vec2		msStart;
	SInt32Vec2		msEnd;
	ELineDirection	meDirection;

public:
	void			Init(SInt32Vec2* psStart, SInt32Vec2* psEnd);
	void			Init(int32 iStartX, int32 iStartY, int32 iEndX, int32 iEndY);
	void			Init(CLine* pcLine);
	bool			InitOrthogonal(SInt32Vec2* psStart, SInt32Vec2* psEnd);

	SInt32Vec2*		GetStart(void);
	SInt32Vec2*		GetEnd(void);
	ELineDirection	GetDirection(void);

	int32			GetMinimumX(void);
	int32			GetMinimumY(void);
	int32			GetMaximumX(void);
	int32			GetMaximumY(void);
	void			GetCenter(SInt32Vec2* psCenter);
	void			GetBoundingBox(SInt32Vec2* psPosition, SInt32Vec2* psDimension);
	int32			GetLength(void);

	bool			IsPositionOn(SInt32Vec2* ps);
	bool			IsPositionOn(int32 x, int32 y);
	bool			IsEastWest(void);
	bool			IsNorthSouth(void);
	int32			GetX(void);
	int32			GetY(void);

	ELineOverlap	GetOverlap(CLine* pcOther, bool bEndInclusive);
	ELineOverlap	Touches(CLine* pcOther);
	void			Extend(CLine* pcOther);

	bool			Equals(CLine* pcOther);
	void			Print(CChars* psz);
	void			Dump(void);

protected:
	ELineOverlap	CalculateOverlap(int32 iLineMinimum, int32 iLineMaximum, int32 iMinimum, int32 iMaximum, int32 iStart, int32 iEnd, bool bEndInclusive);
	ELineOverlap	TouchesOrthogonal(CLine* pcOther);
};


typedef CArrayTemplate<CLine>	CArrayLine;


#endif // __LINE_H__

