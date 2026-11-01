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
#ifndef __LINE_OVERLAP_H__
#define __LINE_OVERLAP_H__
#include "PrimitiveTypes.h"


//How one line overlaps another (see CLine::GetOverlap and CLine::Touches).
enum ELineOverlap
{
	LO_Fully,		//The line covers all of the other.
	LO_Start,		//The line covers the other's start but not its end.
	LO_End,			//The line covers the other's end but not its start.
	LO_Center,		//The line covers part of the other's middle, neither its start nor its end.
	LO_Orthogonal,	//The lines cross at right angles where an end of one is on the other.
	LO_None,
};


class CLine;


int32	LineOverlapLength(CLine* pcLine1, CLine* pcLine2);
int32	LineOverlapLength(int32 iMinimum1, int32 iMaximum1, int32 iMinimum2, int32 iMaximum2);


#endif // __LINE_OVERLAP_H__

