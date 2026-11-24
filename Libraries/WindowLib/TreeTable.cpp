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
#include "Window.h"
#include "ClearCanvasDraw.h"
#include "TreeTable.h"


#define TREE_TABLE_HEADER_LINE		1
#define TREE_TABLE_PADDING_Y		4
#define TREE_TABLE_VISIBLE_ROWS		6


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CContainer::Init(pWindow);
	mpTheme = pTheme;
	maCaptions.Init();
	maSashes.Init();
	maiWidths.Init();
	mabEditable.Init();
	maiLefts.Init();
	maiLaidWidths.Init();
	mcEdited.Init();
	miEditNode = TREE_NO_NODE;
	miEditColumn = -1;
	miEditedNode = TREE_NO_NODE;
	miEditedColumn = -1;
	mszEditedText.Init();
	mszName.Init();
	mbEnding = false;

	mpHeaderBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_Panel)));
	mpHeaderBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	CContainer::AddComponent(mpHeaderBackground);
	mpHeaderLine = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_Separator)));
	mpHeaderLine->SetAlignment(CSH_Stretch, CSV_Stretch);
	CContainer::AddComponent(mpHeaderLine);

	mpTree = OMalloc<CTreeContainer>(pWindow, pTheme);
	mpTree->SetAlignment(CSH_Stretch, CSV_Top);
	mpTree->SetActivationToggles(false);
	mpTree->GetActivationEvent()->Add(this, &CTreeTable::Activated);
	mpScroll = OMalloc<CScrollPane>(pWindow, pTheme);
	mpScroll->SetContent(mpTree);
	mpScroll->SetPolicy(SBO_Horizontal, SPP_Never);
	mpScroll->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpScroll->GetScrollEvent()->Add(this, &CTreeTable::Scrolled);
	CContainer::AddComponent(mpScroll);

	mpField = OMalloc<CCellField>(pWindow, pTheme, "");
	mpField->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpField->GetEndedEvent()->Add(this, &CTreeTable::FieldEnded);
	CContainer::AddComponent(mpField);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::Class(void)
{
	CContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpHeaderBackground);
	M_Pointer(mpHeaderLine);
	M_Embedded(maCaptions);
	M_Embedded(maSashes);
	M_Pointer(mpScroll);
	M_Pointer(mpTree);
	M_Pointer(mpField);
	U_Unknown(CArrayInt, maiWidths);
	U_Unknown(CArrayBit, mabEditable);
	U_Unknown(CArrayInt, maiLefts);
	U_Unknown(CArrayInt, maiLaidWidths);
	U_Unknown(CComponentEvent<CTreeTable>, mcEdited);
	U_Int32(miEditNode);
	U_Int32(miEditColumn);
	U_Int32(miEditedNode);
	U_Int32(miEditedColumn);
	U_String(mszEditedText);
	U_String(mszName);
	U_Bool(mbEnding);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::Free(void)
{
	size	i;

	for (i = 0; i < maSashes.NumElements(); i++)
	{
		maSashes.Get(i)->GetMovedEvent()->Remove(this);
	}
	if (mpTree.IsNotNull())
	{
		mpTree->GetActivationEvent()->Remove(this);
	}
	if (mpScroll.IsNotNull())
	{
		mpScroll->GetScrollEvent()->Remove(this);
	}
	if (mpField.IsNotNull())
	{
		mpField->GetEndedEvent()->Remove(this);
	}
	mszName.Kill();
	mszEditedText.Kill();
	mcEdited.Kill();
	maiLaidWidths.Kill();
	maiLefts.Kill();
	mabEditable.Kill();
	maiWidths.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTreeTable::Save(CObjectWriter* pcFile) { return false; }
bool CTreeTable::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// A column iWidth wide, after those there are (the first is the nodes'
// names); its number.  Columns are added before nodes.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeTable::AddColumn(const char* szCaption, int32 iWidth, bool bEditable)
{
	Ptr<CLabel>	pCaption;
	Ptr<CSash>	pSash;

	if ((maiWidths.NumElements() > 0) && !mpTree->SetNumColumns(maiWidths.NumElements()))
	{
		return -1;
	}
	if (maiWidths.NumElements() > 0)
	{
		pSash = OMalloc<CSash>(mpWindow, true, mpTheme->GetColour(TC_Separator));
		pSash->GetMovedEvent()->Add(this, &CTreeTable::SashMoved);
		maSashes.Add(pSash);
		CContainer::AddComponent(pSash);
	}
	pCaption = OMalloc<CLabel>(mpWindow, mpTheme->GetFont(), szCaption);
	maCaptions.Add(pCaption);
	CContainer::AddComponent(pCaption);
	maiWidths.Add(iWidth);
	mabEditable.Add(bEditable);
	LayoutChanged();
	return (int32)maiWidths.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// As dragged (the last's as laid out, what was left for it).
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeTable::GetColumnWidth(int32 iColumn)
{
	if ((iColumn < 0) || (iColumn >= NumColumns()))
	{
		return 0;
	}
	if ((iColumn == NumColumns() - 1) && ((size)iColumn < maiLaidWidths.NumElements()))
	{
		return maiLaidWidths.GetValue(iColumn);
	}
	return maiWidths.GetValue(iColumn);
}


//////////////////////////////////////////////////////////////////////////
//
// A node's cell in iColumn (1 on: the first is its name).
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::SetCellText(int32 iNode, int32 iColumn, const char* szText)
{
	if (iColumn > 0)
	{
		mpTree->SetCellText(iNode, (size)(iColumn - 1), szText);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The node's name for column 0.
//
//////////////////////////////////////////////////////////////////////////
const char* CTreeTable::GetCellText(int32 iNode, int32 iColumn)
{
	if (iColumn == 0)
	{
		mszName.Clear();
		mpTree->GetNodeText(iNode, &mszName);
		return mszName.Text();
	}
	return mpTree->GetCellText(iNode, (size)(iColumn - 1));
}


//////////////////////////////////////////////////////////////////////////
//
// NULL for the theme's font; a changed value in red, say.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::SetCellFont(int32 iNode, int32 iColumn, Ptr<CFont> pFont)
{
	if (iColumn == 0)
	{
		mpTree->SetNodeFont(iNode, pFont);
	}
	else
	{
		mpTree->SetCellFont(iNode, (size)(iColumn - 1), pFont);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Edits a node's cell in an editable column (not the name): the field is
// over it, with its text selected.  False if it can't be edited.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeTable::BeginEdit(int32 iNode, int32 iColumn)
{
	if ((iColumn < 0) || (iColumn >= NumColumns()) || !mabEditable.Get(iColumn) || (iNode < 0) || (iNode >= mpTree->NumNodes()))
	{
		return false;
	}
	EndEdit(true);
	mpTree->SelectNode(iNode);
	miEditNode = iNode;
	miEditColumn = iColumn;
	mpField->SetText(GetCellText(iNode, iColumn));
	mpField->ClearUndo();
	mpField->SelectAll();
	mpWindow->GetFocus()->SetFocus(mpField);
	LayoutField();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Ends the edit: with bKeep and a changed text, the edit event says what
// was typed.  The tree has the keys again.  False if nothing was edited.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeTable::EndEdit(bool bKeep)
{
	int32	iNode;
	int32	iColumn;
	bool	bChanged;

	if ((miEditNode == TREE_NO_NODE) || mbEnding)
	{
		return false;
	}
	mbEnding = true;
	iNode = miEditNode;
	iColumn = miEditColumn;
	bChanged = bKeep && (strcmp(mpField->GetText(), GetCellText(iNode, iColumn)) != 0);
	miEditNode = TREE_NO_NODE;
	miEditColumn = -1;
	if (mpField->IsFocussed())
	{
		mpWindow->GetFocus()->SetFocus(mpTree);
	}
	LayoutField();
	if (bChanged)
	{
		miEditedNode = iNode;
		miEditedColumn = iColumn;
		mszEditedText.Set(mpField->GetText());
		mcEdited.Call(this);
	}
	mbEnding = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a node's cell is, in window client space: its name's from where
// the name starts.  False if its row isn't laid out or it has no such
// column.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeTable::GetCellBounds(int32 iNode, int32 iColumn, CInt32Rect* pcDest)
{
	CInt32Rect	cTable;
	int32		iTop;
	int32		iHeight;
	int32		iLeft;

	if ((iColumn < 0) || ((size)iColumn >= maiLefts.NumElements()) || !GetContainerBounds(&cTable) ||
		!mpTree->GetRowBounds(mpTree->FindNodeRow(iNode), &iTop, &iHeight))
	{
		return false;
	}
	iLeft = cTable.GetLeft() + maiLefts.GetValue(iColumn) + ((iColumn == 0) ? mpTree->GetNameLeft(iNode) : 0);
	pcDest->Init(iLeft, iTop, cTable.GetLeft() + maiLefts.GetValue(iColumn) + maiLaidWidths.GetValue(iColumn), iTop + iHeight);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// As wide as its columns, and as tall as its captions and a few rows.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::SetRequiredSize(void)
{
	size	i;

	msRequiredSize.Init(0, GetHeaderHeight() + TREE_TABLE_VISIBLE_ROWS * ((int32)mpTheme->GetFont()->GetLineHeight() + mpTheme->GetMetric(TM_TreeRowPadding)));
	for (i = 0; i < maiWidths.NumElements(); i++)
	{
		msRequiredSize.x += maiWidths.GetValue(i);
	}
	AddInsets(&msRequiredSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The captions over their columns, a sash at each one's right, the rows
// under them in the scroll pane, the tree told where the columns are, and
// the field over the cell being edited.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	CArrayInt	aiLefts;
	CArrayInt	aiWidths;
	SInt32Vec2	sOrigin;
	SInt32Vec2	sArea;
	SInt32Vec2	sCaption;
	int32		iHeader;
	int32		iLeft;
	int32		iWidth;
	int32		iSash;
	int32		iPadding;
	size		i;

	sOrigin = sPosition;
	sArea = sAreaSize;
	InsetArea(&sOrigin, &sArea);
	SetContainerBounds(sOrigin, sArea);
	iHeader = GetHeaderHeight();
	iHeader = (iHeader > sArea.y) ? sArea.y : iHeader;
	iSash = mpTheme->GetMetric(TM_SplitSashSize);
	iPadding = mpTheme->GetMetric(TM_FieldPaddingX);

	//Each column as dragged; the last to the right edge.
	maiLefts.ReInit();
	maiLaidWidths.ReInit();
	iLeft = 0;
	for (i = 0; i < maiWidths.NumElements(); i++)
	{
		iWidth = (i + 1 < maiWidths.NumElements()) ? maiWidths.GetValue(i) : sArea.x - iLeft;
		iWidth = (iWidth < TREE_TABLE_LEAST_WIDTH) ? TREE_TABLE_LEAST_WIDTH : iWidth;
		maiLefts.Add(iLeft);
		maiLaidWidths.Add(iWidth);
		sCaption = maCaptions.Get(i)->GetDesiredSize();
		sCaption.x = (sCaption.x < iWidth - 2 * iPadding) ? sCaption.x : iWidth - 2 * iPadding;
		maCaptions.Get(i)->Layout(SInt32Vec2(sOrigin.x + iLeft + iPadding, sOrigin.y + (iHeader - TREE_TABLE_HEADER_LINE - sCaption.y) / 2), sCaption);
		if (i > 0)
		{
			LayoutComponentInArea(maSashes.Get(i - 1), SInt32Vec2(sOrigin.x + iLeft - iSash / 2, sOrigin.y), SInt32Vec2(iSash, iHeader - TREE_TABLE_HEADER_LINE));
		}
		iLeft += iWidth;
	}
	LayoutComponentInArea(mpHeaderBackground, sOrigin, SInt32Vec2(sArea.x, iHeader));
	LayoutComponentInArea(mpHeaderLine, SInt32Vec2(sOrigin.x, sOrigin.y + iHeader - TREE_TABLE_HEADER_LINE), SInt32Vec2(sArea.x, (iHeader >= TREE_TABLE_HEADER_LINE) ? TREE_TABLE_HEADER_LINE : 0));

	//The tree's columns are the cells', after the name.
	aiLefts.Init();
	aiWidths.Init();
	for (i = 1; i < maiLefts.NumElements(); i++)
	{
		aiLefts.Add(maiLefts.GetValue(i) + iPadding);
		aiWidths.Add(maiLaidWidths.GetValue(i) - 2 * iPadding);
	}
	mpTree->SetColumnBounds((maiLaidWidths.NumElements() > 0) ? maiLaidWidths.GetValue(0) - iPadding : 0, &aiLefts, &aiWidths);
	aiWidths.Kill();
	aiLefts.Kill();

	LayoutComponentInArea(mpScroll, SInt32Vec2(sOrigin.x, sOrigin.y + iHeader), SInt32Vec2(sArea.x, sArea.y - iHeader));
	SetActualBounds(sPosition, sAreaSize);
	LayoutField();
}


//////////////////////////////////////////////////////////////////////////
//
// The field while a cell is edited; a column's sash only between columns.
//
//////////////////////////////////////////////////////////////////////////
bool CTreeTable::IsChildDrawn(Ptr<CComponent> pChild)
{
	if (pChild == mpField)
	{
		return miEditNode != TREE_NO_NODE;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The column left of the sash is wider or narrower by iDelta, no
// narrower than TREE_TABLE_LEAST_WIDTH.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::SashMoved(CSash* pcSash, int32 iDelta)
{
	size	i;
	int32	iWidth;

	for (i = 0; i < maSashes.NumElements(); i++)
	{
		if (&maSashes.Get(i) == pcSash)
		{
			iWidth = maiWidths.GetValue(i) + iDelta;
			maiWidths.SetValue(i, (iWidth < TREE_TABLE_LEAST_WIDTH) ? TREE_TABLE_LEAST_WIDTH : iWidth);
			LayoutChanged();
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Enter or a double click on a node: its editable cell under the pointer
// is edited, else its first editable cell.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::Activated(void)
{
	int32	iColumn;

	if (BeginEdit(mpTree->GetSelectedNode(), FindPointerColumn()))
	{
		return;
	}
	for (iColumn = 0; iColumn < NumColumns(); iColumn++)
	{
		if (mabEditable.Get(iColumn))
		{
			BeginEdit(mpTree->GetSelectedNode(), iColumn);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The column of the selected node's cell under the pointer, -1 if the
// pointer isn't on one.
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeTable::FindPointerColumn(void)
{
	SInt32Vec2	sPointer;
	CInt32Rect	cCell;
	int32		iColumn;

	if (!mpWindow->GetFocus()->IsPointerIn())
	{
		return -1;
	}
	sPointer = mpWindow->GetFocus()->GetPointer();
	for (iColumn = 0; iColumn < NumColumns(); iColumn++)
	{
		if (GetCellBounds(mpTree->GetSelectedNode(), iColumn, &cCell) && cCell.ContainsPoint(sPointer.x, sPointer.y))
		{
			return iColumn;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The field's ended event: escape doesn't keep what was typed, anything
// else does.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::FieldEnded(CCellField* pcField, int32 eEnd)
{
	EndEdit(eEnd != CFE_Cancel);
}


//////////////////////////////////////////////////////////////////////////
//
// The field follows the cell it is over.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::Scrolled(CScrollPane* pcScroll, SInt32Vec2 sPosition)
{
	LayoutField();
}


//////////////////////////////////////////////////////////////////////////
//
// The field over the cell being edited, or nowhere.
//
//////////////////////////////////////////////////////////////////////////
void CTreeTable::LayoutField(void)
{
	CInt32Rect	cCell;

	if ((miEditNode != TREE_NO_NODE) && GetCellBounds(miEditNode, miEditColumn, &cCell))
	{
		mpField->Layout(SInt32Vec2(cCell.GetLeft(), cCell.GetTop()), SInt32Vec2(cCell.GetWidth(), cCell.GetHeight()));
	}
	else
	{
		mpField->Layout(SInt32Vec2(0, 0), SInt32Vec2(0, 0));
	}
	Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTreeTable::GetHeaderHeight(void)
{
	return (int32)mpTheme->GetFont()->GetLineHeight() + 2 * TREE_TABLE_PADDING_Y + TREE_TABLE_HEADER_LINE;
}


int32 CTreeTable::NumColumns(void) const { return (int32)maiWidths.NumElements(); }
Ptr<CTreeContainer> CTreeTable::GetTree(void) const { return mpTree; }
bool CTreeTable::IsEditing(void) const { return miEditNode != TREE_NO_NODE; }
Ptr<CCellField> CTreeTable::GetEditField(void) const { return mpField; }
CComponentEvent<CTreeTable>* CTreeTable::GetEditEvent(void) { return &mcEdited; }
int32 CTreeTable::GetEditedNode(void) const { return miEditedNode; }
int32 CTreeTable::GetEditedColumn(void) const { return miEditedColumn; }
const char* CTreeTable::GetEditedText(void) const { return mszEditedText.Text(); }
Ptr<CLabel> CTreeTable::GetCaption(int32 iColumn) { return ((iColumn >= 0) && (iColumn < NumColumns())) ? maCaptions.Get(iColumn) : Ptr<CLabel>(); }
Ptr<CSash> CTreeTable::GetSash(int32 iColumn) { return ((iColumn >= 0) && ((size)iColumn < maSashes.NumElements())) ? maSashes.Get(iColumn) : Ptr<CSash>(); }

