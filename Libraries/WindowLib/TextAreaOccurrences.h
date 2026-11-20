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
#define TEXT_AREA_MAX_HIGHLIGHTS	2000	//The most of a layer's ranges drawn: those nearest its current one.
#define TEXT_AREA_NO_HIGHLIGHT		MAX_SIZE	//A layer's current range when it has none.


//A range of a text area's text drawn as an occurrence (CTextArea::SetOccurrenceRanges): a symbol's usage, or with
//bWrite where it is declared.
struct STextOccurrenceRange : public STextRange
{
	bool	bWrite;
};


typedef CArrayTemplate<STextOccurrenceRange>	CArrayTextOccurrenceRange;


//Ranges of a text area's text drawn under it in a colour of their own (CTextArea::AddHighlightLayer): a search's
//matches, say.  One of them can be the current one, drawn in another colour.
struct STextHighlightLayer
{
	CArrayTextRange	asRanges;  //In the text's order.
	ARGB32			sColour;
	ARGB32			sCurrentColour;
	size			uiCurrent;  //TEXT_AREA_NO_HIGHLIGHT for none.
};


typedef CArrayTemplate<STextHighlightLayer>	CArrayTextHighlightLayer;


//What a text area draws under its text as occurrences, as IntelliJ IDEA's editor does.  Highlighting (SetHighlighting),
//they are the selected text's other places in the text, exactly, case and all: the text area asks for them when its
//selection changes (Request) and finds them at a tick (Find), up to TEXT_AREA_MAX_OCCURRENCES of them; a selection
//over more than one line, or of only spaces and tabs, has none.  Ranges can be given too (SetRanges), a symbol's usages
//say, which the text area draws while nothing is selected.
//
//A text area can have layers of highlights too (AddLayer), each a set of ranges in its own colour that whoever made
//the layer gives it (a Find bar's matches): they are drawn under the occurrences and the selection, selected text or
//not, with the layer's current range in its second colour.  Of a layer with very many ranges only those nearest the
//current one are drawn (TEXT_AREA_MAX_HIGHLIGHTS).
//
//All are of the text as it was: an edit of the document makes the found ones stale (they are asked for again) and
//drops the ranges given and every layer's, and so does setting or appending text, which isn't an edit, so the text
//area says (TextSet).  Whoever gave them gives them again for the text as it is.
class CTextAreaOccurrences : public CTextDocumentListener
{
protected:
	CTextDocument*				mpcDocument;  //Not held.
	bool						mbHighlighting;  //The selected text's other occurrences are found.
	CArrayTextRange				masFound;  //Where they are.
	STextRange					msFoundFor;  //That selection, empty with nothing selected.
	bool						mbStale;  //The text changed since they were found.
	CArrayTextOccurrenceRange	masRanges;  //Given (SetRanges).
	CArrayTextHighlightLayer	masLayers;

public:
	void					Init(CTextDocument* pcDocument);
	void					Kill(void);

	void					SetHighlighting(bool bHighlighting);
	bool					IsHighlighting(void) const;
	void					TextSet(void);
	void					DocumentChanged(STextDocumentChange* psChange) override;

	bool					IsFoundFor(size uiStart, size uiEnd);
	bool					Request(size uiStart, size uiEnd);
	bool					Find(size uiStart, size uiEnd);
	size					NumFound(void) const;
	STextRange*				GetFound(size uiIndex);
	void					AddFound(CTextAreaShapes* pcShapes, Ptr<CGeometryMap> pMap, ARGB32 sColour);

	void					SetRanges(STextOccurrenceRange* asRanges, size uiNumRanges);
	bool					ClearRanges(void);
	size					NumRanges(void) const;
	STextOccurrenceRange*	GetRange(size uiIndex);
	void					AddRanges(CTextAreaShapes* pcShapes, Ptr<CGeometryMap> pMap, ARGB32 sColour, ARGB32 sWriteColour);

	size					AddLayer(ARGB32 sColour, ARGB32 sCurrentColour);
	size					NumLayers(void) const;
	void					SetLayerRanges(size uiLayer, STextRange* asRanges, size uiNumRanges, size uiCurrent);
	bool					ClearLayer(size uiLayer);
	bool					SetLayerCurrent(size uiLayer, size uiCurrent);
	size					GetLayerCurrent(size uiLayer);
	size					NumLayerRanges(size uiLayer);
	STextRange*				GetLayerRange(size uiLayer, size uiIndex);
	void					AddLayers(CTextAreaShapes* pcShapes, Ptr<CGeometryMap> pMap);
};


#endif // __TEXT_AREA_OCCURRENCES_H__

