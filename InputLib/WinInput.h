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
#ifndef __WIN_INPUT_H__
#define __WIN_INPUT_H__
#include "BaseLib/Define.h"
#include "WinDirectInput.h"
#include "WinXInput.h"
#include "WinRaw.h"
#include "InputDevices.h"
#include "InputDeviceValue.h"


class CWinInput
{
public:
	CDirectInput	mcJoystick;
	CWinRawInput	mcRaw;
	CWinXInput		mcXInput;
	HWND			mhWnd;
	BOOL			mbExclusive;
	BOOL			mbNotInitialised;
	BOOL			mbUsable;

	void Init(void);
	void Init(HWND hWnd, BOOL bExclusive);
	void Kill(void);
	void AddDevicesTo(CInputDevices* pcDevices);
	void Update(CInputDevices* pcDevices, unsigned int uiSequence);

	CDirectInput*	GetJoystick(void);
	CWinRawInput*	GetRaw(void);
	CWinXInput*		GetXInput(void);
	HWND			GetHWnd(void);
	BOOL			IsExclusive(void);
	BOOL			IsNotInitialised(void);
	BOOL			IsUsable(void);
};


#endif // __WIN_INPUT_H__

