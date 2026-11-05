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
#include "DockGroup.h"
#include "DockPane.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDockPlace::BesideWell(EDockSide eSide, int32 iSize)
{
	eKind = DPL_BesideWell;
	this->eSide = eSide;
	this->iSize = iSize;
	szPaneID = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDockPlace::WithPane(const char* szPaneID)
{
	eKind = DPL_WithPane;
	eSide = DS_Centre;
	iSize = 0;
	this->szPaneID = szPaneID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDockPlace::InWell(void)
{
	eKind = DPL_InWell;
	eSide = DS_Centre;
	iSize = 0;
	szPaneID = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDockPlace::Hidden(void)
{
	eKind = DPL_Hidden;
	eSide = DS_None;
	iSize = 0;
	szPaneID = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// psDefault can be NULL: a tool goes beside the well on the left, a
// document in the well.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::Init(const char* szID, const char* szTitle, Ptr<CImageCel> pIcon, Ptr<CComponent> pContent, EDockPaneKind eKind, SDockPlace* psDefault)
{
	PreInit();

	mszID.Init(szID);
	mszTitle.Init(szTitle);
	mpIcon = pIcon;
	mpTabFont = NULL;
	mpContent = pContent;
	meKind = eKind;
	mbClosable = true;
	meState = DPS_Removed;
	mpGroup = NULL;

	mszDefaultPane.Init();
	if (psDefault)
	{
		meDefaultKind = psDefault->eKind;
		meDefaultSide = psDefault->eSide;
		miDefaultSize = psDefault->iSize;
		if (psDefault->szPaneID)
		{
			mszDefaultPane.Append(psDefault->szPaneID);
		}
	}
	else if (eKind == DPK_Document)
	{
		meDefaultKind = DPL_InWell;
		meDefaultSide = DS_Centre;
		miDefaultSize = 0;
	}
	else
	{
		meDefaultKind = DPL_BesideWell;
		meDefaultSide = DS_Left;
		miDefaultSize = 0;
	}

	mszRestoreNeighbour.Init();
	ClearRestore();
	mcFloatRectangle.Init(0, 0, 0, 0);
	meAutoHideSide = DS_None;
	miAutoHideSize = 0;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::Class(void)
{
	U_String(mszID);
	U_String(mszTitle);
	M_Pointer(mpIcon);
	M_Pointer(mpTabFont);
	M_Pointer(mpContent);
	U_Enum(meKind);
	U_Bool(mbClosable);
	U_Enum(meState);
	M_Pointer(mpGroup);
	U_Enum(meDefaultKind);
	U_Enum(meDefaultSide);
	U_Int32(miDefaultSize);
	U_String(mszDefaultPane);
	U_Int32(miRestoreGroup);
	U_Int32(miRestoreIndex);
	U_String(mszRestoreNeighbour);
	U_Enum(meRestoreSide);
	U_Int32(miRestoreSize);
	U_Bool(mbRestoreInWell);
	U_Data(CInt32Rect, mcFloatRectangle);
	U_Enum(meAutoHideSide);
	U_Int32(miAutoHideSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::Free(void)
{
	mszID.Kill();
	mszTitle.Kill();
	mszDefaultPane.Kill();
	mszRestoreNeighbour.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Layouts are written as XML by CDockLayoutWriter.
//
//////////////////////////////////////////////////////////////////////////
bool CDockPane::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDockPane::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The group shows the new title on its tab and title bar.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetTitle(const char* szTitle)
{
	mszTitle.Kill();
	mszTitle.Init(szTitle);
	if (mpGroup.IsNotNull())
	{
		mpGroup->PaneChanged(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetIcon(Ptr<CImageCel> pIcon)
{
	mpIcon = pIcon;
	if (mpGroup.IsNotNull())
	{
		mpGroup->PaneChanged(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The tab's text is drawn in pFont, the theme's if NULL, wherever the
// pane goes; an IDE colours its files' tabs by their git status.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetTabFont(Ptr<CFont> pFont)
{
	if (mpTabFont == pFont)
	{
		return;
	}
	mpTabFont = pFont;
	if (mpGroup.IsNotNull())
	{
		mpGroup->PaneChanged(this);
	}
}


Ptr<CFont> CDockPane::GetTabFont(void) { return mpTabFont; }


//////////////////////////////////////////////////////////////////////////
//
// iGroup 0 for no group, szNeighbour NULL for no neighbour.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetRestore(int32 iGroup, int32 iIndex, const char* szNeighbour, EDockSide eSide, int32 iSize, bool bInWell)
{
	miRestoreGroup = iGroup;
	miRestoreIndex = iIndex;
	mszRestoreNeighbour.Kill();
	mszRestoreNeighbour.Init();
	if (szNeighbour)
	{
		mszRestoreNeighbour.Append(szNeighbour);
	}
	meRestoreSide = eSide;
	miRestoreSize = iSize;
	mbRestoreInWell = bInWell;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::ClearRestore(void)
{
	miRestoreGroup = 0;
	miRestoreIndex = 0;
	mszRestoreNeighbour.Kill();
	mszRestoreNeighbour.Init();
	meRestoreSide = DS_None;
	miRestoreSize = 0;
	mbRestoreInWell = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Screen pixels, of the floating window's client area.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetFloatRectangle(CInt32Rect* pcRectangle)
{
	mcFloatRectangle.Init(pcRectangle);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false before the pane has floated.
//
//////////////////////////////////////////////////////////////////////////
bool CDockPane::GetFloatRectangle(CInt32Rect* pcDest)
{
	pcDest->Init(&mcFloatRectangle);
	return (mcFloatRectangle.GetWidth() > 0) && (mcFloatRectangle.GetHeight() > 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetAutoHide(EDockSide eSide, int32 iSize)
{
	meAutoHideSide = eSide;
	miAutoHideSize = iSize;
}


const char*			CDockPane::GetID(void) { return mszID.Text(); }
const char*			CDockPane::GetTitle(void) { return mszTitle.Text(); }
Ptr<CImageCel>		CDockPane::GetIcon(void) { return mpIcon; }
Ptr<CComponent>		CDockPane::GetContent(void) { return mpContent; }
EDockPaneKind		CDockPane::GetKind(void) { return meKind; }
bool				CDockPane::IsDocument(void) { return meKind == DPK_Document; }
bool				CDockPane::IsTool(void) { return meKind == DPK_Tool; }
bool				CDockPane::IsClosable(void) { return mbClosable; }
void				CDockPane::SetClosable(bool bClosable) { mbClosable = bClosable; }
EDockPaneState		CDockPane::GetState(void) { return meState; }
void				CDockPane::SetState(EDockPaneState eState) { meState = eState; }
Ptr<CDockGroup>		CDockPane::GetGroup(void) { return mpGroup; }
void				CDockPane::SetGroup(Ptr<CDockGroup> pGroup) { mpGroup = pGroup; }
bool				CDockPane::IsShown(void) { return (meState == DPS_Docked) || (meState == DPS_Floating) || (meState == DPS_AutoHidden); }
EDockPlaceKind		CDockPane::GetDefaultKind(void) { return meDefaultKind; }
EDockSide			CDockPane::GetDefaultSide(void) { return meDefaultSide; }
int32				CDockPane::GetDefaultSize(void) { return miDefaultSize; }
const char*			CDockPane::GetDefaultPane(void) { return mszDefaultPane.Text(); }
int32				CDockPane::GetRestoreGroup(void) { return miRestoreGroup; }
int32				CDockPane::GetRestoreIndex(void) { return miRestoreIndex; }
const char*			CDockPane::GetRestoreNeighbour(void) { return mszRestoreNeighbour.Text(); }
EDockSide			CDockPane::GetRestoreSide(void) { return meRestoreSide; }
int32				CDockPane::GetRestoreSize(void) { return miRestoreSize; }
bool				CDockPane::IsRestoreInWell(void) { return mbRestoreInWell; }
bool				CDockPane::HasRestore(void) { return (miRestoreGroup != 0) || !mszRestoreNeighbour.Empty() || mbRestoreInWell; }
EDockSide			CDockPane::GetAutoHideSide(void) { return meAutoHideSide; }
int32				CDockPane::GetAutoHideSize(void) { return miAutoHideSize; }
void				CDockPane::SetAutoHideSize(int32 iSize) { miAutoHideSize = iSize; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* DockPaneStateToString(EDockPaneState eState)
{
	switch (eState)
	{
	case DPS_Docked:
		return "Docked";
	case DPS_Floating:
		return "Floating";
	case DPS_AutoHidden:
		return "AutoHidden";
	case DPS_Hidden:
		return "Hidden";
	default:
		return "Removed";
	}
}

