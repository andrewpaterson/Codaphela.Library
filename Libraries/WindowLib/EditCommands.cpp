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
#include "Window.h"
#include "ComponentInput.h"
#include "EditCommands.h"


//The standard commands an Edit menu acts with.
static EComponentInputCommand gaeEditCommands[] = { CIC_Undo, CIC_Redo, CIC_Cut, CIC_Copy, CIC_Paste, CIC_PasteFromHistory, CIC_SelectAll };


#define EDIT_EDITOR		COMPONENT_INPUT_EDITOR_CONTEXT
#define EDIT_WINDOW		COMPONENT_INPUT_GLOBAL_CONTEXT


//A standard action as a command whose keys can be changed, with the keys it has until they are.
struct SEditKeymapRow
{
	const char*				szName;
	const char*				szCaption;
	EComponentInputCommand	eCommand;
	const char*				szContext;  //Where its keys are: an editor's, or anywhere (the window's).
	const char*				szKeys;
	const char*				szOtherKeys;  //NULL for none.
};


//The Edit commands, which are there already, have their keys given; the others are made.
static SEditKeymapRow gasEditKeymapRows[] =
{
	{ "Edit.Undo",						"Undo",						CIC_Undo,						EDIT_EDITOR, 	"Control+Z",				NULL },
	{ "Edit.Redo",						"Redo",						CIC_Redo,						EDIT_EDITOR, 	"Control+Shift+Z",			NULL },
	{ "Edit.Cut",						"Cut",						CIC_Cut,						EDIT_EDITOR, 	"Control+X",				"Shift+Delete" },
	{ "Edit.Copy",						"Copy",						CIC_Copy,						EDIT_EDITOR, 	"Control+C",				"Control+Insert" },
	{ "Edit.Paste",						"Paste",					CIC_Paste,						EDIT_EDITOR, 	"Control+V",				"Shift+Insert" },
	{ "Edit.PasteFromHistory",			"Paste from History...",	CIC_PasteFromHistory,			EDIT_EDITOR, 	"Control+Shift+V",			NULL },
	{ "Edit.SelectAll",					"Select All",				CIC_SelectAll,					EDIT_EDITOR, 	"Control+A",				NULL },

	{ "Editor.Up",						"Up",						CIC_Up,							EDIT_EDITOR, 	"Cursor Up",				NULL },
	{ "Editor.Down",					"Down",						CIC_Down,						EDIT_EDITOR, 	"Cursor Down",				NULL },
	{ "Editor.Left",					"Left",						CIC_Left,						EDIT_EDITOR, 	"Cursor Left",				NULL },
	{ "Editor.Right",					"Right",					CIC_Right,						EDIT_EDITOR, 	"Cursor Right",				NULL },
	{ "Editor.WordLeft",				"Previous Word",			CIC_WordLeft,					EDIT_EDITOR, 	"Control+Cursor Left",		NULL },
	{ "Editor.WordRight",				"Next Word",				CIC_WordRight,					EDIT_EDITOR, 	"Control+Cursor Right",		NULL },
	{ "Editor.Home",					"Line Start",				CIC_Home,						EDIT_EDITOR, 	"Home",						NULL },
	{ "Editor.End",						"Line End",					CIC_End,						EDIT_EDITOR, 	"End",						NULL },
	{ "Editor.DocumentHome",			"Document Start",			CIC_DocumentHome,				EDIT_EDITOR, 	"Control+Home",				NULL },
	{ "Editor.DocumentEnd",				"Document End",				CIC_DocumentEnd,				EDIT_EDITOR, 	"Control+End",				NULL },
	{ "Editor.PageUp",					"Page Up",					CIC_PageUp,						EDIT_EDITOR, 	"Page Up",					NULL },
	{ "Editor.PageDown",				"Page Down",				CIC_PageDown,					EDIT_EDITOR, 	"Page Down",				NULL },
	{ "Editor.LineUp",					"Scroll Up",				CIC_LineUp,						EDIT_EDITOR, 	"Control+Cursor Up",		NULL },
	{ "Editor.LineDown",				"Scroll Down",				CIC_LineDown,					EDIT_EDITOR, 	"Control+Cursor Down",		NULL },

	{ "Editor.SelectUp",				"Select Up",				CIC_SelectUp,					EDIT_EDITOR, 	"Shift+Cursor Up",			NULL },
	{ "Editor.SelectDown",				"Select Down",				CIC_SelectDown,					EDIT_EDITOR, 	"Shift+Cursor Down",		NULL },
	{ "Editor.SelectLeft",				"Select Left",				CIC_SelectLeft,					EDIT_EDITOR, 	"Shift+Cursor Left",		NULL },
	{ "Editor.SelectRight",				"Select Right",				CIC_SelectRight,				EDIT_EDITOR, 	"Shift+Cursor Right",		NULL },
	{ "Editor.SelectWordLeft",			"Select Previous Word",		CIC_SelectWordLeft,				EDIT_EDITOR, 	"Control+Shift+Cursor Left",	NULL },
	{ "Editor.SelectWordRight",			"Select Next Word",			CIC_SelectWordRight,			EDIT_EDITOR, 	"Control+Shift+Cursor Right",	NULL },
	{ "Editor.SelectHome",				"Select to Line Start",		CIC_SelectHome,					EDIT_EDITOR, 	"Shift+Home",				NULL },
	{ "Editor.SelectEnd",				"Select to Line End",		CIC_SelectEnd,					EDIT_EDITOR, 	"Shift+End",				NULL },
	{ "Editor.SelectDocumentHome",		"Select to Document Start",	CIC_SelectDocumentHome,			EDIT_EDITOR, 	"Control+Shift+Home",		NULL },
	{ "Editor.SelectDocumentEnd",		"Select to Document End",	CIC_SelectDocumentEnd,			EDIT_EDITOR, 	"Control+Shift+End",		NULL },
	{ "Editor.SelectPageUp",			"Select Page Up",			CIC_SelectPageUp,				EDIT_EDITOR, 	"Shift+Page Up",			NULL },
	{ "Editor.SelectPageDown",			"Select Page Down",			CIC_SelectPageDown,				EDIT_EDITOR, 	"Shift+Page Down",			NULL },
	{ "Editor.ExtendSelection",			"Extend Selection",			CIC_ExtendSelection,			EDIT_EDITOR, 	"Control+W",				NULL },
	{ "Editor.ShrinkSelection",			"Shrink Selection",			CIC_ShrinkSelection,			EDIT_EDITOR, 	"Control+Shift+W",			NULL },
	{ "Editor.BoxSelectUp",				"Column Select Up",			CIC_BoxSelectUp,				EDIT_EDITOR, 	"Alt+Shift+Cursor Up",		NULL },
	{ "Editor.BoxSelectDown",			"Column Select Down",		CIC_BoxSelectDown,				EDIT_EDITOR, 	"Alt+Shift+Cursor Down",	NULL },
	{ "Editor.BoxSelectLeft",			"Column Select Left",		CIC_BoxSelectLeft,				EDIT_EDITOR, 	"Alt+Shift+Cursor Left",	NULL },
	{ "Editor.BoxSelectRight",			"Column Select Right",		CIC_BoxSelectRight,				EDIT_EDITOR, 	"Alt+Shift+Cursor Right",	NULL },
	{ "Editor.BoxSelectHome",			"Column Select to Line Start",	CIC_BoxSelectHome,			EDIT_EDITOR, 	"Alt+Shift+Home",			NULL },
	{ "Editor.BoxSelectEnd",			"Column Select to Line End",	CIC_BoxSelectEnd,			EDIT_EDITOR, 	"Alt+Shift+End",			NULL },
	{ "Editor.Escape",					"Escape",					CIC_ClearSelection,				EDIT_EDITOR, 	"Escape",					NULL },

	{ "Editor.Enter",					"Enter",					CIC_Enter,						EDIT_EDITOR, 	"Enter",					"Numpad Enter" },
	{ "Editor.EnterAtLineStart",		"Enter at Line Start",		CIC_HomeEnter,					EDIT_EDITOR, 	"Control+Shift+Enter",		"Control+Shift+Numpad Enter" },
	{ "Editor.EnterAtLineEnd",			"Enter at Line End",		CIC_EndEnter,					EDIT_EDITOR, 	"Shift+Enter",				"Shift+Numpad Enter" },
	{ "Editor.Tab",						"Tab",						CIC_Tab,						EDIT_EDITOR, 	"Tab",						NULL },
	{ "Editor.BackTab",					"Back Tab",					CIC_BackTab,					EDIT_EDITOR, 	"Shift+Tab",				NULL },
	{ "Editor.Delete",					"Delete",					CIC_Delete,						EDIT_EDITOR, 	"Delete",					NULL },
	{ "Editor.Backspace",				"Backspace",				CIC_Backspace,					EDIT_EDITOR, 	"Backspace",				NULL },
	{ "Editor.DeleteWordLeft",			"Delete Word Left",			CIC_DeleteWordRemainingLeft,	EDIT_EDITOR, 	"Control+Backspace",		NULL },
	{ "Editor.DeleteWordRight",			"Delete Word Right",		CIC_DeleteWordRemainingRight,	EDIT_EDITOR, 	"Control+Delete",			NULL },
	{ "Editor.Duplicate",				"Duplicate",				CIC_Duplicate,					EDIT_EDITOR, 	"Control+D",				NULL },
	{ "Editor.DeleteLine",				"Delete Line",				CIC_DeleteLine,					EDIT_EDITOR, 	"Control+Y",				NULL },
	{ "Editor.ToggleOverwrite",			"Toggle Overwrite",			CIC_ToggleInsert,				EDIT_EDITOR, 	"Insert",					NULL },

	{ "Window.NextDifference",			"Next Difference",			CIC_NextDifference,				EDIT_WINDOW, 	"F7",						NULL },
	{ "Window.PreviousDifference",		"Previous Difference",		CIC_PreviousDifference,			EDIT_WINDOW, 	"Shift+F7",					NULL },
	{ "Window.ContextMenu",				"Show Context Menu",		CIC_ContextMenuKey,				EDIT_WINDOW, 	"Menu",						"Shift+F10" },
	{ "Window.CloseDocument",			"Close Document",			CIC_CloseDocument,				EDIT_WINDOW, 	"Control+F4",				NULL },
	{ "Window.HideToolWindow",			"Hide Tool Window",			CIC_HideToolWindow,				EDIT_WINDOW, 	"Shift+Escape",				NULL },
	{ "Window.JumpToSource",			"Jump to Source",			CIC_JumpToSource,				EDIT_WINDOW, 	"F4",						NULL },
};


