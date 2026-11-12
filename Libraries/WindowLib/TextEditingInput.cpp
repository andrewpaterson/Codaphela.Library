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
#include "Component.h"
#include "Window.h"
#include "Focus.h"
#include "Clipboard.h"
#include "TextEditingInput.h"


//The pointer's commands that select (SetPointerEnabled).  The context menu's press isn't one: it goes on to the parent
//anyway.
static EComponentInputCommand	gaeTextEditingPointerCommands[] = { CIC_ActivateComponent, CIC_ExecuteComponent, CIC_StartDragSelect, CIC_StartDragBox, CIC_ContDragSelect, CIC_StopDragSelect };


//////////////////////////////////////////////////////////////////////////
//
// Adds the input actions to pcComponent.  pcEditing is pcComponent's.
// They keep the layout: the editing's view lays the component out again
// when an edit or a move changes its size or what shows.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::Init(CComponent* pcComponent, CTextEditing* pcEditing, bool bMultiLine)
{
	size	uiFirst;

	mpcComponent = pcComponent;
	mpcEditing = pcEditing;
	meDrag = TED_None;
	meStarting = TED_None;
	msClicks.Init();
	muiOriginStart = 0;
	muiOriginEnd = 0;
	mbTextDragEnabled = false;
	msPress.Init(0, 0);
	meCursor = NC_Arrow;
	muiDrop = 0;
	mbDropRowEnd = false;

	uiFirst = pcComponent->NumInputActions();
	AddActions(pcComponent, pcEditing, bMultiLine);
	pcComponent->SetInputActionsKeepLayout(uiFirst);
}


