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
#include "Line.h"
#include "LineOverlap.h"


//////////////////////////////////////////////////////////////////////////
//
// The length two lines on the same horizontal or vertical share; 0 if they
// aren't on the same one.
//
//////////////////////////////////////////////////////////////////////////
int32 LineOverlapLength(CLine* pcLine1, CLine* pcLine2)
{
	if (pcLine1->IsNorthSouth() && pcLine2->IsNorthSouth())
	{
		if (pcLine1->GetX() == pcLine2->GetX())
		{
			return LineOverlapLength(pcLine1->GetMinimumY(), pcLine1->GetMaximumY(), pcLine2->GetMinimumY(), pcLine2->GetMaximumY());
		}
	}
	else if (pcLine1->IsEastWest() && pcLine2->IsEastWest())
	{
		if (pcLine1->GetY() == pcLine2->GetY())
		{
			return LineOverlapLength(pcLine1->GetMinimumX(), pcLine1->GetMaximumX(), pcLine2->GetMinimumX(), pcLine2->GetMaximumX());
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 LineOverlapLength(int32 iMinimum1, int32 iMaximum1, int32 iMinimum2, int32 iMaximum2)
{
	if ((iMinimum2 <= iMaximum1) && (iMaximum2 >= iMaximum1))
	{
		return iMaximum1 - iMinimum2;
	}
	if ((iMinimum2 <= iMinimum1) && (iMaximum2 >= iMinimum1))
	{
		return iMaximum2 - iMinimum1;
	}
	if ((iMinimum1 <= iMinimum2) && (iMaximum1 >= iMaximum2))
	{
		return iMaximum2 - iMinimum2;
	}
	if ((iMinimum2 <= iMinimum1) && (iMaximum2 >= iMaximum1))
	{
		return iMaximum1 - iMaximum2;
	}
	return 0;
}

