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
#include "TextAreaShapes.h"
#include "TextAreaOccurrences.h"


//////////////////////////////////////////////////////////////////////////
//
// The occurrences of pcDocument's text: it tells them when it is edited.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaOccurrences::Init(CTextDocument* pcDocument)
{
	mpcDocument = pcDocument;
	mbHighlighting = false;
	masFound.Init();
	msFoundFor.uiStart = 0;
	msFoundFor.uiEnd = 0;
	mbStale = false;
	masRanges.Init();
	mpcDocument->AddListener(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaOccurrences::Kill(void)
{
	mpcDocument->RemoveListener(this);
	masRanges.Kill();
	masFound.Kill();
	mpcDocument = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the selected text's other occurrences are found.  Changing it
// makes the ones found stale: off, they aren't found again until it is
// turned on.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaOccurrences::SetHighlighting(bool bHighlighting)
{
	mbHighlighting = bHighlighting;
	mbStale = true;
}


bool CTextAreaOccurrences::IsHighlighting(void) { return mbHighlighting; }


//////////////////////////////////////////////////////////////////////////
//
// The text changed: the occurrences found are of the text before, and so
// are the ranges given, which are dropped.  An edit of the document says
// so itself; text set or appended isn't an edit, so its text area does.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaOccurrences::TextSet(void)
{
	mbStale = true;
	masRanges.ReInit();
}


void CTextAreaOccurrences::DocumentChanged(STextDocumentChange* psChange) { TextSet(); }


//////////////////////////////////////////////////////////////////////////
//
// The occurrences found are of the text as it is and of the selection
// uiStart to uiEnd (equal with nothing selected): they can be drawn.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaOccurrences::IsFoundFor(size uiStart, size uiEnd)
{
	return !mbStale && (uiStart == msFoundFor.uiStart) && (uiEnd == msFoundFor.uiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// The selection is uiStart to uiEnd now.  True if its occurrences must be
// found (Find, at the next tick, once however many keys or moves come
// before it): highlighting, the selection or the text changed since they
// were.  With nothing selected and none found there is nothing to find.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaOccurrences::Request(size uiStart, size uiEnd)
{
	if (!mbHighlighting || IsFoundFor(uiStart, uiEnd))
	{
		return false;
	}
	if ((uiStart == uiEnd) && (masFound.NumElements() == 0))
	{
		msFoundFor.uiStart = uiStart;
		msFoundFor.uiEnd = uiEnd;
		mbStale = false;
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the other occurrences of the selection uiStart to uiEnd.  True if
// what is drawn changed: there were some or there are some.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaOccurrences::Find(size uiStart, size uiEnd)
{
	bool	bHadFound;

	bHadFound = masFound.NumElements() > 0;
	masFound.ReInit();
	msFoundFor.uiStart = uiStart;
	msFoundFor.uiEnd = uiEnd;
	mbStale = false;
	if (mbHighlighting && mpcDocument->GetNavigation()->IsOccurrenceText(uiStart, uiEnd))
	{
		mpcDocument->GetNavigation()->FindOccurrences(uiStart, uiEnd, TEXT_AREA_MAX_OCCURRENCES, &masFound);
	}
	return bHadFound || (masFound.NumElements() > 0);
}


size CTextAreaOccurrences::NumFound(void) { return masFound.NumElements(); }
STextRange* CTextAreaOccurrences::GetFound(size uiIndex) { return masFound.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// The occurrences found, as rectangles on their rows in sColour.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaOccurrences::AddFound(CTextAreaShapes* pcShapes, Ptr<CGeometryMap> pMap, ARGB32 sColour)
{
	STextRange*	psRange;
	size		i;

	for (i = 0; i < masFound.NumElements(); i++)
	{
		psRange = masFound.Get(i);
		pcShapes->AddRange(pMap, psRange->uiStart, psRange->uiEnd, sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ranges of the text to draw as occurrences.  They replace the ones given
// before, and go when the text changes.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaOccurrences::SetRanges(STextOccurrenceRange* asRanges, size uiNumRanges)
{
	size	i;

	masRanges.ReInit();
	for (i = 0; i < uiNumRanges; i++)
	{
		*masRanges.Add() = asRanges[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False if there were none.
//
//////////////////////////////////////////////////////////////////////////
bool CTextAreaOccurrences::ClearRanges(void)
{
	if (masRanges.NumElements() == 0)
	{
		return false;
	}
	masRanges.ReInit();
	return true;
}


size CTextAreaOccurrences::NumRanges(void) { return masRanges.NumElements(); }
STextOccurrenceRange* CTextAreaOccurrences::GetRange(size uiIndex) { return masRanges.Get(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
// The ranges given, as rectangles on their rows: sColour, or sWriteColour
// for a range that is written (bWrite).
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaOccurrences::AddRanges(CTextAreaShapes* pcShapes, Ptr<CGeometryMap> pMap, ARGB32 sColour, ARGB32 sWriteColour)
{
	STextOccurrenceRange*	psRange;
	size					i;

	for (i = 0; i < masRanges.NumElements(); i++)
	{
		psRange = masRanges.Get(i);
		pcShapes->AddRange(pMap, psRange->uiStart, psRange->uiEnd, psRange->bWrite ? sWriteColour : sColour);
	}
}

