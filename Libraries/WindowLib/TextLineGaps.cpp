/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "TextLineGaps.h"


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CTextLineGaps::Init(void)
{
	masGaps.Init();
	muiNextId = 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CTextLineGaps::Kill(void)
{
	masGaps.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Leaves iHeight pixels empty below line uiLine; 0 removes its gap.  A
// new gap gets a new id.  False if nothing changed.
//
//////////////////////////////////////////////////////////////////////////
bool CTextLineGaps::Set(size uiLine, int32 iHeight)
{
	size			uiGap;
	STextLayoutGap*	psGap;

	uiGap = Find(uiLine);
	psGap = (uiGap < masGaps.NumElements()) ? masGaps.Get(uiGap) : NULL;
	if (psGap && (psGap->uiLine == uiLine))
	{
		if (psGap->iHeight == iHeight)
		{
			return false;
		}
		if (iHeight <= 0)
		{
			masGaps.RemoveAt(uiGap, true);
		}
		else
		{
			psGap->iHeight = iHeight;
		}
		return true;
	}

	if (iHeight <= 0)
	{
		return false;
	}
	psGap = masGaps.InsertAt(uiGap);
	psGap->uiLine = uiLine;
	psGap->iHeight = iHeight;
	psGap->uiId = muiNextId;
	muiNextId++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The gaps become a copy of pasGaps (sorted by line, at most one a line,
// with their ids), such as another text's, but for those below a line past
// uiNumLines or of no height.  New gaps' ids come after the highest copied.
// False if the lines and heights are those there were.
//
//////////////////////////////////////////////////////////////////////////
bool CTextLineGaps::Copy(CArrayTextLayoutGap* pasGaps, size uiNumLines)
{
	size			i;
	size			uiKept;
	STextLayoutGap*	psGap;
	STextLayoutGap*	psOther;
	bool			bSame;

	bSame = true;
	uiKept = 0;
	for (i = 0; bSame && (i < pasGaps->NumElements()); i++)
	{
		psOther = pasGaps->Get(i);
		if ((psOther->uiLine < uiNumLines) && (psOther->iHeight > 0))
		{
			psGap = (uiKept < masGaps.NumElements()) ? masGaps.Get(uiKept) : NULL;
			bSame = psGap && (psGap->uiLine == psOther->uiLine) && (psGap->iHeight == psOther->iHeight);
			uiKept++;
		}
	}
	if (bSame && (uiKept == masGaps.NumElements()))
	{
		return false;
	}

	masGaps.ReInit();
	for (i = 0; i < pasGaps->NumElements(); i++)
	{
		psOther = pasGaps->Get(i);
		if ((psOther->uiLine < uiNumLines) && (psOther->iHeight > 0))
		{
			*masGaps.Add() = *psOther;
			if (psOther->uiId >= muiNextId)
			{
				muiNextId = psOther->uiId + 1;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every gap goes.  False if there were none.
//
//////////////////////////////////////////////////////////////////////////
bool CTextLineGaps::Clear(void)
{
	if (masGaps.NumElements() == 0)
	{
		return false;
	}
	masGaps.ReInit();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The height of line uiLine's gap, 0 if it has none.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextLineGaps::GetHeight(size uiLine)
{
	STextLayoutGap*	psGap;

	psGap = Get(uiLine);
	return psGap ? psGap->iHeight : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The id of line uiLine's gap, 0 if it has none.
//
//////////////////////////////////////////////////////////////////////////
uint32 CTextLineGaps::GetId(size uiLine)
{
	STextLayoutGap*	psGap;

	psGap = Get(uiLine);
	return psGap ? psGap->uiId : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The line the gap with id uiId is below, MAX_SIZE if it has gone.  It
// looks through every gap; to find many, go through GetGaps once.
//
//////////////////////////////////////////////////////////////////////////
size CTextLineGaps::FindLine(uint32 uiId)
{
	size			i;
	STextLayoutGap*	psGap;

	for (i = 0; i < masGaps.NumElements(); i++)
	{
		psGap = masGaps.Get(i);
		if (psGap->uiId == uiId)
		{
			return psGap->uiLine;
		}
	}
	return MAX_SIZE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextLineGaps::NumGaps(void)
{
	return masGaps.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The gaps, sorted by line.  Don't change them.
//
//////////////////////////////////////////////////////////////////////////
CArrayTextLayoutGap* CTextLineGaps::GetGaps(void)
{
	return &masGaps;
}


//////////////////////////////////////////////////////////////////////////
//
// The gaps at or below line uiFirstLine added to pasDest with their lines
// counted from it, for laying out the text from that line on.
//
//////////////////////////////////////////////////////////////////////////
void CTextLineGaps::GetGapsFrom(size uiFirstLine, CArrayTextLayoutGap* pasDest)
{
	size			i;
	STextLayoutGap*	psGap;

	for (i = Find(uiFirstLine); i < masGaps.NumElements(); i++)
	{
		psGap = pasDest->Add(masGaps.Get(i));
		psGap->uiLine -= uiFirstLine;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiNewLines line feeds went in on line uiLine (at its start if
// bLineStart): the gaps of the lines after it move down, and so does
// uiLine's if the line was pushed down from its start.
//
//////////////////////////////////////////////////////////////////////////
void CTextLineGaps::LinesInserted(size uiLine, bool bLineStart, size uiNewLines)
{
	size			i;
	STextLayoutGap*	psGap;

	for (i = 0; i < masGaps.NumElements(); i++)
	{
		psGap = masGaps.Get(i);
		if ((psGap->uiLine > uiLine) || ((psGap->uiLine == uiLine) && bLineStart))
		{
			psGap->uiLine += uiNewLines;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The text from line uiFirst to line uiLast was removed (bWhole: from the
// start of uiFirst to the start of uiLast).  The lines after move back.  A
// line that goes takes its gap with it, unless it was removed whole: then
// uiLast takes uiFirst's place, and keeps its own gap.
//
//////////////////////////////////////////////////////////////////////////
void CTextLineGaps::LinesRemoved(size uiFirst, size uiLast, bool bWhole)
{
	size			i;
	size			uiRemoved;
	STextLayoutGap*	psGap;

	uiRemoved = uiLast - uiFirst;
	i = masGaps.NumElements();
	while (i > 0)
	{
		i--;
		psGap = masGaps.Get(i);
		if (psGap->uiLine > uiLast)
		{
			psGap->uiLine -= uiRemoved;
		}
		else if (bWhole && (psGap->uiLine == uiLast))
		{
			//Going back through the gaps, uiFirst's (removed next) is still to come.
			psGap->uiLine = uiFirst;
		}
		else if (bWhole ? (psGap->uiLine >= uiFirst) : (psGap->uiLine > uiFirst))
		{
			masGaps.RemoveAt(i, true);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first gap at or below line uiLine, or the number of gaps.
//
//////////////////////////////////////////////////////////////////////////
size CTextLineGaps::Find(size uiLine)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = masGaps.NumElements();
	while (uiLow < uiHigh)
	{
		uiMiddle = (uiLow + uiHigh) / 2;
		if (masGaps.Get(uiMiddle)->uiLine < uiLine)
		{
			uiLow = uiMiddle + 1;
		}
		else
		{
			uiHigh = uiMiddle;
		}
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// Line uiLine's gap, NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
STextLayoutGap* CTextLineGaps::Get(size uiLine)
{
	size			uiGap;
	STextLayoutGap*	psGap;

	uiGap = Find(uiLine);
	if (uiGap >= masGaps.NumElements())
	{
		return NULL;
	}
	psGap = masGaps.Get(uiGap);
	return (psGap->uiLine == uiLine) ? psGap : NULL;
}

