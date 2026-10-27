#ifndef __SCROLL_PANE_H__
#define __SCROLL_PANE_H__
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
#include "Container.h"
#include "ScrollBar.h"
#include "WindowTheme.h"


enum EScrollPanePolicy
{
	SPP_AsNeeded,  //The content can be scrolled on the axis and its scroll bar is shown while it doesn't fit.
	SPP_Never,  //The content is fitted to the view on the axis, it is never scrolled on it.
};


//Shows part of one component, its content, and scrolls it in the style of IntelliJ IDEA: the scroll bars are
//translucent thumbs drawn over the content, not beside it, so the view is the whole scroll pane.
//
//The content is laid out at its desired size, or the view's size on an axis where its desired size is -1 or smaller
//than the view, so a word wrapped text area gets the view's width and its own height.  On an axis with a policy of
//never the content always gets the view's size.  The content is moved by the scroll position and only drawn inside
//the scroll pane.
//
//A scroll bar is shown along the bottom or right side while the content is bigger than the view on its axis.  Each
//is inset from the edge by the margin and stops short of the other when both are shown.
//
//The wheel scrolls the scroll pane under the pointer by the wheel step, vertically or, with shift, horizontally.
class CScrollPane : public CContainer
{
CONSTRUCTABLE(CScrollPane);
DESTRUCTABLE(CScrollPane);
protected:
	Ptr<CComponent>		mpContent;
	Ptr<CScrollBar>		mpHorizontal;
	Ptr<CScrollBar>		mpVertical;
	EScrollPanePolicy	meHorizontalPolicy;
	EScrollPanePolicy	meVerticalPolicy;
	SInt32Vec2			msScrollPosition;  //As asked for, it is kept between 0 and the maximum when used.
	SInt32Vec2			msContentSize;  //The size the content was last laid out to.
	SInt32Vec2			msLaidOutScroll;  //The scroll position the content was last laid out at.
	int32				miMargin;  //Between a scroll bar and the edge of the scroll pane.
	int32				miWheelStep;  //How far one notch of the wheel scrolls.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void				Class(void) override;
	void 				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetContent(Ptr<CComponent> pContent);
	Ptr<CComponent>		GetContent(void);
	bool				AddComponent(Ptr<CComponent> pComponent) override;

	void				SetPolicy(EScrollBarOrientation eOrientation, EScrollPanePolicy ePolicy);
	EScrollPanePolicy	GetPolicy(EScrollBarOrientation eOrientation);
	void				SetMargin(int32 iMargin);
	void				SetWheelStep(int32 iWheelStep);
	int32				GetWheelStep(void);

	void				SetScrollPosition(int32 x, int32 y);
	void				SetScrollPosition(EScrollBarOrientation eOrientation, int32 iPosition);
	SInt32Vec2			GetScrollPosition(void);
	SInt32Vec2			GetMaximumScrollPosition(void);
	SInt32Vec2			GetContentSize(void);
	Ptr<CScrollBar>		GetScrollBar(EScrollBarOrientation eOrientation);

	void				SetRequiredSize(void) override;
	void				Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool				ClipsChildren(void) override;
	void				WheelAction(SComponentInputEvent* psEvent);
	bool				Wheel(int iRight, int iDown);
	void				ScrollToShow(CInt32Rect* pcRect) override;
	bool				GetScrollView(CInt32Rect* pcDest) override;

protected:
	int32				GetContentLength(int32 iDesired, int32 iView, EScrollPanePolicy ePolicy);
	void				ClampScrollPosition(void);
	void				LayoutContent(SInt32Vec2 sContentPosition);
};


#endif // __SCROLL_PANE_H__

