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
#ifndef __TEXT_AREA_OCCURRENCES_H__
#define __TEXT_AREA_OCCURRENCES_H__
#include "BaseLib/TextDocument.h"
#include "BaseLib/TextEditing.h"
#include "SupportLib/GeometryMap.h"


class CTextAreaShapes;


#define TEXT_AREA_MAX_OCCURRENCES	1000	//The most occurrences of the selected text found.


//A range of a text area's text drawn as an occurrence (CTextArea::SetOccurrenceRanges): a symbol's usage, or with
//bWrite where it is declared.
struct STextOccurrenceRange
{
	size	uiStart;
	size	uiEnd;
	bool	bWrite;
};


typedef CArrayTemplate<STextOccurrenceRange>	CArrayTextOccurrenceRange;


//What a text area draws under its text as occurrences, as IntelliJ IDEA's editor does.  Highlighting (SetHighlighting),
//they are the selected text's other places in the text, exactly, case and all: the text area asks for them when its
//selection changes (Request) and finds them at a tick (Find), up to TEXT_AREA_MAX_OCCURRENCES of them; a selection
//over more than one line, or of only spaces and tabs, has none.  Ranges can be given too (SetRanges), a symbol's usages
//say, which the text area draws while nothing is selected.
//
//Both are of the text as it was: an edit of the document makes the found ones stale (they are asked for again) and
//drops the ranges given, and so does setting or appending text, which isn't an edit, so the text area says (TextSet).
class CTextAreaOccurrences : public CTextDocumentListener
{
protected:
	CTextDocument*				mpcDocument;  //Not held.
	bool						mbHighlighting;  //The selected text's other occurrences are found.
	CArrayTextOccurrence		mauiFound;  //Their starts, each as long as the selection they were found for.
	STextRange					msFoundFor;  //That selection, empty with nothing selected.
	bool						mbStale;  //The text changed since they were found.
	CArrayTextOccurrenceRange	masRanges;  //Given (SetRanges).

public:
	void					Init(CTextDocument* pcDocument);
	void					Kill(void);

	void					SetHighlighting(bool bHighlighting);
	bool					IsHighlighting(void);
	void					TextSet(void);
	void					DocumentChanged(STextDocumentChange* psChange) override;

	bool					IsFoundFor(size uiStart, size uiEnd);
	bool					Request(size uiStart, size uiEnd);
	bool					Find(size uiStart, size uiEnd);
	size					NumFound(void);
	size					GetFoundStart(size uiIndex);
	size					GetFoundLength(void);
	void					AddFound(CTextAreaShapes* pcShapes, Ptr<CGeometryMap> pMap, ARGB32 sColour);

	void					SetRanges(STextOccurrenceRange* asRanges, size uiNumRanges);
	bool					ClearRanges(void);
	size					NumRanges(void);
	STextOccurrenceRange*	GetRange(size uiIndex);
	void					AddRanges(CTextAreaShapes* pcShapes, Ptr<CGeometryMap> pMap, ARGB32 sColour, ARGB32 sWriteColour);
};


#endif // __TEXT_AREA_OCCURRENCES_H__

