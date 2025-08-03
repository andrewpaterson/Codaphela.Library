/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "InputDevice.h"
#include "InputDevices.h"
#include "InputActions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputActions::Init(CInputDevices* pcInputDevices)
{
	mpcInputDevices = pcInputDevices;
	mcActions.Init();
	AddActionWithListener<CInputDeviceVariableListener>(INPUT_DEVICE_VARIABLE_ACTION, (ListenerFunc)&CInputActions::VariableAction, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputActions::Kill(void)
{
	mcActions.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAction* CInputActions::GetAction(char* szName)
{
	return mcActions.GetAction(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputActions::VariableAction(CUnknown* pcSource, void* pvContext)
{
	SChordInputEvent*		psChordInputEvent;

	psChordInputEvent = (SChordInputEvent*)pvContext;
	psChordInputEvent->pcValue->pcDevice->VariableAction(psChordInputEvent->pcChord);
}

