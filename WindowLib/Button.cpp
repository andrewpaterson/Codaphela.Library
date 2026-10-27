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


static EComponentInputCommand	gaeButtonCommands[] = { CIC_ActivateComponent, CIC_ExecuteComponent, CIC_Space, CIC_Enter };


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

	CImageBorderContainer::Init(pWindow, pImages);
	mpNormalImages = pImages;
	mpDefaultImages = pDefaultImages;
	msPadding.Init(8, 2);
	miIconGap = 4;
	mcListeners.Init();
	mcListeners.AddAllowedClass<CButtonListener>();
	meState = BS_Normal;
	mbMouseOver = false;
	mbFocussed = false;
	mbPressed = false;
	mbDefault = false;
	mbShowsPushed = true;
	mbHighlighted = false;
	SetCanGetFocus(true);

	mpPadded = OMalloc<CFillContainer>(pWindow);
	mpPadded->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpFace->AddComponent(mpPadded);
	mpContent = OMalloc<CGridContainer>(pWindow);
	mpContent->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpPadded->AddComponent(mpContent);
	mpIcon = OMalloc<CIconComponent>(pWindow, pIcon);
	mpCaption = OMalloc<CLabel>(pWindow, pFont, szCaption == NULL ? "" : szCaption);
	UpdateContent();
	AddInputActions(gaeButtonCommands, sizeof(gaeButtonCommands) / sizeof(EComponentInputCommand), this, &CButton::CommandAction);

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
	M_Pointer(mpPadded);
	M_Pointer(mpContent);
	M_Pointer(mpIcon);
	M_Pointer(mpCaption);
	U_2Int32(msPadding);
	U_Int32(miIconGap);
	U_Data(CListenerCall, mcListeners);
	U_Enum(meState);
	U_Bool(mbMouseOver);
	U_Bool(mbFocussed);
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
	mcListeners.Kill();
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetIconGap(int32 iIconGap)
{
	miIconGap = iIconGap;
	UpdateContent();
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
//
//////////////////////////////////////////////////////////////////////////
void CButton::SetPadding(int32 iHorizontal, int32 iVertical)
{
	msPadding.Init(iHorizontal, iVertical);
	UpdateFaceSize();
}


//////////////////////////////////////////////////////////////////////////
//
// The face's desired size is what the grid makes room for.  Being
// stretched it still fills its cell.  The content is inset by the padding
// so an icon on the left starts the padding in from the face's edge; the
// face's background still covers the padding.
//
//////////////////////////////////////////////////////////////////////////
void CButton::UpdateFaceSize(void)
{
	SInt32Vec2	sContent;
	SInsets		sInsets;
	int			aiInsets[4];

	aiInsets[0] = msPadding.y;
	aiInsets[1] = msPadding.x;
	aiInsets[2] = msPadding.y;
	aiInsets[3] = msPadding.x;
	sInsets.Init(aiInsets);
	mpPadded->SetInsets(&sInsets);

	mpContent->SetRequiredSize();
	sContent = mpContent->GetRequiredSize();
	mpFace->SetDesiredSize(sContent.x + msPadding.x * 2, sContent.y + msPadding.y * 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::AddListener(CButtonListener* pcListener)
{
	return mcListeners.AddListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CButton::RemoveListener(CButtonListener* pcListener)
{
	return mcListeners.RemoveListener(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Click(void)
{
	mcListeners.CallListeners(&CButtonListener::ButtonClicked, this, NULL);
}


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
void CButton::FocusGained(void)
{
	mbFocussed = true;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::FocusLost(void)
{
	mbFocussed = false;
	mbPressed = false;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::MovedInto(void)
{
	mbMouseOver = true;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::MovedOutof(void)
{
	mbMouseOver = false;
	UpdateState();
}


//////////////////////////////////////////////////////////////////////////
//
// The input actions of gaeButtonCommands.
//
//////////////////////////////////////////////////////////////////////////
void CButton::CommandAction(SComponentInputEvent* psEvent)
{
	switch (psEvent->eCommand)
	{
	case CIC_ActivateComponent:
		ActivateComponent(psEvent->x, psEvent->y);
		break;
	case CIC_ExecuteComponent:
		ExecuteComponent(psEvent->x, psEvent->y);
		break;
	case CIC_Space:
		Space(psEvent->bInsert);
		break;
	case CIC_Enter:
		Enter();
		break;
	default:
		break;
	}
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
		if (mbMouseOver)
		{
			Click();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Space(bool bInsert)
{
	Click();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CButton::Enter(void)
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

	if (mbPressed && mbMouseOver && mbShowsPushed)
	{
		eState = BS_Pushed;
	}
	else if (mbMouseOver || mbHighlighted)
	{
		eState = BS_MouseOver;
	}
	else if (mbFocussed)
	{
		eState = BS_Selected;
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

