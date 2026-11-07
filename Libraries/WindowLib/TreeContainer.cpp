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
#include "SupportLib/ColourFormat.h"
#include "Window.h"
#include "FillContainer.h"
#include "GridContainer.h"
#include "IconComponent.h"
#include "Label.h"
#include "TreeContainer.h"




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	Ptr<CFont>			pFont;

	PreInit();

	CListContainer::Init(pWindow, pTheme);
	mpTheme = pTheme;
	masNodes.Init();
	mszTexts.Init();
	maIcons.Init();
	maFonts.Init();
	maiRowNodes.Init();
	maiTopNodes.Init();
	miIndent = pTheme->GetMetric(TM_TreeIndent);
	pFont = pTheme->GetFont();
	miRowHeight = (pFont.IsNotNull() ? pFont->GetLineHeight() : 16) + pTheme->GetMetric(TM_TreeRowPadding);
	mbRowsDirty = false;
	SetDoubleClickActivates(true);
	AddInputAction(CIC_Left, this, &CTreeContainer::Left);
	AddInputAction(CIC_Right, this, &CTreeContainer::Right);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Class(void)
{
	CListContainer::Class();
	M_Pointer(mpTheme);
	U_Unknown(CArrayTreeNode, masNodes);
	U_String(mszTexts);
	M_Embedded(maIcons);
	M_Embedded(maFonts);
	U_Unknown(CArrayInt, maiRowNodes);
	U_Unknown(CArrayInt, maiTopNodes);
	U_Int32(miIndent);
	U_Int32(miRowHeight);
	U_Bool(mbRowsDirty);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Free(void)
{
	maiTopNodes.Kill();
	maiRowNodes.Kill();
	mszTexts.Kill();
	masNodes.Kill();
	CListContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds a node after iParent's other children (or after the other nodes
// at the top when iParent is TREE_NO_NODE).  It starts closed.  iData is
// for the caller to know what the node is of.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::AddNode(int32 iParent, const char* szText, Ptr<CImageCel> pIcon, int64 iData)
{
	STreeNode*	psNode;
	STreeNode*	psParent;
	int32		iNode;

	if ((iParent != TREE_NO_NODE) && ((iParent < 0) || (iParent >= NumNodes())))
	{
		return TREE_NO_NODE;
	}

	iNode = NumNodes();
	psNode = masNodes.Add();
	psNode->iParent = iParent;
	psNode->iFirstChild = TREE_NO_NODE;
	psNode->iLastChild = TREE_NO_NODE;
	psNode->iNextSibling = TREE_NO_NODE;
	psNode->iData = iData;
	psNode->uiTextStart = mszTexts.Length();
	psNode->uiTextLength = strlen(szText);
	psNode->bExpanded = false;
	mszTexts.Append(szText);
	maIcons.Add(pIcon);
	maFonts.Add(NULL);

	if (iParent == TREE_NO_NODE)
	{
		psNode->iDepth = 0;
		maiTopNodes.Add(iNode);
	}
	else
	{
		psParent = masNodes.Get(iParent);
		psNode->iDepth = psParent->iDepth + 1;
		if (psParent->iLastChild == TREE_NO_NODE)
		{
			psParent->iFirstChild = iNode;
		}
		else
		{
			masNodes.Get(psParent->iLastChild)->iNextSibling = iNode;
		}
		psParent->iLastChild = iNode;
	}

	if ((iParent == TREE_NO_NODE) || IsNodeVisible(iParent))
	{
		mbRowsDirty = true;
		InvalidateLayout();
	}
	return iNode;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::RemoveAllNodes(void)
{
	masNodes.ReInit();
	mszTexts.Clear();
	maIcons.RemoveAll();
	maFonts.RemoveAll();
	maiTopNodes.ReInit();
	mbRowsDirty = true;
	UpdateRows();
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The node's text appended to pszDest.
//
//////////////////////////////////////////////////////////////////////////
char* CTreeContainer::GetNodeText(int32 iNode, CChars* pszDest)
{
	STreeNode*	psNode;

	psNode = masNodes.Get(iNode);
	pszDest->Append(mszTexts.Text() + psNode->uiTextStart, (int)psNode->uiTextLength);
	return pszDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// The child of iParent (or node at the top) named szText, TREE_NO_NODE
// if there is none.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::FindNode(int32 iParent, const char* szText)
{
	int32		iNode;
	size		i;
	STreeNode*	psNode;
	size		uiLength;

	uiLength = strlen(szText);
	if (iParent == TREE_NO_NODE)
	{
		for (i = 0; i < maiTopNodes.NumElements(); i++)
		{
			iNode = maiTopNodes.GetValue(i);
			psNode = masNodes.Get(iNode);
			if ((psNode->uiTextLength == uiLength) && (memcmp(mszTexts.Text() + psNode->uiTextStart, szText, uiLength) == 0))
			{
				return iNode;
			}
		}
		return TREE_NO_NODE;
	}

	for (iNode = GetFirstChild(iParent); iNode != TREE_NO_NODE; iNode = GetNextSibling(iNode))
	{
		psNode = masNodes.Get(iNode);
		if ((psNode->uiTextLength == uiLength) && (memcmp(mszTexts.Text() + psNode->uiTextStart, szText, uiLength) == 0))
		{
			return iNode;
		}
	}
	return TREE_NO_NODE;
}


//////////////////////////////////////////////////////////////////////////
//
// The node's name is drawn in pFont, or the theme's font if it is NULL.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::SetNodeFont(int32 iNode, Ptr<CFont> pFont)
{
	if ((iNode < 0) || (iNode >= NumNodes()) || (maFonts.Get(iNode) == pFont))
	{
		return;
	}
	maFonts.Set(iNode, pFont);
	if (IsNodeVisible(iNode))
	{
		mbRowsDirty = true;
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the node is in the theme's font.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTreeContainer::GetNodeFont(int32 iNode)
{
	return maFonts.Get(iNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Opening a node shows its children; closing it hides them (and the
// selection moves to it if it was in them).
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::SetExpanded(int32 iNode, bool bExpanded)
{
	STreeNode*	psNode;

	psNode = masNodes.Get(iNode);
	if (psNode->bExpanded != bExpanded)
	{
		psNode->bExpanded = bExpanded;
		if (IsNodeVisible(iNode) && (psNode->iFirstChild != TREE_NO_NODE))
		{
			mbRowsDirty = true;
			UpdateRows();
			InvalidateLayout();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Toggle(int32 iNode)
{
	SetExpanded(iNode, !IsExpanded(iNode));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::ExpandAll(void)
{
	size	i;

	for (i = 0; i < masNodes.NumElements(); i++)
	{
		masNodes.Get(i)->bExpanded = true;
	}
	mbRowsDirty = true;
	UpdateRows();
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::CollapseAll(void)
{
	size	i;

	for (i = 0; i < masNodes.NumElements(); i++)
	{
		masNodes.Get(i)->bExpanded = false;
	}
	mbRowsDirty = true;
	UpdateRows();
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the nodes iNode is in so it can be seen, and selects it.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::SelectNode(int32 iNode)
{
	int32	iParent;
	bool	bOpened;

	if (iNode == TREE_NO_NODE)
	{
		return Select(-1);
	}
	if ((iNode < 0) || (iNode >= NumNodes()))
	{
		return false;
	}

	bOpened = false;
	for (iParent = GetNodeParent(iNode); iParent != TREE_NO_NODE; iParent = GetNodeParent(iParent))
	{
		if (!masNodes.Get(iParent)->bExpanded)
		{
			masNodes.Get(iParent)->bExpanded = true;
			bOpened = true;
		}
	}
	if (bOpened)
	{
		mbRowsDirty = true;
		InvalidateLayout();
	}
	UpdateRows();
	return Select(FindNodeRow(iNode));
}


//////////////////////////////////////////////////////////////////////////
//
// TREE_NO_NODE if nothing is selected.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::GetSelectedNode(void)
{
	return GetRowNode(GetSelected());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::GetRowNode(int32 iRow)
{
	if ((iRow < 0) || (iRow >= (int32)maiRowNodes.NumElements()))
	{
		return TREE_NO_NODE;
	}
	return maiRowNodes.GetValue(iRow);
}


//////////////////////////////////////////////////////////////////////////
//
// -1 if the node can't be seen.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::FindNodeRow(int32 iNode)
{
	size	i;

	UpdateRows();
	for (i = 0; i < maiRowNodes.NumElements(); i++)
	{
		if (maiRowNodes.GetValue(i) == iNode)
		{
			return (int32)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// A node can be seen when every node it is in is open.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::IsNodeVisible(int32 iNode)
{
	int32	iParent;

	for (iParent = GetNodeParent(iNode); iParent != TREE_NO_NODE; iParent = GetNodeParent(iParent))
	{
		if (!masNodes.Get(iParent)->bExpanded)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the rows again from the nodes that can be seen if they changed.
// The selected node stays selected, or if it was hidden the open node it
// is in.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::UpdateRows(void)
{
	int32	iSelected;
	int32	iOldSelected;
	int32	iRow;
	size	i;

	if (!mbRowsDirty)
	{
		return;
	}
	mbRowsDirty = false;

	//The nodes may all have been removed.
	iOldSelected = GetRowNode(GetSelected());
	if (iOldSelected >= NumNodes())
	{
		iOldSelected = TREE_NO_NODE;
	}
	iSelected = iOldSelected;
	while ((iSelected != TREE_NO_NODE) && !IsNodeVisible(iSelected))
	{
		iSelected = GetNodeParent(iSelected);
	}

	//The selection is carried over the new rows quietly: the listeners only hear of it when it is a different node.
	miSelected = -1;
	CListContainer::RemoveAllComponents();
	maiRowNodes.ReInit();
	for (i = 0; i < maiTopNodes.NumElements(); i++)
	{
		AddVisibleRows(maiTopNodes.GetValue(i));
	}

	iRow = -1;
	if (iSelected != TREE_NO_NODE)
	{
		for (i = 0; i < maiRowNodes.NumElements(); i++)
		{
			if (maiRowNodes.GetValue(i) == iSelected)
			{
				iRow = (int32)i;
				break;
			}
		}
	}
	miSelected = iRow;
	UpdateHighlights();
	if (iSelected != iOldSelected)
	{
		RowSelected(iRow);
		mcSelectionChanged.Call(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The node's row, then its children's if it is open.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::AddVisibleRows(int32 iNode)
{
	int32	iChild;

	AddNodeRow(iNode);
	if (masNodes.Get(iNode)->bExpanded)
	{
		for (iChild = GetFirstChild(iNode); iChild != TREE_NO_NODE; iChild = GetNextSibling(iChild))
		{
			AddVisibleRows(iChild);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The indent, the chevron (a gap for a node without children), the icon
// and the name, in a grid of their own so rows of different depths
// don't share columns.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::AddNodeRow(int32 iNode)
{
	STreeNode*				psNode;
	Ptr<CGridContainer>		pRow;
	Ptr<CFillContainer>		pGap;
	Ptr<CIconComponent>		pIcon;
	Ptr<CLabel>				pLabel;
	Ptr<CImageCel>			pCel;
	Ptr<CFont>				pFont;
	CChars					szText;
	int32					iListRow;
	int						iColumn;
	int32					iWidth;

	psNode = masNodes.Get(iNode);
	pRow = OMalloc<CGridContainer>(mpWindow);
	pRow->SetAlignment(CSH_Stretch, CSV_Center);
	iColumn = 0;
	iWidth = psNode->iDepth * miIndent + mpTheme->GetMetric(TM_TreeChevronWidth);

	pGap = OMalloc<CFillContainer>(mpWindow);
	pGap->SetDesiredSize(psNode->iDepth * miIndent, miRowHeight);
	pRow->AddComponent(pGap, iColumn++, 0);

	if (psNode->iFirstChild != TREE_NO_NODE)
	{
		pIcon = OMalloc<CIconComponent>(mpWindow, mpTheme->GetImage(psNode->bExpanded ? TI_DownChevron : TI_Chevron), mpTheme->GetMetric(TM_TreeChevronWidth), miRowHeight);
		pIcon->SetCelAlignment(CSH_Center, CSV_Center);
		pRow->AddComponent(pIcon, iColumn++, 0);
	}
	else
	{
		pGap = OMalloc<CFillContainer>(mpWindow);
		pGap->SetDesiredSize(mpTheme->GetMetric(TM_TreeChevronWidth), miRowHeight);
		pRow->AddComponent(pGap, iColumn++, 0);
	}

	pCel = maIcons.Get(iNode);
	if (pCel.IsNotNull())
	{
		pIcon = OMalloc<CIconComponent>(mpWindow, pCel, mpTheme->GetMetric(TM_TreeIconWidth), miRowHeight);
		pIcon->SetCelAlignment(CSH_Left, CSV_Center);
		pRow->AddComponent(pIcon, iColumn++, 0);
		iWidth += mpTheme->GetMetric(TM_TreeIconWidth);
	}

	pFont = maFonts.Get(iNode);
	if (pFont.IsNull())
	{
		pFont = mpTheme->GetFont();
	}
	szText.Init();
	GetNodeText(iNode, &szText);
	pLabel = OMalloc<CLabel>(mpWindow, pFont, szText.Text());
	szText.Kill();
	pLabel->SetAlignment(CSH_Left, CSV_Center);
	pRow->AddComponent(pLabel, iColumn, 0, 1, 1, 1, 0);

	//A grid in a grid isn't measured by its parent, so the row says how big it is.
	iWidth += pLabel->GetDesiredSize().x;
	pRow->SetDesiredSize(iWidth, miRowHeight);

	iListRow = AddRow(true);
	AddToRow(iListRow, pRow, 0, 1, 1);
	maiRowNodes.Add(iNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The x (from the tree's left) is over the row's chevron.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::IsOverChevron(int32 iRow, int x)
{
	int32		iNode;
	STreeNode*	psNode;
	int			iLeft;

	iNode = GetRowNode(iRow);
	if (iNode == TREE_NO_NODE)
	{
		return false;
	}
	psNode = masNodes.Get(iNode);
	if (psNode->iFirstChild == TREE_NO_NODE)
	{
		return false;
	}
	iLeft = psNode->iDepth * miIndent;
	return (x >= iLeft) && (x < iLeft + mpTheme->GetMetric(TM_TreeChevronWidth));
}


//////////////////////////////////////////////////////////////////////////
//
// A press selects the row; on a chevron it also opens or closes its node.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::ActivateComponent(int x, int y)
{
	SInt32Vec2	sPosition;
	int32		iRow;

	CListContainer::ActivateComponent(x, y);
	sPosition = GetPosition();
	iRow = FindRowAt(x + sPosition.x, y + sPosition.y);
	if ((iRow >= 0) && IsOverChevron(iRow, x))
	{
		Toggle(GetRowNode(iRow));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A node with children opens or closes; the listeners are told after.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::RowActivated(int32 iRow)
{
	int32	iNode;

	iNode = GetRowNode(iRow);
	if ((iNode != TREE_NO_NODE) && HasChildren(iNode))
	{
		Toggle(iNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// As in Solution Explorer: closes the selected node, or if it is closed
// (or has no children) selects the node it is in.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Left(void)
{
	int32	iNode;

	iNode = GetSelectedNode();
	if (iNode == TREE_NO_NODE)
	{
		return;
	}
	if (HasChildren(iNode) && IsExpanded(iNode))
	{
		SetExpanded(iNode, false);
	}
	else if (GetNodeParent(iNode) != TREE_NO_NODE)
	{
		SelectNode(GetNodeParent(iNode));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Opens the selected node, or if it is open selects its first child.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Right(void)
{
	int32	iNode;

	iNode = GetSelectedNode();
	if ((iNode == TREE_NO_NODE) || !HasChildren(iNode))
	{
		return;
	}
	if (!IsExpanded(iNode))
	{
		SetExpanded(iNode, true);
	}
	else
	{
		SelectNode(GetFirstChild(iNode));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The rows are made from the nodes before they are measured or laid out.
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CTreeContainer::GetDesiredSize(void)
{
	UpdateRows();
	return CListContainer::GetDesiredSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	UpdateRows();
	CListContainer::Layout(sPosition, sAreaSize);
}


int32 CTreeContainer::NumNodes(void) { return (int32)masNodes.NumElements(); }
int64 CTreeContainer::GetNodeData(int32 iNode) { return masNodes.Get(iNode)->iData; }
int32 CTreeContainer::GetNodeParent(int32 iNode) { return masNodes.Get(iNode)->iParent; }
int32 CTreeContainer::GetNodeDepth(int32 iNode) { return masNodes.Get(iNode)->iDepth; }
int32 CTreeContainer::GetFirstChild(int32 iNode) { return masNodes.Get(iNode)->iFirstChild; }
int32 CTreeContainer::GetNextSibling(int32 iNode) { return masNodes.Get(iNode)->iNextSibling; }
bool CTreeContainer::HasChildren(int32 iNode) { return masNodes.Get(iNode)->iFirstChild != TREE_NO_NODE; }
bool CTreeContainer::IsExpanded(int32 iNode) { return masNodes.Get(iNode)->bExpanded; }

