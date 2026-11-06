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
#include "BaseLib/XMLWriter.h"
#include "BaseLib/XMLFile.h"
#include "BaseLib/MarkupTag.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/Logger.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "Window.h"
#include "DockArea.h"
#include "DockSplit.h"
#include "DocumentWell.h"
#include "DockFloatingWindow.h"
#include "AutoHide.h"
#include "DockManager.h"
#include "DockLayoutFile.h"


#define DOCK_LAYOUT_VERSION		1


//The layout file, as Visual Studio keeps a project's user settings beside it (a .vcxproj.user):
//
//  <DockLayout Version="1" NextGroup="6">
//    <MainWindow X="40" Y="30" Width="1600" Height="1000">
//      <Split Direction="Horizontal">
//        <Group ID="1" Kind="Tool" Size="300" Selected="SolutionExplorer">
//          <Pane ID="SolutionExplorer"/>
//        </Group>
//        <DocumentWell>
//          <Group ID="2" Kind="Document" Selected="Document:Main.cpp">
//            <Pane ID="Document:Main.cpp"/>
//          </Group>
//        </DocumentWell>
//      </Split>
//      <AutoHide Side="Right">
//        <Pane ID="Claude"/>
//      </AutoHide>
//    </MainWindow>
//    <FloatingWindow X="1700" Y="100" Width="400" Height="600">
//      <Group ID="5" Kind="Tool" Selected="Registers">
//        <Pane ID="Registers"/>
//      </Group>
//    </FloatingWindow>
//    <Panes>
//      <Pane ID="Output" State="Hidden" RestoreGroup="1" RestoreIndex="1" RestoreNeighbour="SolutionExplorer" RestoreSide="Centre"/>
//    </Panes>
//  </DockLayout>
//
//A node's Size is its dock size along its parent split, left out when 0.  <Panes> has every pane's state and where it
//goes back to, where it last floated and its auto hide edge and size.  Positions are screen pixels of client areas.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void WriteRectangle(CXMLWriter* pcWriter, CInt32Rect* pcRect, const char* szPrefix)
{
	CChars	sz;

	sz.Init(szPrefix);
	sz.Append("X");
	pcWriter->Attribute(sz.Text(), (int32)pcRect->GetLeft());
	sz.Kill();
	sz.Init(szPrefix);
	sz.Append("Y");
	pcWriter->Attribute(sz.Text(), (int32)pcRect->GetTop());
	sz.Kill();
	sz.Init(szPrefix);
	sz.Append("Width");
	pcWriter->Attribute(sz.Text(), (int32)pcRect->GetWidth());
	sz.Kill();
	sz.Init(szPrefix);
	sz.Append("Height");
	pcWriter->Attribute(sz.Text(), (int32)pcRect->GetHeight());
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if any part is missing.
//
//////////////////////////////////////////////////////////////////////////
static bool ReadRectangle(CMarkupTag* pcTag, CInt32Rect* pcRect, const char* szPrefix)
{
	CChars	sz;
	int32	aiValues[4];
	int		i;
	bool	bResult;
	static const char*	aszNames[4] = { "X", "Y", "Width", "Height" };

	bResult = true;
	for (i = 0; i < 4; i++)
	{
		sz.Init(szPrefix);
		sz.Append(aszNames[i]);
		aiValues[i] = 0;
		bResult &= pcTag->GetInt32Attribute(sz.Text(), &aiValues[i]);
		sz.Kill();
	}
	pcRect->Init(aiValues[0], aiValues[1], aiValues[0] + aiValues[2], aiValues[1] + aiValues[3]);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::Init(CDockManager* pcManager)
{
	mpcManager = pcManager;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::Kill(void)
{
	mpcManager = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the layout to szFileName.
//
//////////////////////////////////////////////////////////////////////////
bool CDockLayoutWriter::Save(const char* szFileName)
{
	CXMLWriter	cWriter;
	bool		bResult;

	cWriter.Init();
	Write(&cWriter);
	bResult = cWriter.Write(szFileName);
	cWriter.Kill();
	if (!bResult)
	{
		gcLogger.Error2(__METHOD__, " Could not write the layout [", szFileName, "].", NULL);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The whole layout, see the comment at the top of this file.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::Write(CXMLWriter* pcWriter)
{
	size	i;

	pcWriter->Begin("DockLayout");
	pcWriter->Attribute("Version", (int32)DOCK_LAYOUT_VERSION);
	pcWriter->Attribute("NextGroup", (int32)mpcManager->miNextGroupID);

	WriteMainWindow(pcWriter);
	WriteFloatingWindows(pcWriter);

	pcWriter->Begin("Panes");
	for (i = 0; i < mpcManager->maPanes.Size(); i++)
	{
		WriteRestore(pcWriter, mpcManager->maPanes.Get(i));
	}
	pcWriter->End();

	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// <MainWindow>: its rectangle, its tree and its auto hide strips.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteMainWindow(CXMLWriter* pcWriter)
{
	CInt32Rect	cRect;
	EDockSide	eSide;

	pcWriter->Begin("MainWindow");
	mpcManager->mpWindow->GetScreenRectangle(&cRect);
	WriteRectangle(pcWriter, &cRect, "");
	if (mpcManager->mpArea->GetRoot().IsNotNull())
	{
		WriteNode(pcWriter, mpcManager->mpArea->GetRoot());
	}
	for (eSide = DS_Left; eSide <= DS_Bottom; eSide = (EDockSide)(eSide + 1))
	{
		WriteAutoHide(pcWriter, mpcManager->mpArea->GetStrip(eSide), eSide);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// <AutoHide>: the panes on a strip, in order, if it has any.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteAutoHide(CXMLWriter* pcWriter, Ptr<CAutoHideStrip> pStrip, EDockSide eSide)
{
	size	i;

	if (!pStrip->HasTabs())
	{
		return;
	}
	pcWriter->Begin("AutoHide");
	pcWriter->Attribute("Side", DockSideToString(eSide));
	for (i = 0; i < pStrip->NumTabs(); i++)
	{
		WritePaneID(pcWriter, pStrip->GetTab(i)->GetPane());
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// <FloatingWindow>: each floating window with something in it, with
// its rectangle and its tree.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteFloatingWindows(CXMLWriter* pcWriter)
{
	size						i;
	Ptr<CDockFloatingWindow>	pFloating;
	CInt32Rect					cRect;

	for (i = 0; i < mpcManager->maFloating.Size(); i++)
	{
		pFloating = mpcManager->maFloating.Get(i);
		if (pFloating->GetRoot().IsNull())
		{
			continue;
		}
		pcWriter->Begin("FloatingWindow");
		pFloating->GetRectangle(&cRect);
		WriteRectangle(pcWriter, &cRect, "");
		WriteNode(pcWriter, pFloating->GetRoot());
		pcWriter->End();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteNode(CXMLWriter* pcWriter, Ptr<CDockNode> pNode)
{
	switch (pNode->GetKind())
	{
	case DNK_Split:
		WriteSplit(pcWriter, pNode->AsSplit());
		break;
	case DNK_Group:
		WriteGroup(pcWriter, pNode->AsGroup());
		break;
	case DNK_DocumentWell:
		WriteWell(pcWriter, pNode->AsWell());
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteSplit(CXMLWriter* pcWriter, Ptr<CDockSplit> pSplit)
{
	size	i;

	pcWriter->Begin("Split");
	pcWriter->Attribute("Direction", pSplit->IsHorizontal() ? "Horizontal" : "Vertical");
	WriteSize(pcWriter, pSplit);
	for (i = 0; i < pSplit->NumNodes(); i++)
	{
		WriteNode(pcWriter, pSplit->GetNode(i));
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteGroup(CXMLWriter* pcWriter, Ptr<CDockGroup> pGroup)
{
	Ptr<CDockPane>	pSelected;
	size			i;

	pcWriter->Begin("Group");
	pcWriter->Attribute("ID", (int32)pGroup->GetID());
	pcWriter->Attribute("Kind", pGroup->IsDocumentGroup() ? "Document" : "Tool");
	WriteSize(pcWriter, pGroup);
	pSelected = pGroup->GetSelectedPane();
	if (pSelected.IsNotNull())
	{
		pcWriter->Attribute("Selected", pSelected->GetID());
	}
	for (i = 0; i < pGroup->NumPanes(); i++)
	{
		WritePaneID(pcWriter, pGroup->GetPane(i));
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteWell(CXMLWriter* pcWriter, Ptr<CDocumentWell> pWell)
{
	pcWriter->Begin("DocumentWell");
	WriteSize(pcWriter, pWell);
	if (pWell->GetRoot().IsNotNull())
	{
		WriteNode(pcWriter, pWell->GetRoot());
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// A node's size along its split, if it has one.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteSize(CXMLWriter* pcWriter, Ptr<CDockNode> pNode)
{
	if (pNode->GetDockSize() > 0)
	{
		pcWriter->Attribute("Size", (int32)pNode->GetDockSize());
	}
}


//////////////////////////////////////////////////////////////////////////
//
// <Pane ID="..."/>, a pane named in a group or an auto hide strip.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WritePaneID(CXMLWriter* pcWriter, Ptr<CDockPane> pPane)
{
	pcWriter->Begin("Pane");
	pcWriter->Attribute("ID", pPane->GetID());
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// A pane's state, where it goes back to, where it floated and where it
// auto hides.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutWriter::WriteRestore(CXMLWriter* pcWriter, Ptr<CDockPane> pPane)
{
	CInt32Rect		cRect;
	SDockRestore*	psRestore;

	pcWriter->Begin("Pane");
	pcWriter->Attribute("ID", pPane->GetID());
	pcWriter->Attribute("State", DockPaneStateToString(pPane->GetState()));
	psRestore = pPane->GetRestore();
	if (psRestore->iGroup != 0)
	{
		pcWriter->Attribute("RestoreGroup", psRestore->iGroup);
		pcWriter->Attribute("RestoreIndex", psRestore->iIndex);
	}
	if (!psRestore->szNeighbour.Empty())
	{
		pcWriter->Attribute("RestoreNeighbour", psRestore->szNeighbour.Text());
	}
	if (psRestore->eSide != DS_None)
	{
		pcWriter->Attribute("RestoreSide", DockSideToString(psRestore->eSide));
	}
	if (psRestore->iSize > 0)
	{
		pcWriter->Attribute("RestoreSize", psRestore->iSize);
	}
	if (psRestore->bInWell)
	{
		pcWriter->Attribute("RestoreInWell", true);
	}
	if (pPane->GetFloatRectangle(&cRect))
	{
		WriteRectangle(pcWriter, &cRect, "Float");
	}
	if (pPane->GetAutoHideSide() != DS_None)
	{
		pcWriter->Attribute("AutoHideSide", DockSideToString(pPane->GetAutoHideSide()));
		pcWriter->Attribute("AutoHideSize", (int32)pPane->GetAutoHideSize());
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::Init(CDockManager* pcManager)
{
	mpcManager = pcManager;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::Kill(void)
{
	mpcManager = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the layout's file and replaces the manager's layout with it.
// Returns false, changing nothing, if there is no file or it doesn't
// read.
//
//////////////////////////////////////////////////////////////////////////
bool CDockLayoutReader::Load(const char* szFileName)
{
	CFileUtil	cFileUtil;
	CXMLFile	cFile;
	CChars		szName;
	CChars		szDirectory;
	bool		bResult;
	CMarkupTag*	pcRoot;

	if (!cFileUtil.Exists(szFileName))
	{
		return false;
	}
	szName.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(szFileName, &szName, &szDirectory);
	if (szDirectory.Empty())
	{
		szDirectory.Set(".");
	}
	cFile.Init();
	bResult = cFile.Read(szName.Text(), szDirectory.Text());
	pcRoot = NULL;
	if (bResult && (cFile.mcMarkup.mpcDoc != NULL))
	{
		pcRoot = cFile.mcMarkup.mpcDoc->GetRootTag();
	}
	if (pcRoot)
	{
		bResult = Read(pcRoot);
	}
	else
	{
		bResult = false;
		gcLogger.Error2(__METHOD__, " Could not read the layout [", szFileName, "].", NULL);
	}
	cFile.Kill();
	szDirectory.Kill();
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the layout with the one in pcRoot, a <DockLayout>.  Panes the
// layout doesn't mention go to their default places.  Returns false, and
// leaves the layout as it was, for a layout that isn't one or is of a
// later version.
//
//////////////////////////////////////////////////////////////////////////
bool CDockLayoutReader::Read(CMarkupTag* pcRoot)
{
	CArray<CDockPane>	aMentioned;
	CArray<CDockPane>	aHidden;
	Ptr<CDockNode>		pRoot;
	int32				iNextGroup;

	if (!IsLayout(pcRoot))
	{
		return false;
	}
	mpcManager->ClearLayout();
	iNextGroup = 1;
	pcRoot->GetInt32Attribute("NextGroup", &iNextGroup);
	//No group is left after clearing: the layout's own IDs are used.
	mpcManager->miNextGroupID = (iNextGroup > 0) ? iNextGroup : 1;
	mpcManager->mpArea->SetRoot(NULL);

	aMentioned.Init();
	aHidden.Init();
	ReadPaneStates(pcRoot->GetTag("Panes", (STagIterator*)NULL), &aMentioned, &aHidden);
	pRoot = ReadMainWindow(pcRoot->GetTag("MainWindow", (STagIterator*)NULL), &aMentioned);
	PlaceMainRoot(pRoot);
	ReadFloatingWindows(pcRoot);
	PlaceTheRest(&aMentioned, &aHidden);
	aHidden.Kill();
	aMentioned.Kill();
	mpcManager->LayoutChanged();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A <DockLayout> of a version that is known.
//
//////////////////////////////////////////////////////////////////////////
bool CDockLayoutReader::IsLayout(CMarkupTag* pcRoot)
{
	int32	iVersion;

	if (!pcRoot->Is("DockLayout"))
	{
		return gcLogger.Error2(__METHOD__, " The layout's root isn't <DockLayout>.", NULL);
	}
	iVersion = 0;
	pcRoot->GetInt32Attribute("Version", &iVersion);
	if ((iVersion <= 0) || (iVersion > DOCK_LAYOUT_VERSION))
	{
		return gcLogger.Error2(__METHOD__, " The layout's version isn't known.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// <Panes>: every pane's state and where it goes back to, read first so
// the panes made for the layout know them.  paMentioned gets every pane
// named, paHidden those that stay hidden.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::ReadPaneStates(CMarkupTag* pcPanes, CArray<CDockPane>* paMentioned, CArray<CDockPane>* paHidden)
{
	STagIterator	sIter;
	CMarkupTag*		pcPane;
	Ptr<CDockPane>	pPane;
	char*			szState;

	if (pcPanes == NULL)
	{
		return;
	}
	pcPane = pcPanes->GetTag("Pane", &sIter);
	while (pcPane)
	{
		pPane = ReadPane(pcPane);
		if (pPane.IsNotNull())
		{
			ReadRestore(pcPane, pPane);
			paMentioned->Add(pPane);
			szState = pcPane->GetStringAttribute("State");
			if ((szState != NULL) && (StringCompare(szState, "Hidden") == 0))
			{
				paHidden->Add(pPane);
			}
		}
		pcPane = pcPanes->GetNextTag(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// <MainWindow>: where the window was, its auto hidden panes and its tree,
// which is returned.  Auto hidden panes are read first so a group naming
// one leaves it out.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockLayoutReader::ReadMainWindow(CMarkupTag* pcMain, CArray<CDockPane>* paMentioned)
{
	STagIterator	sIter;
	CMarkupTag*		pcTag;
	CInt32Rect		cRect;

	mpcManager->mbLayoutWindow = false;
	if (pcMain == NULL)
	{
		return NULL;
	}
	mpcManager->mbLayoutWindow = ReadRectangle(pcMain, &mpcManager->mcLayoutWindow, "");
	if (mpcManager->mbLayoutWindow)
	{
		//The window goes where it was: read as the application starts, before its window is shown, or later, as a
		//solution is opened in a window already shown.
		cRect.Init(&mpcManager->mcLayoutWindow);
		mpcManager->mcMover.KeepOnScreen(&cRect);
		mpcManager->mpWindow->SetScreenRectangle(&cRect);
	}

	pcTag = pcMain->GetTag("AutoHide", &sIter);
	while (pcTag)
	{
		ReadAutoHide(pcTag, paMentioned);
		pcTag = pcMain->GetNextTag(&sIter);
	}
	mpcManager->mpArea->StripsChanged();

	return ReadFirstNode(pcMain, mpcManager->mpWindow, true);
}


//////////////////////////////////////////////////////////////////////////
//
// <AutoHide>: tools auto hidden on one edge.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::ReadAutoHide(CMarkupTag* pcAutoHide, CArray<CDockPane>* paMentioned)
{
	STagIterator		sIter;
	CMarkupTag*			pcPane;
	Ptr<CDockPane>		pPane;
	EDockSide			eSide;
	Ptr<CAutoHideStrip>	pStrip;

	eSide = StringToDockSide(pcAutoHide->GetStringAttribute("Side"));
	pStrip = mpcManager->mpArea->GetStrip(eSide);
	if (pStrip.IsNull())
	{
		return;
	}
	pcPane = pcAutoHide->GetTag("Pane", &sIter);
	while (pcPane)
	{
		pPane = ReadPane(pcPane);
		if (pPane.IsNotNull() && pPane->IsTool() && !pPane->IsShown())
		{
			pPane->SetAutoHide(eSide, (pPane->GetAutoHideSize() > 0) ? pPane->GetAutoHideSize() : DOCK_DEFAULT_SIZE);
			pStrip->AddPane(pPane);
			pPane->SetState(DPS_AutoHidden);
			if (!paMentioned->Contains(pPane))
			{
				paMentioned->Add(pPane);
			}
		}
		pcPane = pcAutoHide->GetNextTag(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first node under pcTag: a split, a group or, in the main window
// (bWell), the document well.  NULL if there is none or it is empty.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockLayoutReader::ReadFirstNode(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bWell)
{
	STagIterator	sIter;
	CMarkupTag*		pcChild;

	pcChild = pcTag->GetTag(&sIter);
	while (pcChild)
	{
		if (pcChild->Is("Split") || pcChild->Is("Group") || (bWell && pcChild->Is("DocumentWell")))
		{
			return ReadNode(pcChild, pWindow, false);
		}
		pcChild = pcTag->GetNextTag(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The main area's root, with the well in it somewhere: a layout without
// the well has it right of everything.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::PlaceMainRoot(Ptr<CDockNode> pRoot)
{
	mpcManager->mpArea->SetRoot(pRoot);
	if (pRoot.IsNull())
	{
		mpcManager->mpArea->SetRoot(mpcManager->mpWell);
	}
	else if (mpcManager->mpWell->GetParent().IsNull())
	{
		mpcManager->mcTree.DockBeside(pRoot, mpcManager->mpWell, DS_Right, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// <FloatingWindow>s, each shown where it was.  One left empty, its panes
// not known, isn't made.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::ReadFloatingWindows(CMarkupTag* pcRoot)
{
	STagIterator				sIter;
	CMarkupTag*					pcTag;
	CInt32Rect					cRect;
	Ptr<CDockFloatingWindow>	pFloating;
	Ptr<CDockNode>				pRoot;

	pcTag = pcRoot->GetTag("FloatingWindow", &sIter);
	while (pcTag)
	{
		if (ReadRectangle(pcTag, &cRect, ""))
		{
			pFloating = mpcManager->mcMover.CreateFloating(&cRect);
			pRoot = ReadFirstNode(pcTag, pFloating->GetWindow(), false);
			if (pRoot.IsNotNull())
			{
				pFloating->SetRoot(pRoot);
				pFloating->Show();
			}
			else
			{
				mpcManager->RemoveFloating(pFloating);
			}
		}
		pcTag = pcRoot->GetNextTag(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Panes the layout doesn't place go back where they were, or to their
// defaults, unless it hid them.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::PlaceTheRest(CArray<CDockPane>* paMentioned, CArray<CDockPane>* paHidden)
{
	size			i;
	Ptr<CDockPane>	pPane;

	for (i = 0; i < mpcManager->maPanes.Size(); i++)
	{
		pPane = mpcManager->maPanes.Get(i);
		if (pPane->IsShown() || paHidden->Contains(pPane))
		{
			continue;
		}
		if (paMentioned->Contains(pPane) && pPane->HasRestore())
		{
			mpcManager->mcRestorer.Restore(pPane);
		}
		else if (pPane->GetDefaultKind() != DPL_Hidden)
		{
			mpcManager->mcRestorer.DockDefault(pPane);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A node of the window pWindow: a split, a group or the main window's
// document well.  Panes already placed, or not known, are left out, and
// nodes left empty are dropped.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockLayoutReader::ReadNode(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bInWell)
{
	Ptr<CDockNode>	pNode;
	int32			iSize;

	if (pcTag->Is("Split"))
	{
		pNode = ReadSplit(pcTag, pWindow, bInWell);
	}
	else if (pcTag->Is("Group"))
	{
		pNode = ReadGroup(pcTag, pWindow, bInWell);
	}
	else if (pcTag->Is("DocumentWell") && (pWindow == mpcManager->mpWindow) && mpcManager->mpWell->GetParent().IsNull())
	{
		pNode = ReadWell(pcTag);
	}
	if (pNode.IsNotNull())
	{
		iSize = 0;
		pcTag->GetInt32Attribute("Size", &iSize);
		pNode->SetDockSize(iSize);
	}
	return pNode;
}


//////////////////////////////////////////////////////////////////////////
//
// <Split>: its nodes.  A split with one node is that node; a split in a
// split of the same direction gives its nodes to it.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockLayoutReader::ReadSplit(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bInWell)
{
	STagIterator	sIter;
	CMarkupTag*		pcChild;
	Ptr<CDockSplit>	pSplit;
	Ptr<CDockNode>	pNode;
	Ptr<CDockNode>	pOnly;
	char*			szDirection;

	szDirection = pcTag->GetStringAttribute("Direction");
	pSplit = OMalloc<CDockSplit>(pWindow, Ptr<CDockManager>(mpcManager), StringCompare(szDirection, "Vertical") != 0, mpcManager->GetColour(DC_Sash));
	pcChild = pcTag->GetTag(&sIter);
	while (pcChild)
	{
		pNode = ReadNode(pcChild, pWindow, bInWell);
		if (pNode.IsNotNull())
		{
			pSplit->AddNode(pNode);
		}
		pcChild = pcTag->GetNextTag(&sIter);
	}
	if (pSplit->NumNodes() == 0)
	{
		return NULL;
	}
	if (pSplit->NumNodes() == 1)
	{
		pOnly = pSplit->GetNode(0);
		pSplit->RemoveNode(pOnly);
		return pOnly;
	}
	mpcManager->mcTree.Flatten(pSplit);
	return pSplit;
}


//////////////////////////////////////////////////////////////////////////
//
// <Group>: its panes that aren't placed yet, and the one selected.  A
// group in the well is a document group.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockLayoutReader::ReadGroup(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bInWell)
{
	Ptr<CDockGroup>	pGroup;
	Ptr<CDockPane>	pSelected;
	int32			iID;
	bool			bDocuments;

	iID = 0;
	pcTag->GetInt32Attribute("ID", &iID);
	bDocuments = bInWell || (StringCompare(pcTag->GetStringAttribute("Kind"), "Document") == 0);
	pGroup = mpcManager->CreateGroup(bDocuments, iID);
	if (pWindow != mpcManager->mpWindow)
	{
		pGroup->MoveToWindow(pWindow);
	}
	pSelected = ReadGroupPanes(pcTag, pGroup, pWindow);
	if (pGroup->NumPanes() == 0)
	{
		return NULL;
	}
	if (pSelected.IsNotNull())
	{
		pGroup->SelectPane(pSelected);
	}
	if (bInWell && mpcManager->mpDocumentGroup.IsNull())
	{
		mpcManager->mpDocumentGroup = pGroup;
	}
	return pGroup;
}


//////////////////////////////////////////////////////////////////////////
//
// The group's <Pane>s that aren't placed yet go into it, in pWindow.
// Returns the one the group says is selected, if it is among them.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockLayoutReader::ReadGroupPanes(CMarkupTag* pcTag, Ptr<CDockGroup> pGroup, Ptr<CWindow> pWindow)
{
	STagIterator	sIter;
	CMarkupTag*		pcChild;
	Ptr<CDockPane>	pPane;
	Ptr<CDockPane>	pSelected;
	char*			szSelected;

	szSelected = pcTag->GetStringAttribute("Selected");
	pcChild = pcTag->GetTag("Pane", &sIter);
	while (pcChild)
	{
		pPane = ReadPane(pcChild);
		if (pPane.IsNotNull() && pPane->GetGroup().IsNull() && (pPane->GetState() != DPS_AutoHidden))
		{
			if (pPane->GetContent()->GetWindow() != pWindow)
			{
				pPane->GetContent()->MoveToWindow(pWindow);
			}
			mpcManager->PlaceInGroup(pGroup, pPane);
			if ((szSelected != NULL) && (StringCompare(szSelected, pPane->GetID()) == 0))
			{
				pSelected = pPane;
			}
		}
		pcChild = pcTag->GetNextTag(&sIter);
	}
	return pSelected;
}


//////////////////////////////////////////////////////////////////////////
//
// <DocumentWell>: the main window's well, with its tree.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockLayoutReader::ReadWell(CMarkupTag* pcTag)
{
	Ptr<CDocumentWell>	pWell;
	STagIterator		sIter;
	CMarkupTag*			pcChild;
	Ptr<CDockNode>		pNode;

	pWell = mpcManager->mpWell;
	pWell->SetRoot(NULL);
	pcChild = pcTag->GetTag(&sIter);
	while (pcChild)
	{
		pNode = ReadNode(pcChild, mpcManager->mpWindow, true);
		if (pNode.IsNotNull())
		{
			pWell->SetRoot(pNode);
			break;
		}
		pcChild = pcTag->GetNextTag(&sIter);
	}
	return pWell;
}


//////////////////////////////////////////////////////////////////////////
//
// The pane with the ID, made by the listener if it isn't added.  NULL if
// there is none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockLayoutReader::ReadPane(CMarkupTag* pcTag)
{
	char*			szID;
	Ptr<CDockPane>	pPane;

	szID = pcTag->GetStringAttribute("ID");
	if (szID == NULL)
	{
		return NULL;
	}
	pPane = mpcManager->GetPane(szID);
	if (pPane.IsNull() && mpcManager->mpcListener)
	{
		pPane = mpcManager->mpcListener->CreatePane(szID);
		if (pPane.IsNotNull() && !mpcManager->maPanes.Contains(pPane))
		{
			pPane = NULL;
		}
	}
	return pPane;
}


//////////////////////////////////////////////////////////////////////////
//
// A pane's state and where it goes back to, from <Panes>.
//
//////////////////////////////////////////////////////////////////////////
void CDockLayoutReader::ReadRestore(CMarkupTag* pcTag, Ptr<CDockPane> pPane)
{
	SDockRestore	sRestore;
	EDockSide		eSide;
	int32			iSize;
	CInt32Rect		cRect;

	sRestore.Init();
	pcTag->GetInt32Attribute("RestoreGroup", &sRestore.iGroup);
	pcTag->GetInt32Attribute("RestoreIndex", &sRestore.iIndex);
	pcTag->GetInt32Attribute("RestoreSize", &sRestore.iSize);
	pcTag->GetBoolAttribute("RestoreInWell", &sRestore.bInWell);
	sRestore.SetNeighbour(pcTag->GetStringAttribute("RestoreNeighbour"));
	sRestore.eSide = StringToDockSide(pcTag->GetStringAttribute("RestoreSide"));
	pPane->SetRestore(&sRestore);
	sRestore.Kill();
	if (ReadRectangle(pcTag, &cRect, "Float"))
	{
		pPane->SetFloatRectangle(&cRect);
	}
	eSide = StringToDockSide(pcTag->GetStringAttribute("AutoHideSide"));
	iSize = 0;
	pcTag->GetInt32Attribute("AutoHideSize", &iSize);
	pPane->SetAutoHide(eSide, iSize);
}


