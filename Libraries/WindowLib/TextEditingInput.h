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
#ifndef __TEXT_EDITING_INPUT_H__
#define __TEXT_EDITING_INPUT_H__
#include "BaseLib/TextEditing.h"
#include "ComponentInputEvent.h"
#include "DoubleClick.h"


class CComponent;


#define TEXT_DRAG_DISTANCE	4	//Pixels a press in the selection moves before it drags the selected text.


//Where dragging the selected text is (SetTextDragEnabled).
enum ETextEditingTextDrag
{
	TETD_None,
	TETD_Pressed,	//Pressed in the selection; not moved far enough to drag it yet.
	TETD_Dragging,	//The text is being dragged, a drop caret where it would go.
};


//What a drag selects by: characters after a single press, whole words after a double click, whole lines after a
//triple.
enum ETextEditingDragUnit
{
	TEDU_Character,
	TEDU_Word,
	TEDU_Line,
};


//The input actions that edit a text (CTextArea, CTextField): the keys, given as commands, go to the text's
//CTextEditing; the pointer puts the caret and selects; cut, copy and paste use the window's clipboard.  A double click
//selects the word under the pointer and a triple click its line (with its line feed); dragging on from either selects
//by whole words or lines, keeping the first selected.  A shift press extends the selection by characters.  A line drag
//(StartLineDrag, from a code editor's gutter) selects by whole lines from a press beside the text.  With text drags
//enabled (SetTextDragEnabled), a press in the selection of an editable text that moves more than TEXT_DRAG_DISTANCE
//pixels drags the selected text: a drop caret follows the pointer (GetDropPosition) and the release moves the text
//there, or with control held copies it, as one undo step.  Dropped in the selection or at its ends nothing changes,
//Escape cancels it, and a press in the selection released without dragging puts the caret there.  Shift+Alt (or Alt)
//and button one drag a box selection (StartDragBox); Shift+Alt and the arrows, Home and End move its corner.  A single
//line text has no up and down, pages, tabs, line feeds or line commands.  The pointer's actions can be turned off (a
//text that can't be selected), so its presses go on to the parent.
class CTextEditingInput
{
protected:
	CComponent*				mpcComponent;  //Not held: the text area or field the input reaches.
	CTextEditing*			mpcEditing;  //Not held.
	bool					mbDragging;
	SClickCount				msClicks;
	ETextEditingDragUnit	meUnit;
	size					muiOriginStart;  //What the double or triple click selected, kept as a drag goes on.
	size					muiOriginEnd;
	bool					mbUnitPress;  //The drag being started was started by a press that selected by meUnit.
	bool					mbTextDragEnabled;
	ETextEditingTextDrag	meTextDrag;
	int32					miPressX;  //Where the press in the selection was.
	int32					miPressY;
	size					muiDrop;  //Where the dragged text would be dropped.
	bool					mbDropRowEnd;
	bool					mbBoxDrag;  //The drag is a box selection's (StartDragBox).
	size					muiBoxAnchorRow;  //Where it was pressed.
	int32					miBoxAnchorX;

public:
	void	Init(CComponent* pcComponent, CTextEditing* pcEditing, bool bMultiLine);
	void	Kill(void);

	void	SetPointerEnabled(bool bEnabled);
	void	SetTextDragEnabled(bool bEnabled);
	bool	GetDropPosition(size* puiPosition, bool* pbRowEnd);
	void	FocusLost(void);

	void	PrintableAction(SComponentInputEvent* psEvent);
	void	SpaceAction(SComponentInputEvent* psEvent);
	void	TabAction(SComponentInputEvent* psEvent);
	void	UndoAction(void);
	void	RedoAction(void);
	void	CutToClipboard(void);
	void	CopyToClipboard(void);
	void	PasteFromClipboard(void);

	void	ActivateComponent(int x, int y);
	void	ExecuteComponent(int x, int y);
	void	ContextMenuClick(SComponentInputEvent* psEvent);
	void	StartDragSelect(int x, int y);
	void	ContDragSelect(int x, int y);
	void	StopDragSelect(int x, int y);
	void	StartLineDrag(int x, int y, bool bExtend);
	void	StartDragBox(int x, int y);
	void	ClearSelectionAction(void);

protected:
	void	AddActions(CComponent* pcComponent, CTextEditing* pcEditing, bool bMultiLine);
	bool	FindUnitAt(int x, int y, size* puiStart, size* puiEnd);
	void	DragByUnit(int x, int y);
	bool	IsInSelection(int x, int y);
	void	DragText(int x, int y);
	void	DropText(int x, int y);
	void	CancelTextDrag(void);
	bool	IsCopyHeld(void);
	void	DragBox(int x, int y);
	int32	FindBoxX(size uiRow, int x);
};


#endif // __TEXT_EDITING_INPUT_H__

