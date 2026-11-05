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
void CDockManager::WriteNode(CXMLWriter* pcWriter, Ptr<CDockNode> pNode)
{
	Ptr<CDockSplit>		pSplit;
	Ptr<CDockGroup>		pGroup;
	Ptr<CDocumentWell>	pWell;
	Ptr<CDockPane>		pSelected;
	size				i;

	switch (pNode->GetKind())
	{
	case DNK_Split:
		pSplit = Ptr<CDockSplit>(dynamic_cast<CDockSplit*>(&pNode));
		pcWriter->Begin("Split");
		pcWriter->Attribute("Direction", pSplit->IsHorizontal() ? "Horizontal" : "Vertical");
		if (pNode->GetDockSize() > 0)
		{
			pcWriter->Attribute("Size", (int32)pNode->GetDockSize());
		}
		for (i = 0; i < pSplit->NumNodes(); i++)
		{
			WriteNode(pcWriter, pSplit->GetNode(i));
		}
		pcWriter->End();
		break;

	case DNK_Group:
		pGroup = Ptr<CDockGroup>(dynamic_cast<CDockGroup*>(&pNode));
		pcWriter->Begin("Group");
		pcWriter->Attribute("ID", (int32)pGroup->GetID());
		pcWriter->Attribute("Kind", pGroup->IsDocumentGroup() ? "Document" : "Tool");
		if (pNode->GetDockSize() > 0)
		{
			pcWriter->Attribute("Size", (int32)pNode->GetDockSize());
		}
		pSelected = pGroup->GetSelectedPane();
		if (pSelected.IsNotNull())
		{
			pcWriter->Attribute("Selected", pSelected->GetID());
		}
		for (i = 0; i < pGroup->NumPanes(); i++)
		{
			pcWriter->Begin("Pane");
			pcWriter->Attribute("ID", pGroup->GetPane(i)->GetID());
			pcWriter->End();
		}
		pcWriter->End();
		break;

	case DNK_DocumentWell:
		pWell = Ptr<CDocumentWell>(dynamic_cast<CDocumentWell*>(&pNode));
		pcWriter->Begin("DocumentWell");
		if (pNode->GetDockSize() > 0)
		{
			pcWriter->Attribute("Size", (int32)pNode->GetDockSize());
		}
		if (pWell->GetRoot().IsNotNull())
		{
			WriteNode(pcWriter, pWell->GetRoot());
		}
		pcWriter->End();
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A pane's state, where it goes back to, where it floated and where it
// auto hides.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::WriteRestore(CXMLWriter* pcWriter, Ptr<CDockPane> pPane)
{
	CInt32Rect	cRect;

	pcWriter->Begin("Pane");
	pcWriter->Attribute("ID", pPane->GetID());
	pcWriter->Attribute("State", DockPaneStateToString(pPane->GetState()));
	if (pPane->GetRestoreGroup() != 0)
	{
		pcWriter->Attribute("RestoreGroup", (int32)pPane->GetRestoreGroup());
		pcWriter->Attribute("RestoreIndex", (int32)pPane->GetRestoreIndex());
	}
	if (pPane->GetRestoreNeighbour()[0] != '\0')
	{
		pcWriter->Attribute("RestoreNeighbour", pPane->GetRestoreNeighbour());
	}
	if (pPane->GetRestoreSide() != DS_None)
	{
		pcWriter->Attribute("RestoreSide", DockSideToString(pPane->GetRestoreSide()));
	}
	if (pPane->GetRestoreSize() > 0)
	{
		pcWriter->Attribute("RestoreSize", (int32)pPane->GetRestoreSize());
	}
	if (pPane->IsRestoreInWell())
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
// The whole layout, see the comment at the top of this file.
//
//////////////////////////////////////////////////////////////////////////
void CDockManager::WriteLayout(CXMLWriter* pcWriter)
{
	CInt32Rect					cRect;
	size						i;
	size						j;
	Ptr<CDockFloatingWindow>	pFloating;
	Ptr<CAutoHideStrip>			pStrip;
	EDockSide					eSide;

	pcWriter->Begin("DockLayout");
	pcWriter->Attribute("Version", (int32)DOCK_LAYOUT_VERSION);
	pcWriter->Attribute("NextGroup", (int32)miNextGroupID);

	pcWriter->Begin("MainWindow");
	mpWindow->GetScreenRectangle(&cRect);
	WriteRectangle(pcWriter, &cRect, "");
	if (mpArea->GetRoot().IsNotNull())
	{
		WriteNode(pcWriter, mpArea->GetRoot());
	}
	for (eSide = DS_Left; eSide <= DS_Bottom; eSide = (EDockSide)(eSide + 1))
	{
		pStrip = mpArea->GetStrip(eSide);
		if (pStrip->HasTabs())
		{
			pcWriter->Begin("AutoHide");
			pcWriter->Attribute("Side", DockSideToString(eSide));
			for (j = 0; j < pStrip->NumTabs(); j++)
			{
				pcWriter->Begin("Pane");
				pcWriter->Attribute("ID", pStrip->GetTab(j)->GetPane()->GetID());
				pcWriter->End();
			}
			pcWriter->End();
		}
	}
	pcWriter->End();

	for (i = 0; i < maFloating.Size(); i++)
	{
		pFloating = maFloating.Get(i);
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

	pcWriter->Begin("Panes");
	for (i = 0; i < maPanes.Size(); i++)
	{
		WriteRestore(pcWriter, maPanes.Get(i));
	}
	pcWriter->End();

	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the layout to szFileName.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::SaveLayout(const char* szFileName)
{
	CXMLWriter	cWriter;
	bool		bResult;

	cWriter.Init();
	WriteLayout(&cWriter);
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
// Reads the layout from szFileName.  A file that isn't there, or doesn't
// read, leaves the layout as it is and returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::LoadLayout(const char* szFileName)
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
		bResult = ReadLayout(pcRoot);
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
// The pane with the ID, made by the listener if it isn't added.  NULL if
// there is none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockPane> CDockManager::ReadPane(CMarkupTag* pcTag)
{
	char*			szID;
	Ptr<CDockPane>	pPane;

	szID = pcTag->GetStringAttribute("ID");
	if (szID == NULL)
	{
		return NULL;
	}
	pPane = GetPane(szID);
	if (pPane.IsNull() && mpcListener)
	{
		pPane = mpcListener->CreatePane(szID);
		if (pPane.IsNotNull() && !maPanes.Contains(pPane))
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
void CDockManager::ReadRestore(CMarkupTag* pcTag, Ptr<CDockPane> pPane)
{
	int32		iGroup;
	int32		iIndex;
	int32		iSize;
	bool		bInWell;
	char*		szNeighbour;
	EDockSide	eSide;
	CInt32Rect	cRect;

	iGroup = 0;
	iIndex = 0;
	iSize = 0;
	bInWell = false;
	pcTag->GetInt32Attribute("RestoreGroup", &iGroup);
	pcTag->GetInt32Attribute("RestoreIndex", &iIndex);
	pcTag->GetInt32Attribute("RestoreSize", &iSize);
	pcTag->GetBoolAttribute("RestoreInWell", &bInWell);
	szNeighbour = pcTag->GetStringAttribute("RestoreNeighbour");
	eSide = StringToDockSide(pcTag->GetStringAttribute("RestoreSide"));
	pPane->SetRestore(iGroup, iIndex, szNeighbour, eSide, iSize, bInWell);
	if (ReadRectangle(pcTag, &cRect, "Float"))
	{
		pPane->SetFloatRectangle(&cRect);
	}
	eSide = StringToDockSide(pcTag->GetStringAttribute("AutoHideSide"));
	iSize = 0;
	pcTag->GetInt32Attribute("AutoHideSize", &iSize);
	pPane->SetAutoHide(eSide, iSize);
}


//////////////////////////////////////////////////////////////////////////
//
// A node of the main area, of the window it will be in.  Panes already
// placed, or not known, are left out, and nodes left empty are dropped.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CDockNode> CDockManager::ReadNode(CMarkupTag* pcTag, Ptr<CWindow> pWindow, bool bInWell)
{
	STagIterator		sIter;
	CMarkupTag*			pcChild;
	Ptr<CDockSplit>		pSplit;
	Ptr<CDockGroup>		pGroup;
	Ptr<CDockNode>		pNode;
	Ptr<CDockNode>		pOnly;
	Ptr<CDockPane>		pPane;
	Ptr<CDockPane>		pSelected;
	int32				iSize;
	int32				iID;
	char*				szKind;
	char*				szDirection;
	char*				szSelected;
	bool				bDocuments;

	iSize = 0;
	pcTag->GetInt32Attribute("Size", &iSize);

	if (pcTag->Is("Split"))
	{
		szDirection = pcTag->GetStringAttribute("Direction");
		pSplit = OMalloc<CDockSplit>(pWindow, Ptr<CDockManager>(this), StringCompare(szDirection, "Vertical") != 0, GetColour(DC_Sash));
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
			pOnly->SetDockSize(iSize);
			return pOnly;
		}
		pSplit->SetDockSize(iSize);
		return pSplit;
	}

	if (pcTag->Is("Group"))
	{
		iID = 0;
		pcTag->GetInt32Attribute("ID", &iID);
		szKind = pcTag->GetStringAttribute("Kind");
		bDocuments = bInWell || (StringCompare(szKind, "Document") == 0);
		pGroup = CreateGroup(bDocuments, iID);
		if (pWindow != mpWindow)
		{
			pGroup->MoveToWindow(pWindow);
		}
		szSelected = pcTag->GetStringAttribute("Selected");
		pcChild = pcTag->GetTag("Pane", &sIter);
		while (pcChild)
		{
			pPane = ReadPane(pcChild);
			if (pPane.IsNotNull() && (pPane->GetGroup().IsNull()) && (pPane->GetState() != DPS_AutoHidden))
			{
				if (pPane->GetContent()->GetWindow() != pWindow)
				{
					pPane->GetContent()->MoveToWindow(pWindow);
				}
				pGroup->AddPane(pPane);
				pPane->SetState((pWindow == mpWindow) ? DPS_Docked : DPS_Floating);
				if ((szSelected != NULL) && (StringCompare(szSelected, pPane->GetID()) == 0))
				{
					pSelected = pPane;
				}
			}
			pcChild = pcTag->GetNextTag(&sIter);
		}
		if (pGroup->NumPanes() == 0)
		{
			return NULL;
		}
		if (pSelected.IsNotNull())
		{
			pGroup->SelectPane(pSelected);
		}
		if (bInWell && mpDocumentGroup.IsNull())
		{
			mpDocumentGroup = pGroup;
		}
		pGroup->SetDockSize(iSize);
		return pGroup;
	}

	if (pcTag->Is("DocumentWell") && (pWindow == mpWindow) && (mpWell->GetParent().IsNull()))
	{
		mpWell->SetRoot(NULL);
		pcChild = pcTag->GetTag(&sIter);
		while (pcChild)
		{
			pNode = ReadNode(pcChild, pWindow, true);
			if (pNode.IsNotNull())
			{
				mpWell->SetRoot(pNode);
				break;
			}
			pcChild = pcTag->GetNextTag(&sIter);
		}
		mpWell->SetDockSize(iSize);
		return mpWell;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the layout with the one in pcRoot, a <DockLayout>.  Panes the
// layout doesn't mention go to their default places.  Returns false, and
// leaves the layout as it was, for a layout that isn't one or is of a
// later version.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::ReadLayout(CMarkupTag* pcRoot)
{
	STagIterator				sIter;
	STagIterator				sPaneIter;
	CMarkupTag*					pcMain;
	CMarkupTag*					pcTag;
	CMarkupTag*					pcPane;
	CMarkupTag*					pcPanes;
	int32						iVersion;
	int32						iNextGroup;
	Ptr<CDockNode>				pRoot;
	Ptr<CDockPane>				pPane;
	CArray<CDockPane>			aMentioned;
	CArray<CDockPane>			aHidden;
	size						i;
	EDockSide					eSide;
	Ptr<CAutoHideStrip>			pStrip;
	CInt32Rect					cRect;
	Ptr<CDockFloatingWindow>	pFloating;
	char*						szState;

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

	ClearLayout();
	iNextGroup = 1;
	pcRoot->GetInt32Attribute("NextGroup", &iNextGroup);
	//No group is left after clearing: the layout's own IDs are used.
	miNextGroupID = (iNextGroup > 0) ? iNextGroup : 1;
	mpArea->SetRoot(NULL);

	//Every pane's state and where it goes back to first, so the panes made for the layout know them.
	aMentioned.Init();
	aHidden.Init();
	pcPanes = pcRoot->GetTag("Panes", (STagIterator*)NULL);
	if (pcPanes)
	{
		pcPane = pcPanes->GetTag("Pane", &sPaneIter);
		while (pcPane)
		{
			pPane = ReadPane(pcPane);
			if (pPane.IsNotNull())
			{
				ReadRestore(pcPane, pPane);
				aMentioned.Add(pPane);
				szState = pcPane->GetStringAttribute("State");
				if ((szState != NULL) && (StringCompare(szState, "Hidden") == 0))
				{
					aHidden.Add(pPane);
				}
			}
			pcPane = pcPanes->GetNextTag(&sPaneIter);
		}
	}

	pcMain = pcRoot->GetTag("MainWindow", (STagIterator*)NULL);
	mbLayoutWindow = false;
	if (pcMain)
	{
		mbLayoutWindow = ReadRectangle(pcMain, &mcLayoutWindow, "");

		//Auto hidden panes first so a group naming one leaves it out.
		pcTag = pcMain->GetTag("AutoHide", &sIter);
		while (pcTag)
		{
			eSide = StringToDockSide(pcTag->GetStringAttribute("Side"));
			pStrip = mpArea->GetStrip(eSide);
			if (pStrip.IsNotNull())
			{
				pcPane = pcTag->GetTag("Pane", &sPaneIter);
				while (pcPane)
				{
					pPane = ReadPane(pcPane);
					if (pPane.IsNotNull() && pPane->IsTool() && !pPane->IsShown())
					{
						if (pPane->GetAutoHideSize() <= 0)
						{
							pPane->SetAutoHide(eSide, DOCK_DEFAULT_SIZE);
						}
						pPane->SetAutoHide(eSide, pPane->GetAutoHideSize());
						pStrip->AddPane(pPane);
						pPane->SetState(DPS_AutoHidden);
						if (!aMentioned.Contains(pPane))
						{
							aMentioned.Add(pPane);
						}
					}
					pcPane = pcTag->GetNextTag(&sPaneIter);
				}
			}
			pcTag = pcMain->GetNextTag(&sIter);
		}
		mpArea->StripsChanged();

		pcTag = pcMain->GetTag(&sIter);
		while (pcTag)
		{
			if (pcTag->Is("Split") || pcTag->Is("Group") || pcTag->Is("DocumentWell"))
			{
				pRoot = ReadNode(pcTag, mpWindow, false);
				break;
			}
			pcTag = pcMain->GetNextTag(&sIter);
		}
	}
	mpArea->SetRoot(pRoot);
	if (pRoot.IsNull())
	{
		mpArea->SetRoot(mpWell);
	}
	else if (mpWell->GetParent().IsNull())
	{
		//A layout without the well: it goes right of everything.
		DockBeside(pRoot, mpWell, DS_Right, 0);
	}

	pcTag = pcRoot->GetTag("FloatingWindow", &sIter);
	while (pcTag)
	{
		if (ReadRectangle(pcTag, &cRect, ""))
		{
			pFloating = CreateFloating(&cRect);
			pcPane = pcTag->GetTag(&sPaneIter);
			pRoot = NULL;
			while (pcPane)
			{
				if (pcPane->Is("Split") || pcPane->Is("Group"))
				{
					pRoot = ReadNode(pcPane, pFloating->GetWindow(), false);
					break;
				}
				pcPane = pcTag->GetNextTag(&sPaneIter);
			}
			if (pRoot.IsNotNull())
			{
				pFloating->SetRoot(pRoot);
				pFloating->Show();
			}
			else
			{
				maFloating.Remove(pFloating);
			}
		}
		pcTag = pcRoot->GetNextTag(&sIter);
	}

	//Panes the layout doesn't place go to their defaults, unless it hid them.
	for (i = 0; i < maPanes.Size(); i++)
	{
		pPane = maPanes.Get(i);
		if (!pPane->IsShown() && !aHidden.Contains(pPane))
		{
			if (aMentioned.Contains(pPane) && pPane->HasRestore())
			{
				Restore(pPane);
			}
			else if (pPane->GetDefaultKind() != DPL_Hidden)
			{
				DockDefault(pPane);
			}
		}
	}
	aHidden.Kill();
	aMentioned.Kill();
	LayoutChanged();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The main window's client area on the screen in the last layout read.
// Returns false if there wasn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CDockManager::GetLayoutWindowRectangle(CInt32Rect* pcDest)
{
	pcDest->Init(&mcLayoutWindow);
	return mbLayoutWindow;
}

