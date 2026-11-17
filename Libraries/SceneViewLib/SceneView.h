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
#ifndef __SCENE_VIEW_H__
#define __SCENE_VIEW_H__
#include "WindowLib/Canvas.h"
#include "WindowLib/ComponentEvent.h"
#include "SceneCanvasDraw.h"


#define SCENE_VIEW_ZOOM_FACTOR		1.1f	//For each roll of the wheel.
#define SCENE_VIEW_CLICK_PIXELS		4		//How far the pointer may move between pressing and letting go of the left button for a click.


//Which buttons move the view.
enum ESceneViewButtons
{
	SVB_LeftTurns,		//The left button's drag turns the view and the middle or right button's pans.
	SVB_MiddleTurns,	//The middle button's drag turns the view, and with Shift pans, as Blender's: the left button is left for what's being worked on.
};


//A canvas showing a scene (its draw is a CSceneCanvasDraw) with the view's mouse and keys: a drag turns or pans the view
//(by meButtons), the wheel zooms, R switches between Vulkan and the rasteriser and Home puts the view back.  The left
//button pressed and let go without moving further than SCENE_VIEW_CLICK_PIXELS is a click, which its click event tells of
//(GetClick is where); with SVB_LeftTurns anything further is a drag.  What else a page wants of its keys it adds
//(AddInputAction).
class CSceneView : public CCanvas
{
CONSTRUCTABLE(CSceneView);
DESTRUCTABLE(CSceneView);
protected:
	Ptr<CSceneCanvasDraw>		mpDraw;
	ESceneViewButtons			meButtons;
	SInt32Vec2					msLastPointer;  //In the view, at the last movement over it.
	SInt32Vec2					msPressed;  //Where the left button went down.
	bool						mbClick;  //The left button is down and hasn't moved far enough to be a drag.
	CComponentEvent<CSceneView>	mcClicked;

public:
	void							Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, CLabel* pcStatus, ESceneViewButtons eButtons);
	void							Class(void) override;
	void							Free(void) override;

	Ptr<CSceneCanvasDraw>			GetDraw(void) const;
	CComponentEvent<CSceneView>*	GetClickEvent(void);
	SInt32Vec2						GetClick(void) const;
	void							FocusView(void);

	void							MovementAction(SComponentInputEvent* psEvent);
	void							WheelAction(SComponentInputEvent* psEvent);
	void							PressAction(SComponentInputEvent* psEvent);
	void							ReleaseAction(SComponentInputEvent* psEvent);
	void							RendererAction(SComponentInputEvent* psEvent);
	void							HomeAction(SComponentInputEvent* psEvent);

protected:
	void							Drag(int32 iRight, int32 iDown);
};


#endif // __SCENE_VIEW_H__

