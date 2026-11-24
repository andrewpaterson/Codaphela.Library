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
#ifndef __SASH_H__
#define __SASH_H__
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "ComponentEvent.h"


class CSash;
typedef CComponentValueEvent<CSash, int32>	CSashMovedEvent;  //How far the sash was dragged along.
typedef CComponentEvent<CSash>				CSashEvent;


//A bar between two things that is dragged to give one more room and the other less: a split's, docking's, a tree
//table's between two columns.  The pointer shows which way it moves.  As it is dragged its moved event says how far
//the pointer has come since it was pressed or last moved it, and whoever laid it out moves it; its released event says
//the drag has ended.
class CSash : public CCanvas
{
CONSTRUCTABLE(CSash);
DESTRUCTABLE(CSash);
protected:
	bool				mbHorizontal;  //Upright, moved left and right.
	bool				mbDragging;
	int32				miPressed;  //Where it was pressed along, in window client space.
	CSashMovedEvent		mcMoved;
	CSashEvent			mcReleased;

public:
	void				Init(Ptr<CWindow> pWindow, bool bHorizontal, ARGB32 sColour);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetHorizontal(bool bHorizontal);
	bool				IsHorizontal(void) const;
	bool				IsDragging(void) const;
	CSashMovedEvent*	GetMovedEvent(void);
	CSashEvent*			GetReleasedEvent(void);

	void				ActivateComponent(int x, int y);
	void				ExecuteComponent(int x, int y);
	void				ContDragSelect(int x, int y);
	void				StopDragSelect(int x, int y);
};


#endif // __SASH_H__

