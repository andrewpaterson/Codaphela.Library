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
#include "FillContainer.h"
#include "PromptPopup.h"


#define PROMPT_POPUP_INSET		8		//Between the popup's edge and what it holds.
#define PROMPT_POPUP_COLUMNS	16		//The field's width, in characters.


//////////////////////////////////////////////////////////////////////////
//
// The look isn't held.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::Init(Ptr<CWindow> pWindow, SDialogLook* psLook)
{
	Ptr<CFillContainer>	pGap;
	int32				iGap;

	PreInit();
	mpWindow = pWindow;
	mpsLook = psLook;
	mcAccepted.Init();
	mcClosed.Init();

	iGap = psLook->pcTheme->GetMetric(TM_GoToGap);
	mpCaption = OMalloc<CLabel>(pWindow, psLook->pcTheme->GetFont(), "");
	mpCaption->SetAlignment(CSH_Left, CSV_Center);
	mpField = OMalloc<CTextField>(pWindow, psLook->pcTheme, "");
	mpField->SetColumns(PROMPT_POPUP_COLUMNS);
	mpField->SetAlignment(CSH_Stretch, CSV_Center);
	mpField->AddInputAction(CIC_Enter, this, &CPromptPopup::Accept);
	mpField->GetChangeEvent()->Add(this, &CPromptPopup::TextChanged);
	mpProblem = OMalloc<CLabel>(pWindow, psLook->pcDimFont, "");
	mpProblem->SetAlignment(CSH_Left, CSV_Center);

	mpPanel = OMalloc<CGridContainer>(pWindow);
	mpPanel->AddComponent(mpCaption, 0, 0);
	pGap = OMalloc<CFillContainer>(pWindow);
	pGap->SetDesiredSize(iGap, 1);
	mpPanel->AddComponent(pGap, 1, 0);
	mpPanel->AddComponent(mpField, 2, 0, 1, 1, 1, 0);
	mpPanel->AddComponent(mpProblem, 0, 1, 3, 1, 1, 0);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::Class(void)
{
	M_Pointer(mpWindow);
	U_Pointer(mpsLook);
	M_Pointer(mpPanel);
	M_Pointer(mpCaption);
	M_Pointer(mpField);
	M_Pointer(mpProblem);
	M_Pointer(mpPopup);
	M_Pointer(mpPreviousFocus);
	U_Unknown(CComponentEvent<CPromptPopup>, mcAccepted);
	U_Unknown(CComponentEvent<CPromptPopup>, mcClosed);
}


//////////////////////////////////////////////////////////////////////////
//
// An object's memory is freed without its destructor running, so the
// prompt takes itself off the widgets still here.  Its popup is closed.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::Free(void)
{
	if (mpPopup.IsNotNull())
	{
		mpPopup->GetClosedEvent()->Remove(this);
		if (mpWindow.IsNotNull() && mpPanel.IsNotNull())
		{
			mpWindow->ClosePopup(mpPanel);
		}
	}
	if (mpField.IsNotNull())
	{
		mpField->GetChangeEvent()->Remove(this);
	}
	mcClosed.Kill();
	mcAccepted.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPromptPopup::Save(CObjectWriter* pcFile) { return false; }
bool CPromptPopup::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// Opens the prompt with its top left at sPosition, in the window's client
// space (it is moved back in if that would put it past an edge), its
// caption szCaption and its field holding szText, selected so typing
// replaces it.  pOwner is the component it is opened for: a press on it
// closes the prompt, as one anywhere else outside it does.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::Open(SInt32Vec2 sPosition, Ptr<CComponent> pOwner, const char* szCaption, const char* szText)
{
	SInsets		sInsets;
	int32		iWidth;
	int32		iHeight;

	Close();
	mpCaption->SetText(szCaption);
	mpProblem->SetText("");
	mpField->GetChangeEvent()->Remove(this);
	mpField->SetText(szText);
	mpField->ClearUndo();
	mpField->SelectAll();
	mpField->GetChangeEvent()->Add(this, &CPromptPopup::TextChanged);

	//A grid is the size it is given: the caption, the gap and the field wide, and the field and the problem's line high.
	iWidth = mpCaption->GetDesiredSize().x + mpsLook->pcTheme->GetMetric(TM_GoToGap) + mpField->GetDesiredSize().x;
	iHeight = mpField->GetDesiredSize().y + mpsLook->pcTheme->GetMetric(TM_GoToRowHeight);
	mpPanel->SetDesiredSize(iWidth, iHeight);

	mpPreviousFocus = mpWindow->GetFocus()->GetFocussedComponent();
	mpPopup = mpWindow->OpenPopup(mpPanel, sPosition, pOwner);
	sInsets.Init(PROMPT_POPUP_INSET, PROMPT_POPUP_INSET);
	mpPopup->SetLook(mpsLook->pcTheme->GetColour(TC_MenuBackground), mpsLook->pcTheme->GetColour(TC_MenuBorder), &sInsets);
	mpPopup->SetOwnerPressKeepsOpen(false);
	mpPopup->GetClosedEvent()->Add(this, &CPromptPopup::PopupClosed);
	mpWindow->GetFocus()->SetFocus(mpField);
}


//////////////////////////////////////////////////////////////////////////
//
// Closes it if it is open.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::Close(void)
{
	if (mpPopup.IsNotNull())
	{
		mpWindow->ClosePopup(mpPanel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The popup has gone, however it went: the keys go back to what had
// them, if they were still in the prompt.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::PopupClosed(void)
{
	Ptr<CComponent>	pFocussed;
	Ptr<CComponent>	pPrevious;

	mpPopup = NULL;
	pPrevious = mpPreviousFocus;
	mpPreviousFocus = NULL;
	pFocussed = mpWindow->GetFocus()->GetFocussedComponent();
	if (pPrevious.IsNotNull() && (pFocussed.IsNull() || pFocussed->IsWithin(mpPanel)))
	{
		mpWindow->GetFocus()->SetFocus(pPrevious);
	}
	mcClosed.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// Enter: whoever hears the accepted event takes the text, and closes the
// prompt or says what is wrong with it.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::Accept(void)
{
	mcAccepted.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// What is wrong with what was typed, shown under the field until it is
// typed in again; "" for nothing.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::SetProblem(const char* szProblem)
{
	mpProblem->SetText(szProblem);
}


//////////////////////////////////////////////////////////////////////////
//
// Typing takes what was wrong away: it was said of what the field had.
//
//////////////////////////////////////////////////////////////////////////
void CPromptPopup::TextChanged(void)
{
	mpProblem->SetText("");
}


bool CPromptPopup::IsOpen(void) { return mpPopup.IsNotNull(); }
const char* CPromptPopup::GetText(void) { return mpField->GetText(); }
void CPromptPopup::SetText(const char* szText) { mpField->SetText(szText); }
const char* CPromptPopup::GetProblem(void) { return mpProblem->GetText(); }
const char* CPromptPopup::GetCaption(void) { return mpCaption->GetText(); }
Ptr<CTextField> CPromptPopup::GetField(void) const { return mpField; }
Ptr<CComponent> CPromptPopup::GetPanel(void) const { return mpPanel; }
CComponentEvent<CPromptPopup>* CPromptPopup::GetAcceptedEvent(void) { return &mcAccepted; }
CComponentEvent<CPromptPopup>* CPromptPopup::GetClosedEvent(void) { return &mcClosed; }

