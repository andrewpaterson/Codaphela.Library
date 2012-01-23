/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "WinInput.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinInput::Init(void)
{
	mbNotInitialised = FALSE;
	mbUsable = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinInput::Init(HWND hWnd, BOOL bExclusive)
{
	mbNotInitialised = FALSE;
	mbUsable = TRUE;
	mhWnd = hWnd;
	mbExclusive = bExclusive;
	mcRaw.Init(this);
	mcJoystick.Init(this);
	mcXInput.Init(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinInput::Kill(void)
{
	if ((mbNotInitialised == FALSE) && (mbUsable == TRUE))
	{
		mcXInput.Kill();
		mcJoystick.Kill();
		mcRaw.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinInput::AddDevicesTo(CInputDevices* pcDevices)
{
	mcRaw.AddDevicesTo(pcDevices);
	mcJoystick.AddDevicesTo(pcDevices);
	mcXInput.AddDevicesTo(pcDevices);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinInput::Update(CInputDevices* pcDevices, unsigned int uiSequence)
{
	if ((mbNotInitialised == FALSE) && (mbUsable == TRUE))
	{
		mcRaw.Update(pcDevices, uiSequence);
		mcJoystick.Update(pcDevices, uiSequence);
		mcXInput.Update(pcDevices, uiSequence);
	}
}


CDirectInput* CWinInput::GetJoystick(void) { return &mcJoystick; }
CWinRawInput* CWinInput::GetRaw(void) { return &mcRaw; }
CWinXInput* CWinInput::GetXInput(void) { return &mcXInput; }
HWND CWinInput::GetHWnd(void) { return mhWnd; }
BOOL CWinInput::IsExclusive(void) { return mbExclusive; }
BOOL CWinInput::IsNotInitialised(void) { return mbNotInitialised; }
BOOL CWinInput::IsUsable(void) { return mbUsable; }
