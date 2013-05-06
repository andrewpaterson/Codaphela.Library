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
#ifndef __WIN_RAW_H__
#define __WIN_RAW_H__
#include <WinReg.h>
#include <Mmsystem.h>
#include "BaseLib/Define.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/Chars.h"
#include "CoreLib/GuidClass.h"
#include "InputDevices.h"
#include "InputDeviceValue.h"
#include "NativeDetail.h"

#include "KeyboardKeyList.h"


enum ERawDeviceType
{
	RDT_Mouse,
	RDT_Keyboard,
	RDT_General,
	RDT_Joystick,
};


struct SKeyboardID
{
	DWORD dwType;
	DWORD dwSubType;
	DWORD dwKeyboardMode;
	DWORD dwNumberOfFunctionKeys;
	DWORD dwNumberOfIndicators;
	DWORD dwNumberOfKeysTotal;
};


struct SMouseID
{
	DWORD dwId;
	DWORD dwNumberOfButtons;
	DWORD dwSampleRate;
};


struct SGeneralID
{
	DWORD dwVendorId;
	DWORD dwProductId;
	DWORD dwVersionNumber;
	USHORT usUsagePage;
	USHORT usUsage;
};


union URawID
{
	SKeyboardID	sKeyboardID;
	SMouseID	sMouseID;
	SGeneralID	sGeneralID;
};


class CRawInputDeviceDetail : public CNativeDetail
{
public:
	ERawDeviceType	eType;
	GUID			guid;
	HANDLE			hDevice;
	CChars			szRegistryKey;
	CChars			szFriendlyName;
	CChars			szID;
	CChars			szAdditional;  //This contains the IG_ for XBox gamepads.
	URawID			uID;  //In theory this should uniquely identify a device type so that a real description can be loaded.  Most are very generic though.

	void	Init(ERawDeviceType eType, CGuidClass* pcGuid, HANDLE hDevice, char* szRegistryKey, char* szFriendlyName, URawID* puID);
	void	Kill(void);
	void	AppendDescription(CChars* sz);
	void	Dump(void);
};


struct SRawMouseEvent
{
	unsigned short	usFlags;
	unsigned int	ulButtons;
	unsigned int	ulRawButtons;
	int				lLastX;
	int				lLastY;
	unsigned int	ulExtraInformation;
};


struct SRawKeyboardEvent
{
	unsigned short	usMakeCode;
	unsigned short	usFlags;
	unsigned short	usReserved;
	unsigned short	usVKey;
	unsigned int	uiMessage;
	unsigned int	uiExtraInformation;
};


struct SRawInputEvent
{
	ERawDeviceType	eType;
	HANDLE			hDevice;
	union
	{
		SRawMouseEvent		sMouse;
		SRawKeyboardEvent	sKeyboard;
	} uEvent;
};


typedef CArrayTemplate<CRawInputDeviceDetail> CArrayRIDeviceDetail;
typedef CArrayTemplate<SRawInputEvent> CArrayRawInputEvent;


class CWinInput;
class CWinRawInput
{
public:
	CArrayRIDeviceDetail	masRIDeviceDetail;
	CWinInput*				mpcWinInput;
	CArrayRawInputEvent		masRawEvents;  //This needs to be thread safe.

	void 					Init(CWinInput* pcWinInput);
	void 					Kill(void);
	void 					ResetDeviceDetails(void);
	void 					AddDeviceDetails(void* pDevice);
	CRawInputDeviceDetail*	AddRawDeviceDetails(void* pDevice, char* szDeviceName, void* psRidInfo);
	void					UpdateJoystickDetails(void* pDevice, CRawInputDeviceDetail* psRIDeviceDetail, char* szDeviceName);
	void					DumpDetails(void);
	CRawInputDeviceDetail*	GetDetailForName(char* szName);
	CRawInputDeviceDetail*	GetDetailForHandle(ERawDeviceType eType, HANDLE hDevice);
	CInputDeviceDesc*		GetStandardMouseDesc(CInputDevices* pcDevices, int iNumButtons);
	CInputDeviceDesc*		GetStandardKeyboardDesc(CInputDevices* pcDevices);
	void					AddDevicesTo(CInputDevices* pcDevices);
	void					Update(CInputDevices* pcDevices, unsigned int uiSequence);
};


void WinRawInputEvent(CWinRawInput* pcWinRaw, void* pRawInput);


#endif // __WIN_RAW_H__

