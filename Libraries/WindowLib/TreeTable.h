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
#ifndef __TREE_TABLE_H__
#define __TREE_TABLE_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayBit.h"
#include "Container.h"
#include "ScrollPane.h"
#include "TreeContainer.h"
#include "CellField.h"
#include "Sash.h"
#include "Label.h"
#include "WindowTheme.h"


#define TREE_TABLE_LEAST_WIDTH	24		//The least a column is dragged to.


//A tree whose rows have columns, as Visual Studio's Locals and Watch windows are (Name | Value | Type, DebuggerPlan.txt
//D1): a CTreeContainer's nodes and rows (only the rows in view have components) under captions, the first column the
//nodes' names, each other a cell of each node's.  The sash between two captions is dragged to make the column left of
//it wider or narrower; the last column takes what is left of the table's width.
//
//A column can be editable.  Enter or a double click on a node edits its first editable cell in a field over it (the
//chevron and left and right open and close nodes); enter (or the field losing focus) ends the edit keeping what was
//typed, escape without.  The edit event says a cell was typed into (GetEditedNode, GetEditedColumn, GetEditedText):
//its owner changes the cell, or doesn't, as a debugger writes a variable and shows what it then reads.
class CTreeTable : public CContainer
{
CONSTRUCTABLE(CTreeTable);
DESTRUCTABLE(CTreeTable);
protected:
	Ptr<CWindowTheme>			mpTheme;
	Ptr<CCanvas>				mpHeaderBackground;
	Ptr<CCanvas>				mpHeaderLine;
	CArray<CLabel>				maCaptions;
	CArray<CSash>				maSashes;		//maSashes[i] is right of column i.
	Ptr<CScrollPane>			mpScroll;
	Ptr<CTreeContainer>			mpTree;
	Ptr<CCellField>				mpField;
	CArrayInt					maiWidths;		//Each column's, as dragged; the last takes what is left.
	CArrayBit					mabEditable;
	CArrayInt					maiLefts;		//As last laid out, from the table's left.
	CArrayInt					maiLaidWidths;
	CComponentEvent<CTreeTable>	mcEdited;
	int32						miEditNode;		//TREE_NO_NODE while nothing is edited.
	int32						miEditColumn;
	int32						miEditedNode;
	int32						miEditedColumn;
	CChars						mszEditedText;
	CChars						mszName;		//A node's name, as GetCellText gives it.
	bool						mbEnding;		//An edit is being ended: the field losing focus doesn't end it again.

public:
	void							Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme);
	void							Class(void) override;
	void							Free(void) override;
	bool							Save(CObjectWriter* pcFile) override;
	bool							Load(CObjectReader* pcFile) override;

	int32							AddColumn(const char* szCaption, int32 iWidth, bool bEditable = false);
	int32							NumColumns(void) const;
	int32							GetColumnWidth(int32 iColumn);
	Ptr<CTreeContainer>				GetTree(void) const;
	void							SetCellText(int32 iNode, int32 iColumn, const char* szText);
	const char*						GetCellText(int32 iNode, int32 iColumn);
	void							SetCellFont(int32 iNode, int32 iColumn, Ptr<CFont> pFont);

	bool							BeginEdit(int32 iNode, int32 iColumn);
	bool							EndEdit(bool bKeep);
	bool							IsEditing(void) const;
	Ptr<CCellField>					GetEditField(void) const;
	CComponentEvent<CTreeTable>*	GetEditEvent(void);
	int32							GetEditedNode(void) const;
	int32							GetEditedColumn(void) const;
	const char*						GetEditedText(void) const;

	Ptr<CLabel>						GetCaption(int32 iColumn);
	Ptr<CSash>						GetSash(int32 iColumn);
	bool							GetCellBounds(int32 iNode, int32 iColumn, CInt32Rect* pcDest);

	void							SetRequiredSize(void) override;
	void							Layout(SInt32Vec2 sPosition, SInt32Vec2 sAreaSize) override;
	bool							IsChildDrawn(Ptr<CComponent> pChild) override;

protected:
	void							SashMoved(CSash* pcSash, int32 iDelta);
	void							Activated(void);
	void							FieldEnded(CCellField* pcField, int32 eEnd);
	void							Scrolled(CScrollPane* pcScroll, SInt32Vec2 sPosition);
	void							LayoutField(void);
	int32							GetHeaderHeight(void);
};


#endif // __TREE_TABLE_H__

