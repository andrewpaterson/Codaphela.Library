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
#ifndef __SPLIT_CONTAINER_H__
#define __SPLIT_CONTAINER_H__
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "ComponentEvent.h"
#include "Container.h"
#include "WindowTheme.h"


#define SPLIT_CONTAINER_WHOLE		1000	//What a share is out of.
#define SPLIT_CONTAINER_MINIMUM		40		//The least the sash leaves either side, while there is room.


class CSplitContainer;


//The bar between a split container's two components.  Dragging it moves the bar, resizing the components on each side
//of it as it goes.  The pointer shows which way it moves.
class CSplitSash : public CCanvas
{
CONSTRUCTABLE(CSplitSash);
DESTRUCTABLE(CSplitSash);
protected:
	CSplitContainer*	mpcSplit;  //Not held: it holds the sash.
	bool				mbDragging;
	int32				miPressed;  //Where it was pressed along the split, in window client space.

public:
	void			Init(Ptr<CWindow> pWindow, CSplitContainer* pcSplit, bool bHorizontal, ARGB32 sColour);
	void			Class(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			ActivateComponent(int x, int y);
	void			ExecuteComponent(int x, int y);
	void			ContDragSelect(int x, int y);
	void			StopDragSelect(int x, int y);
	bool			IsDragging(void);
};


//Two components side by side (horizontal) or one above the other, with a sash between them that is dragged to give
//one more of the room and the other less, as a tool window's tree and the preview beside it.  The first has a share of
//the room (out of SPLIT_CONTAINER_WHOLE, half to start with), kept as the container is resized; the second has the
//rest.  Neither is made smaller than SPLIT_CONTAINER_MINIMUM while there is room.  The share event is called when the
//sash is dragged, for whoever keeps the share.
//
//The second can be hidden (SetSecondShown): then the first has all the room and there is no sash.
//
//Each component fills its side.  (Docking's splits are CDockSplit, which hold dock nodes and know of the document
//well.)
class CSplitContainer : public CContainer
{
CONSTRUCTABLE(CSplitContainer);
DESTRUCTABLE(CSplitContainer);
protected:
	Ptr<CComponent>						mpFirst;
	Ptr<CComponent>						mpSecond;
	Ptr<CSplitSash>						mpSash;
	bool								mbHorizontal;
	bool								mbSecondShown;
	int32								miShare;  //The first's, out of SPLIT_CONTAINER_WHOLE.
	int32								miSashSize;
	int32								miFirstSize;  //Along the split, as last laid out.
	int32								miAlong;  //The room along the split, as last laid out.
	CComponentEvent<CSplitContainer>	mcShareChanged;

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, bool bHorizontal);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetComponents(Ptr<CComponent> pFirst, Ptr<CComponent> pSecond);
	Ptr<CComponent>	GetFirst(void);
	Ptr<CComponent>	GetSecond(void);
	Ptr<CSplitSash>	GetSash(void);
	bool			IsHorizontal(void);
	void			SetSecondShown(bool bShown);
	bool			IsSecondShown(void);
	void			SetShare(int32 iShare);
	int32			GetShare(void);
	int32			GetFirstSize(void);
	void			SashMoved(int32 iDelta);
	CComponentEvent<CSplitContainer>*	GetShareEvent(void);

	bool			AddComponent(Ptr<CComponent> pComponent) override;  //Fails: the components are given with SetComponents.
	void			SetRequiredSize(void) override;
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool			IsChildDrawn(Ptr<CComponent> pChild) override;

protected:
	int32			WorkOutFirstSize(int32 iAlong);
};


#endif // __SPLIT_CONTAINER_H__

