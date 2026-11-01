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
#include "StandardLib/ListenerCall.h"
#include "SupportLib/Font.h"
#include "ImageBorderContainer.h"
#include "WindowTheme.h"
#include "TextMarksCanvasDraw.h"
#include "TextFieldListener.h"
#include "Label.h"


//One line of editable text in a border, like IntelliJ's text field: the border is drawn from CButtonImages, in its
//selected state (the focus ring) while the field has focus, its mouse over state while the pointer is over it, like a
//combo box's, and its normal state otherwise.  The text is a label over a
//canvas that draws the selection's block and the caret.  When the text is wider than the field it scrolls sideways to
//keep the caret in view.
//
//The text is edited by a CPlainTextEditor.  Every key a CComponentInput gives that makes sense on one line works:
//moving and selecting by character, word and to the ends, deleting, cut, copy and paste with the window factory's
//clipboard, and select all.  Pressing places the caret at the nearest gap between characters, dragging selects from
//there and shift pressing selects from the caret.  Enter isn't wanted, so it clicks the window's default button.
//
//Its width is a number of the font's average characters, not its text, unless a desired width is set.
class CTextField : public CImageBorderContainer
{
CONSTRUCTABLE(CTextField);
DESTRUCTABLE(CTextField);
protected:
	CPlainTextEditor			mcEditor;
	Ptr<CFont>					mpFont;
	Ptr<CFillContainer>			mpPadded;  //The face less its padding.
	Ptr<CTextMarksCanvasDraw>	mpMarksDraw;
	Ptr<CCanvas>				mpMarks;
	Ptr<CLabel>					mpText;
	CListenerCall				mcListeners;
	SInt32Vec2					msPadding;
	int32						miColumns;
	int32						miScroll;  //How far the text is moved left to keep the caret in view.
	size						muiDragAnchor;  //Where a drag's selection started.
	bool						mbFocussed;
	bool						mbMouseOver;
	bool						mbDragging;

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
	void						SetColumns(int32 iColumns);
	void						SetPadding(int32 iHorizontal, int32 iVertical);
	int32						GetScroll(void);
	int32						GetCharacterX(size uiPosition);
	size						FindPositionAt(int x);
	Ptr<CLabel>					GetTextLabel(void);
	Ptr<CTextMarksCanvasDraw>	GetMarksDraw(void);

	bool						AddListener(CTextFieldListener* pcListener);
	bool						RemoveListener(CTextFieldListener* pcListener);

	void						Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool						WantsEnter(void) override;

	void						FocusGained(void) override;
	void						FocusLost(void) override;
	void						MovedInto(void) override;
	void						MovedOutof(void) override;
	void						CommandAction(SComponentInputEvent* psEvent);
	void						PrintableAction(SComponentInputEvent* psEvent);
	void						ActivateComponent(int x, int y);
	void						ExecuteComponent(int x, int y);
	void						StartDragSelect(int x, int y);
	void						ContDragSelect(int x, int y);
	void						StopDragSelect(int x, int y);

	void						Left(void);
	void						Right(void);
	void						WordLeft(void);
	void						WordRight(void);
	void						Home(void);
	void						End(void);
	void						DocumentHome(void);
	void						DocumentEnd(void);
	void						SelectLeft(void);
	void						SelectRight(void);
	void						SelectWordLeft(void);
	void						SelectWordRight(void);
	void						SelectHome(void);
	void						SelectEnd(void);
	void						SelectDocumentHome(void);
	void						SelectDocumentEnd(void);
	void						SelectWholeWord(void);
	void						SelectAll(void);
	void						ClearSelection(void);
	void						Printable(char c, bool bInsert);
	void						Space(bool bInsert);
	void						Delete(void);
	void						Backspace(void);
	void						Cut(CChars* pszDest);
	void						Copy(CChars* pszDest);
	void						Paste(char* szSource);
	void						DeleteWordRemainingRight(void);
	void						DeleteWordRemainingLeft(void);

protected:
	void						TextEdited(void);
	void						UpdateFaceSize(void);
	void						UpdateLook(void);
	void						SelectTo(size uiPosition);
	void						LayoutText(void);
};


#endif // __TEXT_FIELD_H__

