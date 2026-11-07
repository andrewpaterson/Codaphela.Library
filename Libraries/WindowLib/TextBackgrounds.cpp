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
#include "TextBackgrounds.h"


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CTextBackgrounds::Init(void)
{
	masBackgrounds.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CTextBackgrounds::Kill(void)
{
	masBackgrounds.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A background added over those before it: for a range uiStart to uiEnd
// are bytes, for lines they are lines, for a boundary both are its line.
//
//////////////////////////////////////////////////////////////////////////
void CTextBackgrounds::Add(ETextAreaBackground eType, size uiStart, size uiEnd, int32 iThickness, ARGB32 sColour)
{
	STextAreaBackground*	psBackground;

	psBackground = masBackgrounds.Add();
	psBackground->eType = eType;
	psBackground->uiStart = uiStart;
	psBackground->uiEnd = uiEnd;
	psBackground->iThickness = iThickness;
	psBackground->sColour = sColour;
}


//////////////////////////////////////////////////////////////////////////
//
// Every background goes.  False if there were none.
//
//////////////////////////////////////////////////////////////////////////
bool CTextBackgrounds::Clear(void)
{
	if (masBackgrounds.NumElements() == 0)
	{
		return false;
	}
	masBackgrounds.ReInit();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
size CTextBackgrounds::NumBackgrounds(void)
{
	return masBackgrounds.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// In the order they were added, as they are now the text has been edited;
// NULL past the last.
//
//////////////////////////////////////////////////////////////////////////
STextAreaBackground* CTextBackgrounds::Get(size uiIndex)
{
	return masBackgrounds.SafeGet(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The backgrounds follow the text through an edit of it.  Whether any
// moved isn't needed: the text is laid out again after an edit, and the
// backgrounds drawn again with it.
//
//////////////////////////////////////////////////////////////////////////
void CTextBackgrounds::DocumentChanged(STextDocumentChange* psChange)
{
	if (psChange->eChange == TDC_Inserted)
	{
		if (psChange->uiLines > 0)
		{
			LinesInserted(psChange->uiLine, psChange->bLineStart, psChange->uiLines);
		}
		TextInserted(psChange->uiStart, psChange->Length());
	}
	else
	{
		if (psChange->uiLines > 0)
		{
			LinesRemoved(psChange->uiLine, psChange->LastLine(), psChange->bLineStart);
		}
		TextRemoved(psChange->uiStart, psChange->uiEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// uiLength bytes went in at uiPosition.  Ranges move with the text after
// it and grow with text put strictly inside them.  False if none moved.
//
//////////////////////////////////////////////////////////////////////////
bool CTextBackgrounds::TextInserted(size uiPosition, size uiLength)
{
	size					i;
	STextAreaBackground*	psBackground;
	bool					bChanged;

	bChanged = false;
	for (i = 0; i < masBackgrounds.NumElements(); i++)
	{
		psBackground = masBackgrounds.Get(i);
		if (psBackground->eType == TAB_Range)
		{
			if (uiPosition <= psBackground->uiStart)
			{
				psBackground->uiStart += uiLength;
				psBackground->uiEnd += uiLength;
				bChanged = true;
			}
			else if (uiPosition < psBackground->uiEnd)
			{
				psBackground->uiEnd += uiLength;
				bChanged = true;
			}
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Bytes uiStart to uiEnd were removed.  Ranges shrink by what is removed
// from them and go if nothing is left.  False if there are no ranges.
//
//////////////////////////////////////////////////////////////////////////
bool CTextBackgrounds::TextRemoved(size uiStart, size uiEnd)
{
	size					i;
	size					uiLength;
	STextAreaBackground*	psBackground;
	bool					bChanged;

	bChanged = false;
	uiLength = uiEnd - uiStart;
	i = masBackgrounds.NumElements();
	while (i > 0)
	{
		i--;
		psBackground = masBackgrounds.Get(i);
		if (psBackground->eType == TAB_Range)
		{
			psBackground->uiStart = (psBackground->uiStart <= uiStart) ? psBackground->uiStart : ((psBackground->uiStart >= uiEnd) ? psBackground->uiStart - uiLength : uiStart);
			psBackground->uiEnd = (psBackground->uiEnd <= uiStart) ? psBackground->uiEnd : ((psBackground->uiEnd >= uiEnd) ? psBackground->uiEnd - uiLength : uiStart);
			if (psBackground->uiStart == psBackground->uiEnd)
			{
				masBackgrounds.RemoveAt(i, true);
			}
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// uiNewLines lines were put in after line uiLine's start (at it, if
// bLineStart).  Line backgrounds and boundaries move down with their
// lines; a line background grows when lines are put in inside it.  False
// if there are none.
//
//////////////////////////////////////////////////////////////////////////
bool CTextBackgrounds::LinesInserted(size uiLine, bool bLineStart, size uiNewLines)
{
	size					i;
	STextAreaBackground*	psBackground;
	bool					bChanged;

	bChanged = false;
	for (i = 0; i < masBackgrounds.NumElements(); i++)
	{
		psBackground = masBackgrounds.Get(i);
		if (psBackground->eType != TAB_Range)
		{
			if ((psBackground->uiStart > uiLine) || ((psBackground->uiStart == uiLine) && bLineStart))
			{
				psBackground->uiStart += uiNewLines;
			}
			if (psBackground->uiEnd > uiLine)
			{
				psBackground->uiEnd += uiNewLines;
			}
			if (psBackground->eType == TAB_Boundary)
			{
				psBackground->uiEnd = psBackground->uiStart;
			}
			bChanged = true;
		}
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// The lines after uiFirst up to uiLast were joined onto uiFirst, or if
// bWhole lines uiFirst up to before uiLast were removed and uiLast took
// uiFirst's place.  Line backgrounds and boundaries move up with their
// lines; one on lines that are gone is gone with them.  False if there
// are none.
//
//////////////////////////////////////////////////////////////////////////
bool CTextBackgrounds::LinesRemoved(size uiFirst, size uiLast, bool bWhole)
{
	size					i;
	size					uiRemoved;
	size					uiJoined;
	STextAreaBackground*	psBackground;
	bool					bChanged;

	bChanged = false;
	uiRemoved = uiLast - uiFirst;
	uiJoined = bWhole ? uiFirst : uiFirst + 1;
	i = masBackgrounds.NumElements();
	while (i > 0)
	{
		i--;
		psBackground = masBackgrounds.Get(i);
		if (psBackground->eType != TAB_Range)
		{
			psBackground->uiStart = (psBackground->uiStart <= uiFirst) ? psBackground->uiStart : ((psBackground->uiStart > uiLast) ? psBackground->uiStart - uiRemoved : uiJoined);
			psBackground->uiEnd = (psBackground->uiEnd <= uiFirst) ? psBackground->uiEnd : ((psBackground->uiEnd > uiLast) ? psBackground->uiEnd - uiRemoved : uiJoined);
			if (psBackground->eType == TAB_Boundary)
			{
				psBackground->uiEnd = psBackground->uiStart;
			}
			else if (psBackground->uiStart == psBackground->uiEnd)
			{
				masBackgrounds.RemoveAt(i, true);
			}
			bChanged = true;
		}
	}
	return bChanged;
}

