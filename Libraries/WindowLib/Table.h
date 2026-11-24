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
#ifndef __TABLE_H__
#define __TABLE_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayBit.h"
#include "BaseLib/ArrayInt.h"
#include "Container.h"
#include "ListContainer.h"
#include "ScrollPane.h"
#include "CellField.h"
#include "ComboBox.h"
#include "CheckBox.h"
#include "Label.h"
#include "Canvas.h"
#include "ClearCanvasDraw.h"
#include "ComponentEvent.h"
#include "WindowTheme.h"


class CTable;


//What a column of a table holds.
enum ETableColumn
{
	TCK_Text,		//Text, edited in a text field.
	TCK_Tick,		//A tick, changed by a press or the space bar.
	TCK_Choice,		//One of the column's choices, chosen in a combo box.
};


//A table's rows: a list whose presses and keys are the table's.
class CTableList : public CListContainer
{
CONSTRUCTABLE(CTableList);
DESTRUCTABLE(CTableList);
protected:
	CTable*		mpcTable;  //Not held: the table holds the list.

public:
	void	Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, CTable* pcTable);
	void	Class(void) override;
	void	Free(void) override;

	void	ActivateComponent(int x, int y) override;
	void	Enter(void);
	void	Space(void);
	void	Left(void);
	void	Right(void);
	void	Typed(SComponentInputEvent* psEvent);
	void	MoveRowUp(SComponentInputEvent* psEvent);
	void	MoveRowDown(SComponentInputEvent* psEvent);
	bool	WantsEnter(void) override;

protected:
	void	RowSelected(int32 iRow) override;
	void	RowActivated(int32 iRow) override;
};


//The combo box a table's choice cell is edited in.
class CTableCombo : public CComboBox
{
CONSTRUCTABLE(CTableCombo);
DESTRUCTABLE(CTableCombo);
protected:
	CTable*		mpcTable;  //Not held.

public:
	void	Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, CTable* pcTable);
	void	Class(void) override;
	void	Free(void) override;

	bool	WantsEscape(void) override;
	bool	IsTabStop(void) const override;
	void	PopupClosed(Ptr<CComponent> pContent) override;
	void	Cancel(void);
};


//A table whose cells are edited where they are, like IntelliJ IDEA's (Change Signature's parameters): columns
//with captions over them, and rows of cells that are text, a tick, or one of a column's choices.
//
//One cell is the selected cell: its row is the list's selected row, drawn over the selected highlight, and the cell
//itself is marked.  Up, down, home, end, page up and page down move it by rows as a list's; left and right, and tab
//and shift tab, move it along its row (tab goes on to the next row's first cell from the last).  Pressing a cell
//selects it.
//
//A text cell is edited in a text field that takes its place: by enter, by pressing the selected cell again, or by
//typing, which replaces what it had.  Enter ends the edit and keeps what was typed, as does up or down (which also
//move a row), tab (which goes on to edit the next cell), and the field losing focus; escape ends it and keeps what the
//cell had.  A choice cell is edited the same way in a combo box of its column's choices.  A tick cell is changed by
//pressing it or by the space bar.  A cell or a whole column can be made not editable.
//
//Rows are added, inserted, removed and moved by the table's owner (buttons beside it, usually); Alt with up or down
//moves the selected row itself, if the table lets rows be moved.
//
//The captions stay at the top while the rows scroll under them: the rows are a list in a scroll pane, and each
//caption is laid out over its column of the list.  The table is as wide as it is given, and as tall as its captions
//and a number of rows (SetVisibleRows) unless a desired height is set.
//
//The change event is called when a cell's text or tick has been changed by the user (GetChangedRow and
//GetChangedColumn say which); the selection event when the selected cell changes; the order event when the user has
//moved a row.  What the owner sets itself calls none of them.
class CTable : public CContainer
{
CONSTRUCTABLE(CTable);
DESTRUCTABLE(CTable);
protected:
	Ptr<CWindowTheme>		mpTheme;
	Ptr<CCanvas>			mpHeaderBackground;
	Ptr<CCanvas>			mpHeaderLine;  //Between the captions and the rows.
	CArray<CComponent>		maHeaders;  //Each column's caption, in its padding.
	Ptr<CScrollPane>		mpScroll;
	Ptr<CTableList>			mpList;
	Ptr<CCanvas>			mpCellMark;  //A line under the selected cell.
	Ptr<CCellField>			mpField;  //Over a text cell while it is edited; it is always there, with no size otherwise.
	Ptr<CTableCombo>		mpCombo;  //Over a choice cell while it is edited, the same.
	CArray<CComponent>		maCells;  //What shows each cell (a label or a check box), row by row.

