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
#include "BaseLib/TextEditing.h"
#include "SupportLib/Font.h"
#include "ImageBorderContainer.h"
#include "WindowTheme.h"
#include "TextMarksCanvasDraw.h"
#include "TextEditingInput.h"
#include "Label.h"
#include "ComponentEvent.h"


//One line of editable text in a border, like IntelliJ's text field: the border is drawn from CButtonImages, in its
//selected state (the focus ring) while the field has focus, its mouse over state while the pointer is over it, like a
//combo box's, and its normal state otherwise.  The text is a label over a canvas that draws the selection's block and
//the caret.  When the text is wider than the field it scrolls sideways to keep the caret in view.
//
//The text is kept in a CTextDocument and edited by a single line CTextEditing, as a CTextArea's is, through the same
//input actions (CTextEditingInput): moving and selecting by character, word and to the ends (home is smart), deleting,
//cut, copy and paste with the window's clipboard, and select all.  Line breaks pasted or set become spaces.  Pressing
//places the caret at the nearest gap between characters, dragging selects from there and shift pressing selects from
//the caret.  Enter isn't wanted, so it clicks the window's default button.
//
//Edits can be undone (Control Z) and redone (Control Shift Z) as a CTextArea's: the text after each edit is kept,
//consecutive typing as one edit until a pause, and the caret and selection go back to how they were before the edit
//undone or after the edit redone.
//
//Its width is a number of the font's average characters, not its text, unless a desired width is set.
class CTextField : public CImageBorderContainer, public CTextEditingView
{
CONSTRUCTABLE(CTextField);
DESTRUCTABLE(CTextField);
protected:
	CTextDocument				mcDocument;
	CTextEditing				mcEditing;  //The caret, the selection and the edits of mcDocument.
	CTextEditingInput			mcInput;  //The input actions that reach mcEditing.
	Ptr<CFont>					mpFont;
	Ptr<CTextMarksCanvasDraw>	mpMarksDraw;
	Ptr<CCanvas>				mpMarks;
	Ptr<CLabel>					mpText;
	CComponentEvent<CTextField>	mcChanged;
	int32						miColumns;
	int32						miScroll;  //How far the text is moved left to keep the caret in view.

public:
	void						Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText);
	void						Class(void) override;
	void						Free(void) override;

	bool						Save(CObjectWriter* pcFile) override;
	bool						Load(CObjectReader* pcFile) override;

	void						SetText(const char* szText);
	char*						GetText(void);
	CTextEditing*				GetEditing(void);
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
	bool						TakesTyping(void) override;
	bool						CanDoCommand(EComponentInputCommand eCommand) override;
	bool						GetCaretRect(CInt32Rect* pcDest, ARGB32* psColour) override;
	void						FocusOrMouseOverChanged(void) override;

	void						CutToClipboard(void);
	void						CopyToClipboard(void);
	void						PasteFromClipboard(void);

	//The editing's view (CTextEditingView): one row, x relative to the field's left.
	bool						HasRows(void) override;
	size						NumRows(void) override;
	size						FindRow(size uiPosition, bool bRowEnd) override;
	size						GetRowStart(size uiRow) override;
	size						GetRowEnd(size uiRow, bool* pbRowEnd) override;
	size						FindRowAt(int32 y) override;
	size						FindPositionInRow(size uiRow, int32 x, bool* pbRowEnd) override;
	int32						GetPositionX(size uiPosition, bool bRowEnd) override;
	size						ScrollPage(size uiRow, bool bDown) override;
	void						MarksChanged(void) override;
	void						ShowCaret(void) override;
	void						TextEdited(void) override;
	void						UndoChanged(STextUndoChange* psChange) override;

protected:
	SInt32Vec2					GetContentSize(void) override;
	void						UpdateLook(void);
	void						LayoutText(void);
	int32						GetTextLeft(void);
};


#endif // __TEXT_FIELD_H__