//////////////////////////////////////////////////////////////////////////
//
// Adds the Edit commands to pCommands and watches pWindow's focus for
// what they act on.
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::Init(Ptr<CWindow> pWindow, Ptr<CCommandSet> pCommands)
{
	PreInit();
	mpWindow = pWindow;
	mpTarget = NULL;
	AddCommand(pCommands, "Edit.Undo", "Undo", CIC_Undo, &CEditCommands::Undo, &CEditCommands::CanUndo);
	AddCommand(pCommands, "Edit.Redo", "Redo", CIC_Redo, &CEditCommands::Redo, &CEditCommands::CanRedo);
	AddCommand(pCommands, "Edit.Cut", "Cut", CIC_Cut, &CEditCommands::Cut, &CEditCommands::CanCut);
	AddCommand(pCommands, "Edit.Copy", "Copy", CIC_Copy, &CEditCommands::Copy, &CEditCommands::CanCopy);
	AddCommand(pCommands, "Edit.Paste", "Paste", CIC_Paste, &CEditCommands::Paste, &CEditCommands::CanPaste);
	AddCommand(pCommands, "Edit.PasteFromHistory", "Paste from History...", CIC_PasteFromHistory, &CEditCommands::PasteFromHistory, &CEditCommands::CanPasteFromHistory);
	AddCommand(pCommands, "Edit.SelectAll", "Select All", CIC_SelectAll, &CEditCommands::SelectAll, &CEditCommands::CanSelectAll);
	mpWindow->GetFocus()->GetFocusChangeEvent()->Add(this, &CEditCommands::FocusChanged);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::Class(void)
{
	M_Pointer(mpWindow);
	M_Pointer(mpTarget);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::Free(void)
{
	if (mpWindow.IsNotNull())
	{
		mpWindow->GetFocus()->GetFocusChangeEvent()->Remove(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Edit commands aren't saved; the application makes them each run.
//
//////////////////////////////////////////////////////////////////////////
bool CEditCommands::Save(CObjectWriter* pcFile) { return false; }
bool CEditCommands::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::AddCommand(Ptr<CCommandSet> pCommands, const char* szName, const char* szCaption, EComponentInputCommand eStandard, void(CEditCommands::*fRun)(void), bool(CEditCommands::*fCan)(void))
{
	Ptr<CCommand>	pCommand;

	pCommand = pCommands->Add(szName, szCaption, NULL, Ptr<CImageCel>(), this, fRun);
	if (pCommand.IsNotNull())
	{
		pCommand->SetStandardCommand(eStandard);
		pCommand->SetEnabledCheck(this, fCan);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the keys of the Edit commands, and of every other standard action
// that's an editor's or a window's, commands' keys that can be changed.
// They come into effect, in place of the standard ones, when pCommands is
// next bound (CCommandSet::BindKeys).
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::AddKeymapCommands(Ptr<CCommandSet> pCommands)
{
	SEditKeymapRow*	psRow;
	Ptr<CCommand>	pCommand;
	size			i;

	for (i = 0; i < sizeof(gasEditKeymapRows) / sizeof(gasEditKeymapRows[0]); i++)
	{
		psRow = &gasEditKeymapRows[i];
		pCommand = pCommands->Find(psRow->szName);
		if (pCommand.IsNull())
		{
			pCommand = pCommands->Add(psRow->szName, psRow->szCaption, NULL, Ptr<CImageCel>(), this, &CEditCommands::RunStandard);
			pCommand->SetStandardCommand(psRow->eCommand);
		}
		pCommand->SetContext(psRow->szContext);
		pCommand->SetGivesStandard(true);
		pCommand->AddDefaultKeys(psRow->szKeys);
		pCommand->AddDefaultKeys(psRow->szOtherKeys);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What a command made by AddKeymapCommands runs.
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::RunStandard(CCommand* pcCommand)
{
	Run(pcCommand->GetStandardCommand());
}


//////////////////////////////////////////////////////////////////////////
//
// The keys going to a component that takes an edit command make it what
// the commands act on.  A menu taking them doesn't.
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::FocusChanged(CFocus* pcFocus)
{
	Ptr<CComponent>	pFocussed;

	pFocussed = pcFocus->GetFocussedComponent();
	if (pFocussed.IsNotNull() && IsTakingEdits(pFocussed))
	{
		mpTarget = pFocussed;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pComponent has an action for any of the edit commands.
//
//////////////////////////////////////////////////////////////////////////
bool CEditCommands::IsTakingEdits(Ptr<CComponent> pComponent)
{
	size	i;

	for (i = 0; i < sizeof(gaeEditCommands) / sizeof(gaeEditCommands[0]); i++)
	{
		if (pComponent->CComponent::CanDoCommand(gaeEditCommands[i]))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The component the commands act on: the one the keys last went to, if
// it is still in the window (its tab may have closed; it is then let go),
// else GetDefaultTarget's.  NULL if none.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CEditCommands::GetTarget(void)
{
	Ptr<CComponent>	pTop;

	if (mpTarget.IsNotNull())
	{
		for (pTop = mpTarget; pTop->GetParent().IsNotNull(); pTop = pTop->GetParent())
		{
		}
		if (&pTop != &mpWindow)
		{
			mpTarget = NULL;
		}
	}
	return mpTarget.IsNotNull() ? mpTarget : GetDefaultTarget();
}


//////////////////////////////////////////////////////////////////////////
//
// What the commands act on when the keys haven't gone to anything that
// takes them, or it has gone.  None, unless overridden.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CEditCommands::GetDefaultTarget(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The target gets the keys again, then eCommand as if its keys were
// pressed.
//
//////////////////////////////////////////////////////////////////////////
void CEditCommands::Run(EComponentInputCommand eCommand)
{
	Ptr<CComponent>	pTarget;

	pTarget = GetTarget();
	if (pTarget.IsNotNull())
	{
		mpWindow->GetFocus()->SetFocus(pTarget);
		mpWindow->GetFocus()->Command(eCommand, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CEditCommands::Can(EComponentInputCommand eCommand)
{
	Ptr<CComponent>	pTarget;

	pTarget = GetTarget();
	return pTarget.IsNotNull() && pTarget->CanDoCommand(eCommand);
}


void CEditCommands::Undo(void) { Run(CIC_Undo); }
void CEditCommands::Redo(void) { Run(CIC_Redo); }
void CEditCommands::Cut(void) { Run(CIC_Cut); }
void CEditCommands::Copy(void) { Run(CIC_Copy); }
void CEditCommands::Paste(void) { Run(CIC_Paste); }
void CEditCommands::PasteFromHistory(void) { Run(CIC_PasteFromHistory); }
void CEditCommands::SelectAll(void) { Run(CIC_SelectAll); }
bool CEditCommands::CanUndo(void) { return Can(CIC_Undo); }
bool CEditCommands::CanRedo(void) { return Can(CIC_Redo); }
bool CEditCommands::CanCut(void) { return Can(CIC_Cut); }
bool CEditCommands::CanCopy(void) { return Can(CIC_Copy); }
bool CEditCommands::CanPaste(void) { return Can(CIC_Paste); }
bool CEditCommands::CanPasteFromHistory(void) { return Can(CIC_PasteFromHistory); }
bool CEditCommands::CanSelectAll(void) { return Can(CIC_SelectAll); }

