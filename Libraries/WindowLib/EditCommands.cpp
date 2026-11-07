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
#include "EditCommands.h"


//The standard commands an Edit menu acts with.
static EComponentInputCommand gaeEditCommands[] = { CIC_Undo, CIC_Redo, CIC_Cut, CIC_Copy, CIC_Paste, CIC_SelectAll };


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
// The component the commands act on, NULL if there is none or it isn't in
// the window any more (its tab closed, say), which is then let go.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CComponent> CEditCommands::GetTarget(void)
{
	Ptr<CComponent>	pTop;

	if (mpTarget.IsNull())
	{
		return NULL;
	}
	for (pTop = mpTarget; pTop->GetParent().IsNotNull(); pTop = pTop->GetParent())
	{
	}
	if (&pTop != &mpWindow)
	{
		mpTarget = NULL;
	}
	return mpTarget;
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
void CEditCommands::SelectAll(void) { Run(CIC_SelectAll); }
bool CEditCommands::CanUndo(void) { return Can(CIC_Undo); }
bool CEditCommands::CanRedo(void) { return Can(CIC_Redo); }
bool CEditCommands::CanCut(void) { return Can(CIC_Cut); }
bool CEditCommands::CanCopy(void) { return Can(CIC_Copy); }
bool CEditCommands::CanPaste(void) { return Can(CIC_Paste); }
bool CEditCommands::CanSelectAll(void) { return Can(CIC_SelectAll); }

