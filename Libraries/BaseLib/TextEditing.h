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
#ifndef __TEXT_EDITING_H__
#define __TEXT_EDITING_H__
#include "TextDocument.h"


//What happened to a text's undo history (CTextEditingView::UndoChanged).
enum ETextUndoChange
{
	TUC_Kept,	//An edit was kept: the text went from state uiIdBefore to the new state uiIdAfter.
	TUC_Merged,	//An edit was kept as part of the one before (typing a word): state uiIdBefore became uiIdAfter.
	TUC_Undone,	//The text went back from state uiIdBefore to uiIdAfter.
	TUC_Redone,	//The text went forward from state uiIdBefore to uiIdAfter.
};


struct STextUndoChange
{
	ETextUndoChange	eChange;
	size			uiIdBefore;
	size			uiIdAfter;
};


//Where up and down move the caret: a row or a page of rows.
enum ETextEditingMove
{
	TEM_RowUp,
	TEM_RowDown,
	TEM_PageUp,
	TEM_PageDown,
};


//What shows a text being edited (a CTextEditing): the rows its text is laid out in, and what it does when the caret
//moves or the text is edited.  Rows are numbered from 0, and x and y are the view's.  A position at the start of a
//wrapped row can be drawn at the end of the row above (bRowEnd).  A view with no rows (HasRows false, no text laid out
//yet) is asked nothing about them.
class CTextEditingView
{
public:
	virtual bool	HasRows(void) = 0;
	virtual size	NumRows(void) = 0;
	virtual size	FindRow(size uiPosition, bool bRowEnd) = 0;
	virtual size	GetRowStart(size uiRow) = 0;
	virtual size	GetRowEnd(size uiRow, bool* pbRowEnd) = 0;
	virtual size	FindRowAt(int32 y) = 0;
	virtual size	FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) = 0;
	virtual int32	GetPositionX(size uiPosition, bool bRowEnd) = 0;
	virtual size	ScrollPage(size uiRow, bool bDown) = 0;

	virtual void	MarksChanged(void) = 0;
	virtual void	ShowCaret(void) = 0;
	virtual void	TextEdited(void) = 0;
	virtual void	UndoChanged(STextUndoChange* psChange) = 0;
};


//The caret, the selection and the edits of a text document, as IntelliJ's editor has them, shown by a view.  Positions
//are bytes of the UTF8 text, always at the start of a character, and the caret and selection follow the document's
//edits.  Up, down, home and end work on the view's rows, so on wrapped lines too; up and down keep the caret's x and
//home is smart, it goes to the first character that isn't a space and then to the start of the line, and a second home
//or end on a wrapped row goes to the ends of the line.  Tab inserts spaces to the next tab stop, or with a selection
//over more than one line indents its lines; back tab removes a tab's worth of spaces from them.  A text that isn't
//editable can still be navigated, selected and copied from.
//
//Each edit begins with the caret and selection as they are (BeginEdit, for undo to put back), changes the document and
//ends (EditDone): it is kept in the document's history, the view is told what was kept (UndoChanged) and that the text
//was edited (TextEdited).
class CTextEditing : public CTextDocumentListener
{
protected:
	CTextDocument*		mpcDocument;  //Not held.
	CTextNavigation*	mpcNavigation;  //The document's.
	CTextEditingView*	mpcView;  //Not held.
	size				muiCaret;
	size				muiAnchor;  //Where the selection started, MAX_SIZE for no selection.
	bool				mbCaretAtRowEnd;  //A caret at the start of a wrapped row is drawn at the end of the row above.
	int32				miUpDownX;  //Where up and down try to keep the caret, -1 until they are used.
	size				muiTabWidth;  //In spaces.
	bool				mbEditable;

public:
	void		Init(CTextDocument* pcDocument, CTextEditingView* pcView);
	void		Kill(void);

	void		Reset(void);
	void		SetEditable(bool bEditable);
	bool		IsEditable(void);
	void		SetTabWidth(size uiTabWidth);
	size		GetTabWidth(void);

	size		GetCaretPosition(void);
	bool		IsCaretAtRowEnd(void);
	size		GetCaretRow(void);
	int32		GetCaretX(void);
	void		SetCaretPosition(size uiPosition);
	void		MoveCaret(size uiPosition, bool bSelect, bool bRowEnd = false, bool bKeepUpDown = false);
	bool		IsSelected(void);
	bool		GetSelection(size* puiStart, size* puiEnd);
	void		AnchorSelection(void);
	void		Select(size uiStart, size uiEnd);
	bool		FindPositionAt(int32 x, int32 y, size* puiPosition, bool* pbRowEnd);

	void		Up(void);
	void		Down(void);
	void		Left(void);
	void		Right(void);
	void		WordLeft(void);
	void		WordRight(void);
	void		Home(void);
	void		End(void);
	void		DocumentHome(void);
	void		DocumentEnd(void);
	void		PageUp(void);
	void		PageDown(void);
	void		SelectUp(void);
	void		SelectDown(void);
	void		SelectLeft(void);
	void		SelectRight(void);
	void		SelectWordLeft(void);
	void		SelectWordRight(void);
	void		SelectHome(void);
	void		SelectEnd(void);
	void		SelectDocumentHome(void);
	void		SelectDocumentEnd(void);
	void		SelectPageUp(void);
	void		SelectPageDown(void);
	void		SelectWholeWord(void);
	void		SelectAll(void);
	void		ClearSelection(void);

	void		Printable(char c, bool bInsert);
	void		Tab(bool bInsert);
	void		BackTab(void);
	void		Enter(void);
	void		HomeEnter(void);
	void		EndEnter(void);
	void		Delete(void);
	void		Backspace(void);
	void		DeleteWordRemainingRight(void);
	void		DeleteWordRemainingLeft(void);
	void		DeleteLine(void);
	void		Duplicate(void);
	void		Cut(CChars* pszDest);
	void		CutLine(CChars* pszDest);
	void		Copy(CChars* pszDest);
	void		Paste(char* szSource);
	bool		ReplaceRanges(size* auiStarts, size* auiEnds, size uiNumRanges, const char* szText);

	bool		Undo(void);
	bool		Redo(void);
	bool		UndoTo(size uiId);
	void		ClearUndo(void);

	void		DocumentChanged(STextDocumentChange* psChange) override;

protected:
	void		MoveUpDown(ETextEditingMove eMove, bool bSelect);
	void		MoveHome(bool bSelect);
	void		MoveEnd(bool bSelect);

	void		InsertText(size uiPosition, const char* szText);
	void		RemoveText(size uiStart, size uiEnd);
	void		ReplaceSelection(const char* szText);
	bool		DeleteSelection(void);
	void		FindLines(size uiStart, size uiEnd, CArrayTextLineStart* pauiLineStarts);
	void		IndentLines(size uiStart, size uiEnd);
	void		OutdentLines(size uiStart, size uiEnd);

	void		BeginEdit(void);
	bool		BeginEditIfEditable(void);
	void		EditDone(void);
	void		KeepEdit(void);
	bool		LoadUndo(size uiIndex);
	void		UndoChanged(ETextUndoChange eChange, size uiIdBefore, size uiIdAfter);
};


#endif // __TEXT_EDITING_H__

