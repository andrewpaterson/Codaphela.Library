#ifndef __LIST_CONTAINER_H__
#define __LIST_CONTAINER_H__
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
#include "BaseLib/ArrayBit.h"
#include "StandardLib/ListenerCall.h"
#include "SupportLib/ColourARGB32.h"
#include "GridContainer.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "WindowTheme.h"
#include "ListListener.h"


//Rows one below the other that one row at a time can be selected in; what menus, combo boxes and list boxes are made
//of.  The list is a grid and each row is one of its rows, holding one or more components in its columns, so that the
//parts of every row line up (a menu's captions and shortcuts say).  A disabled row, like a separator, can't be selected.
//
//The list is hit as a whole, like a button, so it has the pointer over it and takes focus, not the components in its
//rows.  The selected row is drawn over the selected highlight and the row under the pointer, unless it is selected or
//disabled, over the hover highlight.  Each highlight covers its row across the whole width of the list.
//
//Pressing a row selects it and releasing over the selected row activates it.  Up, down, home, end, page up and page
//down move the selection over the enabled rows, and enter or space activates the selected row.  When the selection
//follows the pointer, as in a menu, moving over a row selects it so pressing, dragging to a row and releasing activates
//that row.
//
//Like a button, a list keeps the size its rows need unless a desired size is set on an axis, so in a scroll pane it is
//its rows' height.
class CListContainer : public CGridContainer
{
CONSTRUCTABLE(CListContainer);
DESTRUCTABLE(CListContainer);
protected:
	Ptr<CClearCanvasDraw>	mpSelectedDraw;
	Ptr<CCanvas>			mpSelectedHighlight;
	Ptr<CClearCanvasDraw>	mpHoverDraw;
	Ptr<CCanvas>			mpHoverHighlight;
	CArrayBit				mabEnabled;  //One for each row.
	CListenerCall			mcListeners;
	int32					miSelected;  //-1 if no row is selected.
	int32					miHover;  //The row under the pointer, -1 if the pointer isn't over a row.
	int32					miPageRows;  //How far page up and page down move the selection.
	bool					mbPressed;  //Pressed on a row and not yet released.
	bool					mbSelectOnHover;

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void			Class(void) override;
	void 			Free(void) override;

	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	int32			AddRow(bool bEnabled = true);
	bool			AddToRow(int32 iRow, Ptr<CComponent> pComponent, int iColumn = 0, int iColumnSpan = 1, int iWidthWeight = 0);
	int32			AddItem(Ptr<CComponent> pItem, bool bEnabled = true);
	bool			RemoveAllComponents(void) override;  //Removes every row.
	int32			NumRows(void);
	void			SetRowEnabled(int32 iRow, bool bEnabled);
	bool			IsRowEnabled(int32 iRow);

	bool			Select(int32 iRow);
	int32			GetSelected(void);
	int32			GetHover(void);
	void			Activate(void);
	int32			FindRowAt(int x, int y);

	void			SetSelectOnHover(bool bSelectOnHover);
	bool			IsSelectOnHover(void);
	void			SetPageRows(int32 iPageRows);
	void			SetHighlightColours(ARGB32 sSelected, ARGB32 sHover);
	void			SetHighlightDraws(Ptr<CCanvasDraw> pSelected, Ptr<CCanvasDraw> pHover);
	Ptr<CCanvas>	GetSelectedHighlight(void);
	Ptr<CCanvas>	GetHoverHighlight(void);

	bool			AddListener(CListListener* pcListener);
	bool			RemoveListener(CListListener* pcListener);

	SInt32Vec2		GetDesiredSize(void) override;
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool			Draw(void) override;
	Ptr<CComponent>	FindComponentAt(int x, int y) override;
	bool			WantsEnter(void) override;

	void			FocusLost(void) override;
	void			MovedOutof(void) override;
	void			MovedOver(int x, int y) override;
	void			CommandAction(SComponentInputEvent* psEvent);
	void			ActivateComponent(int x, int y);
	void			ExecuteComponent(int x, int y);
	void			Up(void);
	void			Down(void);
	void			Home(void);
	void			End(void);
	void			DocumentHome(void);
	void			DocumentEnd(void);
	void			PageUp(void);
	void			PageDown(void);
	void			Space(bool bInsert);
	void			Enter(void);

protected:
	virtual void	RowSelected(int32 iRow);  //Called before the listeners are told, iRow is -1 when the selection is cleared.
	virtual void	RowActivated(int32 iRow);  //Called before the listeners are told.

	void			AddHighlights(void);
	Ptr<CCanvas>	CreateHighlight(Ptr<CClearCanvasDraw> pDraw);
	bool			IsHoverShown(void);
	void			UpdateHighlights(void);
	void			LayoutHighlight(Ptr<CCanvas> pHighlight, int32 iRow);
	int32			FindEnabled(int32 iFrom, int32 iStep);
	void			SelectEnabled(int32 iRow);
	void			SetHover(int32 iRow);
};


#endif // __LIST_CONTAINER_H__

