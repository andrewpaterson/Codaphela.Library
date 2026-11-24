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
#include "TreeContainer.h"


#define TREE_ROW_COMPONENTS_STEP	8	//How many more components are made when more rows show than there are.




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
	masRanges.Init();
	maRangeFonts.Init();
	maiRowNodes.Init();
	maiTopNodes.Init();
	muiNumColumns = 0;
	maszCells.Init();
	maCellFonts.Init();
	maiColumnLefts.Init();
	maiColumnWidths.Init();
	miNameWidth = 0;
	maRows.Init();
	msRowsOrigin.Init(0, 0);
	miRowsWidth = 0;
	mbRowsBound = false;
	mbLaidOut = false;
	miIndent = pTheme->GetMetric(TM_TreeIndent);
	pFont = pTheme->GetFont();
	miRowHeight = (pFont.IsNotNull() ? pFont->GetLineHeight() : 16) + pTheme->GetMetric(TM_TreeRowPadding);
	mbRowsDirty = false;
	mcExpanded.Init();
	mbActivationToggles = true;
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
	U_Unknown(CArrayTreeNodeRange, masRanges);
	M_Embedded(maRangeFonts);
	U_Unknown(CArrayInt, maiRowNodes);
	U_Unknown(CArrayInt, maiTopNodes);
	U_Size(muiNumColumns);
	U_Unknown(CArrayChars, maszCells);
	M_Embedded(maCellFonts);
	U_Unknown(CArrayInt, maiColumnLefts);
	U_Unknown(CArrayInt, maiColumnWidths);
	U_Int32(miNameWidth);
	M_Embedded(maRows);
	U_2Int32(msRowsOrigin);
	U_Int32(miRowsWidth);
	U_Int32(miIndent);
	U_Int32(miRowHeight);
	U_Bool(mbRowsDirty);
	U_Bool(mbRowsBound);
	U_Bool(mbLaidOut);
	U_Unknown(CTreeNodeEvent, mcExpanded);
	U_Bool(mbActivationToggles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Free(void)
{
	mcExpanded.Kill();
	maiColumnWidths.Kill();
	maiColumnLefts.Kill();
	maszCells.Kill();
	maiTopNodes.Kill();
	maiRowNodes.Kill();
	masRanges.Kill();
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
	size		uiColumn;

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
	psNode->uiFirstRange = 0;
	psNode->uiNumRanges = 0;
	psNode->iWidth = -1;
	psNode->bExpanded = false;
	mszTexts.Append(szText);
	maIcons.Add(pIcon);
	maFonts.Add(NULL);
	for (uiColumn = 0; uiColumn < muiNumColumns; uiColumn++)
	{
		maszCells.Add("");
		maCellFonts.Add(NULL);
	}

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
		LayoutChanged();
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
	maszCells.ReInit();
	maCellFonts.RemoveAll();
	masRanges.ReInit();
	maRangeFonts.RemoveAll();
	maiTopNodes.ReInit();
	mbRowsDirty = true;
	UpdateRows();
	LayoutChanged();
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
	masNodes.Get(iNode)->iWidth = -1;
	if (IsNodeVisible(iNode))
	{
		mbRowsDirty = true;
		LayoutChanged();
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
// NULL if the node has no icon, or isn't one.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CTreeContainer::GetNodeIcon(int32 iNode)
{
	if ((iNode < 0) || (iNode >= NumNodes()))
	{
		return NULL;
	}
	return maIcons.Get(iNode);
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
			LayoutChanged();
		}

		//Told once the rows are as they will be: a listener may make the tree again.
		if (bExpanded)
		{
			mcExpanded.Call(this, iNode);
		}
	}
}


CTreeNodeEvent* CTreeContainer::GetExpandedEvent(void) { return &mcExpanded; }  //Called when a node is opened, with the node; not by ExpandAll.


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
	LayoutChanged();
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
	LayoutChanged();
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
		LayoutChanged();
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
// What a row is found by when typing finds one: its node's text.  Only the
// nodes that can be seen are rows, so those are the ones found.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::GetRowText(int32 iRow, CChars* pszDest)
{
	int32	iNode;

	iNode = GetRowNode(iRow);
	if (iNode == TREE_NO_NODE)
	{
		return false;
	}
	pszDest->Clear();
	GetNodeText(iNode, pszDest);
	return true;
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
// Makes the rows again from the nodes that can be seen if they changed:
// which node each row is of, and how wide the widest is.  No components
// are made, the rows in view's are given their nodes when the tree is
// next laid out.  The selected node stays selected, or if it was hidden
// the open node it is in.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::UpdateRows(void)
{
	int32	iSelected;
	int32	iOldSelected;
	int32	iRow;
	int32	iWidth;
	size	i;

	if (!mbRowsDirty)
	{
		return;
	}
	mbRowsDirty = false;
	mbRowsBound = false;

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
	maiRowNodes.ReInit();
	for (i = 0; i < maiTopNodes.NumElements(); i++)
	{
		AddVisibleRows(maiTopNodes.GetValue(i));
	}
	SetNumRows((int32)maiRowNodes.NumElements());

	iRow = -1;
	miRowsWidth = 0;
	for (i = 0; i < maiRowNodes.NumElements(); i++)
	{
		if (maiRowNodes.GetValue(i) == iSelected)
		{
			iRow = (int32)i;
		}
		iWidth = GetNodeWidth(maiRowNodes.GetValue(i));
		if (iWidth > miRowsWidth)
		{
			miRowsWidth = iWidth;
		}
	}
	miSelected = iRow;
	miAnchor = iRow;
	if (iRow >= 0)
	{
		mabSelected.Set(iRow, true);
	}
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

	maiRowNodes.Add(iNode);
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
// The next piece of the node's name after psPiece's (the first, after
// psPiece->Init()): a range, in its font, or the text before the next
// range, in the node's.  False when there are no more.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::NextNodePiece(STreeNode* psNode, STreeNodePiece* psPiece)
{
	STreeNodeRange*		psRange;
	size				uiPosition;

	uiPosition = psPiece->uiStart + psPiece->uiLength;
	if (uiPosition >= psNode->uiTextLength)
	{
		return false;
	}

	psRange = (psPiece->uiNextRange < psNode->uiNumRanges) ? masRanges.Get(psNode->uiFirstRange + psPiece->uiNextRange) : NULL;
	psPiece->uiStart = uiPosition;
	if (psRange && (psRange->uiStart == uiPosition))
	{
		psPiece->uiLength = psRange->uiLength;
		psPiece->uiFont = psRange->uiFont;
		psPiece->uiNextRange++;
	}
	else
	{
		psPiece->uiLength = (psRange ? psRange->uiStart : psNode->uiTextLength) - uiPosition;
		psPiece->uiFont = TREE_NODE_FONT;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The font a piece of iNode's name is drawn in.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTreeContainer::GetPieceFont(int32 iNode, STreeNodePiece* psPiece)
{
	Ptr<CFont>	pFont;

	if (psPiece->uiFont != TREE_NODE_FONT)
	{
		return maRangeFonts.Get(psPiece->uiFont);
	}
	pFont = maFonts.Get(iNode);
	if (pFont.IsNull())
	{
		pFont = mpTheme->GetFont();
	}
	return pFont;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the node's name starts across its row: after its indent, its
// chevron and its icon (CTreeRow::Layout).
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::GetNameLeft(int32 iNode)
{
	int32	iLeft;

	iLeft = masNodes.Get(iNode)->iDepth * miIndent + mpTheme->GetMetric(TM_TreeChevronWidth);
	if (maIcons.Get(iNode).IsNotNull())
	{
		iLeft += mpTheme->GetMetric(TM_TreeIconWidth);
	}
	return iLeft;
}


//////////////////////////////////////////////////////////////////////////
//
// What the node's row needs across: its indent, the chevron, its icon
// and each piece of its name in its font.  Worked out from the fonts, as
// its labels would be laid out, and kept until its font or ranges change.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::GetNodeWidth(int32 iNode)
{
	STreeNode*		psNode;
	STreeNodePiece	sPiece;
	int32			iWidth;

	psNode = masNodes.Get(iNode);
	if (psNode->iWidth < 0)
	{
		iWidth = GetNameLeft(iNode);
		sPiece.Init();
		while (NextNodePiece(psNode, &sPiece))
		{
			iWidth += (int32)GetPieceFont(iNode, &sPiece)->CalculateWidth(mszTexts.Text() + psNode->uiTextStart + sPiece.uiStart, sPiece.uiLength);
		}
		psNode->iWidth = iWidth;
	}
	return psNode->iWidth;
}


//////////////////////////////////////////////////////////////////////////
//
// The rows that can be seen, of the tree laid out with its first row at
// sOrigin in sArea: those in the scroll pane's view if it is in one, in
// the window, and in the tree's own area.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::GetRowsInView(SInt32Vec2 sOrigin, SInt32Vec2 sArea, int32* piFirst, int32* piNum)
{
	CInt32Rect	cView;
	CInt32Rect	cClip;
	SInt32Vec2	sWindow;
	int32		iFirst;
	int32		iLast;

	*piFirst = 0;
	*piNum = 0;
	if ((miRowHeight <= 0) || (NumRows() == 0))
	{
		return;
	}

	cView.Init(sOrigin.x, sOrigin.y, sOrigin.x + sArea.x, sOrigin.y + sArea.y);
	if (GetScrollView(&cClip))
	{
		cView.ClipTo(&cClip);
	}
	if (mpWindow.IsNotNull())
	{
		sWindow = mpWindow->GetActualSize();
		cClip.Init(0, 0, sWindow.x, sWindow.y);
		cView.ClipTo(&cClip);
	}
	if (cView.GetBottom() <= cView.GetTop())
	{
		return;
	}

	iFirst = (cView.GetTop() - sOrigin.y) / miRowHeight;
	iLast = (cView.GetBottom() - 1 - sOrigin.y) / miRowHeight;
	if (iLast >= NumRows())
	{
		iLast = NumRows() - 1;
	}
	if (iFirst <= iLast)
	{
		*piFirst = iFirst;
		*piNum = iLast - iFirst + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives the rows in view their components and lays them out, each across
// the tree at its row's height down.  Row i's component is the one at i
// modulo how many there are, so a row keeps its component while it stays
// in view and scrolling only changes those of the rows that come into
// view.  There are as many components as rows in view (more are made
// when more show, a few at a time); those left over show no node.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::LayoutRows(SInt32Vec2 sOrigin, SInt32Vec2 sArea)
{
	Ptr<CTreeRow>	pRow;
	SInt32Vec2		sRowPosition;
	SInt32Vec2		sRowSize;
	int32			iFirst;
	int32			iNum;
	int32			iNumComponents;
	int32			iFirstComponent;
	int32			iRow;
	int32			iNode;
	int32			i;

	GetRowsInView(sOrigin, sArea, &iFirst, &iNum);
	while ((int32)maRows.NumElements() < iNum)
	{
		for (i = 0; i < TREE_ROW_COMPONENTS_STEP; i++)
		{
			pRow = OMalloc<CTreeRow>(mpWindow, mpTheme->GetMetric(TM_TreeChevronWidth), mpTheme->GetMetric(TM_TreeIconWidth), miRowHeight);
			maRows.Add(pRow);
			CGridContainer::AddComponent(pRow, 0, 0);
		}
	}

	iNumComponents = (int32)maRows.NumElements();
	iFirstComponent = (iNumComponents > 0) ? (iFirst % iNumComponents) : 0;
	sRowSize.Init(sArea.x, miRowHeight);
	for (i = 0; i < iNumComponents; i++)
	{
		//The row the component is for: iFirst's is at iFirstComponent, and they go round from there.
		pRow = maRows.Get(i);
		iRow = iFirst + ((i - iFirstComponent + iNumComponents) % iNumComponents);
		if (iRow < iFirst + iNum)
		{
			iNode = maiRowNodes.GetValue(iRow);
			if (!mbRowsBound || (pRow->GetNode() != iNode))
			{
				BindRow(pRow, iNode);
			}
			sRowPosition.Init(sOrigin.x, sOrigin.y + iRow * miRowHeight);
			pRow->SetColumns(miNameWidth, &maiColumnLefts, &maiColumnWidths);
			pRow->Layout(sRowPosition, sRowSize);
		}
		else
		{
			pRow->SetNode(TREE_NO_NODE, 0);
		}
	}
	mbRowsBound = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The row shows iNode: its chevron, open or closed, if it has children,
// its icon, and its name, in one label, or with ranges a label for each
// range and for each run of the text between them.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::BindRow(Ptr<CTreeRow> pRow, int32 iNode)
{
	STreeNode*		psNode;
	STreeNodePiece	sPiece;
	Ptr<CImageCel>	pChevron;
	CChars			szPiece;
	size			uiNumLabels;
	size			uiColumn;

	psNode = masNodes.Get(iNode);
	pRow->SetNode(iNode, psNode->iDepth * miIndent);
	if (psNode->iFirstChild != TREE_NO_NODE)
	{
		pChevron = mpTheme->GetImage(psNode->bExpanded ? TI_DownChevron : TI_Chevron);
	}
	pRow->SetChevron(pChevron);
	pRow->SetIcon(maIcons.Get(iNode));

	uiNumLabels = 0;
	sPiece.Init();
	while (NextNodePiece(psNode, &sPiece))
	{
		szPiece.Init();
		szPiece.Append(mszTexts.Text() + psNode->uiTextStart + sPiece.uiStart, (int)sPiece.uiLength);
		pRow->SetLabel(uiNumLabels, GetPieceFont(iNode, &sPiece), szPiece.Text());
		szPiece.Kill();
		uiNumLabels++;
	}
	pRow->SetNumLabels(uiNumLabels);

	for (uiColumn = 0; uiColumn < muiNumColumns; uiColumn++)
	{
		pRow->SetCell(uiColumn, GetCellFont(iNode, uiColumn), GetCellText(iNode, uiColumn));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The tree's nodes have uiNumColumns cells each after their names.  False
// once nodes have been added.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::SetNumColumns(size uiNumColumns)
{
	if (NumNodes() != 0)
	{
		return false;
	}
	muiNumColumns = uiNumColumns;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the columns are across each row, from its left (paiLefts and
// paiWidths, a column each), and where the name is cut off.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::SetColumnBounds(int32 iNameWidth, CArrayInt* paiLefts, CArrayInt* paiWidths)
{
	miNameWidth = iNameWidth;
	maiColumnLefts.ReInit();
	maiColumnLefts.Copy(paiLefts);
	maiColumnWidths.ReInit();
	maiColumnWidths.Copy(paiWidths);
	LayoutChanged();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::SetCellText(int32 iNode, size uiColumn, const char* szText)
{
	CChars*	psz;

	if ((iNode < 0) || (iNode >= NumNodes()) || (uiColumn >= muiNumColumns))
	{
		return;
	}
	psz = maszCells.Get(iNode * muiNumColumns + uiColumn);
	if (!psz->Equals(szText))
	{
		psz->Set(szText);
		CellChanged(iNode);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// "" for no such cell.
//
//////////////////////////////////////////////////////////////////////////
const char* CTreeContainer::GetCellText(int32 iNode, size uiColumn)
{
	if ((iNode < 0) || (iNode >= NumNodes()) || (uiColumn >= muiNumColumns))
	{
		return "";
	}
	return maszCells.Get(iNode * muiNumColumns + uiColumn)->Text();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for the theme's font (as a changed value is drawn red, say).
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::SetCellFont(int32 iNode, size uiColumn, Ptr<CFont> pFont)
{
	if ((iNode < 0) || (iNode >= NumNodes()) || (uiColumn >= muiNumColumns) || (maCellFonts.Get(iNode * muiNumColumns + uiColumn) == pFont))
	{
		return;
	}
	maCellFonts.Set(iNode * muiNumColumns + uiColumn, pFont);
	CellChanged(iNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The cell's font, the theme's when it hasn't one of its own.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTreeContainer::GetCellFont(int32 iNode, size uiColumn)
{
	Ptr<CFont>	pFont;

	if ((iNode >= 0) && (iNode < NumNodes()) && (uiColumn < muiNumColumns))
	{
		pFont = maCellFonts.Get(iNode * muiNumColumns + uiColumn);
	}
	return pFont.IsNotNull() ? pFont : mpTheme->GetFont();
}


//////////////////////////////////////////////////////////////////////////
//
// A node's cell changed: its row, if it is in view, shows it as it is now.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::CellChanged(int32 iNode)
{
	if (IsNodeVisible(iNode))
	{
		mbRowsBound = false;
		LayoutChanged();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A part of the node's name, uiLength of it from uiStart, is drawn in
// pFont.  A node's ranges are added together, before another node's, in
// the order of its text, and don't overlap.  False if it isn't in the
// name, isn't after the node's last or another node's were added since.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::AddNodeRange(int32 iNode, size uiStart, size uiLength, Ptr<CFont> pFont)
{
	STreeNode*		psNode;
	STreeNodeRange*	psRange;
	size			uiFont;

	if ((iNode < 0) || (iNode >= NumNodes()) || (uiLength == 0) || pFont.IsNull())
	{
		return false;
	}
	psNode = masNodes.Get(iNode);
	if ((uiStart >= psNode->uiTextLength) || (uiLength > psNode->uiTextLength - uiStart))
	{
		return false;
	}
	if (psNode->uiNumRanges > 0)
	{
		psRange = masRanges.Get(psNode->uiFirstRange + psNode->uiNumRanges - 1);
		if ((psNode->uiFirstRange + psNode->uiNumRanges != masRanges.NumElements()) || (uiStart < psRange->uiStart + psRange->uiLength))
		{
			return false;
		}
	}
	else
	{
		psNode->uiFirstRange = masRanges.NumElements();
	}

	for (uiFont = 0; uiFont < maRangeFonts.NumElements(); uiFont++)
	{
		if (maRangeFonts.Get(uiFont) == pFont)
		{
			break;
		}
	}
	if (uiFont == maRangeFonts.NumElements())
	{
		maRangeFonts.Add(pFont);
	}

	psRange = masRanges.Add();
	psRange->uiStart = uiStart;
	psRange->uiLength = uiLength;
	psRange->uiFont = uiFont;
	psNode->uiNumRanges++;
	psNode->iWidth = -1;
	if (IsNodeVisible(iNode))
	{
		mbRowsDirty = true;
		LayoutChanged();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTreeContainer::NumNodeRanges(int32 iNode)
{
	return ((iNode >= 0) && (iNode < NumNodes())) ? masNodes.Get(iNode)->uiNumRanges : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::GetNodeRange(int32 iNode, size uiIndex, size* puiStart, size* puiLength)
{
	STreeNodeRange*	psRange;

	if (uiIndex >= NumNodeRanges(iNode))
	{
		return false;
	}
	psRange = masRanges.Get(masNodes.Get(iNode)->uiFirstRange + uiIndex);
	*puiStart = psRange->uiStart;
	*puiLength = psRange->uiLength;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CFont> CTreeContainer::GetNodeRangeFont(int32 iNode, size uiIndex)
{
	if (uiIndex >= NumNodeRanges(iNode))
	{
		return NULL;
	}
	return maRangeFonts.Get(masRanges.Get(masNodes.Get(iNode)->uiFirstRange + uiIndex)->uiFont);
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
// A node with children opens or closes, unless activation doesn't
// toggle; the listeners are told after.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::RowActivated(int32 iRow)
{
	int32	iNode;

	iNode = GetRowNode(iRow);
	if (mbActivationToggles && (iNode != TREE_NO_NODE) && HasChildren(iNode))
	{
		Toggle(iNode);
	}
}


void CTreeContainer::SetActivationToggles(bool bToggles) { mbActivationToggles = bToggles; }  //False: enter and a double click only tell the listeners; the chevron and left and right open and close.
bool CTreeContainer::IsActivationToggles(void) const { return mbActivationToggles; }


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
// As wide as the widest row (or its columns) and as tall as all of them, whether they are
// in view or not.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::SetRequiredSize(void)
{
	UpdateRows();
	msRequiredSize.Init(miRowsWidth, NumRows() * miRowHeight);
	if (maiColumnLefts.NumElements() > 0)
	{
		msRequiredSize.x = maiColumnLefts.GetValue(maiColumnLefts.NumElements() - 1) + maiColumnWidths.GetValue(maiColumnWidths.NumElements() - 1);
	}
	AddInsets(&msRequiredSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The rows aren't the grid's: only those in view are laid out, where
// their heights put them (LayoutRows).  A scroll pane lays the tree out
// again each time it scrolls, which is when rows come into view.
//
//////////////////////////////////////////////////////////////////////////
void CTreeContainer::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sOrigin;
	SInt32Vec2	sArea;

	UpdateRows();

	sOrigin = sPosition;
	sArea = sAreaSize;
	InsetArea(&sOrigin, &sArea);
	SetContainerBounds(sOrigin, sArea);
	SetActualBounds(sPosition, sAreaSize);
	msRowsOrigin = sOrigin;
	mbLaidOut = true;

	LayoutRows(sOrigin, sArea);
	UpdateHighlights();
}


//////////////////////////////////////////////////////////////////////////
//
// Where iRow is, in window client space, from where the tree was laid
// out last: every row is as tall as the others.  False if there is no
// such row or the tree hasn't been laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeContainer::GetRowBounds(int32 iRow, int32* piTop, int32* piHeight)
{
	if (!mbLaidOut || (iRow < 0) || (iRow >= NumRows()))
	{
		return false;
	}
	*piTop = msRowsOrigin.y + iRow * miRowHeight;
	*piHeight = miRowHeight;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The row y, in window client space, is in, -1 for none.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeContainer::FindRowAtY(int32 y)
{
	int32	iRow;

	if (!mbLaidOut || (miRowHeight <= 0) || (y < msRowsOrigin.y))
	{
		return -1;
	}
	iRow = (y - msRowsOrigin.y) / miRowHeight;
	return (iRow < NumRows()) ? iRow : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The component showing iRow, NULL if the row isn't in view (as the tree
// was laid out last).
//
//////////////////////////////////////////////////////////////////////////
Ptr<CTreeRow> CTreeContainer::GetRowComponent(int32 iRow)
{
	Ptr<CTreeRow>	pRow;

	if ((iRow < 0) || (iRow >= NumRows()) || (maRows.NumElements() == 0) || !mbRowsBound)
	{
		return NULL;
	}
	pRow = maRows.Get((size)iRow % maRows.NumElements());
	if (pRow->GetNode() != maiRowNodes.GetValue(iRow))
	{
		return NULL;
	}
	return pRow;
}


void STreeNodePiece::Init(void) { uiStart = 0; uiLength = 0; uiFont = TREE_NODE_FONT; uiNextRange = 0; }
size CTreeContainer::NumRowComponents(void) { return maRows.NumElements(); }  //How many components the tree has for its rows in view.
int32 CTreeContainer::NumNodes(void) const { return (int32)masNodes.NumElements(); }
size CTreeContainer::NumColumns(void) const { return muiNumColumns; }  //After the name.
int64 CTreeContainer::GetNodeData(int32 iNode) { return masNodes.Get(iNode)->iData; }
int32 CTreeContainer::GetNodeParent(int32 iNode) { return masNodes.Get(iNode)->iParent; }
int32 CTreeContainer::GetNodeDepth(int32 iNode) { return masNodes.Get(iNode)->iDepth; }
int32 CTreeContainer::GetFirstChild(int32 iNode) { return masNodes.Get(iNode)->iFirstChild; }
int32 CTreeContainer::GetNextSibling(int32 iNode) { return masNodes.Get(iNode)->iNextSibling; }
bool CTreeContainer::HasChildren(int32 iNode) { return masNodes.Get(iNode)->iFirstChild != TREE_NO_NODE; }
bool CTreeContainer::IsExpanded(int32 iNode) { return masNodes.Get(iNode)->bExpanded; }