//////////////////////////////////////////////////////////////////////////
//
// A single line text has no up and down, pages, tabs, line feeds or line
// commands.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::AddActions(CComponent* pcComponent, CTextEditing* pcEditing, bool bMultiLine)
{
	pcComponent->AddInputAction(CIC_ActivateComponent, this, &CTextEditingInput::ActivateComponent);
	pcComponent->AddInputAction(CIC_ExecuteComponent, this, &CTextEditingInput::ExecuteComponent);
	pcComponent->AddInputAction(CIC_ContextMenu, this, &CTextEditingInput::ContextMenuClick);
	pcComponent->AddInputAction(CIC_StartDragSelect, this, &CTextEditingInput::StartDragSelect);
	pcComponent->AddInputAction(CIC_StartDragBox, this, &CTextEditingInput::StartDragBox);
	pcComponent->AddInputAction(CIC_ContDragSelect, this, &CTextEditingInput::ContDragSelect);
	pcComponent->AddInputAction(CIC_StopDragSelect, this, &CTextEditingInput::StopDragSelect);

	pcComponent->AddInputAction(CIC_Left, pcEditing, &CTextEditing::Left);
	pcComponent->AddInputAction(CIC_Right, pcEditing, &CTextEditing::Right);
	pcComponent->AddInputAction(CIC_WordLeft, pcEditing, &CTextEditing::WordLeft);
	pcComponent->AddInputAction(CIC_WordRight, pcEditing, &CTextEditing::WordRight);
	pcComponent->AddInputAction(CIC_Home, pcEditing, &CTextEditing::Home);
	pcComponent->AddInputAction(CIC_End, pcEditing, &CTextEditing::End);
	pcComponent->AddInputAction(CIC_DocumentHome, pcEditing, &CTextEditing::DocumentHome);
	pcComponent->AddInputAction(CIC_DocumentEnd, pcEditing, &CTextEditing::DocumentEnd);
	pcComponent->AddInputAction(CIC_SelectLeft, pcEditing, &CTextEditing::SelectLeft);
	pcComponent->AddInputAction(CIC_SelectRight, pcEditing, &CTextEditing::SelectRight);
	pcComponent->AddInputAction(CIC_SelectWordLeft, pcEditing, &CTextEditing::SelectWordLeft);
	pcComponent->AddInputAction(CIC_SelectWordRight, pcEditing, &CTextEditing::SelectWordRight);
	pcComponent->AddInputAction(CIC_SelectHome, pcEditing, &CTextEditing::SelectHome);
	pcComponent->AddInputAction(CIC_SelectEnd, pcEditing, &CTextEditing::SelectEnd);
	pcComponent->AddInputAction(CIC_SelectDocumentHome, pcEditing, &CTextEditing::SelectDocumentHome);
	pcComponent->AddInputAction(CIC_SelectDocumentEnd, pcEditing, &CTextEditing::SelectDocumentEnd);
	pcComponent->AddInputAction(CIC_ExtendSelection, pcEditing, &CTextEditing::ExtendSelection);
	pcComponent->AddInputAction(CIC_ShrinkSelection, pcEditing, &CTextEditing::ShrinkSelection);
	pcComponent->AddInputAction(CIC_BoxSelectUp, pcEditing, &CTextEditing::BoxSelectUp);
	pcComponent->AddInputAction(CIC_BoxSelectDown, pcEditing, &CTextEditing::BoxSelectDown);
	pcComponent->AddInputAction(CIC_BoxSelectLeft, pcEditing, &CTextEditing::BoxSelectLeft);
	pcComponent->AddInputAction(CIC_BoxSelectRight, pcEditing, &CTextEditing::BoxSelectRight);
	pcComponent->AddInputAction(CIC_BoxSelectHome, pcEditing, &CTextEditing::BoxSelectHome);
	pcComponent->AddInputAction(CIC_BoxSelectEnd, pcEditing, &CTextEditing::BoxSelectEnd);
	pcComponent->AddInputAction(CIC_SelectAll, pcEditing, &CTextEditing::SelectAll);
	pcComponent->AddInputAction(CIC_ClearSelection, this, &CTextEditingInput::ClearSelectionAction);

	pcComponent->AddInputAction(CIC_Space, this, &CTextEditingInput::SpaceAction);
	pcComponent->AddInputAction(CIC_Delete, pcEditing, &CTextEditing::Delete);
	pcComponent->AddInputAction(CIC_Backspace, pcEditing, &CTextEditing::Backspace);
	pcComponent->AddInputAction(CIC_DeleteWordRemainingLeft, pcEditing, &CTextEditing::DeleteWordRemainingLeft);
	pcComponent->AddInputAction(CIC_DeleteWordRemainingRight, pcEditing, &CTextEditing::DeleteWordRemainingRight);
	pcComponent->AddInputAction(CIC_Undo, this, &CTextEditingInput::UndoAction);
	pcComponent->AddInputAction(CIC_Redo, this, &CTextEditingInput::RedoAction);
	pcComponent->AddInputAction(CIC_Cut, this, &CTextEditingInput::CutToClipboard);
	pcComponent->AddInputAction(CIC_Copy, this, &CTextEditingInput::CopyToClipboard);
	pcComponent->AddInputAction(CIC_Paste, this, &CTextEditingInput::PasteFromClipboard);
	pcComponent->AddInputAction(IAF_Printable, NULL, this, &CTextEditingInput::PrintableAction);

	if (!bMultiLine)
	{
		return;
	}

	pcComponent->AddInputAction(CIC_Up, pcEditing, &CTextEditing::Up);
	pcComponent->AddInputAction(CIC_Down, pcEditing, &CTextEditing::Down);
	pcComponent->AddInputAction(CIC_PageUp, pcEditing, &CTextEditing::PageUp);
	pcComponent->AddInputAction(CIC_PageDown, pcEditing, &CTextEditing::PageDown);
	pcComponent->AddInputAction(CIC_SelectUp, pcEditing, &CTextEditing::SelectUp);
	pcComponent->AddInputAction(CIC_SelectDown, pcEditing, &CTextEditing::SelectDown);
	pcComponent->AddInputAction(CIC_SelectPageUp, pcEditing, &CTextEditing::SelectPageUp);
	pcComponent->AddInputAction(CIC_SelectPageDown, pcEditing, &CTextEditing::SelectPageDown);

	pcComponent->AddInputAction(CIC_Tab, this, &CTextEditingInput::TabAction);
	pcComponent->AddInputAction(CIC_BackTab, pcEditing, &CTextEditing::BackTab);
	pcComponent->AddInputAction(CIC_Enter, pcEditing, &CTextEditing::Enter);
	pcComponent->AddInputAction(CIC_HomeEnter, pcEditing, &CTextEditing::HomeEnter);
	pcComponent->AddInputAction(CIC_EndEnter, pcEditing, &CTextEditing::EndEnter);
	pcComponent->AddInputAction(CIC_Duplicate, pcEditing, &CTextEditing::Duplicate);
	pcComponent->AddInputAction(CIC_DeleteLine, pcEditing, &CTextEditing::DeleteLine);
}


