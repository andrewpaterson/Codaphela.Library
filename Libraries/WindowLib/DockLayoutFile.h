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
#ifndef __DOCK_LAYOUT_FILE_H__
#define __DOCK_LAYOUT_FILE_H__
#include "StandardLib/Array.h"
#include "DockNode.h"


class CDockManager;
class CDockState;
class CXMLWriter;
class CMarkupTag;


//Writes a dock manager's layout as XML: its windows, trees, auto hidden panes and every pane's state and where it goes
//back to.  See the comment at the top of DockLayoutFile.cpp for the file.
class CDockLayoutWriter
{
protected:
	CDockManager*	mpcManager;
	CDockState*		mpcState;  //The manager's.

public:
	void	Init(CDockManager* pcManager);
	void	Kill(void);
	bool	Save(const char* szFileName);
	void	Write(CXMLWriter* pcWriter);

protected:
	void	WriteMainWindow(CXMLWriter* pcWriter);
	void	WriteAutoHide(CXMLWriter* pcWriter, Ptr<CAutoHideStrip> pStrip, EDockSide eSide);
	void	WriteFloatingWindows(CXMLWriter* pcWriter);
	void	WriteNode(CXMLWriter* pcWriter, Ptr<CDockNode> pNode);
	void	WriteSplit(CXMLWriter* pcWriter, Ptr<CDockSplit> pSplit);
	void	WriteGroup(CXMLWriter* pcWriter, Ptr<CDockGroup> pGroup);
	void	WriteWell(CXMLWriter* pcWriter, Ptr<CDocumentWell> pWell);
	void	WriteSize(CXMLWriter* pcWriter, Ptr<CDockNode> pNode);
	void	WritePaneID(CXMLWriter* pcWriter, Ptr<CDockPane> pPane);
	void	WriteRestore(CXMLWriter* pcWriter, Ptr<CDockPane> pPane);
};


//Reads a layout written by CDockLayoutWriter into a dock manager, replacing its layout.  Panes the layout names that
//the manager hasn't are made by its listener (CDockManagerListener::CreatePane).
class CDockLayoutReader
{
protected:
	CDockManager*	mpcManager;
	CDockState*		mpcState;  //The manager's.

public:
	void			Init(CDockManager* pcManager);
	void			Kill(void);
	bool			Load(const char* szFileName);
	bool			Read(CMarkupTag* pcRoot);

protected:
	bool			IsLayout(CMarkupTag* pcRoot);
	void			ReadPaneStates(CMarkupTag* pcPanes, CArray<CDockPane>* paMentioned, CArray<CDockPane>* paHidden);
	Ptr<CDockNode>	ReadMainWindow(CMarkupTag* pcMain, CArray<CDockPane>* paMentioned);
	void			ReadAutoHide(CMarkupTag* pcAutoHide, CArray<CDockPane>* paMentioned);
	void			PlaceMainRoot(Ptr<CDockNode> pRoot);
	void			ReadFloatingWindows(CMarkupTag* pcRoot);
	void			PlaceTheRest(CArray<CDockPane>* paMentioned, CArray<CDockPane>* paHidden);
	Ptr<CDockNode>	ReadFirstNode(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bWell);
	Ptr<CDockNode>	ReadNode(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bInWell);
	Ptr<CDockNode>	ReadSplit(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bInWell);
	Ptr<CDockNode>	ReadGroup(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bInWell);
	Ptr<CDockPane>	ReadGroupPanes(CMarkupTag* pcTag, Ptr<CDockGroup> pGroup, Ptr<CWindow> pWindow);
	Ptr<CDockNode>	ReadWell(CMarkupTag* pcTag);
	Ptr<CDockPane>	ReadPane(CMarkupTag* pcTag);
	void			ReadRestore(CMarkupTag* pcTag, Ptr<CDockPane> pPane);
};


#endif // __DOCK_LAYOUT_FILE_H__

