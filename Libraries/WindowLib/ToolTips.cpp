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
#include "Window.h"
#include "Popup.h"
#include "ToolTips.h"


//////////////////////////////////////////////////////////////////////////
//
// The tips look as pTheme says, in its font.
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	mpWindow = pWindow;
	mpTheme = pTheme;
	mpLabel = OMalloc<CLabel>(pWindow, pTheme->GetFont(), "");
	mpTipped = NULL;
	miRestTime = 0;
	miDelay = TOOL_TIPS_DELAY;
	mbShown = false;
	mbDismissed = false;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::Class(void)
{
	M_Pointer(mpWindow);
	M_Pointer(mpTheme);
	M_Pointer(mpLabel);
	M_Pointer(mpTipped);
	U_Int64(miRestTime);
	U_Int32(miDelay);
	U_Bool(mbShown);
	U_Bool(mbDismissed);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CToolTips::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CToolTips::Load(CObjectReader* pcFile)
{
	return false;
}


void CToolTips::SetDelay(int32 iMilliseconds) { miDelay = (iMilliseconds > 0) ? iMilliseconds : 0; }
int32 CToolTips::GetDelay(void) { return miDelay; }


//////////////////////////////////////////////////////////////////////////
//
// The pointer moved, and is over pOver (NULL for nothing).  Onto another
// component with a tip the wait for it starts again; still on the same
// one, waiting, it starts again too, as the pointer hasn't rested.
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::PointerMoved(Ptr<CComponent> pOver, int64 iNow)
{
	Ptr<CComponent>	pTipped;

	pTipped = FindTipped(pOver);
	if (pTipped != mpTipped)
	{
		Hide();
		mpTipped = pTipped;
		mbDismissed = false;
		miRestTime = iNow;
	}
	else if (!mbShown)
	{
		miRestTime = iNow;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer left the window.
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::PointerLeft(void)
{
	Hide();
	mpTipped = NULL;
	mbDismissed = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A press, key or the wheel: the tip goes, and the component under the
// pointer shows no other until the pointer moves onto another.
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::Dismiss(void)
{
	Hide();
	mbDismissed = true;
}


//////////////////////////////////////////////////////////////////////////
//
// pComponent is leaving the window: neither it nor anything in it is
// tipped any more.
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::Forget(Ptr<CComponent> pComponent)
{
	if (mpTipped.IsNotNull() && mpTipped->IsWithin(pComponent))
	{
		Hide();
		mpTipped = NULL;
		mbDismissed = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How long until the tip is due, -1 if none is waited for.
//
//////////////////////////////////////////////////////////////////////////
int64 CToolTips::GetWait(int64 iNow)
{
	int64	iWait;

	if (mpTipped.IsNull() || mbShown || mbDismissed)
	{
		return -1;
	}
	iWait = miRestTime + miDelay - iNow;
	return (iWait > 0) ? iWait : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the tip if the pointer has rested long enough: under the
// pointer, clear of its shape, as the popup's layout keeps it in the
// window.  True if it showed.
//
//////////////////////////////////////////////////////////////////////////
bool CToolTips::ShowIfDue(int64 iNow)
{
	CChars			szText;
	SInt32Vec2		sPosition;
	Ptr<CPopup>		pPopup;
	SInsets			sInsets;
	int32			iPaddingX;
	int32			iPaddingY;

	if (GetWait(iNow) != 0)
	{
		return false;
	}

	szText.Init();
	if (!mpTipped->GetToolTip(&szText))
	{
		//Its tip was taken away while the pointer rested on it.
		szText.Kill();
		mbDismissed = true;
		return false;
	}
	mpLabel->SetText(szText.Text());
	szText.Kill();

	sPosition = mpWindow->GetFocus()->GetPointer();
	sPosition.y += mpTheme->GetMetric(TM_ToolTipOffset);
	pPopup = mpWindow->OpenPopup(mpLabel, sPosition, NULL);
	pPopup->SetPointerPasses(true);
	iPaddingX = mpTheme->GetMetric(TM_ToolTipPaddingX);
	iPaddingY = mpTheme->GetMetric(TM_ToolTipPaddingY);
	sInsets.Init(iPaddingY + 1, iPaddingX + 1, iPaddingY + 1, iPaddingX + 1);  //The border is a pixel inside the insets.
	pPopup->SetLook(mpTheme->GetColour(TC_ToolTip), mpTheme->GetColour(TC_ToolTipBorder), &sInsets);
	mbShown = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CToolTips::Hide(void)
{
	if (mbShown)
	{
		mbShown = false;
		mpWindow->ClosePopup(mpLabel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pOver if it has a tip, else its nearest parent with one, else NULL.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CToolTips::FindTipped(Ptr<CComponent> pOver)
{
	Ptr<CComponent>	pComponent;
	CChars			szText;
	bool			bTip;

	szText.Init();
	pComponent = pOver;
	while (pComponent.IsNotNull())
	{
		bTip = pComponent->GetToolTip(&szText);
		if (bTip)
		{
			break;
		}
		pComponent = pComponent->GetParent();
	}
	szText.Kill();
	return pComponent;
}


bool CToolTips::IsShown(void) { return mbShown; }
char* CToolTips::GetText(void) { return mbShown ? mpLabel->GetText() : NULL; }
Ptr<CComponent> CToolTips::GetTipped(void) { return mpTipped; }

