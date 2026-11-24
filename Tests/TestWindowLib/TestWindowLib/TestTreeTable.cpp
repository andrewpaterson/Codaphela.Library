#include "BaseLib/GlobalMemory.h"
#include "StandardLib/ClassDefines.h"
#include "StandardLib/Objects.h"
#include "WindowLib/Window.h"
#include "WindowLib/FillContainer.h"
#include "WindowLib/JewelTheme.h"
#include "SupportLib/PNGWriter.h"
#include "SupportLib/FontColours.h"
#include "WindowLib/TreeTable.h"
#include "WinRefLib/WinRefWindowFactory.h"
#include "TestLib/AssertGeometric.h"
#include "TestWindow.h"


Ptr<CFont>		GenerateMenuFont(Ptr<CWindow> pWindow);
SInt32Vec2		ComponentCentre(Ptr<CComponent> pComponent);
void			MoveTo(Ptr<CWindow> pWindow, SInt32Vec2 sPosition);


#define AssertLog(e)	AssertString(e, szLog.Text()); szLog.Clear()


//////////////////////////////////////////////////////////////////////////
//
// Logs "Edited:node,column=text;" and, as a debugger shows what it then
// reads, sets the cell to what was typed unless it is "no".
//
//////////////////////////////////////////////////////////////////////////
class CTestTreeTableListener
{
public:
	CChars*		mpszLog;

	void Init(CChars* pszLog)
	{
		mpszLog = pszLog;
	}

	void Edited(CTreeTable* pcTable)
	{
		mpszLog->Append("Edited:")->Append(pcTable->GetEditedNode())->Append(",")->Append(pcTable->GetEditedColumn())->Append("=")->Append(pcTable->GetEditedText())->Append(";");
		if (strcmp(pcTable->GetEditedText(), "no") != 0)
		{
			pcTable->SetCellText(pcTable->GetEditedNode(), pcTable->GetEditedColumn(), pcTable->GetEditedText());
		}
	}
};


//The nodes of the test's tree table.
struct STestTreeTableNodes
{
	int32	iShape;
	int32	iSides;
	int32	iSide;
	int32	iCount;
};


//////////////////////////////////////////////////////////////////////////
//
// A debugger's Locals: Name (120 wide; with bNames editable, as a Watch
// window's), Value (150, editable) and Type, in a window of 400 by 200,
// inset by 10; a class with two members and an int.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CTreeTable> CreateTestTreeTable(Ptr<CWindow> pWindow, CTestTreeTableListener* pcListener, STestTreeTableNodes* psNodes, bool bNames = false)
{
	Ptr<CFillContainer>		pFill;
	Ptr<CTreeTable>			pTable;
	Ptr<CTreeContainer>		pTree;
	Ptr<CWindowTheme>		pTheme;

	pFill = OMalloc<CFillContainer>(pWindow);
	pFill->SetInsets(10, 10, 10, 10);
	pWindow->SetContainer(pFill);
	pTheme = CJewelTheme::CreateDarkTheme(pWindow, GenerateMenuFont(pWindow));
	CJewelTheme::SetImages(pTheme, pWindow);
	pTable = OMalloc<CTreeTable>(pWindow, pTheme);
	pTable->SetAlignment(CSH_Stretch, CSV_Stretch);
	pFill->AddComponent(pTable);
	AssertInt(0, pTable->AddColumn("Name", 120, bNames));
	AssertInt(1, pTable->AddColumn("Value", 150, true));
	AssertInt(2, pTable->AddColumn("Type", 80));
	pTable->GetEditEvent()->Add(pcListener, &CTestTreeTableListener::Edited);

	pTree = pTable->GetTree();
	psNodes->iShape = pTree->AddNode(TREE_NO_NODE, "cShape");
	psNodes->iSides = pTree->AddNode(psNodes->iShape, "miSides");
	psNodes->iSide = pTree->AddNode(psNodes->iShape, "miSide");
	psNodes->iCount = pTree->AddNode(TREE_NO_NODE, "iCountOfEveryShapeEverDrawnSinceTheStart");
	pTable->SetCellText(psNodes->iShape, 1, "{...}");
	pTable->SetCellText(psNodes->iShape, 2, "CSquare");
	pTable->SetCellText(psNodes->iSides, 1, "4");
	pTable->SetCellText(psNodes->iSides, 2, "int");
	pTable->SetCellText(psNodes->iSide, 1, "3");
	pTable->SetCellText(psNodes->iSide, 2, "int");
	pTable->SetCellText(psNodes->iCount, 1, "5");
	pTable->SetCellText(psNodes->iCount, 2, "int");
	return pTable;
}


