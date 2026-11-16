#include "BaseLib/GlobalMemory.h"
#include "BaseLib/FileUtil.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "WindowLib/Table.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestWindow.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);
void			Click(Ptr<CWindow> pWindow);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "Selected:row,column;", "Changed:row,column=text;" and "Moved;".
//
//////////////////////////////////////////////////////////////////////////
class CTestTableListener
{
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void SelectionChanged(CTable* pcTable)
	{
		mpszLog->Append("Selected:")->Append(pcTable->GetSelectedRow())->Append(",")->Append(pcTable->GetSelectedColumn())->Append(";");
	}

	void CellChanged(CTable* pcTable)
	{
		mpszLog->Append("Changed:")->Append(pcTable->GetChangedRow())->Append(",")->Append(pcTable->GetChangedColumn())->Append("=");
		mpszLog->Append(pcTable->GetCellText(pcTable->GetChangedRow(), pcTable->GetChangedColumn()))->Append(";");
	}

	void OrderChanged(CTable* pcTable)
	{
		mpszLog->Append("Moved;");
	}
};


//////////////////////////////////////////////////////////////////////////
//
// A table of three columns (a name, a kind chosen from int and float, and
// a tick) in a window of 400 by 200, inset by 10.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CTable> CreateTestTable(Ptr<CWindow> pWindow, CTestTableListener* pcListener)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CTable>				pTable;

	pFill = OMalloc<CFillContainer>(pWindow);
	pFill->SetInsets(10, 10, 10, 10);
	pWindow->SetContainer(pFill);
	pTable = OMalloc<CTable>(pWindow, CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow)));
	pTable->SetAlignment(CSH_Stretch, CSV_Stretch);
	pFill->AddComponent(pTable);
	AssertInt(0, pTable->AddColumn("Name", TCK_Text, 2));
	AssertInt(1, pTable->AddColumn("Kind", TCK_Choice, 1));
	AssertInt(2, pTable->AddColumn("Used", TCK_Tick, 0));
	pTable->AddColumnChoice(1, "int");
	pTable->AddColumnChoice(1, "float");
	pTable->GetSelectionEvent()->Add(pcListener, &CTestTableListener::SelectionChanged);
	pTable->GetChangeEvent()->Add(pcListener, &CTestTableListener::CellChanged);
	pTable->GetOrderEvent()->Add(pcListener, &CTestTableListener::OrderChanged);
	return pTable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static SInt32Vec2 CellCentre(Ptr<CTable> pTable, int32 iRow, int32 iColumn)
{
	CInt32Rect	cCell;

	AssertTrue(pTable->GetCellBounds(iRow, iColumn, &cCell));
	return SInt32Vec2((cCell.msTopLeft.x + cCell.msBottomRight.x) / 2, (cCell.msTopLeft.y + cCell.msBottomRight.y) / 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Cells are set and read, the captions are over their columns, and the
// selected cell is moved by presses and the arrow keys.
//
//////////////////////////////////////////////////////////////////////////
void TestTableCells(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>		pTestWindow;
		Ptr<CTable>			pTable;
		Ptr<CFocus>			pFocus;
		CTestTableListener	cListener;
		CPointer			pNull;
		CChars				szLog;
		CInt32Rect			cCell;
		CInt32Rect			cHeader;
		int32				i;

		szLog.Init();
		cListener.Init(&szLog);
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 400, 200, (char*)NULL);
		pTestWindow = OMalloc<CWindow>("Table Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pTable = CreateTestTable(pTestWindow, &cListener);
		AssertInt(3, pTable->NumColumns());
		AssertInt(0, pTable->NumRows());
		AssertInt(TCK_Choice, pTable->GetColumnKind(1));

		AssertInt(0, pTable->AddRow());
		AssertInt(1, pTable->AddRow());
		AssertInt(2, pTable->AddRow());
		pTable->SetCellText(0, 0, "iCount");
		pTable->SetCellText(0, 1, "int");
		pTable->SetCellText(1, 0, "fScale");
		pTable->SetCellText(1, 1, "float");
		pTable->SetCellTicked(1, 2, true);
		pTable->SetCellText(2, 0, "iLast");
		pTable->SetCellText(2, 1, "int");
		AssertInt(3, pTable->NumRows());
		AssertString("fScale", pTable->GetCellText(1, 0));
		AssertString("float", pTable->GetCellText(1, 1));
		AssertTrue(pTable->IsCellTicked(1, 2));
		AssertFalse(pTable->IsCellTicked(0, 2));
		AssertNull(pTable->GetCellText(3, 0));
		AssertNull(pTable->GetCellText(0, 3));
		AssertLog("");
		pTestWindow->Layout();

		//Each caption is over its column, along the table's top; the rows are under them.
		for (i = 0; i < 3; i++)
		{
			AssertTrue(pTable->GetCellBounds(0, i, &cCell));
			pTable->GetHeaderComponent(i)->GetActualBounds(&cHeader);
			AssertInt(cCell.msTopLeft.x, cHeader.msTopLeft.x);
			AssertInt(cCell.msBottomRight.x, cHeader.msBottomRight.x);
			AssertInt(10, cHeader.msTopLeft.y);
			AssertTrue(cCell.msTopLeft.y >= cHeader.msBottomRight.y);
		}
		//The columns share the width by their weights: the name's is twice the kind's spare, the tick's is its own.
		AssertTrue(pTable->GetCellBounds(0, 0, &cCell));
		AssertInt(10, cCell.msTopLeft.x);
		AssertTrue(pTable->GetCellBounds(0, 2, &cCell));
		AssertInt(390, cCell.msBottomRight.x);
		AssertFalse(pTable->GetCellBounds(3, 0, &cCell));

		//Pressing a cell selects it and gives the rows focus.
		AssertInt(-1, pTable->GetSelectedRow());
		AssertInt(-1, pTable->GetSelectedColumn());
		MoveTo(pTestWindow, CellCentre(pTable, 1, 0));
		Click(pTestWindow);
		AssertTrue(pFocus->GetFocussedComponent() == pTable->GetList());
		AssertInt(1, pTable->GetSelectedRow());
		AssertInt(0, pTable->GetSelectedColumn());
		AssertLog("Selected:1,0;");
		AssertFalse(pTable->IsEditing());

		//Left and right move along the row and stop at its ends; up and down move by rows.
		pTestWindow->InputCommand(CIC_Right, true);
		AssertLog("Selected:1,1;");
		pTestWindow->InputCommand(CIC_Right, true);
		AssertLog("Selected:1,2;");
		pTestWindow->InputCommand(CIC_Right, true);
		AssertLog("");
		pTestWindow->InputCommand(CIC_Down, true);
		AssertLog("Selected:2,2;");
		pTestWindow->InputCommand(CIC_Left, true);
		pTestWindow->InputCommand(CIC_Left, true);
		pTestWindow->InputCommand(CIC_Left, true);
		AssertLog("Selected:2,1;Selected:2,0;");
		pTestWindow->InputCommand(CIC_Home, true);
		AssertLog("Selected:0,0;");

		//A tick is changed by pressing it, and by the space bar.
		MoveTo(pTestWindow, CellCentre(pTable, 0, 2));
		Click(pTestWindow);
		AssertTrue(pTable->IsCellTicked(0, 2));
		AssertLog("Selected:0,2;Changed:0,2=1;");
		pTestWindow->InputCommand(CIC_Space, true);
		AssertFalse(pTable->IsCellTicked(0, 2));
		AssertLog("Changed:0,2=;");
		AssertFalse(pTable->IsEditing());

		//A column is at least as wide as its caption: the tick's is wider than a tick.
		AssertTrue(pTable->GetCellBounds(0, 2, &cCell));
		AssertTrue(cCell.GetWidth() >= CContainer::GetMeasuredSize(pTable->GetHeaderComponent(2)).x);

		//A tick that can't be changed and isn't ticked shows nothing; one that is ticked still shows.
		AssertSize(1, pTable->GetCellComponent(0, 2)->NumComponents());
		pTable->SetCellEditable(0, 2, false);
		AssertSize(0, pTable->GetCellComponent(0, 2)->NumComponents());
		pTestWindow->InputCommand(CIC_Space, true);
		AssertFalse(pTable->IsCellTicked(0, 2));
		AssertLog("");
		pTable->SetCellEditable(1, 2, false);
		AssertSize(1, pTable->GetCellComponent(1, 2)->NumComponents());
		pTable->SetCellEditable(0, 2, true);
		AssertSize(1, pTable->GetCellComponent(0, 2)->NumComponents());

		//SelectCell is the owner's: no such cell is refused.
		AssertTrue(pTable->SelectCell(2, 1));
		AssertLog("Selected:2,1;");
		AssertFalse(pTable->SelectCell(3, 0));
		AssertFalse(pTable->SelectCell(0, -1));
		AssertLog("");

		pTestWindow->Kill();
		szLog.Kill();
	}
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A text cell is edited in a field over it: by enter, by pressing the
// selected cell again, and by typing.  Enter keeps what was typed and
// escape doesn't; tab keeps it and edits the next cell.
//
//////////////////////////////////////////////////////////////////////////
void TestTableEditing(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>		pTestWindow;
		Ptr<CTable>			pTable;
		Ptr<CFocus>			pFocus;
		CTestTableListener	cListener;
		CPointer			pNull;
		CChars				szLog;
		CInt32Rect			cCell;
		CInt32Rect			cField;

		szLog.Init();
		cListener.Init(&szLog);
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 400, 200, (char*)NULL);
		pTestWindow = OMalloc<CWindow>("Table Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pTable = CreateTestTable(pTestWindow, &cListener);
		pTable->AddRow();
		pTable->AddRow();
		pTable->SetCellText(0, 0, "iCount");
		pTable->SetCellText(0, 1, "int");
		pTable->SetCellText(1, 0, "fScale");
		pTable->SetCellText(1, 1, "float");
		pTestWindow->Layout();

		//Enter edits the selected cell: the field is over it, has what it has, selected, and has focus.
		MoveTo(pTestWindow, CellCentre(pTable, 0, 0));
		Click(pTestWindow);
		AssertLog("Selected:0,0;");
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pTable->IsEditing());
		AssertTrue(pFocus->GetFocussedComponent() == pTable->GetEditField());
		AssertString("iCount", pTable->GetEditField()->GetText());
		pTestWindow->Layout();
		AssertTrue(pTable->GetCellBounds(0, 0, &cCell));
		pTable->GetEditField()->GetActualBounds(&cField);
		AssertInt(cCell.msTopLeft.x, cField.msTopLeft.x);
		AssertInt(cCell.msTopLeft.y, cField.msTopLeft.y);
		AssertInt(cCell.msBottomRight.x, cField.msBottomRight.x);

		//Typing replaces what is selected; enter keeps it, tells the owner, and gives the rows focus again.
		pTestWindow->InputPrintable('i', true);
		pTestWindow->InputPrintable('N', true);
		AssertString("iCount", pTable->GetCellText(0, 0));
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pTable->IsEditing());
		AssertString("iN", pTable->GetCellText(0, 0));
		AssertLog("Changed:0,0=iN;");
		AssertTrue(pFocus->GetFocussedComponent() == pTable->GetList());

		//Escape keeps what the cell had.
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pTable->IsEditing());
		pTestWindow->InputPrintable('q', true);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pTable->IsEditing());
		AssertString("iN", pTable->GetCellText(0, 0));
		AssertLog("");
		AssertTrue(pFocus->GetFocussedComponent() == pTable->GetList());

		//Enter with nothing changed tells nobody.
		pTestWindow->InputCommand(CIC_Enter, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pTable->IsEditing());
		AssertLog("");

		//Typing on the rows edits the selected cell, starting with what was typed.
		pTestWindow->InputPrintable('z', true);
		AssertTrue(pTable->IsEditing());
		AssertString("z", pTable->GetEditField()->GetText());
		pTestWindow->InputPrintable('y', true);
		AssertString("zy", pTable->GetEditField()->GetText());

		//Down keeps it and moves a row; the edit is over.
		pTestWindow->InputCommand(CIC_Down, true);
		AssertFalse(pTable->IsEditing());
		AssertString("zy", pTable->GetCellText(0, 0));
		AssertLog("Changed:0,0=zy;Selected:1,0;");

		//Pressing the selected cell again edits it; pressing another cell keeps what was typed.
		MoveTo(pTestWindow, CellCentre(pTable, 1, 0));
		Click(pTestWindow);
		AssertTrue(pTable->IsEditing());
		AssertString("fScale", pTable->GetEditField()->GetText());
		pTestWindow->InputPrintable('f', true);
		MoveTo(pTestWindow, CellCentre(pTable, 0, 0));
		Click(pTestWindow);
		AssertFalse(pTable->IsEditing());
		AssertString("f", pTable->GetCellText(1, 0));
		AssertLog("Changed:1,0=f;Selected:0,0;");

		//A cell, or a column, that can't be edited isn't.
		pTable->SetCellEditable(0, 0, false);
		AssertFalse(pTable->IsCellEditable(0, 0));
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pTable->IsEditing());
		pTestWindow->InputPrintable('a', true);
		AssertFalse(pTable->IsEditing());
		pTable->SetCellEditable(0, 0, true);
		pTable->SetColumnEditable(0, false);
		AssertFalse(pTable->IsCellEditable(0, 0));
		AssertFalse(pTable->BeginEdit());
		pTable->SetColumnEditable(0, true);
		AssertTrue(pTable->IsCellEditable(0, 0));

		//Tab keeps what was typed and edits the next cell: a choice, in a combo box of its column's choices.
		AssertTrue(pTable->BeginEdit());
		pTestWindow->InputPrintable('t', true);
		pTestWindow->InputCommand(CIC_Tab, true);
		AssertString("t", pTable->GetCellText(0, 0));
		AssertLog("Changed:0,0=t;Selected:0,1;");
		AssertTrue(pTable->IsEditing());
		AssertSize(2, pTable->GetEditCombo()->NumItems());
		AssertString("int", pTable->GetEditCombo()->GetSelectedText());
		AssertTrue(pTable->GetEditCombo()->IsOpen());

		//Ending the edit closes the choices; nothing was chosen.
		AssertTrue(pTable->EndEdit(false));
		AssertFalse(pTable->GetEditCombo()->IsOpen());
		AssertFalse(pTable->IsEditing());
		AssertString("int", pTable->GetCellText(0, 1));
		AssertLog("");
		AssertFalse(pTable->EndEdit(true));

		//Enter on a choice opens its choices; down and enter choose the next, which is the cell's.
		AssertTrue(pTable->SelectCell(0, 1));
		szLog.Clear();
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pTable->IsEditing());
		AssertTrue(pTable->GetEditCombo()->IsOpen());
		pTestWindow->InputCommand(CIC_Down, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pTable->IsEditing());
		AssertFalse(pTable->GetEditCombo()->IsOpen());
		AssertString("float", pTable->GetCellText(0, 1));
		AssertLog("Changed:0,1=float;");
		AssertTrue(pFocus->GetFocussedComponent() == pTable->GetList());

		//Escape closes them and keeps what the cell had.
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pTable->GetEditCombo()->IsOpen());
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pTable->IsEditing());
		AssertString("float", pTable->GetCellText(0, 1));
		AssertLog("");

		pTestWindow->Kill();
		szLog.Kill();
	}
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Rows are added between others, taken out and moved, and the selected
// cell stays the cell it was.  The user moves the selected row only if
// the table lets rows be moved.  The captions stay where they are while
// the rows scroll.
//
//////////////////////////////////////////////////////////////////////////
void TestTableRows(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>		pTestWindow;
		Ptr<CTable>			pTable;
		CTestTableListener	cListener;
		CPointer			pNull;
		CChars				szLog;
		CChars				szName;
		CInt32Rect			cCell;
		CInt32Rect			cHeader;
		int32				iTop;
		int32				i;

		szLog.Init();
		cListener.Init(&szLog);
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 400, 200, (char*)NULL);
		pTestWindow = OMalloc<CWindow>("Table Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTable = CreateTestTable(pTestWindow, &cListener);
		for (i = 0; i < 3; i++)
		{
			pTable->AddRow();
			szName.Init("Row");
			szName.Append(i);
			pTable->SetCellText(i, 0, szName.Text());
			szName.Kill();
		}
		pTable->SetCellTicked(2, 2, true);
		pTable->SetCellEditable(2, 0, false);
		pTestWindow->Layout();
		AssertTrue(pTable->SelectCell(1, 1));
		AssertLog("Selected:1,1;");

		//A row between: those from there on are one further down, the selected cell with them.
		AssertInt(1, pTable->InsertRow(1));
		AssertInt(4, pTable->NumRows());
		AssertString("Row0", pTable->GetCellText(0, 0));
		AssertString("", pTable->GetCellText(1, 0));
		AssertString("Row1", pTable->GetCellText(2, 0));
		AssertString("Row2", pTable->GetCellText(3, 0));
		AssertTrue(pTable->IsCellTicked(3, 2));
		AssertFalse(pTable->IsCellEditable(3, 0));
		AssertTrue(pTable->IsCellEditable(1, 0));
		AssertInt(2, pTable->GetSelectedRow());
		AssertInt(1, pTable->GetSelectedColumn());
		AssertInt(-1, pTable->InsertRow(5));
		AssertInt(-1, pTable->InsertRow(-1));
		szLog.Clear();

		//A row taken out.
		AssertTrue(pTable->RemoveRow(1));
		AssertInt(3, pTable->NumRows());
		AssertString("Row1", pTable->GetCellText(1, 0));
		AssertInt(1, pTable->GetSelectedRow());
		AssertFalse(pTable->RemoveRow(3));
		szLog.Clear();

		//A row moved down, and up again.
		AssertTrue(pTable->MoveRow(0, 2));
		AssertString("Row1", pTable->GetCellText(0, 0));
		AssertString("Row2", pTable->GetCellText(1, 0));
		AssertString("Row0", pTable->GetCellText(2, 0));
		AssertTrue(pTable->IsCellTicked(1, 2));
		AssertInt(0, pTable->GetSelectedRow());
		AssertTrue(pTable->MoveRow(2, 0));
		AssertString("Row0", pTable->GetCellText(0, 0));
		AssertString("Row1", pTable->GetCellText(1, 0));
		AssertString("Row2", pTable->GetCellText(2, 0));
		AssertInt(1, pTable->GetSelectedRow());
		AssertFalse(pTable->MoveRow(0, 3));
		szLog.Clear();

		//The user moves the selected row only where rows can be moved.
		AssertFalse(pTable->MoveSelectedRow(1));
		pTable->SetRowsMovable(true);
		AssertTrue(pTable->MoveSelectedRow(1));
		AssertString("Row2", pTable->GetCellText(1, 0));
		AssertString("Row1", pTable->GetCellText(2, 0));
		AssertInt(2, pTable->GetSelectedRow());
		AssertTrue(szLog.EndsWith("Moved;"));
		AssertFalse(pTable->MoveSelectedRow(1));
		szLog.Clear();

		//More rows than fit: the rows scroll and the captions stay.
		for (i = 3; i < 20; i++)
		{
			pTable->AddRow();
		}
		pTestWindow->Layout();
		pTable->GetHeaderComponent(0)->GetActualBounds(&cHeader);
		AssertTrue(pTable->GetCellBounds(0, 0, &cCell));
		iTop = cCell.msTopLeft.y;
		AssertTrue(pTable->SelectCell(19, 0));
		pTestWindow->Layout();
		AssertTrue(pTable->GetCellBounds(0, 0, &cCell));
		AssertTrue(cCell.msTopLeft.y < iTop);
		AssertTrue(pTable->GetCellBounds(19, 0, &cCell));
		AssertTrue(cCell.msBottomRight.y <= 190);
		AssertInt(cHeader.msTopLeft.y, pTable->GetHeaderComponent(0)->GetPosition().y);

		pTable->RemoveAllRows();
		AssertInt(0, pTable->NumRows());
		AssertInt(-1, pTable->GetSelectedRow());

		pTestWindow->Kill();
		szLog.Kill();
	}
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTable(void)
{
	BeginTests();
	ObjectsInit();

	TestTableCells();
	TestTableEditing();
	TestTableRows();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}
