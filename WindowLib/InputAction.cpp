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
#include "InputAction.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputAction::Init(EInputActionFilter eFilter, const char* szName, EComponentInputCommand eCommand, void* pvObject, InputActionThunk fThunk, void* pvFunction, size uiFunctionSize)
{
	PreInit();

	meFilter = eFilter;
	mszName.Init(szName);
	meCommand = eCommand;
	mpvObject = pvObject;
	mfThunk = fThunk;
	memset(maucFunction, 0, INPUT_ACTION_FUNCTION_SIZE);
	memcpy(maucFunction, pvFunction, uiFunctionSize);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputAction::Class(void)
{
	U_Enum(meFilter);
	U_String(mszName);
	U_Enum(meCommand);
	U_Pointer(mpvObject);
	U_Pointer(mfThunk);
	U_UInt8Array(maucFunction, INPUT_ACTION_FUNCTION_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputAction::Free(void)
{
	mszName.Kill();
	mpvObject = NULL;
	mfThunk = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputAction::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputAction::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputAction::Matches(SComponentInputEvent* psEvent)
{
	switch (meFilter)
	{
	case IAF_AnyKeyDown:
		return psEvent->eType == CIET_KeyDown;
	case IAF_AnyKeyUp:
		return psEvent->eType == CIET_KeyUp;
	case IAF_KeyDown:
		return (psEvent->eType == CIET_KeyDown) && (psEvent->szName != NULL) && mszName.Equals(psEvent->szName);
	case IAF_KeyUp:
		return (psEvent->eType == CIET_KeyUp) && (psEvent->szName != NULL) && mszName.Equals(psEvent->szName);
	case IAF_AnyChord:
		return psEvent->eType == CIET_Chord;
	case IAF_Chord:
		return (psEvent->eType == CIET_Chord) && (psEvent->szName != NULL) && mszName.Equals(psEvent->szName);
	case IAF_Printable:
		return psEvent->eType == CIET_Printable;
	case IAF_AnyCommand:
		return psEvent->eType == CIET_Command;
	case IAF_Command:
		return (psEvent->eType == CIET_Command) && (psEvent->eCommand == meCommand);
	case IAF_Movement:
		return psEvent->eType == CIET_Movement;
	case IAF_Wheel:
		return psEvent->eType == CIET_Wheel;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputAction::Call(SComponentInputEvent* psEvent)
{
	mfThunk(mpvObject, maucFunction, psEvent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EInputActionFilter CInputAction::GetFilter(void) { return meFilter; }
char* CInputAction::GetActionName(void) { return mszName.Text(); }
EComponentInputCommand CInputAction::GetCommand(void) { return meCommand; }
