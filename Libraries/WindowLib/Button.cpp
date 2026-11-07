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
#include "Button.h"


//////////////////////////////////////////////////////////////////////////
//
// A normal button with only a caption.
//
//////////////////////////////////////////////////////////////////////////
void CButton::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szCaption)
{
	Init(pWindow, pTheme, BL_Normal, NULL, szCaption);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CImageCel> pIcon, const char* szCaption)
{
	Init(pWindow, pTheme, BL_Normal, pIcon, szCaption);
}


//////////////////////////////////////////////////////////////////////////
//
// pIcon can be NULL for a button with only a caption, and szCaption NULL
// or empty for a button with only an icon.  A normal button is drawn
// with the theme's default button images while it is its window's default.
//
//////////////////////////////////////////////////////////////////////////
void CButton::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, EButtonLook eLook, Ptr<CImageCel> pIcon, const char* szCaption)
{
	Init(pWindow, pTheme->GetButtonImages(eLook), pTheme->GetDefaultButtonImages(eLook), pIcon, pTheme->GetFont(), szCaption);
	SetPadding(pTheme->GetMetric(TM_ButtonPaddingX), pTheme->GetMetric(TM_ButtonPaddingY));
	SetIconGap(pTheme->GetMetric(TM_ButtonIconGap));
}


