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
#include "SupportLib/SubImage.h"
#include "Window.h"
#include "CheckBox.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szCaption, bool bTicked)
{
	PreInit();

	CGridContainer::Init(pWindow);
	SetSizedByContent(true);
	SetHitAsWhole(true);
	mpImages = pTheme->GetCheckBoxImages();
	mcChanged.Init();
	meState = CBS_Normal;
	miGap = pTheme->GetMetric(TM_CheckBoxGap);
	mbTicked = bTicked;
	mbPressed = false;
	SetCanGetFocus(true);

	//The gap is part of the box's width with the box drawn at its left.
	mpBox = OMalloc<CIconComponent>(pWindow, mpImages->GetCel(mbTicked, meState));
	mpBox->SetCelAlignment(CSH_Left, CSV_Center);
	mpBox->SetAlignment(CSH_Left, CSV_Center);
	mpCaption = OMalloc<CLabel>(pWindow, pTheme->GetFont(), szCaption == NULL ? "" : szCaption);
	mpCaption->SetAlignment(CSH_Left, CSV_Center);
	AddComponent(mpBox, 0, 0);
	AddComponent(mpCaption, 1, 0, 1, 1, 1, 0);
	UpdateGap();
	AddInputAction(CIC_ActivateComponent, this, &CCheckBox::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CCheckBox::ExecuteComponent);
	AddInputAction(CIC_Space, this, &CCheckBox::Toggle);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::Class(void)
{
	CGridContainer::Class();

	M_Pointer(mpImages);
	M_Pointer(mpBox);
	M_Pointer(mpCaption);
	U_Unknown(CComponentEvent<CCheckBox>, mcChanged);
	U_Enum(meState);
	U_Int32(miGap);
	U_Bool(mbTicked);
	U_Bool(mbPressed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::Free(void)
{
	mcChanged.Kill();
	CGridContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCheckBox::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCheckBox::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::UpdateGap(void)
{
	Ptr<CImageCel>	pCel;

	pCel = mpImages->GetCel(mbTicked, meState);
	mpBox->SetDesiredSize(pCel->GetSubImage()->GetFullWidth() + miGap, -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::SetGap(int32 iGap)
{
	miGap = iGap;
	UpdateGap();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The change event isn't called.
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::SetTicked(bool bTicked)
{
	if (mbTicked != bTicked)
	{
		mbTicked = bTicked;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// As a click does: ticks or unticks the box and calls the change event.
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::Toggle(void)
{
	mbTicked = !mbTicked;
	UpdateLook();
	mcChanged.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::SetCaption(const char* szCaption)
{
	mpCaption->SetText(szCaption == NULL ? "" : szCaption);

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the box is ticked or unticked by a click or space, with the
// check box: pCheckBox->GetChangeEvent()->Add(this, &CMyContent::Ticked).
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CCheckBox>* CCheckBox::GetChangeEvent(void)
{
	return &mcChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a press.
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::FocusOrMouseOverChanged(void)
{
	if (!HasFocus())
	{
		mbPressed = false;
	}
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
// The press has already given the check box focus.
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::ActivateComponent(int x, int y)
{
	mbPressed = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The release goes to the check box with focus wherever the pointer is.
// It only toggles if the pointer is still over the check box.
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::ExecuteComponent(int x, int y)
{
	if (mbPressed)
	{
		mbPressed = false;
		if (IsMouseOver())
		{
			Toggle();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Focussed, with or without the pointer over it, or just the pointer over
// it, or normal.
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::UpdateState(void)
{
	ECheckBoxState	eState;

	if (HasFocus())
	{
		eState = IsMouseOver() ? CBS_FocussedMouseOver : CBS_Focussed;
	}
	else
	{
		eState = IsMouseOver() ? CBS_MouseOver : CBS_Normal;
	}

	if (eState != meState)
	{
		meState = eState;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCheckBox::UpdateLook(void)
{
	mpBox->SetCel(mpImages->GetCel(mbTicked, meState));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCheckBox::IsTicked(void) { return mbTicked; }
char* CCheckBox::GetCaption(void) { return mpCaption->GetText(); }
ECheckBoxState CCheckBox::GetState(void) { return meState; }
Ptr<CIconComponent> CCheckBox::GetBoxComponent(void) { return mpBox; }
Ptr<CLabel> CCheckBox::GetCaptionLabel(void) { return mpCaption; }

