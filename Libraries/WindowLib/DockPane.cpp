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
#include "DockManager.h"
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
// Nowhere: no group, no neighbour.
//
//////////////////////////////////////////////////////////////////////////
void SDockRestore::Init(void)
{
	iGroup = 0;
	iIndex = 0;
	szNeighbour.Init();
	eSide = DS_None;
	iSize = 0;
	bInWell = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SDockRestore::Kill(void)
{
	szNeighbour.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The restore must be Init'ed.
//
//////////////////////////////////////////////////////////////////////////
void SDockRestore::Copy(SDockRestore* psSource)
{
	iGroup = psSource->iGroup;
	iIndex = psSource->iIndex;
	SetNeighbour(psSource->szNeighbour.Text());
	eSide = psSource->eSide;
	iSize = psSource->iSize;
	bInWell = psSource->bInWell;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for none.
//
//////////////////////////////////////////////////////////////////////////
void SDockRestore::SetNeighbour(const char* szNew)
{
	szNeighbour.Kill();
	szNeighbour.Init();
	if (szNew)
	{
		szNeighbour.Append(szNew);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SDockRestore::IsSet(void)
{
	return (iGroup != 0) || !szNeighbour.Empty() || bInWell;
}


//////////////////////////////////////////////////////////////////////////
//
// Beside the document well on a side.
//
//////////////////////////////////////////////////////////////////////////
bool SDockRestore::IsBesideWell(void)
{
	return szNeighbour.Equals(DOCK_WELL_NEIGHBOUR) && (eSide != DS_None) && (eSide != DS_Centre);
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
	mpFocusTarget = NULL;
	meKind = eKind;
	mbClosable = true;
	meState = DPS_Removed;
	mpGroup = NULL;
	mpManager = NULL;

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

	msRestore.Init();
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
	M_Pointer(mpFocusTarget);
	U_Enum(meKind);
	U_Bool(mbClosable);
	U_Enum(meState);
	M_Pointer(mpGroup);
	M_Pointer(mpManager);
	U_Enum(meDefaultKind);
	U_Enum(meDefaultSide);
	U_Int32(miDefaultSize);
	U_String(mszDefaultPane);
	U_Int32(msRestore.iGroup);
	U_Int32(msRestore.iIndex);
	U_String(msRestore.szNeighbour);
	U_Enum(msRestore.eSide);
	U_Int32(msRestore.iSize);
	U_Bool(msRestore.bInWell);
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
	msRestore.Kill();
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
// Everything showing the pane shows its new title: its tab and title
// bar, its auto hide tab and flyout, its floating window's title.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetTitle(const char* szTitle)
{
	mszTitle.Kill();
	mszTitle.Init(szTitle);
	Changed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetIcon(Ptr<CImageCel> pIcon)
{
	mpIcon = pIcon;
	Changed();
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
	Changed();
}


Ptr<CFont> CDockPane::GetTabFont(void) { return mpTabFont; }


//////////////////////////////////////////////////////////////////////////
//
// A copy of psRestore is kept.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetRestore(SDockRestore* psRestore)
{
	msRestore.Copy(psRestore);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::ClearRestore(void)
{
	msRestore.Kill();
	msRestore.Init();
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
Ptr<CComponent>		CDockPane::GetFocusTarget(void) { return mpFocusTarget; }


//////////////////////////////////////////////////////////////////////////
//
// The component in the content that has the keys when the pane is
// activated or its tab clicked, an editor say, rather than the first one
// that can (a document's toolbar).
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::SetFocusTarget(Ptr<CComponent> pTarget)
{
	mpFocusTarget = pTarget;
}
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
bool				CDockPane::HasRestore(void) { return msRestore.IsSet(); }
SDockRestore*		CDockPane::GetRestore(void) { return &msRestore; }
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


//////////////////////////////////////////////////////////////////////////
//
// Its title, icon or tab font changed.
//
//////////////////////////////////////////////////////////////////////////
void CDockPane::Changed(void)
{
	if (mpManager.IsNotNull())
	{
		mpManager->PaneChanged(this);
	}
	else if (mpGroup.IsNotNull())
	{
		mpGroup->PaneChanged(this);
	}
}


void CDockPane::SetManager(Ptr<CDockManager> pManager) { mpManager = pManager; }

