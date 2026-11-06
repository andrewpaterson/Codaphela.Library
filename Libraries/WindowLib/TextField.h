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
#ifndef __TEXT_FIELD_H__
#define __TEXT_FIELD_H__
#include "BaseLib/PlainTextEditor.h"
#include "BaseLib/UndoStack.h"
#include "SupportLib/Font.h"
#include "ImageBorderContainer.h"
#include "WindowTheme.h"
#include "TextMarksCanvasDraw.h"
#include "Label.h"
#include "ComponentEvent.h"


//One line of editable text in a border, like IntelliJ's text field: the border is drawn from CButtonImages, in its
//selected state (the focus ring) while the field has focus, its mouse over state while the pointer is over it, like a
//combo box's, and its normal state otherwise.  The text is a label over a
//canvas that draws the selection's block and the caret.  When the text is wider than the field it scrolls sideways to
//keep the caret in view.
//
//The text is edited by a CPlainTextEditor.  Every key a CComponentInput gives that makes sense on one line works:
//moving and selecting by character, word and to the ends, deleting, cut, copy and paste with the window's
//clipboard, and select all.  Pressing places the caret at the nearest gap between characters, dragging selects from
//there and shift pressing selects from the caret.  Enter isn't wanted, so it clicks the window's default button.
//
//How long typing can stop for and the next character still be part of the same edit, as a CTextArea's.
#define TEXT_FIELD_TYPING_PAUSE	1000


//Edits can be undone (Control Z) and redone (Control Shift Z or Control Y) as a CTextArea's: the text after each edit
//is kept on a CUndoStack, consecutive typing as one edit until a pause, and the caret and selection go back to how
//they were before the edit undone or after the edit redone.
//
//Its width is a number of the font's average characters, not its text, unless a desired width is set.
class CTextField : public CImageBorderContainer
{
CONSTRUCTABLE(CTextField);
DESTRUCTABLE(CTextField);
protected:
	CPlainTextEditor			mcEditor;
	Ptr<CFont>					mpFont;
	Ptr<CTextMarksCanvasDraw>	mpMarksDraw;
	Ptr<CCanvas>				mpMarks;
	Ptr<CLabel>					mpText;
	CComponentEvent<CTextField>	mcChanged;
	int32						miColumns;
	int32						miScroll;  //How far the text is moved left to keep the caret in view.
	size						muiDragAnchor;  //Where a drag's selection started.
	bool						mbDragging;

	//Undo, as CTextArea's: the text after each edit, typing kept as one edit until a pause.
	CUndoStack					mcUndo;
	bool						mbUndoing;  //An undo or redo is changing the text: it isn't an edit to keep.
	bool						mbTyping;  //The edit being made is a character typed (Printable).
	bool						mbLastTyping;  //So was the last edit kept.
	char						mcTyped;
	char						mcLastTyped;
	size						muiTypedAt;  //Where the caret was when it was typed, MAX_SIZE over a selection.
	size						muiTypedEnd;  //Where the caret was after the last edit kept.
	int64						miTypedTime;  //When the last character was typed (GetDebugTickCount).
	int32						miTypingPause;  //Typing that stops this long (milliseconds) starts a new edit.
	size						muiEditCaret;  //The caret and selection anchor before the edit being made.
	size						muiEditAnchor;

public:
	void						Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText);
	void						Class(void) override;
	void 						Free(void) override;

	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	void						SetText(const char* szText);
	char*						GetText(void);
	size						GetCaretPosition(void);
	void						SetCaretPosition(size uiPosition);
	bool						GetSelection(size* puiStart, size* puiEnd);
	void						Select(size uiStart, size uiEnd);
	void						SelectAll(void);
	void						SetColumns(int32 iColumns);
	int32						GetScroll(void);
	int32						GetCharacterX(size uiPosition);
	size						FindPositionAt(int x);
	Ptr<CLabel>					GetTextLabel(void);
	Ptr<CTextMarksCanvasDraw>	GetMarksDraw(void);

	CComponentEvent<CTextField>*	GetChangeEvent(void);

	bool						Undo(void);
	bool						Redo(void);
	bool						CanUndo(void);
	bool						CanRedo(void);
	void						ClearUndo(void);
	void						SetTypingPause(int32 iMilliseconds);

	void						Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool						WantsEnter(void) override;
	bool						GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour) override;

	void						FocusOrMouseOverChanged(void) override;
	void						PrintableAction(SComponentInputEvent* psEvent);
	void						SpaceAction(SComponentInputEvent* psEvent);
	void						ActivateComponent(int x, int y);
	void						ExecuteComponent(int x, int y);
	void						StartDragSelect(int x, int y);
	void						ContDragSelect(int x, int y);
	void						StopDragSelect(int x, int y);

	void						Printable(char c, bool bInsert);
	void						Delete(void);
	void						Backspace(void);
	void						Cut(CChars* pszDest);
	void						Copy(CChars* pszDest);
	void						Paste(char* szSource);
	void						DeleteWordRemainingRight(void);
	void						DeleteWordRemainingLeft(void);
	void						CutToClipboard(void);
	void						CopyToClipboard(void);
	void						PasteFromClipboard(void);
	void						UndoAction(void);
	void						RedoAction(void);

protected:
	void						TextEdited(void);
	void						BeginEdit(void);
	void						KeepEdit(void);
	bool						LoadUndo(size uiIndex);
	size						GetSelectionAnchor(void);
	SInt32Vec2						GetContentSize(void) override;
	void						UpdateLook(void);
	void						SelectTo(size uiPosition);
	void						LayoutText(void);
	void						ToClipboard(void (CTextField::*fTake)(CChars* pszDest));
};


#endif // __TEXT_FIELD_H__

