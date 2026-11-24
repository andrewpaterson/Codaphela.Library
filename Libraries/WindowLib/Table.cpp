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
#include <string.h>
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Font.h"
#include "Window.h"
#include "Focus.h"
#include "FillContainer.h"
#include "Table.h"


//How tall the line under the selected cell is.
#define TABLE_CELL_MARK_HEIGHT	2

//Above and below a cell's text, and a caption's.
#define TABLE_CELL_PADDING_Y	4

//The line between the captions and the rows.
#define TABLE_HEADER_LINE		1


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTableList::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, CTable* pcTable)
{
	PreInit();

	CListContainer::Init(pWindow, pTheme);
	mpcTable = pcTable;
	AddInputAction(CIC_Enter, this, &CTableList::Enter);
	AddInputAction(CIC_Space, this, &CTableList::Space);
	AddInputAction(CIC_Left, this, &CTableList::Left);
	AddInputAction(CIC_Right, this, &CTableList::Right);
	AddInputAction(IAF_Printable, NULL, this, &CTableList::Typed);
	AddInputAction(IAF_Chord, "Alt+Cursor Up", this, &CTableList::MoveRowUp);
	AddInputAction(IAF_Chord, "Alt+Cursor Down", this, &CTableList::MoveRowDown);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTableList::Class(void)
{
	CListContainer::Class();

	U_Pointer(mpcTable);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTableList::Free(void)
{
	CListContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
// A press selects the row, as a list's, and then the cell is the table's.
//
//////////////////////////////////////////////////////////////////////////
void CTableList::ActivateComponent(int x, int y)
{
	mpcTable->EndEdit(true);
	mpcTable->ListPressed(x, y);
}


void CTableList::Enter(void) { mpcTable->BeginEdit(); }
void CTableList::Space(void) { mpcTable->ToggleTick(); }
void CTableList::Left(void) { mpcTable->MoveColumn(-1, false); }
void CTableList::Right(void) { mpcTable->MoveColumn(1, false); }
void CTableList::MoveRowUp(SComponentInputEvent* psEvent) { mpcTable->MoveSelectedRow(-1); }
void CTableList::MoveRowDown(SComponentInputEvent* psEvent) { mpcTable->MoveSelectedRow(1); }


//////////////////////////////////////////////////////////////////////////
//
// Typing starts an edit of the selected cell with what was typed; the
// space bar is a tick's.
//
//////////////////////////////////////////////////////////////////////////
void CTableList::Typed(SComponentInputEvent* psEvent)
{
	if (psEvent->cPrintable != ' ')
	{
		mpcTable->Typed(psEvent->cPrintable);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Enter edits the selected cell, so it is the list's while there is one.
//
//////////////////////////////////////////////////////////////////////////
bool CTableList::WantsEnter(void)
{
	return GetSelected() >= 0;
}


void CTableList::RowSelected(int32 iRow) { mpcTable->ListRowSelected(iRow); }


//////////////////////////////////////////////////////////////////////////
//
// A list activates its selected row for enter, the space bar and a
// release over it.  A table's rows aren't activated: enter edits the
// selected cell and the space bar changes a tick (Enter, Space), and a
// press has done its work when it is made.
//
//////////////////////////////////////////////////////////////////////////
void CTableList::RowActivated(int32 iRow)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTableCombo::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, CTable* pcTable)
{
	PreInit();

	CComboBox::Init(pWindow, pTheme);
	mpcTable = pcTable;
	AddInputAction(CIC_ClearSelection, this, &CTableCombo::Cancel);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTableCombo::Class(void)
{
	CComboBox::Class();

	U_Pointer(mpcTable);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTableCombo::Free(void)
{
	CComboBox::Free();
}


bool CTableCombo::WantsEscape(void) { return true; }
bool CTableCombo::IsTabStop(void) const { return false; }
void CTableCombo::Cancel(void) { mpcTable->EndEdit(false); }


//////////////////////////////////////////////////////////////////////////
//
// Its choices closed: the edit is over, whether one was chosen (the
// table has been told, before this or after) or not.
//
//////////////////////////////////////////////////////////////////////////
void CTableCombo::PopupClosed(Ptr<CComponent> pContent)
{
	CComboBox::PopupClosed(pContent);
	mpcTable->EndEdit(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme)
{
	PreInit();

	CContainer::Init(pWindow);
	mpTheme = pTheme;
	maHeaders.Init();
	maCells.Init();
	maszCaptions.Init();
	maiKinds.Init();
	maiWeights.Init();
	maiColumnWidths.Init();
	maiColumnEditable.Init();
	maszChoices.Init();
	maiChoiceColumns.Init();
	maszCells.Init();
	maiCellEditable.Init();
	mcChanged.Init();
	mcSelectionChanged.Init();
	mcOrderChanged.Init();
	miSelectedColumn = -1;
	miEditRow = -1;
	miEditColumn = -1;
	miChangedRow = -1;
	miChangedColumn = -1;
	miChoiceRow = -1;
	miChoiceColumn = -1;
	miVisibleRows = 6;
	mbRowsMovable = false;
	mbEnding = false;
	SetSizedByContent(true);

	mpHeaderBackground = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_Panel)));
	mpHeaderBackground->SetAlignment(CSH_Stretch, CSV_Stretch);
	CContainer::AddComponent(mpHeaderBackground);
	mpHeaderLine = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_Separator)));
	mpHeaderLine->SetAlignment(CSH_Stretch, CSV_Stretch);
	CContainer::AddComponent(mpHeaderLine);
	mpList = OMalloc<CTableList>(pWindow, pTheme, this);
	mpList->SetAlignment(CSH_Stretch, CSV_Top);
	mpScroll = OMalloc<CScrollPane>(pWindow, pTheme);
	mpScroll->SetContent(mpList);
	mpScroll->SetPolicy(SBO_Horizontal, SPP_Never);
	mpScroll->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpScroll->GetScrollEvent()->Add(this, &CTable::Scrolled);
	CContainer::AddComponent(mpScroll);
	mpCellMark = OMalloc<CCanvas>(pWindow, OMalloc<CClearCanvasDraw>(pTheme->GetColour(TC_Accent)));
	mpCellMark->SetAlignment(CSH_Stretch, CSV_Stretch);
	CContainer::AddComponent(mpCellMark);
	mpField = OMalloc<CCellField>(pWindow, pTheme, "");
	mpField->GetEndedEvent()->Add(this, &CTable::FieldEnded);
	mpField->SetAlignment(CSH_Stretch, CSV_Stretch);
	CContainer::AddComponent(mpField);
	mpCombo = OMalloc<CTableCombo>(pWindow, pTheme, this);
	mpCombo->SetAlignment(CSH_Stretch, CSV_Stretch);
	mpCombo->GetSelectionEvent()->Add(this, &CTable::ComboChosen);
	CContainer::AddComponent(mpCombo);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::Class(void)
{
	CContainer::Class();

	M_Pointer(mpTheme);
	M_Pointer(mpHeaderBackground);
	M_Pointer(mpHeaderLine);
	M_Embedded(maHeaders);
	M_Pointer(mpScroll);
	M_Pointer(mpList);
	M_Pointer(mpCellMark);
	M_Pointer(mpField);
	M_Pointer(mpCombo);
	M_Embedded(maCells);
	U_Unknown(CArrayChars, maszCaptions);
	U_Unknown(CArrayInt, maiKinds);
	U_Unknown(CArrayInt, maiWeights);
	U_Unknown(CArrayInt, maiColumnWidths);
	U_Unknown(CArrayInt, maiColumnEditable);
	U_Unknown(CArrayChars, maszChoices);
	U_Unknown(CArrayInt, maiChoiceColumns);
	U_Unknown(CArrayChars, maszCells);
	U_Unknown(CArrayInt, maiCellEditable);
	U_Unknown(CComponentEvent<CTable>, mcChanged);
	U_Unknown(CComponentEvent<CTable>, mcSelectionChanged);
	U_Unknown(CComponentEvent<CTable>, mcOrderChanged);
	U_Int32(miSelectedColumn);
	U_Int32(miEditRow);
	U_Int32(miEditColumn);
	U_Int32(miChangedRow);
	U_Int32(miChangedColumn);
	U_Int32(miChoiceRow);
	U_Int32(miChoiceColumn);
	U_Int32(miVisibleRows);
	U_Bool(mbRowsMovable);
	U_Bool(mbEnding);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::Free(void)
{
	if (mpScroll.IsNotNull())
	{
		mpScroll->GetScrollEvent()->Remove(this);
	}
	if (mpCombo.IsNotNull())
	{
		mpCombo->GetSelectionEvent()->Remove(this);
	}
	if (mpField.IsNotNull())
	{
		mpField->GetEndedEvent()->Remove(this);
	}
	mcOrderChanged.Kill();
	mcSelectionChanged.Kill();
	mcChanged.Kill();
	maiCellEditable.Kill();
	maszCells.Kill();
	maiChoiceColumns.Kill();
	maszChoices.Kill();
	maiColumnEditable.Kill();
	maiColumnWidths.Kill();
	maiWeights.Kill();
	maiKinds.Kill();
	maszCaptions.Kill();
	CContainer::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTable::Save(CObjectWriter* pcFile) { return false; }
bool CTable::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// A column, after those there are; its number.  Columns are added before
// rows.  A column with a weight shares the table's spare width; one with
// a width in characters is at least that wide.
//
//////////////////////////////////////////////////////////////////////////
int32 CTable::AddColumn(const char* szCaption, ETableColumn eKind, int iWidthWeight, int32 iWidthInCharacters)
{
	Ptr<CFillContainer>	pPadding;
	Ptr<CLabel>			pLabel;

	if (NumRows() > 0)
	{
		gcLogger.Error2(__METHOD__, " A table's columns must be added before its rows.", NULL);
		return -1;
	}
	maszCaptions.Add(szCaption);
	maiKinds.Add((int)eKind);
	maiWeights.Add(iWidthWeight);
	maiColumnWidths.Add(iWidthInCharacters);
	maiColumnEditable.Add(1);

	pPadding = OMalloc<CFillContainer>(mpWindow);
	pPadding->SetInsets(TABLE_CELL_PADDING_Y, mpTheme->GetMetric(TM_FieldPaddingX), TABLE_CELL_PADDING_Y, mpTheme->GetMetric(TM_FieldPaddingX));
	pLabel = OMalloc<CLabel>(mpWindow, mpTheme->GetFont(), szCaption);
	pLabel->SetAlignment(CSH_Left, CSV_Center);
	pPadding->AddComponent(pLabel);
	pPadding->SetAlignment(CSH_Stretch, CSV_Stretch);
	maHeaders.Add(pPadding);
	CContainer::AddComponent(pPadding);
	InvalidateLayout();
	return NumColumns() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::AddColumnChoice(int32 iColumn, const char* szChoice)
{
	if ((iColumn >= 0) && (iColumn < NumColumns()))
	{
		maszChoices.Add(szChoice);
		maiChoiceColumns.Add(iColumn);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::SetColumnEditable(int32 iColumn, bool bEditable)
{
	if ((iColumn >= 0) && (iColumn < NumColumns()))
	{
		maiColumnEditable.SetValue((size)iColumn, bEditable ? 1 : 0);
	}
}


int32 CTable::NumColumns(void) const { return (int32)maiKinds.NumElements(); }
int32 CTable::NumRows(void) const { return (NumColumns() > 0) ? (int32)(maszCells.NumElements() / (size)NumColumns()) : 0; }
ETableColumn CTable::GetColumnKind(int32 iColumn) { return (ETableColumn)maiKinds.GetValue((size)iColumn); }
size CTable::CellIndex(int32 iRow, int32 iColumn) { return (size)iRow * (size)NumColumns() + (size)iColumn; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTable::AddRow(void)
{
	return InsertRow(NumRows());
}


//////////////////////////////////////////////////////////////////////////
//
// A row of empty cells before row iBefore (after the last, if it is the
// number of rows); its number, -1 if there is no such place or the table
// has no columns.  The selected cell stays the cell it was.
//
//////////////////////////////////////////////////////////////////////////
int32 CTable::InsertRow(int32 iBefore)
{
	CArrayChars	aszCells;
	CArrayInt	aiEditable;
	int32		iSelected;
	int32		iRows;
	int32		iOld;
	int32		i;
	int32		j;

	iRows = NumRows();
	if ((NumColumns() == 0) || (iBefore < 0) || (iBefore > iRows))
	{
		return -1;
	}
	EndEdit(true);
	iSelected = GetSelectedRow();
	aszCells.Init();
	aiEditable.Init();
	for (i = 0; i < (int32)maszCells.NumElements(); i++)
	{
		aszCells.Add(maszCells.GetText((size)i));
		aiEditable.Add(maiCellEditable.GetValue((size)i));
	}
	maszCells.ReInit();
	maiCellEditable.ReInit();
	for (i = 0; i <= iRows; i++)
	{
		for (j = 0; j < NumColumns(); j++)
		{
			if (i == iBefore)
			{
				maszCells.Add("");
				maiCellEditable.Add(1);
			}
			else
			{
				iOld = ((i < iBefore) ? i : i - 1) * NumColumns() + j;
				maszCells.Add(aszCells.GetText((size)iOld));
				maiCellEditable.Add(aiEditable.GetValue((size)iOld));
			}
		}
	}
	aiEditable.Kill();
	aszCells.Kill();
	Rebuild();
	if (iSelected >= iBefore)
	{
		SelectCell(iSelected + 1, miSelectedColumn);
	}
	return iBefore;
}


//////////////////////////////////////////////////////////////////////////
//
// A row taken out.  The row after it (or the last, if it was the last)
// is selected if it was the selected row.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::RemoveRow(int32 iRow)
{
	int32	iSelected;
	int32	iColumn;
	int32	j;

	if ((iRow < 0) || (iRow >= NumRows()))
	{
		return false;
	}
	EndEdit(false);
	iSelected = GetSelectedRow();
	iColumn = miSelectedColumn;
	for (j = NumColumns() - 1; j >= 0; j--)
	{
		maszCells.Remove(CellIndex(iRow, j));
		maiCellEditable.RemoveAt(CellIndex(iRow, j));
	}
	Rebuild();
	if (iSelected > iRow)
	{
		iSelected--;
	}
	if (iSelected >= NumRows())
	{
		iSelected = NumRows() - 1;
	}
	if (iSelected >= 0)
	{
		SelectCell(iSelected, iColumn);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A row moved to be row iTo, the rows between moving one place the other
// way.  The selected cell stays the cell it was.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::MoveRow(int32 iFrom, int32 iTo)
{
	CArrayChars	aszRow;
	CArrayInt	aiRow;
	int32		iSelected;
	int32		iColumn;
	int32		iStep;
	int32		i;
	int32		j;

	if ((iFrom < 0) || (iFrom >= NumRows()) || (iTo < 0) || (iTo >= NumRows()))
	{
		return false;
	}
	if (iFrom == iTo)
	{
		return true;
	}
	EndEdit(true);
	iSelected = GetSelectedRow();
	iColumn = miSelectedColumn;
	aszRow.Init();
	aiRow.Init();
	for (j = 0; j < NumColumns(); j++)
	{
		aszRow.Add(maszCells.GetText(CellIndex(iFrom, j)));
		aiRow.Add(maiCellEditable.GetValue(CellIndex(iFrom, j)));
	}
	iStep = (iTo > iFrom) ? 1 : -1;
	for (i = iFrom; i != iTo; i += iStep)
	{
		for (j = 0; j < NumColumns(); j++)
		{
			maszCells.Get(CellIndex(i, j))->Set(maszCells.GetText(CellIndex(i + iStep, j)));
			maiCellEditable.SetValue(CellIndex(i, j), maiCellEditable.GetValue(CellIndex(i + iStep, j)));
		}
	}
	for (j = 0; j < NumColumns(); j++)
	{
		maszCells.Get(CellIndex(iTo, j))->Set(aszRow.GetText((size)j));
		maiCellEditable.SetValue(CellIndex(iTo, j), aiRow.GetValue((size)j));
	}
	aiRow.Kill();
	aszRow.Kill();
	Rebuild();
	if (iSelected == iFrom)
	{
		iSelected = iTo;
	}
	else if ((iSelected > iFrom) && (iSelected <= iTo))
	{
		iSelected--;
	}
	else if ((iSelected < iFrom) && (iSelected >= iTo))
	{
		iSelected++;
	}
	if (iSelected >= 0)
	{
		SelectCell(iSelected, iColumn);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::RemoveAllRows(void)
{
	EndEdit(false);
	maszCells.ReInit();
	maiCellEditable.ReInit();
	Rebuild();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::SetCellText(int32 iRow, int32 iColumn, const char* szText)
{
	if ((iRow >= 0) && (iRow < NumRows()) && (iColumn >= 0) && (iColumn < NumColumns()))
	{
		maszCells.Get(CellIndex(iRow, iColumn))->Set(szText);
		UpdateCell(iRow, iColumn);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTable::GetCellText(int32 iRow, int32 iColumn)
{
	if ((iRow >= 0) && (iRow < NumRows()) && (iColumn >= 0) && (iColumn < NumColumns()))
	{
		return maszCells.GetText(CellIndex(iRow, iColumn));
	}
	return NULL;
}


void CTable::SetCellTicked(int32 iRow, int32 iColumn, bool bTicked) { SetCellText(iRow, iColumn, bTicked ? "1" : ""); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTable::IsCellTicked(int32 iRow, int32 iColumn)
{
	char*	szText;

	szText = GetCellText(iRow, iColumn);
	return szText && (szText[0] != '\0');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::SetCellEditable(int32 iRow, int32 iColumn, bool bEditable)
{
	if ((iRow >= 0) && (iRow < NumRows()) && (iColumn >= 0) && (iColumn < NumColumns()))
	{
		maiCellEditable.SetValue(CellIndex(iRow, iColumn), bEditable ? 1 : 0);
		UpdateCell(iRow, iColumn);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the user can change a cell: it and its column both let them.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::IsCellEditable(int32 iRow, int32 iColumn)
{
	return (iRow >= 0) && (iRow < NumRows()) && (iColumn >= 0) && (iColumn < NumColumns()) && (maiColumnEditable.GetValue((size)iColumn) != 0) &&
		   (maiCellEditable.GetValue(CellIndex(iRow, iColumn)) != 0);
}


bool CTable::CanEdit(int32 iRow, int32 iColumn) { return IsCellEditable(iRow, iColumn); }


//////////////////////////////////////////////////////////////////////////
//
// What shows a cell: a label in its padding, or a check box in the
// middle of its own.  It is at least as wide as its column's caption (and
// as the column's width in characters, if it was given one), so a column
// of ticks has room for its caption.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CTable::CreateCell(int32 iRow, int32 iColumn)
{
	Ptr<CFillContainer>	pPadding;
	Ptr<CLabel>			pLabel;

	pPadding = OMalloc<CFillContainer>(mpWindow);
	pPadding->SetAlignment(CSH_Stretch, CSV_Stretch);
	if (GetColumnKind(iColumn) != TCK_Tick)
	{
		pPadding->SetInsets(TABLE_CELL_PADDING_Y, mpTheme->GetMetric(TM_FieldPaddingX), TABLE_CELL_PADDING_Y, mpTheme->GetMetric(TM_FieldPaddingX));
		pLabel = OMalloc<CLabel>(mpWindow, mpTheme->GetFont(), GetCellText(iRow, iColumn));
		pLabel->SetAlignment(CSH_Left, CSV_Center);
		pPadding->AddComponent(pLabel);
	}
	FillCell(pPadding, iRow, iColumn);
	return pPadding;
}


//////////////////////////////////////////////////////////////////////////
//
// What shows a cell made to show what the cell has now: its label's
// text; or its check box, which a tick that can't be changed and isn't
// ticked hasn't (a cell that has no tick to give is empty).  Its width is
// what it needs, or its caption's if that is more.
//
//////////////////////////////////////////////////////////////////////////
void CTable::FillCell(Ptr<CComponent> pCell, int32 iRow, int32 iColumn)
{
	Ptr<CCheckBox>	pCheckBox;
	int32			iWidth;
	int32			iNeeded;

	if (GetColumnKind(iColumn) == TCK_Tick)
	{
		pCell->RemoveAllComponents();
		if (IsCellEditable(iRow, iColumn) || IsCellTicked(iRow, iColumn))
		{
			pCheckBox = OMalloc<CCheckBox>(mpWindow, mpTheme, "", IsCellTicked(iRow, iColumn));
			pCheckBox->SetAlignment(CSH_Center, CSV_Center);
			pCheckBox->SetCanGetFocus(false);
			pCell->AddComponent(pCheckBox);
		}
	}
	else
	{
		((Ptr<CLabel>)pCell->GetComponent(0))->SetText(GetCellText(iRow, iColumn));
	}

	pCell->SetDesiredSize(-1, -1);
	iNeeded = ((Ptr<CFillContainer>)pCell)->GetRequiredSize().x;
	iWidth = CContainer::GetMeasuredSize(maHeaders.Get((size)iColumn)).x;
	if (maiColumnWidths.GetValue((size)iColumn) * (int32)mpTheme->GetFont()->GetAverageWidth() + 2 * mpTheme->GetMetric(TM_FieldPaddingX) > iWidth)
	{
		iWidth = maiColumnWidths.GetValue((size)iColumn) * (int32)mpTheme->GetFont()->GetAverageWidth() + 2 * mpTheme->GetMetric(TM_FieldPaddingX);
	}
	pCell->SetDesiredSize((iNeeded > iWidth) ? iNeeded : iWidth, (GetColumnKind(iColumn) == TCK_Tick) ? GetRowHeight() : -1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::UpdateCell(int32 iRow, int32 iColumn)
{
	Ptr<CComponent>	pCell;

	pCell = GetCellComponent(iRow, iColumn);
	if (pCell.IsNotNull())
	{
		FillCell(pCell, iRow, iColumn);
		InvalidateLayout();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The list's rows made again from the cells, after rows were added, taken
// out or moved.  The list's selection is lost: the caller selects again.
//
//////////////////////////////////////////////////////////////////////////
void CTable::Rebuild(void)
{
	Ptr<CComponent>	pCell;
	int32			iRow;
	int32			i;
	int32			j;

	miChoiceRow = -1;
	miChoiceColumn = -1;
	mpList->RemoveAllComponents();
	maCells.RemoveAll();
	for (i = 0; i < NumRows(); i++)
	{
		iRow = mpList->AddRow();
		for (j = 0; j < NumColumns(); j++)
		{
			pCell = CreateCell(i, j);
			maCells.Add(pCell);
			mpList->AddToRow(iRow, pCell, j, 1, maiWeights.GetValue((size)j));
		}
	}
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The edit field's ended event: what was typed is kept (or not), and the
// selected cell moves as the key that ended it says.
//
//////////////////////////////////////////////////////////////////////////
void CTable::FieldEnded(CCellField* pcField, int32 eEnd)
{
	switch (eEnd)
	{
	case CFE_Cancel:
		EndEdit(false);
		break;
	case CFE_Next:
		EditNext(1);
		break;
	case CFE_Previous:
		EditNext(-1);
		break;
	default:
		EndEdit(true);
		if ((eEnd == CFE_Up) || (eEnd == CFE_Down))
		{
			SelectCell(GetSelectedRow() + ((eEnd == CFE_Up) ? -1 : 1), GetSelectedColumn());
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cell made the selected cell.  False if there is no such cell.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::SelectCell(int32 iRow, int32 iColumn)
{
	bool	bColumn;

	if ((iRow < 0) || (iRow >= NumRows()) || (iColumn < 0) || (iColumn >= NumColumns()))
	{
		return false;
	}
	bColumn = (iColumn != miSelectedColumn);
	miSelectedColumn = iColumn;
	if (iRow != mpList->GetSelected())
	{
		//The list tells the table (ListRowSelected), which tells its owner.
		mpList->Select(iRow);
	}
	else if (bColumn)
	{
		LayoutOverlays();
		mcSelectionChanged.Call(this);
	}
	mpList->ShowRow(iRow);
	return true;
}


int32 CTable::GetSelectedRow(void) { return mpList->GetSelected(); }
int32 CTable::GetSelectedColumn(void) { return (mpList->GetSelected() >= 0) ? miSelectedColumn : -1; }
bool CTable::IsEditing(void) const { return miEditRow >= 0; }
Ptr<CCellField> CTable::GetEditField(void) const { return mpField; }
Ptr<CTableCombo> CTable::GetEditCombo(void) const { return mpCombo; }
Ptr<CTableList> CTable::GetList(void) const { return mpList; }
Ptr<CScrollPane> CTable::GetScrollPane(void) const { return mpScroll; }
void CTable::SetRowsMovable(bool bRowsMovable) { mbRowsMovable = bRowsMovable; }
CComponentEvent<CTable>* CTable::GetChangeEvent(void) { return &mcChanged; }
CComponentEvent<CTable>* CTable::GetSelectionEvent(void) { return &mcSelectionChanged; }
CComponentEvent<CTable>* CTable::GetOrderEvent(void) { return &mcOrderChanged; }
int32 CTable::GetChangedRow(void) const { return miChangedRow; }
int32 CTable::GetChangedColumn(void) const { return miChangedColumn; }
bool CTable::BeginEdit(void) { return BeginEdit(NULL); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::SetVisibleRows(int32 iVisibleRows)
{
	miVisibleRows = iVisibleRows;
	InvalidateLayout();
}


//////////////////////////////////////////////////////////////////////////
//
// The list's selected row changed (or went): the table's owner is told.
// A row selected with no column yet has its first.
//
//////////////////////////////////////////////////////////////////////////
void CTable::ListRowSelected(int32 iRow)
{
	if ((iRow >= 0) && (miSelectedColumn < 0) && (NumColumns() > 0))
	{
		miSelectedColumn = 0;
	}
	LayoutOverlays();
	mcSelectionChanged.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The selected cell's editor shown over it, with focus: a text field
// with szText (what was typed to start the edit), or all of what the cell
// has, selected, if it is NULL; or a combo box of the column's choices,
// open.  False if there is no selected cell, it can't be edited, or it is
// a tick.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::BeginEdit(const char* szText)
{
	int32	iRow;
	int32	iColumn;
	int32	iChosen;
	size	i;

	iRow = GetSelectedRow();
	iColumn = GetSelectedColumn();
	if (!CanEdit(iRow, iColumn) || (GetColumnKind(iColumn) == TCK_Tick))
	{
		return false;
	}
	EndEdit(true);
	mpList->ShowRow(iRow);
	miEditRow = iRow;
	miEditColumn = iColumn;
	if (GetColumnKind(iColumn) == TCK_Text)
	{
		mpField->SetText(szText ? szText : GetCellText(iRow, iColumn));
		mpField->ClearUndo();
		if (szText)
		{
			mpField->SetCaretPosition(strlen(szText));
		}
		else
		{
			mpField->SelectAll();
		}
		LayoutOverlays();
		InvalidateLayout();
		mpWindow->GetFocus()->SetFocus(mpField);
	}
	else
	{
		mpCombo->RemoveAllItems();
		iChosen = -1;
		for (i = 0; i < maszChoices.NumElements(); i++)
		{
			if (maiChoiceColumns.GetValue(i) == iColumn)
			{
				if (strcmp(maszChoices.GetText(i), GetCellText(iRow, iColumn)) == 0)
				{
					iChosen = (int32)mpCombo->NumItems();
				}
				mpCombo->AddItem(maszChoices.GetText(i));
			}
		}
		mpCombo->Select(iChosen);
		miChoiceRow = iRow;
		miChoiceColumn = iColumn;
		LayoutOverlays();
		InvalidateLayout();
		mpWindow->GetFocus()->SetFocus(mpCombo);
		mpCombo->Open();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The edit ended: what the text field has is the cell's if bKeep, and the
// change event is called if that isn't what the cell had.  (A combo box's
// choice is the cell's when it is chosen: ComboChosen.)  Focus goes back
// to the rows if the editor had it.  False if no cell was being edited.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::EndEdit(bool bKeep)
{
	int32	iRow;
	int32	iColumn;
	bool	bFocussed;

	if (!IsEditing() || mbEnding)
	{
		return false;
	}
	mbEnding = true;
	iRow = miEditRow;
	iColumn = miEditColumn;
	miEditRow = -1;
	miEditColumn = -1;
	//The choices closing gives the combo box focus again, from them: it is asked after.
	bFocussed = mpCombo->IsOpen();
	if (mpCombo->IsOpen())
	{
		mpCombo->Close();
	}
	bFocussed |= mpField->IsFocussed() || mpCombo->IsFocussed();
	if (bKeep && (GetColumnKind(iColumn) == TCK_Text) && (strcmp(mpField->GetText(), GetCellText(iRow, iColumn)) != 0))
	{
		SetCellText(iRow, iColumn, mpField->GetText());
		Changed(iRow, iColumn);
	}
	LayoutOverlays();
	InvalidateLayout();
	if (bFocussed)
	{
		mpWindow->GetFocus()->SetFocus(mpList);
	}
	mbEnding = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The combo box's choice is the cell's: the cell its choices were opened
// for.
//
//////////////////////////////////////////////////////////////////////////
void CTable::ComboChosen(CComboBox* pcCombo)
{
	int32	iRow;
	int32	iColumn;

	//The cell the choices were opened for: the edit may be over already, if they closed before the choice came.
	iRow = miChoiceRow;
	iColumn = miChoiceColumn;
	miChoiceRow = -1;
	miChoiceColumn = -1;
	if ((iRow >= 0) && (iRow < NumRows()) && (iColumn >= 0) && (pcCombo->GetSelected() >= 0))
	{
		if (strcmp(pcCombo->GetSelectedText(), GetCellText(iRow, iColumn)) != 0)
		{
			SetCellText(iRow, iColumn, pcCombo->GetSelectedText());
			Changed(iRow, iColumn);
		}
		EndEdit(false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTable::Changed(int32 iRow, int32 iColumn)
{
	miChangedRow = iRow;
	miChangedColumn = iColumn;
	mcChanged.Call(this);
}


//////////////////////////////////////////////////////////////////////////
//
// The selected cell's tick changed, if it is a tick the user can change.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::ToggleTick(void)
{
	int32	iRow;
	int32	iColumn;

	iRow = GetSelectedRow();
	iColumn = GetSelectedColumn();
	if (!CanEdit(iRow, iColumn) || (GetColumnKind(iColumn) != TCK_Tick))
	{
		return false;
	}
	SetCellTicked(iRow, iColumn, !IsCellTicked(iRow, iColumn));
	Changed(iRow, iColumn);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The selected row moved up (-1) or down (1) a place by the user, if the
// table lets rows be moved: the order event is called.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::MoveSelectedRow(int32 iBy)
{
	int32	iRow;

	iRow = GetSelectedRow();
	if (!mbRowsMovable || (iRow < 0) || (iRow + iBy < 0) || (iRow + iBy >= NumRows()))
	{
		return false;
	}
	MoveRow(iRow, iRow + iBy);
	mcOrderChanged.Call(this);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A press on the rows, at x and y from the list's top left: the cell
// there is selected; a tick is changed; and the cell that was selected
// already is edited.
//
//////////////////////////////////////////////////////////////////////////
void CTable::ListPressed(int x, int y)
{
	SInt32Vec2	sPosition;
	int32		iWasRow;
	int32		iWasColumn;
	int32		iRow;
	int32		iColumn;

	iWasRow = GetSelectedRow();
	iWasColumn = GetSelectedColumn();
	sPosition = mpList->GetPosition();
	iRow = mpList->FindRowAt(x + sPosition.x, y + sPosition.y);
	iColumn = FindColumnAt(x + sPosition.x);
	if ((iRow < 0) || (iColumn < 0))
	{
		return;
	}
	SelectCell(iRow, iColumn);
	if (GetColumnKind(iColumn) == TCK_Tick)
	{
		ToggleTick();
	}
	else if ((iRow == iWasRow) && (iColumn == iWasColumn))
	{
		BeginEdit();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The selected cell moved along its row; with bWrap, on to the next
// row's first cell from the last (and back from the first).
//
//////////////////////////////////////////////////////////////////////////
void CTable::MoveColumn(int32 iBy, bool bWrap)
{
	int32	iRow;
	int32	iColumn;

	iRow = GetSelectedRow();
	iColumn = GetSelectedColumn() + iBy;
	if (iRow < 0)
	{
		return;
	}
	if (bWrap && (iColumn >= NumColumns()) && (iRow + 1 < NumRows()))
	{
		iRow++;
		iColumn = 0;
	}
	else if (bWrap && (iColumn < 0) && (iRow > 0))
	{
		iRow--;
		iColumn = NumColumns() - 1;
	}
	SelectCell(iRow, iColumn);
}


//////////////////////////////////////////////////////////////////////////
//
// A character typed on the rows: the selected cell is edited, starting
// with it.
//
//////////////////////////////////////////////////////////////////////////
void CTable::Typed(char cPrintable)
{
	char	szText[2];

	szText[0] = cPrintable;
	szText[1] = '\0';
	BeginEdit(szText);
}


//////////////////////////////////////////////////////////////////////////
//
// Tab in a cell's editor: what was typed is kept and the next cell (or
// the one before, for -1) is edited, if it is one that is edited so.
//
//////////////////////////////////////////////////////////////////////////
void CTable::EditNext(int32 iBy)
{
	EndEdit(true);
	MoveColumn(iBy, true);
	BeginEdit();
}


//////////////////////////////////////////////////////////////////////////
//
// The rows scrolled: what is laid out over them moves with them.
//
//////////////////////////////////////////////////////////////////////////
void CTable::Scrolled(CScrollPane* pcScroll, SInt32Vec2 sPosition)
{
	LayoutOverlays();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CTable::GetCellComponent(int32 iRow, int32 iColumn)
{
	if ((iRow >= 0) && (iRow < NumRows()) && (iColumn >= 0) && (iColumn < NumColumns()) && (CellIndex(iRow, iColumn) < maCells.NumElements()))
	{
		return maCells.Get(CellIndex(iRow, iColumn));
	}
	return Ptr<CComponent>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CTable::GetHeaderComponent(int32 iColumn)
{
	if ((iColumn >= 0) && (iColumn < NumColumns()))
	{
		return maHeaders.Get((size)iColumn);
	}
	return Ptr<CComponent>();
}


//////////////////////////////////////////////////////////////////////////
//
// Where a cell is, in the window, as the rows were last laid out.  False
// if it hasn't been laid out.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::GetCellBounds(int32 iRow, int32 iColumn, CInt32Rect* pcDest)
{
	int32	iLeft;
	int32	iWidth;
	int32	iTop;
	int32	iHeight;

	if ((iRow < 0) || (iColumn < 0) || !mpList->GetTrackBounds(true, (size)iColumn, &iLeft, &iWidth) || !mpList->GetTrackBounds(false, (size)iRow, &iTop, &iHeight))
	{
		return false;
	}
	pcDest->Init(iLeft, iTop, iLeft + iWidth, iTop + iHeight);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The column at x in the window, -1 if none is.
//
//////////////////////////////////////////////////////////////////////////
int32 CTable::FindColumnAt(int x)
{
	int32	iLeft;
	int32	iWidth;
	int32	i;

	for (i = 0; i < NumColumns(); i++)
	{
		if (mpList->GetTrackBounds(true, (size)i, &iLeft, &iWidth) && (x >= iLeft) && (x < iLeft + iWidth))
		{
			return i;
		}
	}
	return -1;
}


int32 CTable::GetRowHeight(void) { return (int32)mpTheme->GetFont()->GetLineHeight() + 2 * TABLE_CELL_PADDING_Y; }
int32 CTable::GetHeaderHeight(void) { return GetRowHeight() + TABLE_HEADER_LINE; }


//////////////////////////////////////////////////////////////////////////
//
// As wide as its rows (or its captions, if they are wider), and as tall
// as its captions and the number of rows it is to show.
//
//////////////////////////////////////////////////////////////////////////
void CTable::SetRequiredSize(void)
{
	SInt32Vec2	sList;
	int32		iCaptions;
	int32		i;

	sList = CContainer::GetMeasuredSize(mpList);
	iCaptions = 0;
	for (i = 0; i < NumColumns(); i++)
	{
		iCaptions += CContainer::GetMeasuredSize(maHeaders.Get((size)i)).x;
	}
	msRequiredSize.x = (sList.x > iCaptions) ? sList.x : iCaptions;
	msRequiredSize.y = GetHeaderHeight() + miVisibleRows * GetRowHeight();
	AddInsets(&msRequiredSize);
}


//////////////////////////////////////////////////////////////////////////
//
// The captions' strip along the top and the rows' scroll pane under it;
// then each caption over its column, the line under the selected cell,
// and the editor over the cell it edits.
//
//////////////////////////////////////////////////////////////////////////
void CTable::Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize)
{
	SInt32Vec2	sOrigin;
	SInt32Vec2	sArea;
	SInt32Vec2	sRows;
	int32		iHeader;

	sOrigin = sPosition;
	sArea = sAreaSize;
	InsetArea(&sOrigin, &sArea);
	SetContainerBounds(sOrigin, sArea);
	iHeader = GetHeaderHeight();
	if (iHeader > sArea.y)
	{
		iHeader = sArea.y;
	}
	LayoutComponentInArea(mpHeaderBackground, sOrigin, SInt32Vec2(sArea.x, iHeader));
	LayoutComponentInArea(mpHeaderLine, SInt32Vec2(sOrigin.x, sOrigin.y + iHeader - TABLE_HEADER_LINE), SInt32Vec2(sArea.x, (iHeader >= TABLE_HEADER_LINE) ? TABLE_HEADER_LINE : 0));
	sRows.Init(sOrigin.x, sOrigin.y + iHeader);
	LayoutComponentInArea(mpScroll, sRows, SInt32Vec2(sArea.x, sArea.y - iHeader));
	SetActualBounds(sPosition, sAreaSize);
	LayoutOverlays();
}


//////////////////////////////////////////////////////////////////////////
//
// What is laid out from where the rows' columns and rows are: the
// captions, the selected cell's line and the editor.  With no rows there
// are no columns to go by, and the captions share the width by their
// weights.
//
//////////////////////////////////////////////////////////////////////////
void CTable::LayoutOverlays(void)
{
	CInt32Rect	cBounds;
	CInt32Rect	cCell;
	CInt32Rect	cView;
	SInt32Vec2	sPosition;
	SInt32Vec2	sSize;
	int32		iHeader;
	int32		iLeft;
	int32		iWidth;
	int32		iWeights;
	int32		iNeeded;
	int32		iSpare;
	int32		i;
	bool		bShown;

	if (!GetContainerBounds(&cBounds))
	{
		return;
	}
	iHeader = mpHeaderBackground->GetActualSize().y;
	iWeights = 0;
	iNeeded = 0;
	for (i = 0; i < NumColumns(); i++)
	{
		iWeights += maiWeights.GetValue((size)i);
		iNeeded += CContainer::GetMeasuredSize(maHeaders.Get((size)i)).x;
	}
	iSpare = cBounds.GetWidth() - iNeeded;
	iLeft = cBounds.msTopLeft.x;
	for (i = 0; i < NumColumns(); i++)
	{
		if ((NumRows() > 0) && mpList->GetTrackBounds(true, (size)i, &iLeft, &iWidth))
		{
			sPosition.Init(iLeft, cBounds.msTopLeft.y);
		}
		else
		{
			iWidth = CContainer::GetMeasuredSize(maHeaders.Get((size)i)).x;
			if ((iWeights > 0) && (iSpare > 0))
			{
				iWidth += iSpare * maiWeights.GetValue((size)i) / iWeights;
			}
			sPosition.Init(iLeft, cBounds.msTopLeft.y);
			iLeft += iWidth;
		}
		LayoutComponentInArea(maHeaders.Get((size)i), sPosition, SInt32Vec2(iWidth, (iHeader >= TABLE_HEADER_LINE) ? iHeader - TABLE_HEADER_LINE : 0));
	}

	//The selected cell's line, while the cell is in view.
	mpScroll->GetActualBounds(&cView);
	bShown = GetCellBounds(GetSelectedRow(), GetSelectedColumn(), &cCell) && (cCell.msTopLeft.y >= cView.msTopLeft.y) && (cCell.msBottomRight.y <= cView.msBottomRight.y);
	if (bShown)
	{
		sPosition.Init(cCell.msTopLeft.x, cCell.msBottomRight.y - TABLE_CELL_MARK_HEIGHT);
		sSize.Init(cCell.GetWidth(), TABLE_CELL_MARK_HEIGHT);
	}
	else
	{
		sPosition.Init(cBounds.msTopLeft.x, cBounds.msTopLeft.y);
		sSize.Init(0, 0);
	}
	LayoutComponentInArea(mpCellMark, sPosition, sSize);
	mpCellMark->Invalidate();

	//The editor, over the cell it edits; the one that isn't editing has no size.
	bShown = IsEditing() && GetCellBounds(miEditRow, miEditColumn, &cCell);
	sPosition.Init(cBounds.msTopLeft.x, cBounds.msTopLeft.y);
	sSize.Init(0, 0);
	if (bShown && (GetColumnKind(miEditColumn) == TCK_Text))
	{
		LayoutComponentInArea(mpField, SInt32Vec2(cCell.msTopLeft.x, cCell.msTopLeft.y), SInt32Vec2(cCell.GetWidth(), cCell.GetHeight()));
	}
	else
	{
		LayoutComponentInArea(mpField, sPosition, sSize);
	}
	if (bShown && (GetColumnKind(miEditColumn) == TCK_Choice))
	{
		LayoutComponentInArea(mpCombo, SInt32Vec2(cCell.msTopLeft.x, cCell.msTopLeft.y), SInt32Vec2(cCell.GetWidth(), cCell.GetHeight()));
	}
	else
	{
		LayoutComponentInArea(mpCombo, sPosition, sSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The editors are always the table's but are only drawn while they edit a
// cell, and the line only while a cell is selected.
//
//////////////////////////////////////////////////////////////////////////
bool CTable::IsChildDrawn(Ptr<CComponent> pChild)
{
	if (&pChild == &mpField)
	{
		return IsEditing() && (GetColumnKind(miEditColumn) == TCK_Text);
	}
	if (&pChild == &mpCombo)
	{
		return IsEditing() && (GetColumnKind(miEditColumn) == TCK_Choice);
	}
	if (&pChild == &mpCellMark)
	{
		return GetSelectedColumn() >= 0;
	}
	return true;
}