//////////////////////////////////////////////////////////////////////////
//
// For a button drawn with images that aren't in a theme, like a menu
// bar's.  The default images can be NULL and must have the same tile size
// as the images.
//
//////////////////////////////////////////////////////////////////////////
void CButton::Init(Ptr<CWindow> pWindow, Ptr<CButtonImages> pImages, Ptr<CButtonImages> pDefaultImages, Ptr<CImageCel> pIcon, Ptr<CFont> pFont, const char* szCaption)
{
	PreInit();

	CImageBorderContainer::Init(pWindow, pImages, 8, 2);
	mpNormalImages = pImages;
	mpDefaultImages = pDefaultImages;
	miIconGap = 4;
	mcClicked.Init();
	mpCommand = NULL;
	meState = BS_Normal;
	mbPressed = false;
	mbDefault = false;
	mbShowsPushed = true;
	mbHighlighted = false;
	SetCanGetFocus(true);

	mpContent = OMalloc<CGridContainer>(pWindow);
	mpContent->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpPadded->AddComponent(mpContent);
	mpIcon = OMalloc<CIconComponent>(pWindow, pIcon);
	mpCaption = OMalloc<CLabel>(pWindow, pFont, szCaption == NULL ? "" : szCaption);
	UpdateContent();
	AddInputAction(CIC_ActivateComponent, this, &CButton::ActivateComponent);
	AddInputAction(CIC_ExecuteComponent, this, &CButton::ExecuteComponent);
	AddInputAction(CIC_Space, this, &CButton::KeyClick);
	AddInputAction(CIC_Enter, this, &CButton::KeyClick);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Class(void)
{
	CImageBorderContainer::Class();

	M_Pointer(mpNormalImages);
	M_Pointer(mpDefaultImages);
	M_Pointer(mpContent);
	M_Pointer(mpIcon);
	M_Pointer(mpCaption);
	U_Int32(miIconGap);
	U_Unknown(CComponentEvent<CButton>, mcClicked);
	M_Pointer(mpCommand);
	U_Enum(meState);
	U_Bool(mbPressed);
	U_Bool(mbDefault);
	U_Bool(mbShowsPushed);
	U_Bool(mbHighlighted);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Free(void)
{
	mcClicked.Kill();
	CImageBorderContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetCaption(const char* szCaption)
{
	mpCaption->SetText(szCaption == NULL ? "" : szCaption);
	UpdateContent();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL takes the icon away.
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetIcon(Ptr<CImageCel> pIcon)
{
	mpIcon->SetCel(pIcon);
	UpdateContent();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetIconGap(int32 iIconGap)
{
	miIconGap = iIconGap;
	UpdateContent();

	//What it is laid out from has changed.
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// An icon with a caption sits on the left with the caption aligned left
// beside it.  An icon or a caption on its own is centred.
//
//////////////////////////////////////////////////////////////////////////
void CButton::UpdateContent(void)
{
	bool			bIcon;
	bool			bCaption;
	Ptr<CImageCel>	pIcon;

	pIcon = mpIcon->GetCel();
	bIcon = pIcon.IsNotNull();
	bCaption = HasCaption();

	mpContent->RemoveAllComponents();
	if (bIcon && bCaption)
	{
		//The gap is part of the icon's width with the icon drawn at its left.
		mpIcon->SetDesiredSize(pIcon->GetSubImage()->GetFullWidth() + miIconGap, -1);
		mpIcon->SetCelAlignment(CSH_Left, CSV_Center);
		mpIcon->SetAlignment(CSH_Left, CSV_Center);
		mpCaption->SetAlignment(CSH_Left, CSV_Center);
		mpContent->AddComponent(mpIcon, 0, 0);
		mpContent->AddComponent(mpCaption, 1, 0, 1, 1, 1, 0);
	}
	else if (bIcon)
	{
		mpIcon->SetDesiredSize(-1, -1);
		mpIcon->SetCelAlignment(CSH_Center, CSV_Center);
		mpIcon->SetAlignment(CSH_Center, CSV_Center);
		mpContent->AddComponent(mpIcon, 0, 0);
	}
	else
	{
		mpCaption->SetAlignment(CSH_Center, CSV_Center);
		mpContent->AddComponent(mpCaption, 0, 0);
	}
	UpdateFaceSize();
}


//////////////////////////////////////////////////////////////////////////
//
// A button with neither an icon nor a caption is as big as an empty
// caption, a line high.
//
//////////////////////////////////////////////////////////////////////////
bool CButton::HasCaption(void)
{
	char*	szCaption;

	szCaption = mpCaption->GetText();
	return (szCaption != NULL) && (szCaption[0] != '\0');
}


//////////////////////////////////////////////////////////////////////////
//
// The icon and the caption.  They are inset by the padding so an icon on
// the left starts the padding in from the face's edge.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CButton::GetContentSize(void)
{
	mpContent->SetRequiredSize();
	return mpContent->GetRequiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the button is clicked, with the button:
// pButton->GetClickEvent()->Add(this, &CMyContent::OKClicked).
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CButton>* CButton::GetClickEvent(void)
{
	return &mcClicked;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Click(void)
{
	if (mpCommand.IsNotNull())
	{
		mpCommand->Run();
	}
	mcClicked.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// A tool button for an application command: clicking it runs pCommand
// (if it can run now).  NULL for none.
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetCommand(Ptr<CCommand> pCommand) { mpCommand = pCommand; }
Ptr<CCommand> CButton::GetCommand(void) { return mpCommand; }


//////////////////////////////////////////////////////////////////////////
//
// A focussed button clicks itself on enter, not the default button.
//
//////////////////////////////////////////////////////////////////////////
bool CButton::WantsEnter(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Only the window calls this, see CWindow::SetDefaultButton.
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetDefault(bool bDefault)
{
	if (mbDefault != bDefault)
	{
		mbDefault = bDefault;
		UpdateLook();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a press.
//
//////////////////////////////////////////////////////////////////////////
void CButton::FocusOrMouseOverChanged(void)
{
	if (!HasFocus())
	{
		mbPressed = false;
	}
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
// The press has already given the button focus.
//
//////////////////////////////////////////////////////////////////////////
void CButton::ActivateComponent(int x, int y)
{
	mbPressed = true;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
// The release goes to the button with focus wherever the pointer is.  It
// only clicks if the pointer is still over the button.
//
//////////////////////////////////////////////////////////////////////////
void CButton::ExecuteComponent(int x, int y)
{
	if (mbPressed)
	{
		mbPressed = false;
		UpdateState();
		if (IsMouseOver())
		{
			Click();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Space, or enter, while the button has focus.  Enter also comes here
// when the button is its window's default.
//
//////////////////////////////////////////////////////////////////////////
void CButton::KeyClick(void)
{
	Click();
}


//////////////////////////////////////////////////////////////////////////
//
// Pushed while pressed with the pointer over it, if it shows being pushed,
// then mouse over, or highlighted, then selected while it has focus.
//
//////////////////////////////////////////////////////////////////////////
void CButton::UpdateState(void)
{
	EButtonState	eState;

	if (mbPressed && IsMouseOver() && mbShowsPushed)
	{
		eState = BS_Pushed;
	}
	else if (IsMouseOver() || mbHighlighted)
	{
		eState = BS_MouseOver;
	}
	else if (HasFocus())
	{
		eState = BS_Focussed;
	}
	else
	{
		eState = BS_Normal;
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
void CButton::UpdateLook(void)
{
	//The default images, while the button is the window's default, are drawn instead of its own.
	if (mbDefault && mpDefaultImages)
	{
		SetImages(mpDefaultImages);
	}
	else
	{
		SetImages(mpNormalImages);
	}
	SetBorderState(meState);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CButton::GetCaption(void) { return mpCaption->GetText(); }
Ptr<CImageCel> CButton::GetIcon(void) { return mpIcon->GetCel(); }
Ptr<CIconComponent> CButton::GetIconComponent(void) { return mpIcon; }
Ptr<CLabel> CButton::GetCaptionLabel(void) { return mpCaption; }
void CButton::SetShowsPushed(bool bShowsPushed) { mbShowsPushed = bShowsPushed; UpdateState(); }
bool CButton::IsShowsPushed(void) { return mbShowsPushed; }
void CButton::SetHighlighted(bool bHighlighted) { mbHighlighted = bHighlighted; UpdateState(); }
bool CButton::IsHighlighted(void) { return mbHighlighted; }
EButtonState CButton::GetState(void) { return meState; }
bool CButton::IsPressed(void) { return mbPressed; }
bool CButton::IsDefault(void) { return mbDefault; }

