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
#ifndef __WIN_DIRECT_INPUT_H__
#define __WIN_DIRECT_INPUT_H__
#include "BaseLib/Define.h"
#define DIRECTINPUT_VERSION 0x0800
#include <DInput.h>
#include "BaseLib/ArrayTemplate.h"
#include "InputLib/InputDevices.h"
#include "InputLib/InputDeviceValue.h"
#include "InputLib/NativeDetail.h"


class CDirectInputDetail : public CNativeDetail
{
public:
	GUID    				guidInstance;
	GUID    				guidProduct;
	CChars    				szInstanceName;
	CChars    				szProductName;
	GUID    				guidFFDriver;
	WORD    				wUsagePage;
	WORD    				wUsage;
	LPDIRECTINPUTDEVICE8	lpDIDevice;
	bool					bXInput;
	CChars					szID;
	
	void Init(GUID guidInstance, GUID guidProduct, char* szInstanceName,  char* szProductName, GUID guidFFDriver, WORD wUsagePage, WORD wUsage);
	void Kill(void);
	void AppendDescription(CChars* sz);
	void ReleaseDevice(void);
	void Dump(void);
};


typedef CArrayTemplate<CDirectInputDetail> CArrayDirectInputDetail;


class CWinInput;
class CDirectInput
{
public:
	LPDIRECTINPUT8				mpDInput;
	CArrayDirectInputDetail		masDIJoystickDetail;
	CWinInput*					mpcWinInput;

	void 				Init(CWinInput* pcWinInput);
	void 				Kill(void);
	void 				Update(void);
	void 				ResetDetails(void);
	void 				CreateDevices(void);
	void 				ReleaseDevices(void);
	void 				AddDetails(DIDEVICEINSTANCE* pcdiDeviceInstance);
	bool 				IsXInputDevice(CDirectInputDetail* psDetail);
	void 				AddDevicesTo(CInputDevices* pcDevices);
	CInputDeviceDesc*	AddStandardJoystick(CInputDevices* pcDevices, size iAxes, size iButtons, size iPOVs);
	void 				Update(CInputDevices* pcDevices, size uiSequence);
	void 				DumpDetaills(void);
};


#endif // __WIN_DIRECT_INPUT_H__

