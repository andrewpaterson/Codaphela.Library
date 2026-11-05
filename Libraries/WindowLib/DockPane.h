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
#ifndef __DOCK_PANE_H__
#define __DOCK_PANE_H__
#include "BaseLib/Chars.h"
#include "SupportLib/ImageCel.h"
#include "SupportLib/Font.h"
#include "SupportLib/Int32Rect.h"
#include "StandardLib/Object.h"
#include "DockNode.h"


enum EDockPaneKind
{
	DPK_Tool,		//Solution Explorer, Build, Find All References.
	DPK_Document,	//An editor's file, a diff.
};


enum EDockPaneState
{
	DPS_Docked,		//In a group in the main window's dock area.
	DPS_Floating,	//In a group in a floating window.
	DPS_AutoHidden,	//A tab on one of the main window's edges.
	DPS_Hidden,		//Closed; it keeps where to go back to.
	DPS_Removed,	//Not in the manager.
};


//Where a pane goes when nothing better is known, see CDockManager::AddPane.  Beside the document well on a side, in
//the group of another pane (by ID), in the document well, or hidden.
enum EDockPlaceKind
{
	DPL_BesideWell,
	DPL_WithPane,
	DPL_InWell,
	DPL_Hidden,
};


struct SDockPlace
{
	EDockPlaceKind	eKind;
	EDockSide		eSide;		//Beside the well.
	int32			iSize;		//Along the side, 0 for DOCK_DEFAULT_SIZE.
	const char*		szPaneID;	//With this pane.

	void	BesideWell(EDockSide eSide, int32 iSize);
	void	WithPane(const char* szPaneID);
	void	InWell(void);
	void	Hidden(void);
};


//One dockable thing: a component (its content), an ID unique in its manager, a title and an icon.  A tool pane or a
//document pane.  The content is shown by whichever group, flyout or floating window it is in; it is never in two
//places.  When a docked pane leaves its group, to float, to auto hide or to be hidden, it remembers where it was so
//Dock and Show put it back (see CDockManager::Restore).
class CDockPane : public CObject
{
CONSTRUCTABLE(CDockPane);
DESTRUCTABLE(CDockPane);
protected:
	CChars				mszID;
	CChars				mszTitle;
	Ptr<CImageCel>		mpIcon;
	Ptr<CFont>			mpTabFont;  //Its tab's text, NULL for the theme's.
	Ptr<CComponent>		mpContent;
	EDockPaneKind		meKind;
	bool				mbClosable;
	EDockPaneState		meState;
	Ptr<CDockGroup>		mpGroup;  //While docked or floating.

	//The default place.
	EDockPlaceKind		meDefaultKind;
	EDockSide			meDefaultSide;
	int32				miDefaultSize;
	CChars				mszDefaultPane;

	//Where it was last docked, to go back to.
	int32				miRestoreGroup;  //The group's ID, 0 for none.
	int32				miRestoreIndex;  //Its tab's index in the group.
	CChars				mszRestoreNeighbour;  //A pane near it, empty for none.
	EDockSide			meRestoreSide;  //Of the neighbour's group it was on, centre for in it.
	int32				miRestoreSize;  //Along the side, 0 for the default.
	bool				mbRestoreInWell;  //It was in the document well.

	//Where it last floated, empty before it has.
	CInt32Rect			mcFloatRectangle;

	//Auto hide.
	EDockSide			meAutoHideSide;
	int32				miAutoHideSize;  //The flyout's width or height.

public:
	void				Init(const char* szID, const char* szTitle, Ptr<CImageCel> pIcon, Ptr<CComponent> pContent, EDockPaneKind eKind, SDockPlace* psDefault);
	void				Class(void) override;
	void				Free(void) override;
	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	const char*			GetID(void);
	const char*			GetTitle(void);
	void				SetTitle(const char* szTitle);
	Ptr<CImageCel>		GetIcon(void);
	void				SetIcon(Ptr<CImageCel> pIcon);
	Ptr<CFont>			GetTabFont(void);
	void				SetTabFont(Ptr<CFont> pFont);
	Ptr<CComponent>		GetContent(void);
	EDockPaneKind		GetKind(void);
	bool				IsDocument(void);
	bool				IsTool(void);
	bool				IsClosable(void);
	void				SetClosable(bool bClosable);
	EDockPaneState		GetState(void);
	void				SetState(EDockPaneState eState);
	Ptr<CDockGroup>		GetGroup(void);
	void				SetGroup(Ptr<CDockGroup> pGroup);
	bool				IsShown(void);

	EDockPlaceKind		GetDefaultKind(void);
	EDockSide			GetDefaultSide(void);
	int32				GetDefaultSize(void);
	const char*			GetDefaultPane(void);

	void				SetRestore(int32 iGroup, int32 iIndex, const char* szNeighbour, EDockSide eSide, int32 iSize, bool bInWell);
	void				ClearRestore(void);
	int32				GetRestoreGroup(void);
	int32				GetRestoreIndex(void);
	const char*			GetRestoreNeighbour(void);
	EDockSide			GetRestoreSide(void);
	int32				GetRestoreSize(void);
	bool				IsRestoreInWell(void);
	bool				HasRestore(void);

	void				SetFloatRectangle(CInt32Rect* pcRectangle);
	bool				GetFloatRectangle(CInt32Rect* pcDest);

	void				SetAutoHide(EDockSide eSide, int32 iSize);
	EDockSide			GetAutoHideSide(void);
	int32				GetAutoHideSize(void);
	void				SetAutoHideSize(int32 iSize);
};


const char*		DockPaneStateToString(EDockPaneState eState);


#endif // __DOCK_PANE_H__

