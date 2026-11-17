/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneViewLib

Codaphela SceneViewLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneViewLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneViewLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/ComponentInput.h"
#include "WindowLib/Window.h"
#include "SceneView.h"


//////////////////////////////////////////////////////////////////////////
//
// pcStatus is the label the draw says what it's drawing on, which whoever
// holds the view holds: NULL for none.
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, CLabel* pcStatus, ESceneViewButtons eButtons)
{
	PreInit();

	mpDraw = OMalloc<CSceneCanvasDraw>(pTheme, pcStatus);
	CCanvas::Init(pWindow, mpDraw);
	meButtons = eButtons;
	msLastPointer.Init(0, 0);
	msPressed.Init(0, 0);
	mbClick = false;
	mcClicked.Init();

	SetAlignment(CSH_Stretch, CSV_Stretch);
	SetCanGetFocus(true);
	AddInputAction(IAF_Movement, NULL, this, &CSceneView::MovementAction);
	AddInputAction(IAF_Wheel, NULL, this, &CSceneView::WheelAction);
	AddInputAction(IAF_KeyDown, "Button 1", this, &CSceneView::PressAction);
	AddInputAction(IAF_KeyUp, "Button 1", this, &CSceneView::ReleaseAction);
	AddInputAction(IAF_KeyDown, "R", this, &CSceneView::RendererAction);
	AddInputAction(IAF_KeyDown, "Home", this, &CSceneView::HomeAction);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::Class(void)
{
	CCanvas::Class();

	M_Pointer(mpDraw);
	U_Enum(meButtons);
	U_2Int32(msLastPointer);
	U_2Int32(msPressed);
	U_Bool(mbClick);
	U_Unknown(CComponentEvent<CSceneView>, mcClicked);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::Free(void)
{
	mcClicked.Kill();
	CCanvas::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSceneCanvasDraw> CSceneView::GetDraw(void) const
{
	return mpDraw;
}


//////////////////////////////////////////////////////////////////////////
//
// Called when the left button is let go where it was pressed:
// pView->GetClickEvent()->Add(this, &CMyPage::Clicked).
//
//////////////////////////////////////////////////////////////////////////
CComponentEvent<CSceneView>* CSceneView::GetClickEvent(void)
{
	return &mcClicked;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the left button last went down, in the view.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CSceneView::GetClick(void) const
{
	return msPressed;
}


//////////////////////////////////////////////////////////////////////////
//
// Once the view is in its window, so its mouse, wheel and keys work
// without clicking it first.
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::FocusView(void)
{
	GetWindow()->GetFocus()->SetFocus(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The left button turns the view only once it's moved further than a
// click may.
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::MovementAction(SComponentInputEvent* psEvent)
{
	int32	iRight;
	int32	iDown;

	psEvent->bPass = true;

	if (mbClick && ((abs(psEvent->x - msPressed.x) > SCENE_VIEW_CLICK_PIXELS) || (abs(psEvent->y - msPressed.y) > SCENE_VIEW_CLICK_PIXELS)))
	{
		mbClick = false;
	}

	iRight = psEvent->x - msLastPointer.x;
	iDown = psEvent->y - msLastPointer.y;
	if ((iRight != 0) || (iDown != 0))
	{
		Drag(iRight, iDown);
	}
	msLastPointer.Init(psEvent->x, psEvent->y);
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer has moved with whatever buttons are down.
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::Drag(int32 iRight, int32 iDown)
{
	CComponentInput*	pcInput;
	bool				bLeft;
	bool				bMiddle;
	bool				bRight;

	pcInput = GetWindow()->GetComponentInput();
	if (pcInput == NULL)
	{
		return;
	}
	bLeft = pcInput->IsSourceDown("Button 1");
	bRight = pcInput->IsSourceDown("Button 2");
	bMiddle = pcInput->IsSourceDown("Button 3");

	if (meButtons == SVB_LeftTurns)
	{
		if (bLeft)
		{
			if (!mbClick)
			{
				mpDraw->Orbit(iRight, iDown);
			}
		}
		else if (bMiddle || bRight)
		{
			mpDraw->Pan(iRight, iDown);
		}
	}
	else if (bMiddle)
	{
		if (pcInput->IsShift())
		{
			mpDraw->Pan(iRight, iDown);
		}
		else
		{
			mpDraw->Orbit(iRight, iDown);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Rolling forward goes closer.
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::WheelAction(SComponentInputEvent* psEvent)
{
	bool	bForward;

	bForward = (psEvent->iDown < 0) || (psEvent->iRight < 0);
	mpDraw->Zoom(bForward ? SCENE_VIEW_ZOOM_FACTOR : (1.0f / SCENE_VIEW_ZOOM_FACTOR));
}


//////////////////////////////////////////////////////////////////////////
//
// The left button going down may be a click or the start of a drag: it
// can only be a click if something is told of clicks.
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::PressAction(SComponentInputEvent* psEvent)
{
	msPressed.Init(psEvent->x, psEvent->y);
	msLastPointer.Init(psEvent->x, psEvent->y);
	mbClick = mcClicked.NumCalls() > 0;
	psEvent->bPass = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Let go without dragging, it's a click where it went down.
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::ReleaseAction(SComponentInputEvent* psEvent)
{
	if (mbClick)
	{
		mbClick = false;
		mcClicked.Call(this);
	}
	psEvent->bPass = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::RendererAction(SComponentInputEvent* psEvent)
{
	mpDraw->SwitchRenderer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSceneView::HomeAction(SComponentInputEvent* psEvent)
{
	mpDraw->ResetView();
}