//////////////////////////////////////////////////////////////////////////
//
// The actions go with the component.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::Kill(void)
{
	mpcComponent = NULL;
	mpcEditing = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Off, the pointer's presses go on to the component's parent.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::SetPointerEnabled(bool bEnabled)
{
	size	i;

	for (i = 0; i < sizeof(gaeTextEditingPointerCommands) / sizeof(EComponentInputCommand); i++)
	{
		mpcComponent->SetInputActionsEnabled(gaeTextEditingPointerCommands[i], bEnabled);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Losing focus, as when the window is deactivated, ends a drag, and the
// next press is a single click.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::FocusLost(void)
{
	SetDrag(TED_None);
	msClicks.Forget();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a press in the selection drags the selected text (a text area's
// does; a text field's doesn't, as it shows no drop caret).
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::SetTextDragEnabled(bool bEnabled)
{
	mbTextDragEnabled = bEnabled;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the text being dragged would be dropped, for the view to draw the
// drop caret there.  False if no text is being dragged.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditingInput::GetDropPosition(size* puiPosition, bool* pbRowEnd)
{
	if (meDrag != TED_Text)
	{
		return false;
	}
	*puiPosition = muiDrop;
	*pbRowEnd = mbDropRowEnd;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A space is typed as a printable character; a tab never overwrites.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::PrintableAction(SComponentInputEvent* psEvent) { mpcEditing->Printable(psEvent->cPrintable, psEvent->bInsert); }
void CTextEditingInput::SpaceAction(SComponentInputEvent* psEvent) { mpcEditing->Printable(' ', psEvent->bInsert); }
void CTextEditingInput::TabAction(SComponentInputEvent* psEvent) { mpcEditing->Tab(psEvent->bInsert); }
void CTextEditingInput::UndoAction(void) { mpcEditing->Undo(); }
void CTextEditingInput::RedoAction(void) { mpcEditing->Redo(); }


//////////////////////////////////////////////////////////////////////////
//
// The clipboard is the window's, the system's.  Cutting or copying
// nothing leaves it as it was.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::CutToClipboard(void) { mpcComponent->GetWindow()->GetClipboard()->CopyFrom(mpcEditing, &CTextEditing::Cut); }
void CTextEditingInput::CopyToClipboard(void) { mpcComponent->GetWindow()->GetClipboard()->CopyFrom(mpcEditing, &CTextEditing::Copy); }
void CTextEditingInput::PasteFromClipboard(void) { mpcComponent->GetWindow()->GetClipboard()->PasteInto(mpcEditing, &CTextEditing::Paste); }


//////////////////////////////////////////////////////////////////////////
//
// The press has already given the component focus.  A single press puts
// the caret in the gap nearest the pointer; a double click selects the
// word under it and a triple click its line.  Dragging selects from
// there.  A double click past a line's end selects nothing.  A single
// press in the selection may drag the selected text instead (DragText).
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ActivateComponent(int x, int y)
{
	ETextEditingDrag	eUnit;

	switch (msClicks.Pressed(x, y))
	{
	case 1:
		if (IsInSelection(x, y))
		{
			msPress.Init(x, y);
			StartDrag(TED_TextPressed, x, y);
		}
		else
		{
			mpcEditing->MoveCaretTo(x, y, false);
			StartDrag(TED_Characters, x, y);
		}
		break;
	default:
		eUnit = (msClicks.iCount == 2) ? TED_Words : TED_Lines;
		FindUnitAt(eUnit, x, y, &muiOriginStart, &muiOriginEnd);
		mpcEditing->Select(muiOriginStart, muiOriginEnd);
		StartDrag(eUnit, x, y);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ExecuteComponent(int x, int y)
{
	mpcComponent->GetWindow()->GetFocus()->StopDrag(FD_Select);
}


//////////////////////////////////////////////////////////////////////////
//
// The right button, as in Visual Studio: pressed on a selected character
// the selection is kept, so the context menu acts on it; anywhere else
// the caret moves there and nothing is selected.  The press goes on up to
// the component with the context menu.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ContextMenuClick(SComponentInputEvent* psEvent)
{
	psEvent->bPass = true;

	if (!mpcEditing->IsSelectedAt(psEvent->x, psEvent->y))
	{
		mpcEditing->MoveCaretTo(psEvent->x, psEvent->y, false);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A press starts dragging eDrag from x, y: the focus starts the drag,
// which comes back as StartDragSelect, where eDrag is what it finds being
// started.  A drag the focus doesn't start (nothing has it) isn't begun.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::StartDrag(ETextEditingDrag eDrag, int x, int y)
{
	meStarting = eDrag;
	mpcComponent->GetWindow()->GetFocus()->StartDrag(FD_Select, x, y);
	meStarting = TED_None;
}


//////////////////////////////////////////////////////////////////////////
//
// A drag begins: the one a press here is starting (StartDrag), which
// selects from what the press selected.  Otherwise it is a shift press:
// it extends the selection from where it started, or from the caret, by
// characters.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::StartDragSelect(int x, int y)
{
	meDrag = meStarting;
	if (meDrag == TED_None)
	{
		meDrag = TED_Characters;
		msClicks.Forget();
	}
	if (meDrag == TED_Characters)
	{
		mpcEditing->AnchorSelection();
	}
	ContDragSelect(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer moved to x, y during the drag.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ContDragSelect(int x, int y)
{
	switch (meDrag)
	{
	case TED_Characters:
		mpcEditing->MoveCaretTo(x, y, true);
		break;
	case TED_Words:
	case TED_Lines:
		DragByUnit(x, y);
		break;
	case TED_TextPressed:
	case TED_Text:
		DragText(x, y);
		break;
	case TED_Box:
		DragBox(x, y);
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Button one was released: the drag ends where the pointer is.  The
// selected text is dropped there, copied if control is held.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::StopDragSelect(SComponentInputEvent* psEvent)
{
	if (IsTextDrag())
	{
		DropText(psEvent->x, psEvent->y, psEvent->bControl);
	}
	else
	{
		ContDragSelect(psEvent->x, psEvent->y);
	}
	SetDrag(TED_None);
}


//////////////////////////////////////////////////////////////////////////
//
// What is being dragged changes.  While it is the selected text the
// pointer is NC_Move over the component, whose own shape is put back
// after.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::SetDrag(ETextEditingDrag eDrag)
{
	if ((eDrag == TED_Text) && (meDrag != TED_Text))
	{
		meCursor = mpcComponent->GetCursor();
		mpcComponent->SetCursor(NC_Move);
	}
	else if ((eDrag != TED_Text) && (meDrag == TED_Text))
	{
		mpcComponent->SetCursor(meCursor);
	}
	meDrag = eDrag;
}


//////////////////////////////////////////////////////////////////////////
//
// The drag ends where it is; the release still to come does nothing.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::CancelDrag(void)
{
	SetDrag(TED_None);
	mpcComponent->GetWindow()->GetFocus()->StopDrag(FD_Select);
	mpcComponent->GetWindow()->RequestPaint();
}


bool CTextEditingInput::IsTextDrag(void) { return (meDrag == TED_TextPressed) || (meDrag == TED_Text); }


//////////////////////////////////////////////////////////////////////////
//
// A press beside the text, in a code editor's gutter, on the row y is in:
// its line is selected with its line feed, and dragging on selects by
// whole lines from it.  Extending (a shift press), the selection's anchor
// stays where it is and the selection goes to the whole line under the
// pointer.  The component must have the focus, as the drag goes to it;
// the focus coming to it from the gutter forgot the presses counted.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::StartLineDrag(int x, int y, bool bExtend)
{
	if (bExtend)
	{
		muiOriginStart = mpcEditing->GetAnchorPosition();
		muiOriginEnd = muiOriginStart;
	}
	else
	{
		FindUnitAt(TED_Lines, x, y, &muiOriginStart, &muiOriginEnd);
		mpcEditing->Select(muiOriginStart, muiOriginEnd);
	}
	StartDrag(TED_Lines, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// Shift+Alt (or Alt) and button one, as Visual Studio's: a box selection
// starts at the press and is dragged from it.  The press has given the
// component focus.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::StartDragBox(int x, int y)
{
	CTextEditingView*	pcView;
	size				uiRow;
	int32				iBoxX;

	pcView = mpcEditing->GetView();
	if (!pcView->HasRows())
	{
		return;
	}
	msClicks.Forget();
	uiRow = pcView->FindRowAt(y);
	iBoxX = FindBoxX(uiRow, x);
	mpcEditing->BoxSelect(uiRow, iBoxX, uiRow, iBoxX);
	StartDrag(TED_Box, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// The box's caret corner goes to the row and x the pointer is at now.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::DragBox(int x, int y)
{
	size	uiRow;

	uiRow = mpcEditing->GetView()->FindRowAt(y);
	mpcEditing->MoveBoxCaret(uiRow, FindBoxX(uiRow, x));
}


//////////////////////////////////////////////////////////////////////////
//
// A box's x for the pointer at x on a row: the gap nearest it, or past
// the row's end the pointer's own x, so a box can be wider than the rows
// it is dragged over.
//
//////////////////////////////////////////////////////////////////////////
int32 CTextEditingInput::FindBoxX(size uiRow, int x)
{
	CTextEditingView*	pcView;
	size				uiPosition;
	bool				bRowEnd;
	int32				iGapX;

	pcView = mpcEditing->GetView();
	uiPosition = pcView->FindPositionInRow(uiRow, x, &bRowEnd);
	iGapX = pcView->GetPositionX(uiPosition, bRowEnd);
	if ((uiPosition >= pcView->GetRowEnd(uiRow, &bRowEnd)) && (x > iGapX))
	{
		return x;
	}
	return iGapX;
}


//////////////////////////////////////////////////////////////////////////
//
// Escape ends a drag.  The selected text being dragged stays where it is,
// still selected; otherwise the selection is cleared, a box too.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ClearSelectionAction(void)
{
	bool	bTextDrag;

	bTextDrag = IsTextDrag();
	if (meDrag != TED_None)
	{
		CancelDrag();
	}
	if (!bTextDrag)
	{
		mpcEditing->ClearSelection();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The press at x, y is on a selected character of an editable text that
// drags its selected text.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditingInput::IsInSelection(int x, int y)
{
	return mbTextDragEnabled && mpcEditing->IsEditable() && mpcEditing->IsSelectedAt(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// The pointer moved, pressed in the selection: once it is more than
// TEXT_DRAG_DISTANCE from the press the selected text is dragged, and the
// drop caret follows it.  The view shows the drop caret as its caret, so
// showing it scrolls to it as a selection's drag scrolls to the caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::DragText(int x, int y)
{
	size	uiDrop;
	bool	bRowEnd;

	if ((meDrag == TED_TextPressed) && IsPointerNear(x, y, &msPress, TEXT_DRAG_DISTANCE))
	{
		return;
	}
	if (!mpcEditing->FindPositionAt(x, y, &uiDrop, &bRowEnd))
	{
		return;
	}
	if ((meDrag != TED_Text) || (uiDrop != muiDrop) || (bRowEnd != mbDropRowEnd))
	{
		SetDrag(TED_Text);
		muiDrop = uiDrop;
		mbDropRowEnd = bRowEnd;
		mpcEditing->GetView()->ShowCaret();
		mpcComponent->GetWindow()->GetFocus()->ResetCaretBlink();
		mpcComponent->GetWindow()->RequestPaint();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The release of a press in the selection.  Dragged, the selected text is
// moved to where it was dropped, or with bCopy copied there.  Not
// dragged, the caret goes where it was pressed.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::DropText(int x, int y, bool bCopy)
{
	size	uiStart;
	size	uiEnd;

	DragText(x, y);
	if (meDrag == TED_TextPressed)
	{
		mpcEditing->MoveCaretTo(msPress.x, msPress.y, false);
		return;
	}

	SetDrag(TED_None);
	mpcEditing->GetSelection(&uiStart, &uiEnd);
	mpcEditing->MoveText(uiStart, uiEnd, muiDrop, bCopy);
	mpcComponent->GetWindow()->RequestPaint();
}


//////////////////////////////////////////////////////////////////////////
//
// The word (TED_Words) or line (TED_Lines) under x, y.  Past a line's end
// there is no word: the start and end are the gap nearest x, y and false
// is returned.
//
//////////////////////////////////////////////////////////////////////////
bool CTextEditingInput::FindUnitAt(ETextEditingDrag eUnit, int x, int y, size* puiStart, size* puiEnd)
{
	size	uiPosition;
	bool	bRowEnd;

	if (eUnit == TED_Lines)
	{
		mpcEditing->FindPositionAt(x, y, &uiPosition, &bRowEnd);
		mpcEditing->GetNavigation()->FindLineAround(uiPosition, puiStart, puiEnd);
		return true;
	}

	if (!mpcEditing->FindCharacterAt(x, y, &uiPosition))
	{
		*puiStart = uiPosition;
		*puiEnd = uiPosition;
		return false;
	}
	return mpcEditing->GetNavigation()->FindWordAround(uiPosition, puiStart, puiEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// Dragging after a double or triple click: the first word or line stays
// selected and the selection grows by whole words or lines to the one
// under the pointer, the caret at its far side.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::DragByUnit(int x, int y)
{
	size	uiStart;
	size	uiEnd;

	FindUnitAt(meDrag, x, y, &uiStart, &uiEnd);
	if (uiStart < muiOriginStart)
	{
		mpcEditing->Select(muiOriginEnd, uiStart);
	}
	else if (uiEnd > muiOriginEnd)
	{
		mpcEditing->Select(muiOriginStart, uiEnd);
	}
	else
	{
		mpcEditing->Select(muiOriginStart, muiOriginEnd);
	}
}

