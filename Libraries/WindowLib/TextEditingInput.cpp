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
static EComponentInputCommand	gaeTextEditingPointerCommands[] = { CIC_ActivateComponent, CIC_ExecuteComponent, CIC_StartDragSelect, CIC_ContDragSelect, CIC_StopDragSelect };


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
	mbDragging = false;

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
	pcComponent->AddInputAction(CIC_SelectWholeWord, pcEditing, &CTextEditing::SelectWholeWord);
	pcComponent->AddInputAction(CIC_SelectAll, pcEditing, &CTextEditing::SelectAll);
	pcComponent->AddInputAction(CIC_ClearSelection, pcEditing, &CTextEditing::ClearSelection);

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
// Losing focus, as when the window is deactivated, ends a drag.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::FocusLost(void)
{
	mbDragging = false;
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
// The press has already given the component focus.  The caret goes to
// the gap nearest the pointer and dragging selects from there.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ActivateComponent(int x, int y)
{
	size	uiPosition;
	bool	bRowEnd;

	mpcEditing->FindPositionAt(x, y, &uiPosition, &bRowEnd);
	mpcEditing->MoveCaret(uiPosition, false, bRowEnd);
	mpcComponent->GetWindow()->GetFocus()->StartDrag(FD_Select, x, y);
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
// The right button, as in Visual Studio: pressed in the selection it is
// kept, so the context menu acts on it; anywhere else the caret moves
// there and nothing is selected.  The press goes on up to the component
// with the context menu.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ContextMenuClick(SComponentInputEvent* psEvent)
{
	size	uiPosition;
	size	uiStart;
	size	uiEnd;
	bool	bRowEnd;

	psEvent->bPass = true;

	mpcEditing->FindPositionAt(psEvent->x, psEvent->y, &uiPosition, &bRowEnd);
	if (mpcEditing->GetSelection(&uiStart, &uiEnd) && (uiPosition >= uiStart) && (uiPosition < uiEnd))
	{
		return;
	}
	mpcEditing->MoveCaret(uiPosition, false, bRowEnd);
}


//////////////////////////////////////////////////////////////////////////
//
// A shift press extends the selection from where it started, or from the
// caret.
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::StartDragSelect(int x, int y)
{
	mpcEditing->AnchorSelection();
	mbDragging = true;
	ContDragSelect(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::ContDragSelect(int x, int y)
{
	size	uiPosition;
	bool	bRowEnd;

	if (mbDragging && mpcEditing->FindPositionAt(x, y, &uiPosition, &bRowEnd))
	{
		mpcEditing->MoveCaret(uiPosition, true, bRowEnd);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextEditingInput::StopDragSelect(int x, int y)
{
	ContDragSelect(x, y);
	mbDragging = false;
}