//////////////////////////////////////////////////////////////////////////
//
// The row component showing iNode, NULL if it isn't in view.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CTreeRow> TestTreeTableRow(Ptr<CTreeTable> pTable, int32 iNode)
{
	return pTable->GetTree()->GetRowComponent(pTable->GetTree()->FindNodeRow(iNode));
}


//////////////////////////////////////////////////////////////////////////
//
// The captions over their columns, the last to the table's right; each
// row's cells in their columns, the name cut off at the first's; a cell
// in a font of its own; a sash dragged to widen the column left of it,
// never under the least width.
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTableColumns(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTreeTable>			pTable;
		Ptr<CTreeRow>			pRow;
		Ptr<CFont>				pFont;
		CTestTreeTableListener	cListener;
		STestTreeTableNodes		sNodes;
		CPointer				pNull;
		CChars					szLog;
		CInt32Rect				cTable;
		CInt32Rect				cCell;
		CInt32Rect				cLabel;
		SInt32Vec2				sSash;

		szLog.Init();
		cListener.Init(&szLog);
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 400, 200, (char*)NULL);
		pTestWindow = OMalloc<CWindow>("Tree Table Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pTable = CreateTestTreeTable(pTestWindow, &cListener, &sNodes);
		pTable->GetTree()->SetExpanded(sNodes.iShape, true);
		pTestWindow->Layout();
		pTable->GetActualBounds(&cTable);
		AssertInt(380, cTable.GetWidth());

		//The cells are cells of the tree's columns, after the name; the name is in column 0.
		AssertInt(3, pTable->NumColumns());
		AssertSize(2, pTable->GetTree()->NumColumns());
		AssertString("4", pTable->GetCellText(sNodes.iSides, 1));
		AssertString("miSides", pTable->GetCellText(sNodes.iSides, 0));
		AssertString("", pTable->GetCellText(sNodes.iSides, 3));
		AssertFalse(pTable->GetTree()->SetNumColumns(1));

		//Columns as wide as they were made, the last taking what is left.
		AssertInt(120, pTable->GetColumnWidth(0));
		AssertInt(150, pTable->GetColumnWidth(1));
		AssertInt(380 - 120 - 150, pTable->GetColumnWidth(2));
		AssertTrue(pTable->GetCaption(1)->GetPosition().x > cTable.GetLeft() + 120);
		AssertTrue(pTable->GetCaption(1)->GetPosition().x < cTable.GetLeft() + 130);
		AssertTrue(pTable->GetCaption(3).IsNull());

		//A row shows its node's cells in their columns; the long name is cut off before the Value column.
		pRow = TestTreeTableRow(pTable, sNodes.iSide);
		AssertTrue(pRow.IsNotNull());
		AssertString("3", pRow->GetCell(0)->GetText());
		AssertString("int", pRow->GetCell(1)->GetText());
		AssertTrue(pTable->GetCellBounds(sNodes.iSide, 1, &cCell));
		AssertInt(cTable.GetLeft() + 120, cCell.GetLeft());
		AssertInt(150, cCell.GetWidth());
		pRow->GetCell(0)->GetActualBounds(&cLabel);
		AssertTrue(cLabel.GetLeft() >= cCell.GetLeft());
		AssertTrue(cLabel.GetLeft() < cCell.GetLeft() + 10);
		pRow = TestTreeTableRow(pTable, sNodes.iCount);
		pRow->GetLabel(0)->GetActualBounds(&cLabel);
		AssertTrue(cLabel.GetRight() <= cTable.GetLeft() + 120);

		//A cell in a font of its own (a changed value's, red), and back.
		pFont = GenerateMenuFont(pTestWindow);
		pTable->SetCellFont(sNodes.iSide, 1, pFont);
		pTestWindow->Layout();
		AssertTrue(TestTreeTableRow(pTable, sNodes.iSide)->GetCell(0)->GetFont() == pFont);
		pTable->SetCellFont(sNodes.iSide, 1, NULL);
		pTestWindow->Layout();
		AssertTrue(TestTreeTableRow(pTable, sNodes.iSide)->GetCell(0)->GetFont() != pFont);

		//The sash right of Name dragged 30 to the right: Name is wider, Value moves along, Type has less.
		sSash = ComponentCentre(pTable->GetSash(0));
		MoveTo(pTestWindow, sSash);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		MoveTo(pTestWindow, SInt32Vec2(sSash.x + 10, sSash.y));
		MoveTo(pTestWindow, SInt32Vec2(sSash.x + 30, sSash.y));
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		pTestWindow->Layout();
		AssertInt(150, pTable->GetColumnWidth(0));
		AssertInt(150, pTable->GetColumnWidth(1));
		AssertInt(380 - 150 - 150, pTable->GetColumnWidth(2));
		AssertTrue(pTable->GetCellBounds(sNodes.iSide, 1, &cCell));
		AssertInt(cTable.GetLeft() + 150, cCell.GetLeft());
		TestTreeTableRow(pTable, sNodes.iSide)->GetCell(0)->GetActualBounds(&cLabel);
		AssertTrue(cLabel.GetLeft() >= cTable.GetLeft() + 150);

		//Dragged far to the left, it stops at the least width.
		sSash = ComponentCentre(pTable->GetSash(0));
		MoveTo(pTestWindow, sSash);
		pTestWindow->InputCommand(CIC_ActivateComponent, true);
		MoveTo(pTestWindow, SInt32Vec2(sSash.x - 300, sSash.y));
		pTestWindow->InputCommand(CIC_ExecuteComponent, true);
		pTestWindow->Layout();
		AssertInt(TREE_TABLE_LEAST_WIDTH, pTable->GetColumnWidth(0));

		pTestWindow->Kill();
		szLog.Kill();
	}
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Enter on a node edits its Value in a field over the cell; enter keeps
// what was typed and tells the owner (who changes the cell, or doesn't),
// escape doesn't; only an editable column's cells are edited.
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTableEditing(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTreeTable>			pTable;
		Ptr<CFocus>				pFocus;
		Ptr<CFontColours>		pColours;
		CTestTreeTableListener	cListener;
		STestTreeTableNodes		sNodes;
		CPointer				pNull;
		CChars					szLog;
		CInt32Rect				cCell;
		CInt32Rect				cField;
		CChars					sz;

		szLog.Init();
		cListener.Init(&szLog);
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 400, 200, (char*)NULL);
		pTestWindow = OMalloc<CWindow>("Tree Table Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pTable = CreateTestTreeTable(pTestWindow, &cListener, &sNodes);
		pTestWindow->Layout();

		//Enter on iCount: its Value is edited, all of it selected, the field over the cell and focussed.
		pTable->GetTree()->SelectNode(sNodes.iCount);
		pFocus->SetFocus(pTable->GetTree());
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pTable->IsEditing());
		AssertTrue(pFocus->GetFocussedComponent() == pTable->GetEditField());
		AssertString("5", pTable->GetEditField()->GetText());
		pTestWindow->Layout();
		AssertTrue(pTable->GetCellBounds(sNodes.iCount, 1, &cCell));
		pTable->GetEditField()->GetActualBounds(&cField);
		AssertInt(cCell.GetLeft(), cField.GetLeft());
		AssertInt(cCell.GetTop(), cField.GetTop());
		AssertInt(cCell.GetRight(), cField.GetRight());
		AssertTrue(pTable->IsChildDrawn(pTable->GetEditField()));

		//TREE_TABLE_PICTURE=file saves the window, to look at: miSide's value changed (red), iCount's being edited.
		if (getenv("TREE_TABLE_PICTURE") != NULL)
		{
			pColours = OMalloc<CFontColours>(GenerateMenuFont(pTestWindow));
			pTable->GetTree()->SetExpanded(sNodes.iShape, true);
			pTable->SetCellFont(sNodes.iSide, 1, pColours->GetFont(Set32BitColour((uint8)240, (uint8)80, (uint8)80, (uint8)255)));
			pTestWindow->Layout();
			pTestWindow->Paint();
			SavePNG(pTestWindow->GetCanvas()->GetImageOrNull(), getenv("TREE_TABLE_PICTURE"));
			pTable->GetTree()->SetExpanded(sNodes.iShape, false);
			pTestWindow->Layout();
		}

		//Typing replaces it; enter tells the owner, who changes the cell, and the tree has the keys again.
		pTestWindow->InputPrintable('4', true);
		pTestWindow->InputPrintable('2', true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertFalse(pTable->IsEditing());
		sz.Init("Edited:")->Append(sNodes.iCount)->Append(",1=42;");
		AssertLog(sz.Text());
		sz.Kill();
		AssertString("42", pTable->GetCellText(sNodes.iCount, 1));
		AssertTrue(pFocus->GetFocussedComponent() == pTable->GetTree());
		AssertFalse(pTable->IsChildDrawn(pTable->GetEditField()));

		//An owner that keeps the cell as it was (a write the target refused).
		pTestWindow->InputCommand(CIC_Enter, true);
		pTestWindow->InputPrintable('n', true);
		pTestWindow->InputPrintable('o', true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(szLog.EndsWith("=no;"));
		szLog.Clear();
		AssertString("42", pTable->GetCellText(sNodes.iCount, 1));

		//Escape, and enter without a change, tell the owner nothing.
		pTestWindow->InputCommand(CIC_Enter, true);
		pTestWindow->InputPrintable('7', true);
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		AssertFalse(pTable->IsEditing());
		pTestWindow->InputCommand(CIC_Enter, true);
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertLog("");
		AssertString("42", pTable->GetCellText(sNodes.iCount, 1));

		//Only an editable column's cells: not a name, not a Type.
		AssertFalse(pTable->BeginEdit(sNodes.iCount, 0));
		AssertFalse(pTable->BeginEdit(sNodes.iCount, 2));
		AssertFalse(pTable->BeginEdit(99, 1));
		AssertFalse(pTable->EndEdit(true));
		AssertTrue(pTable->BeginEdit(sNodes.iShape, 1));
		AssertTrue(pTable->EndEdit(false));

		pTestWindow->Kill();
		szLog.Kill();
	}
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A Watch window's names are edited too: the field from where the name
// starts (after its indent and chevron), and enter or a double click
// edits the cell under the pointer, else the name.
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTableNameEditing(void)
{
	Ptr<CWinRefWindowFactory>	pNativeFactory;

	{
		Ptr<CWindow>			pTestWindow;
		Ptr<CTreeTable>			pTable;
		Ptr<CFocus>				pFocus;
		CTestTreeTableListener	cListener;
		STestTreeTableNodes		sNodes;
		CPointer				pNull;
		CChars					szLog;
		CInt32Rect				cCell;
		CInt32Rect				cParent;
		CInt32Rect				cField;
		CChars					sz;

		szLog.Init();
		cListener.Init(&szLog);
		pNativeFactory = OMalloc<CWinRefWindowFactory>(&gcMemoryAllocator, 400, 200, (char*)NULL);
		pTestWindow = OMalloc<CWindow>("Tree Table Window", &pNativeFactory, pNull, pNull, CFT_RGB, CCO_RGB, CRGB_24bit, ARGB_None);
		ORoot()->Add(pTestWindow);
		pFocus = pTestWindow->GetFocus();
		pTable = CreateTestTreeTable(pTestWindow, &cListener, &sNodes, true);
		pTable->GetTree()->SetExpanded(sNodes.iShape, true);
		pTestWindow->Layout();

		//miSide's name: the field is from where it starts, one indent further in than its parent's, to the Value column.
		AssertTrue(pTable->BeginEdit(sNodes.iSide, 0));
		AssertString("miSide", pTable->GetEditField()->GetText());
		pTestWindow->Layout();
		AssertTrue(pTable->GetCellBounds(sNodes.iSide, 0, &cCell));
		AssertTrue(pTable->GetCellBounds(sNodes.iShape, 0, &cParent));
		AssertInt(pTable->GetTree()->GetNameLeft(sNodes.iSide) - pTable->GetTree()->GetNameLeft(sNodes.iShape), cCell.GetLeft() - cParent.GetLeft());
		AssertTrue(cCell.GetLeft() > cParent.GetLeft());
		AssertInt(cParent.GetRight(), cCell.GetRight());
		pTable->GetEditField()->GetActualBounds(&cField);
		AssertInt(cCell.GetLeft(), cField.GetLeft());
		AssertInt(cCell.GetRight(), cField.GetRight());
		pTestWindow->InputPrintable('x', true);
		pTestWindow->InputCommand(CIC_Enter, true);
		sz.Init("Edited:")->Append(sNodes.iSide)->Append(",0=x;");
		AssertLog(sz.Text());
		sz.Kill();

		//Enter with the pointer on iCount's Value edits it; with the pointer off the row, its name.
		pTable->GetTree()->SelectNode(sNodes.iCount);
		pFocus->SetFocus(pTable->GetTree());
		AssertTrue(pTable->GetCellBounds(sNodes.iCount, 1, &cCell));
		MoveTo(pTestWindow, SInt32Vec2(cCell.GetLeft() + 5, cCell.GetTop() + 2));
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pTable->IsEditing());
		AssertString("5", pTable->GetEditField()->GetText());
		pTestWindow->InputCommand(CIC_ClearSelection, true);
		MoveTo(pTestWindow, SInt32Vec2(cCell.GetLeft() + 5, cCell.GetBottom() + 40));
		pTestWindow->InputCommand(CIC_Enter, true);
		AssertTrue(pTable->IsEditing());
		AssertString("iCountOfEveryShapeEverDrawnSinceTheStart", pTable->GetEditField()->GetText());
		AssertTrue(pTable->EndEdit(false));
		AssertLog("");

		pTestWindow->Kill();
		szLog.Kill();
	}
	pNativeFactory = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestTreeTable(void)
{
	BeginTests();
	ObjectsInit();

	TestTreeTableColumns();
	TestTreeTableEditing();
	TestTreeTableNameEditing();

	ObjectsFlush();
	ObjectsKill();
	TestStatistics();
}

