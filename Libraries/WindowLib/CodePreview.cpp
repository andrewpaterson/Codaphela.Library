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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Window.h"
#include "CodePreview.h"


//////////////////////////////////////////////////////////////////////////
//
// The title is in pTitleFont and the code in pCodeFamily's font.
//
//////////////////////////////////////////////////////////////////////////
void CCodePreview::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pCodeFamily)
{
	Ptr<CTextArea>	pCode;

	PreInit();

	CGridContainer::Init(pWindow);
	mpTheme = pTheme;
	mszTitle.Init();
	mbCentred = true;

	mpTitle = OMalloc<CLabel>(pWindow, pTitleFont, " ");  //Never empty, so its row keeps its height.
	mpTitle->SetAlignment(CSH_Left, CSV_Center);
	AddComponent(mpTitle, 0, 0, 1, 1, 1, 0);

	mpEditor = OMalloc<CCodeEditor>(pWindow, pTheme, pCodeFamily, "");
	mpEditor->SetAlignment(CSH_Stretch, CSV_Stretch);
	pCode = mpEditor->GetTextArea();
	pCode->SetTiledText(true);
	pCode->SetColour(pTheme->GetColour(TC_Text));
	pCode->SetEditable(false);
	pCode->SetCaretLine(true, pTheme->GetColour(TC_CaretLine));
	mpScrollPane = OMalloc<CScrollPane>(pWindow, pTheme);
	mpScrollPane->SetContent(mpEditor);
	mpScrollPane->SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(mpScrollPane, 0, 1, 1, 1, 1, 1);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
void CCodePreview::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpTitle);
	M_Pointer(mpEditor);
	M_Pointer(mpScrollPane);
	U_String(mszTitle);
	U_Bool(mbCentred);
}


//////////////////////////////////////////////////////////////////////////
//
// Its posted scroll isn't made once it has gone.
//
//////////////////////////////////////////////////////////////////////////
void CCodePreview::Free(void)
{
	if (mpWindow.IsNotNull())
	{
		mpWindow->CancelPosted(this);
	}
	mszTitle.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////
bool CCodePreview::Save(CObjectWriter* pcFile) { return false; }
bool CCodePreview::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// Whether it shows the code that was given with szTitle.
//
//////////////////////////////////////////////////////////////////////////
bool CCodePreview::IsShowing(const char* szTitle)
{
	return !mszTitle.Empty() && mszTitle.Equals(szTitle);
}


//////////////////////////////////////////////////////////////////////////
//
// A file's code, in place of what it showed, uncoloured.
//
//////////////////////////////////////////////////////////////////////////
void CCodePreview::SetCode(const char* szTitle, const char* szCode)
{
	Ptr<CTextArea>	pCode;

	pCode = mpEditor->GetTextArea();
	pCode->ClearBackgrounds();
	pCode->ClearSpans();
	pCode->SetText(szCode);
	mszTitle.Set(szTitle);
	mpTitle->SetText(mszTitle.Empty() ? " " : szTitle);
}


//////////////////////////////////////////////////////////////////////////
//
// A place in the code shown: uiLength of it from uiStart is coloured
// (and nothing else), its line is the caret's and it is scrolled to the
// middle of the view.
//
//////////////////////////////////////////////////////////////////////////
void CCodePreview::ShowPlace(size uiStart, size uiLength)
{
	Ptr<CTextArea>	pCode;
	size			uiEnd;

	pCode = mpEditor->GetTextArea();
	uiEnd = strlen(pCode->GetText());
	uiStart = (uiStart < uiEnd) ? uiStart : uiEnd;
	uiEnd = (uiLength < uiEnd - uiStart) ? uiStart + uiLength : uiEnd;
	pCode->ClearBackgrounds();
	if (uiEnd > uiStart)
	{
		pCode->AddBackground(uiStart, uiEnd, mpTheme->GetColour(TC_SearchMatch));
	}
	pCode->SetCaretPosition(uiStart);
	mbCentred = false;
	Centre();
}


//////////////////////////////////////////////////////////////////////////
//
// The code is scrolled so the place, where its caret is, is in the middle
// of the view, and only sideways if it is past the right of the view.  A
// window not shown yet has no size: it is done at a later tick, once it
// has.
//
//////////////////////////////////////////////////////////////////////////
void CCodePreview::Centre(void)
{
	CInt32Rect	cCaret;
	SInt32Vec2	sView;
	int32		iRight;
	int32		x;
	int32		y;

	if (mbCentred)
	{
		return;
	}
	//A window without a size isn't laid out: a tab bar in it, with no room, would put its chosen tab first.
	sView = mpScrollPane->GetActualSize();
	if ((sView.x <= 0) || (sView.y <= 0))
	{
		mpWindow->Post(this, &CCodePreview::Centre);
		return;
	}
	mpWindow->Layout();
	sView = mpScrollPane->GetActualSize();
	mbCentred = true;
	mpEditor->GetTextArea()->GetCaretBounds(&cCaret);
	iRight = mpEditor->GetGutterWidth() + cCaret.GetRight();
	x = (iRight < sView.x - sView.x / 4) ? 0 : iRight - sView.x / 2;
	y = cCaret.GetTop() - (sView.y - cCaret.GetHeight()) / 2;
	mpScrollPane->SetScrollPosition((x > 0) ? x : 0, (y > 0) ? y : 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing is shown.
//
//////////////////////////////////////////////////////////////////////////
void CCodePreview::Clear(void)
{
	if (!mszTitle.Empty())
	{
		SetCode("", "");
	}
}


char* CCodePreview::GetTitle(void) { return mszTitle.Text(); }
Ptr<CCodeEditor> CCodePreview::GetEditor(void) { return mpEditor; }
Ptr<CTextArea> CCodePreview::GetTextArea(void) { return mpEditor->GetTextArea(); }
Ptr<CScrollPane> CCodePreview::GetScrollPane(void) { return mpScrollPane; }

