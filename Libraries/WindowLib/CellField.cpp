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
#include "StandardLib/Objects.h"
#include "CellField.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCellField::Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText)
{
	PreInit();

	CTextField::Init(pWindow, pTheme, szText);
	mcEnded.Init();
	AddInputAction(CIC_Enter, this, &CCellField::Commit);
	AddInputAction(CIC_ClearSelection, this, &CCellField::Cancel);
	AddInputAction(CIC_Up, this, &CCellField::CommitUp);
	AddInputAction(CIC_Down, this, &CCellField::CommitDown);
	AddInputAction(CIC_Tab, this, &CCellField::CommitNext);
	AddInputAction(CIC_BackTab, this, &CCellField::CommitPrevious);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCellField::Class(void)
{
	CTextField::Class();

	U_Unknown(CCellFieldEvent, mcEnded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCellField::Free(void)
{
	mcEnded.Kill();
	CTextField::Free();
}


bool CCellField::WantsEnter(void) { return true; }
bool CCellField::WantsEscape(void) { return true; }
bool CCellField::IsTabStop(void) const { return false; }  //It is reached by editing a cell, not by Tab.
CCellFieldEvent* CCellField::GetEndedEvent(void) { return &mcEnded; }
void CCellField::Commit(void) { mcEnded.Call(this, CFE_Commit); }
void CCellField::Cancel(void) { mcEnded.Call(this, CFE_Cancel); }
void CCellField::CommitUp(void) { mcEnded.Call(this, CFE_Up); }
void CCellField::CommitDown(void) { mcEnded.Call(this, CFE_Down); }
void CCellField::CommitNext(void) { mcEnded.Call(this, CFE_Next); }
void CCellField::CommitPrevious(void) { mcEnded.Call(this, CFE_Previous); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCellField::FocusLost(void)
{
	CTextField::FocusLost();
	mcEnded.Call(this, CFE_Commit);
}

