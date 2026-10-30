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
#include "LineBucket.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineBucket::Init(CLine* pcLine)
{
	mcLine.Init(pcLine);
	macLines.Init();
	macLines.Add(pcLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineBucket::Kill(void)
{
	macLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes pcOther's lines and grows this bucket's line over its line.
// pcOther is killed.
//
//////////////////////////////////////////////////////////////////////////
void CLineBucket::Merge(CLineBucket* pcOther)
{
	size	i;
	size	uiNumLines;

	uiNumLines = pcOther->macLines.NumElements();
	for (i = 0; i < uiNumLines; i++)
	{
		macLines.Add(pcOther->macLines.Get(i));
	}
	mcLine.Extend(&pcOther->mcLine);
	pcOther->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The buckets' lines are on the same horizontal or vertical and overlap or
// meet end to end.
//
//////////////////////////////////////////////////////////////////////////
bool CLineBucket::Overlaps(CLineBucket* pcOther)
{
	return mcLine.GetOverlap(&pcOther->mcLine, true) != LO_None;
}


CLine* CLineBucket::GetLine(void) { return &mcLine; }
CArrayLine* CLineBucket::GetLines(void) { return &macLines; }

