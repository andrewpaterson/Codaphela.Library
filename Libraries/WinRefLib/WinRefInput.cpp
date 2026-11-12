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
#include "BaseLib/Logger.h"
#include "WinRefInput.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefInput::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefInput::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The standard keyboard and mouse, at rest.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefInput::AddDevicesTo(CInputDevices* pcDevices)
{
	AddDevice(pcDevices, "WinRAW Keyboard: Standard 104 Button Keyboard");
	AddDevice(pcDevices, "WinRAW Mouse: Standard 5 Button Mouse");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefInput::AddDevice(CInputDevices* pcDevices, const char* szDescription)
{
	CInputDeviceDesc*	pcDesc;
	CInputDevice*		pcDevice;

	pcDesc = pcDevices->GetDescription(szDescription);
	if (pcDesc == NULL)
	{
		gcLogger.Error2(__METHOD__, " [", szDescription, "] has not been described.  Has WinInputLib's XML been read?", NULL);
		return;
	}
	pcDevice = pcDevices->CreateDevice(pcDesc);
	pcDevice->GenerateRestState();
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing is ever pressed.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefInput::Update(CInputDevices* pcDevices, uint32 uiSequence)
{
}

