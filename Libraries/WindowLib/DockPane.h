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


//Where a pane goes back to when it is docked again: the group it was in (by ID) and its tab's place there, and a pane
//near it (by ID, or DOCK_WELL_NEIGHBOUR or DOCK_AROUND_WELL_NEIGHBOUR) with the side of that pane's node it was on, centre for in its group, and its
//size along that side.  bInWell if it was in the document well.
struct SDockRestore
{
	int32		iGroup;  //0 for none.
	int32		iIndex;
	CChars		szNeighbour;  //Empty for none.
	EDockSide	eSide;
	int32		iSize;  //0 for the default.
	bool		bInWell;

	void	Init(void);
	void	Kill(void);
	void	Copy(SDockRestore* psSource);
	void	SetNeighbour(const char* szNeighbour);
	bool	IsSet(void);
	bool	IsBesideWell(void);
	bool	IsAroundWell(void);
};


//One dockable thing: a component (its content), an ID unique in its manager, a title and an icon.  A tool pane or a
//document pane.  The content is shown by whichever group, flyout or floating window it is in; it is never in two
//places.  When a docked pane leaves its group, to float, to auto hide or to be hidden, it remembers where it was so
//Dock and Show put it back (see CDockRestorer::Restore).
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
	Ptr<CComponent>		mpFocusTarget;  //What has the keys when the pane is activated, NULL for its first focusable component.
	EDockPaneKind		meKind;
	bool				mbClosable;
	EDockPaneState		meState;
	Ptr<CDockGroup>		mpGroup;  //While docked or floating.
	Ptr<CDockManager>	mpManager;  //Told when the title or icon changes.

	//The default place.
	EDockPlaceKind		meDefaultKind;
	EDockSide			meDefaultSide;
	int32				miDefaultSize;
	CChars				mszDefaultPane;

	SDockRestore		msRestore;  //Where it was last docked, to go back to.

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
	void				SetID(const char* szID);
	const char*			GetTitle(void);
	void				SetTitle(const char* szTitle);
	Ptr<CImageCel>		GetIcon(void);
	void				SetIcon(Ptr<CImageCel> pIcon);
	Ptr<CFont>			GetTabFont(void);
	void				SetTabFont(Ptr<CFont> pFont);
	Ptr<CComponent>		GetContent(void);
	void				SetFocusTarget(Ptr<CComponent> pTarget);
	Ptr<CComponent>		GetFocusTarget(void);
	EDockPaneKind		GetKind(void);
	bool				IsDocument(void);
	bool				IsTool(void);
	bool				IsClosable(void);
	void				SetClosable(bool bClosable);
	EDockPaneState		GetState(void);
	void				SetState(EDockPaneState eState);
	Ptr<CDockGroup>		GetGroup(void);
	void				SetGroup(Ptr<CDockGroup> pGroup);
	void				SetManager(Ptr<CDockManager> pManager);
	bool				IsShown(void);

	EDockPlaceKind		GetDefaultKind(void);
	EDockSide			GetDefaultSide(void);
	int32				GetDefaultSize(void);
	const char*			GetDefaultPane(void);

	void				SetRestore(SDockRestore* psRestore);
	SDockRestore*		GetRestore(void);
	void				ClearRestore(void);
	bool				HasRestore(void);

	void				SetFloatRectangle(CInt32Rect* pcRectangle);
	bool				GetFloatRectangle(CInt32Rect* pcDest);

	void				SetAutoHide(EDockSide eSide, int32 iSize);
	EDockSide			GetAutoHideSide(void);
	int32				GetAutoHideSize(void);
	void				SetAutoHideSize(int32 iSize);

protected:
	void				Changed(void);
};


const char*		DockPaneStateToString(EDockPaneState eState);


#endif // __DOCK_PANE_H__

