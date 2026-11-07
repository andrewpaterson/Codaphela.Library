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
#include "StandardLib/Objects.h"
#include "StandardLib/ClassDefines.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "LineZone.h"


//////////////////////////////////////////////////////////////////////////
//
// The header in pTitleFont over the text in pTextFamily's fixed width
// font (as tiles) in sText, on sBackground so it stands apart from the
// code.  The
// < and > come first, while there are pages to step through: the zone is
// as wide as the code, which can be wider than the view.
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFont> pTitleFont, Ptr<CFontFamily> pTextFamily, ARGB32 sBackground, ARGB32 sText)
{
	Ptr<CCanvas>	pBackground;

	PreInit();

	CGridContainer::Init(pWindow);
	mpcSource = NULL;
	muiLine = 0;
	muiPages = 0;
	muiPage = 0;
	mszTitle.Init();
	mbOutOfDate = false;
	msText = sText;
	msOutOfDate = pTheme->GetColour(TC_LineNumber);

	mpTitle = OMalloc<CLabel>(pWindow, pTitleFont, "");
	mpTitle->SetAlignment(CSH_Left, CSV_Center);
	mpPrevious = CreateButton(pWindow, pTheme, "<");
	mpNext = CreateButton(pWindow, pTheme, ">");
	mpPrevious->GetClickEvent()->Add(this, &CLineZone::StepPrevious);
	mpNext->GetClickEvent()->Add(this, &CLineZone::StepNext);
	mpText = OMalloc<CTextArea>(pWindow, pTheme, pTextFamily, "");
	mpText->SetEditable(false);
	mpText->SetTiledText(true);
	mpText->SetAlignment(CSH_Left, CSV_Top);
	pBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(sBackground));
	pBackground->SetAlignment(CSH_Stretch, CSV_Stretch);

	SetAlignment(CSH_Stretch, CSV_Stretch);
	AddComponent(pBackground, 0, 0, 3, 2, 1, 1);
	AddComponent(mpTitle, 2, 0, 1, 1, 1, 0);
	AddComponent(mpText, 0, 1, 3, 1, 1, 1);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A small < or > button, so it doesn't crowd the title.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CButton> CLineZone::CreateButton(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szCaption)
{
	Ptr<CButton>	pButton;

	pButton = OMalloc<CButton>(pWindow, pTheme, BL_Small, Ptr<CImageCel>(), szCaption);
	pButton->SetPadding(4, 0);
	return pButton;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::Class(void)
{
	CGridContainer::Class();
	M_Pointer(mpTitle);
	M_Pointer(mpPrevious);
	M_Pointer(mpNext);
	M_Pointer(mpText);
	U_Pointer(mpcSource);
	U_Size(muiLine);
	U_Size(muiPages);
	U_Size(muiPage);
	U_String(mszTitle);
	U_Bool(mbOutOfDate);
	U_UInt32(msText);
	U_UInt32(msOutOfDate);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::Free(void)
{
	if (mpPrevious.IsNotNull())
	{
		mpPrevious->GetClickEvent()->Remove(this);
	}
	if (mpNext.IsNotNull())
	{
		mpNext->GetClickEvent()->Remove(this);
	}
	mszTitle.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A zone isn't saved.
//
//////////////////////////////////////////////////////////////////////////
bool CLineZone::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLineZone::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The pages of line uiLine of pcSource.  The page shown stays if there
// are as many, otherwise the first is.  The zone is up to date.
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::SetSource(CLineZoneSource* pcSource, size uiLine)
{
	mpcSource = pcSource;
	muiLine = uiLine;
	muiPages = pcSource->NumPages(uiLine);
	if (muiPage >= muiPages)
	{
		muiPage = 0;
	}
	mbOutOfDate = false;
	ShowPage();
}


//////////////////////////////////////////////////////////////////////////
//
// To the next page (iDirection 1) or the one before (-1), round from the
// last to the first.  An out of date zone stays on its page: the source
// may not have the line's pages now.
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::Step(int iDirection)
{
	if ((muiPages < 2) || mbOutOfDate)
	{
		return;
	}
	muiPage = (muiPage + ((iDirection < 0) ? muiPages - 1 : 1)) % muiPages;
	ShowPage();
}


//////////////////////////////////////////////////////////////////////////
//
// The code was edited since the source was made: what is shown may not be
// the line's now.
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::SetOutOfDate(bool bOutOfDate)
{
	if (mbOutOfDate != bOutOfDate)
	{
		mbOutOfDate = bOutOfDate;
		mpText->SetColour(mbOutOfDate ? msOutOfDate : msText);
		ShowTitle();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::StepPrevious(void) { Step(-1); }  //The < button.
void CLineZone::StepNext(void) { Step(1); }  //The > button.


//////////////////////////////////////////////////////////////////////////
//
// The page's title and text from the source, and the zone as high as they
// are (a grid is variable sized: the code editor's gap is this high).
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::ShowPage(void)
{
	CChars	szText;
	int32	iHeader;

	mszTitle.Clear();
	szText.Init();
	if (muiPages > 0)
	{
		mpcSource->GetTitle(muiLine, muiPage, &mszTitle);
		mpcSource->GetText(muiLine, muiPage, &szText);
	}
	mpText->SetText(szText.Text());
	mpText->SetColour(mbOutOfDate ? msOutOfDate : msText);
	szText.Kill();
	ShowTitle();
	ShowButtons();

	iHeader = mpTitle->GetDesiredSize().y;
	if (HasButtons() && (mpNext->GetDesiredSize().y > iHeader))
	{
		iHeader = mpNext->GetDesiredSize().y;
	}
	SetDesiredSize(-1, iHeader + mpText->GetTextSize().y);
}


//////////////////////////////////////////////////////////////////////////
//
// "Title - 2 of 3", and "(out of date)" after when it is.
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::ShowTitle(void)
{
	CChars	szTitle;

	szTitle.Init(mszTitle);
	if (muiPages > 0)
	{
		szTitle.Append(" - ");
		szTitle.Append((uint32)(muiPage + 1));
		szTitle.Append(" of ");
		szTitle.Append((uint32)muiPages);
	}
	if (mbOutOfDate)
	{
		szTitle.Append("  (out of date)");
	}
	mpTitle->SetText(szTitle.Text());
	szTitle.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The < and > if there is more than one page, none otherwise.
//
//////////////////////////////////////////////////////////////////////////
void CLineZone::ShowButtons(void)
{
	if ((muiPages > 1) == HasButtons())
	{
		return;
	}

	if (HasButtons())
	{
		RemoveComponent(mpPrevious);
		RemoveComponent(mpNext);
	}
	else
	{
		AddComponent(mpPrevious, 0, 0);
		AddComponent(mpNext, 1, 0);
	}
}


bool CLineZone::HasButtons(void) { return mpNext->GetParent().IsNotNull(); }
size CLineZone::NumPages(void) { return muiPages; }
size CLineZone::GetPage(void) { return muiPage; }
char* CLineZone::GetTitle(void) { return mpTitle->GetText(); }
char* CLineZone::GetText(void) { return mpText->GetText(); }
bool CLineZone::IsOutOfDate(void) { return mbOutOfDate; }
Ptr<CButton> CLineZone::GetPreviousButton(void) { return mpPrevious; }
Ptr<CButton> CLineZone::GetNextButton(void) { return mpNext; }

