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
#ifndef __TEXT_DOCUMENT_H__
#define __TEXT_DOCUMENT_H__
#include "Chars.h"
#include "ArrayTemplatePtr.h"
#include "TextNavigation.h"
#include "TextUndo.h"


typedef CArrayTemplate<size>	CArrayTextLineStart;


enum ETextDocumentChange
{
	TDC_Inserted,
	TDC_Removed
};


//An edit of a text document (CTextDocument::Insert and Remove), as its listeners are told it.  Positions are bytes of
//the UTF8 text and lines are numbered from 0, both as they were before the edit.
struct STextDocumentChange
{
	ETextDocumentChange	eChange;
	size				uiStart;  //Where text was put in or taken out.
	size				uiEnd;  //The end of the text put in (as it is after the edit) or taken out.
	size				uiLine;  //The line uiStart is on.
	size				uiLines;  //How many line feeds were put in or taken out.
	bool				bLineStart;  //Put in at the start of line uiLine, or whole lines taken out.

	size	Length(void);
	size	LastLine(void);
};


//Told each edit of a text document, after the text and its lines have changed, to keep what is anchored to positions
//or lines of the text (the caret, styles, colours, gaps below lines) in step with it.
class CTextDocumentListener
{
public:
	virtual void	DocumentChanged(STextDocumentChange* psChange) = 0;
};


typedef CArrayTemplatePtr<CTextDocumentListener>	CArrayTextDocumentListener;


//A UTF8 text being edited (by CTextArea and CTextField): the text, where its lines start (lines are ended by line
//feeds), its undo history and how many times it has been edited.  Insert and Remove are its only edits; each tells its
//listeners what changed.  Setting or appending text isn't an edit: nothing anchored to the text moves and nothing is
//kept in the history.
//
//The history is that of a CTextUndo, which the editor drives: it begins an edit with the caret and selection before it
//(BeginEdit, StartEdit), makes the edit's changes and ends it (EndEdit) with the caret and selection after it.  A
//document that isn't undoable (SetUndoable) keeps no history.  Undoing or redoing (Load) changes only the part of the
//text that differs from the state loaded, so the listeners move as little as they can.
class CTextDocument
{
protected:
	CChars						mszText;
	CArrayTextLineStart			mauiLineStarts;  //The first byte of each line, at least one.
	CTextNavigation				mcNavigation;  //Characters, lines and words of mszText.
	CTextUndo					mcUndo;
	bool						mbUndoable;
	size						muiChangeCount;
	CArrayTextDocumentListener	mapcListeners;  //Not held.

public:
	void					Init(const char* szText);
	void					Kill(void);

	void					AddListener(CTextDocumentListener* pcListener);
	void					RemoveListener(CTextDocumentListener* pcListener);

	void					SetText(const char* szText);
	void					Append(const char* szText);
	void					Insert(size uiPosition, const char* szText);
	void					Remove(size uiStart, size uiEnd);

	char*					Text(void);
	size					Length(void);
	size					GetChangeCount(void);
	CTextNavigation*		GetNavigation(void);

	size					NumLines(void);
	size					GetLineStart(size uiLine);
	size					FindLine(size uiPosition);
	CArrayTextLineStart*	GetLineStarts(void);

	void					SetUndoable(bool bUndoable);
	bool					IsUndoable(void);
	CTextUndo*				GetUndo(void);
	void					BeginEdit(size uiCaret, size uiAnchor);
	void					StartEdit(size uiCaret, size uiAnchor);
	ETextUndoKeep			EndEdit(size uiCaret, size uiAnchor);
	void					ClearUndo(size uiCaret, size uiAnchor);
	bool					CanUndo(void);
	bool					CanRedo(void);
	bool					Load(size uiIndex, size* puiCaret, size* puiAnchor);

protected:
	void					FindLineStarts(size uiFrom);
	void					Changed(STextDocumentChange* psChange);
};


#endif // __TEXT_DOCUMENT_H__

