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
#ifndef __WIN_X_INPUT_H__
#define __WIN_X_INPUT_H__
#include <windows.h>
//#undef XINPUT_USE_9_1_0
#include <XInput.h>
#include "BaseLib/Define.h"
#include "InputDevices.h"
#include "InputDeviceValue.h"
#include "NativeDetail.h"
#define MAX_XINPUT_CONTROLLERS	4


class CXInputDetail : public CNativeDetail
{
public:
	BOOL			bConnected;

	unsigned char	ucType;
	unsigned char	ucSubType;
	unsigned short	usFlags;

	CChars			szID;

	void Init(unsigned char	ucType, unsigned char ucSubType, unsigned short	usFlags);
	void Kill(void);
	void AppendDescription(CChars* sz);
	void Disconnect(void);
	void Dump(void);
};


class CWinInput;
class CWinXInput
{
public:
	CXInputDetail	masXInputDetail[MAX_XINPUT_CONTROLLERS];
	CWinInput*		mpcWinInput;

	void 				Init(CWinInput* pcWinInput);
	void 				Kill(void);
	void 				ResetDetails(void);
	void 				Update(CInputDevices* pcDevices, unsigned int uiSequence);
	void 				AddDevicesTo(CInputDevices* pcDevices);
	CInputDeviceDesc*	AddStandardGamepad(CInputDevices* pcDevices);
	void				DumpDetails(void);
};


#endif // __WIN_X_INPUT_H__