	CArrayChars				maszCaptions;
	CArrayInt				maiKinds;  //ETableColumn, for each column.
	CArrayInt				maiWeights;
	CArrayInt				maiColumnWidths;  //In the font's average characters, 0 for what the cells need.
	CArrayInt				maiColumnEditable;
	CArrayChars				maszChoices;  //Every column's choices, one after another,
	CArrayInt				maiChoiceColumns;  //and the column each is for.
	CArrayChars				maszCells;  //Each cell's text, row by row; a tick's is "1" or "".
	CArrayInt				maiCellEditable;

	CComponentEvent<CTable>	mcChanged;
	CComponentEvent<CTable>	mcSelectionChanged;
	CComponentEvent<CTable>	mcOrderChanged;
	int32					miSelectedColumn;
	int32					miEditRow;  //The cell being edited, -1 if none is.
	int32					miEditColumn;
	int32					miChangedRow;
	int32					miChangedColumn;
	int32					miChoiceRow;  //The cell the combo box's choices were opened for, -1 once one is chosen or the rows change.
	int32					miChoiceColumn;
	int32					miVisibleRows;
	bool					mbRowsMovable;
	bool					mbEnding;  //An edit is being ended: its editor losing focus doesn't end it again.

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	int32			AddColumn(const char* szCaption, ETableColumn eKind = TCK_Text, int iWidthWeight = 1, int32 iWidthInCharacters = 0);
	void			AddColumnChoice(int32 iColumn, const char* szChoice);
	void			SetColumnEditable(int32 iColumn, bool bEditable);
	int32			NumColumns(void) const;
	ETableColumn	GetColumnKind(int32 iColumn);

	int32			AddRow(void);
	int32			InsertRow(int32 iBefore);
	bool			RemoveRow(int32 iRow);
	bool			MoveRow(int32 iFrom, int32 iTo);
	void			RemoveAllRows(void);
	int32			NumRows(void) const;

	void			SetCellText(int32 iRow, int32 iColumn, const char* szText);
	char*			GetCellText(int32 iRow, int32 iColumn);
	void			SetCellTicked(int32 iRow, int32 iColumn, bool bTicked);
	bool			IsCellTicked(int32 iRow, int32 iColumn);
	void			SetCellEditable(int32 iRow, int32 iColumn, bool bEditable);
	bool			IsCellEditable(int32 iRow, int32 iColumn);

	bool			SelectCell(int32 iRow, int32 iColumn);
	int32			GetSelectedRow(void);
	int32			GetSelectedColumn(void);
	bool			BeginEdit(void);
	bool			BeginEdit(const char* szText);
	bool			EndEdit(bool bKeep);
	bool			IsEditing(void) const;
	Ptr<CCellField>		GetEditField(void) const;
	Ptr<CTableCombo>	GetEditCombo(void) const;
	bool			ToggleTick(void);
	bool			MoveSelectedRow(int32 iBy);
	void			SetRowsMovable(bool bRowsMovable);
	void			SetVisibleRows(int32 iVisibleRows);

	Ptr<CTableList>		GetList(void) const;
	Ptr<CScrollPane>	GetScrollPane(void) const;
	Ptr<CComponent>		GetCellComponent(int32 iRow, int32 iColumn);
	Ptr<CComponent>		GetHeaderComponent(int32 iColumn);
	bool				GetCellBounds(int32 iRow, int32 iColumn, CInt32Rect* pcDest);
	int32				FindColumnAt(int x);

	CComponentEvent<CTable>*	GetChangeEvent(void);
	CComponentEvent<CTable>*	GetSelectionEvent(void);
	CComponentEvent<CTable>*	GetOrderEvent(void);
	int32						GetChangedRow(void) const;
	int32						GetChangedColumn(void) const;

	void			SetRequiredSize(void) override;
	void			Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool			IsChildDrawn(Ptr<CComponent> pChild) override;

	//The list's and the editors', not an owner's.
	void			ListPressed(int x, int y);
	void			ListRowSelected(int32 iRow);
	void			MoveColumn(int32 iBy, bool bWrap);
	void			Typed(char cPrintable);
	void			EditNext(int32 iBy);
	void			ComboChosen(CComboBox* pcCombo);
	void			FieldEnded(CCellField* pcField, int32 eEnd);
	void			Scrolled(CScrollPane* pcScroll, SInt32Vec2 sPosition);

protected:
	size			CellIndex(int32 iRow, int32 iColumn);
	void			Rebuild(void);
	Ptr<CComponent>	CreateCell(int32 iRow, int32 iColumn);
	void			FillCell(Ptr<CComponent> pCell, int32 iRow, int32 iColumn);
	void			UpdateCell(int32 iRow, int32 iColumn);
	void			LayoutOverlays(void);
	void			Changed(int32 iRow, int32 iColumn);
	int32			GetHeaderHeight(void);
	int32			GetRowHeight(void);
	bool			CanEdit(int32 iRow, int32 iColumn);
};


#endif // __TABLE_H__
