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
#ifndef __TRACE_LINES_H__
#define __TRACE_LINES_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Int32Vec2.h"


//A horizontal or vertical line between two grid points, as Logicim's Line: where a wire is or will be.
struct STraceLine
{
	SInt32Vec2	sStart;
	SInt32Vec2	sEnd;

	void	Init(int32 x0, int32 y0, int32 x1, int32 y1);
	void	Init(SInt32Vec2 sStart, SInt32Vec2 sEnd);
	bool	IsHorizontal(void);
	bool	IsVertical(void);
	bool	IsPoint(void);
	bool	Contains(SInt32Vec2 sPoint);
	bool	IsInside(SInt32Vec2 sPoint);
	bool	Touches(STraceLine* psOther);
	bool	Overlaps(STraceLine* psOther, bool bEndsCount);
	void	Extend(STraceLine* psOther);
	int32	GetMinimumX(void);
	int32	GetMaximumX(void);
	int32	GetMinimumY(void);
	int32	GetMaximumY(void);
};


typedef CArrayTemplate<STraceLine>	CArrayTraceLine;
typedef CArrayTemplate<SInt32Vec2>	CArrayInt32Vec2;


//Merging and splitting the lines of a net's wires so that they are as Logicim makes them (its LineMinimiser and
//LineSplitter): lines along each other that overlap or meet end to end are one line, and lines are split where a line
//ends on another (a T, so each arm is a wire of its own) and where a port is on them.  Lines that cross without
//either ending there aren't split; they aren't joined.
void	MergeTraceLines(CArrayTraceLine* pasLines);
void	SplitTraceLines(CArrayTraceLine* pasLines, CArrayInt32Vec2* pasEnds, CArrayInt32Vec2* pasPorts);
void	SplitTraceLinesAt(CArrayTraceLine* pasLines, SInt32Vec2 sPoint);


#endif // __TRACE_LINES_H__

