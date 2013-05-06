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
#include "BaseLib/Chars.h"
#include "WinInput.h"
#include "WinRaw.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRawInputDeviceDetail::Init(ERawDeviceType eType, CGuidClass* pcGuid, HANDLE hDevice, char* szRegistryKey, char* szFriendlyName, URawID* puID)
{
	CNativeDetail::Init();
	this->eType = eType;
	memcpy(&this->guid, pcGuid, sizeof(GUID));
	this->hDevice = hDevice;
	this->szRegistryKey.Init(szRegistryKey);
	this->szFriendlyName.Init(szFriendlyName);
	szAdditional.Init();
	szID.Init();
	memcpy(&uID, puID, sizeof(URawID));
	AppendDescription(&szID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRawInputDeviceDetail::Kill(void)
{
	szAdditional.Kill();
	szFriendlyName.Kill();
	szRegistryKey.Kill();
	CNativeDetail::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRawInputDeviceDetail::AppendDescription(CChars* sz)
{
	sz->Append("WinRAW ");
	if (eType == RDT_Mouse)
	{
		sz->Append("Mouse");
	}
	else if (eType == RDT_Keyboard)
	{
		sz->Append("Keyboard");
	}
	else if (eType == RDT_General)
	{
		sz->Append("HID");
	}
	else if (eType == RDT_Joystick)
	{
		sz->Append("Joystick");
	}
	sz->Append(": Name(");
	sz->Append(szFriendlyName);
	sz->Append("), ");
	if (eType == RDT_Mouse)
	{
		sz->Append("Id(");
		sz->AppendHexHiLo(&uID.sMouseID.dwId, sizeof(DWORD));
		sz->Append(") NumberOfButtons(");
		sz->AppendHexHiLo(&uID.sMouseID.dwNumberOfButtons, sizeof(DWORD));
		sz->Append(")");
	}
	else if (eType == RDT_Keyboard)
	{
		sz->Append("Type(");
		sz->AppendHexHiLo(&uID.sKeyboardID.dwType, sizeof(DWORD));
		sz->Append("), SubType(");
		sz->AppendHexHiLo(&uID.sKeyboardID.dwSubType, sizeof(DWORD));
		sz->Append("), KeyboardMode(");
		sz->AppendHexHiLo(&uID.sKeyboardID.dwKeyboardMode, sizeof(DWORD));
		sz->Append("), NumberOfFunctionKeys(");
		sz->AppendHexHiLo(&uID.sKeyboardID.dwNumberOfFunctionKeys, sizeof(DWORD));
		sz->Append("), NumberOfIndicators(");
		sz->AppendHexHiLo(&uID.sKeyboardID.dwNumberOfIndicators, sizeof(DWORD));
		sz->Append("), NumberOfKeysTotal(");
		sz->AppendHexHiLo(&uID.sKeyboardID.dwNumberOfKeysTotal, sizeof(DWORD));
		sz->Append(")");
	}
	else if (eType == RDT_General)
	{
		sz->Append("VendorId(");
		sz->AppendHexHiLo(&uID.sGeneralID.dwVendorId, sizeof(DWORD));
		sz->Append("), ProductId(");
		sz->AppendHexHiLo(&uID.sGeneralID.dwProductId, sizeof(DWORD));
		sz->Append("), VersionNumber(");
		sz->AppendHexHiLo(&uID.sGeneralID.dwVersionNumber, sizeof(DWORD));
		sz->Append("), UsagePage(");
		sz->AppendHexHiLo(&uID.sGeneralID.usUsagePage, sizeof(DWORD));
		sz->Append("), Usage(");
		sz->AppendHexHiLo(&uID.sGeneralID.usUsage, sizeof(DWORD));
		sz->Append(")");
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRawInputDeviceDetail::Dump(void)
{
	CChars		sz;

	sz.Init();
	if (eType == RDT_Mouse)
	{
		sz.Append("Mouse    ");
	}
	else if (eType == RDT_Keyboard)
	{
		sz.Append("Keyboard ");
	}
	else if (eType == RDT_General)
	{
		sz.Append("HID      ");
	}
	else if (eType == RDT_Joystick)
	{
		sz.Append("Joystick ");
	}
	sz.Append(": ");
	sz.Append(szID);
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::Init(CWinInput* pcWinInput)
{
	RAWINPUTDEVICE rid[3];

	mpcWinInput = pcWinInput;
	rid[0].usUsagePage = 0x01;
	rid[0].usUsage = 0x02;  //Mouse
	rid[0].dwFlags = 0;
	rid[0].hwndTarget = NULL;
	rid[1].usUsagePage = 0x01;
	rid[1].usUsage = 0x06;  //Keyboard
	rid[1].dwFlags = 0;
	rid[1].hwndTarget = NULL;
	rid[2].usUsagePage = 0x01;
	rid[2].usUsage = 0x08;  //Multi axis device
	rid[2].dwFlags = 0;
	rid[2].hwndTarget = NULL;
	masRawEvents.Init(128);
	RegisterRawInputDevices(rid, 3, sizeof(RAWINPUTDEVICE));

	masRIDeviceDetail.Init(8);
	ResetDeviceDetails();
	DumpDetails();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::Kill(void)
{
	int					i;
	CRawInputDeviceDetail*	psRIDeviceDetail;

	for (i = 0; i < masRIDeviceDetail.NumElements(); i++)
	{
		psRIDeviceDetail = masRIDeviceDetail.Get(i);
		psRIDeviceDetail->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::ResetDeviceDetails(void)
{
	UINT					uiNumDevices;
	UINT					uiReturn;
	RAWINPUTDEVICELIST*		pRawList;
	int						i;

	uiReturn = GetRawInputDeviceList(NULL, &uiNumDevices, sizeof(RAWINPUTDEVICELIST));
	pRawList = (RAWINPUTDEVICELIST*)malloc(uiNumDevices*sizeof(RAWINPUTDEVICELIST));
	uiReturn = GetRawInputDeviceList(pRawList, &uiNumDevices, sizeof(RAWINPUTDEVICELIST));

	for (i = 0; i < (int)uiNumDevices; i++)
	{
		AddDeviceDetails(&pRawList[i]);
	}

	free(pRawList);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRawInputDeviceDetail* CWinRawInput::AddRawDeviceDetails(void* pDevice, char* szDeviceName, void* psRidInfo)
{
	HKEY 				hKey;
	DWORD				dwLength;
	CChars				szKeyName;
	int					iEnd;
	LONG				lRet;
	CChars				szGuid;
	CGuidClass			cGuid;
	CRawInputDeviceDetail*	psRIDeviceDetail;
	char 				lszValue[1000];
	RAWINPUTDEVICELIST*	pRawInputDeviceList;
	RID_DEVICE_INFO*	pRidDeviceInfo;
	URawID				uRawID;
	HANDLE				hDevice;

	pRawInputDeviceList = (RAWINPUTDEVICELIST*)pDevice;
	pRidDeviceInfo = (RID_DEVICE_INFO*)psRidInfo;
	dwLength=1000;
	szGuid.Init(szDeviceName);
	iEnd = szGuid.FindFromEnd('#');
	szGuid.RemoveFromStart(iEnd+1);
	cGuid.Init(szGuid.Text());

	szKeyName.Init(szDeviceName);
	szKeyName.RemoveFromStart(4); // Remove the \??\ 
	szKeyName.Replace('#', '\\');
	szKeyName.Insert(0, "System\\CurrentControlSet\\Enum\\");
	iEnd = szKeyName.FindFromEnd('\\');
	szKeyName.RemoveFromEnd(szKeyName.Length() - iEnd);

	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKeyName.Text(), 0L, KEY_READ , &hKey);
	lszValue[0] = '\0';
	if(lRet == ERROR_SUCCESS)
	{
		lRet = RegQueryValueEx(hKey, "DeviceDesc", NULL, NULL, (unsigned char*)lszValue, &dwLength);
	}

	memset(&uRawID, 0, sizeof(URawID));
	hDevice = pRawInputDeviceList->hDevice;
	if (pRidDeviceInfo->dwType == RIM_TYPEKEYBOARD)
	{
		uRawID.sKeyboardID.dwKeyboardMode = pRidDeviceInfo->keyboard.dwKeyboardMode;
		uRawID.sKeyboardID.dwNumberOfFunctionKeys = pRidDeviceInfo->keyboard.dwNumberOfFunctionKeys;
		uRawID.sKeyboardID.dwNumberOfIndicators = pRidDeviceInfo->keyboard.dwNumberOfIndicators;
		uRawID.sKeyboardID.dwNumberOfKeysTotal = pRidDeviceInfo->keyboard.dwNumberOfKeysTotal;
		uRawID.sKeyboardID.dwSubType = pRidDeviceInfo->keyboard.dwSubType;
		if (strstr(szDeviceName, "\\??\\Root#RDP_KBD#0000#") == szDeviceName)
		{
			hDevice = NULL;
		}
	}
	else if (pRidDeviceInfo->dwType == RIM_TYPEMOUSE)
	{
		uRawID.sMouseID.dwId = pRidDeviceInfo->mouse.dwId;
		uRawID.sMouseID.dwNumberOfButtons = pRidDeviceInfo->mouse.dwNumberOfButtons;
		uRawID.sMouseID.dwSampleRate = pRidDeviceInfo->mouse.dwSampleRate;
		if (strstr(szDeviceName, "\\??\\Root#RDP_MOU#0000#") == szDeviceName)
		{
			hDevice = NULL;
		}
	}
	else if (pRidDeviceInfo->dwType == RIM_TYPEHID)
	{
		uRawID.sGeneralID.dwProductId = pRidDeviceInfo->hid.dwProductId;
		uRawID.sGeneralID.dwVendorId = pRidDeviceInfo->hid.dwVendorId;
		uRawID.sGeneralID.dwVersionNumber = pRidDeviceInfo->hid.dwVersionNumber;
		uRawID.sGeneralID.usUsage = pRidDeviceInfo->hid.usUsage;
		uRawID.sGeneralID.usUsagePage = pRidDeviceInfo->hid.usUsagePage;
	}

	psRIDeviceDetail = masRIDeviceDetail.Add();
	psRIDeviceDetail->Init((ERawDeviceType)pRawInputDeviceList->dwType, &cGuid, hDevice, szKeyName.Text(), lszValue, &uRawID);

	RegCloseKey(hKey);
	szKeyName.Kill();
	szGuid.Kill();

	return psRIDeviceDetail;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::UpdateJoystickDetails(void* pDevice, CRawInputDeviceDetail* psRIDeviceDetail, char* szDeviceName)
{
	LONG 				lRet;
	LONG 				lEnumRet;
	HKEY 				hKey;
	HKEY 				hKey2;
	DWORD				dwLength=100;
	int					j = 0;
	CChars				szKeyName;
	CChars				szFullKey;
	int					iStart;
	char 				szValueName[1000];
	char 				szJoystickName[1000];
	char				szJoystickRegPath[] = "SYSTEM\\CurrentControlSet\\Control\\MediaProperties\\PrivateProperties\\Joystick\\OEM";
	RAWINPUTDEVICELIST*	pRawInputDeviceList;
	int					iLen;

	pRawInputDeviceList = (RAWINPUTDEVICELIST*)pDevice;
	szKeyName.Init(szDeviceName);
	iStart = szKeyName.Find(0, '#');
	szKeyName.RemoveFromStart(iStart+1);
	iStart = szKeyName.Find(0, '#');
	szKeyName.RemoveFromEnd(szKeyName.Length() - iStart);

	lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szJoystickRegPath, 0L, KEY_READ , &hKey);
	if(lRet == ERROR_SUCCESS)
	{
		lEnumRet = RegEnumKey(hKey, j, szValueName, dwLength);
		while (lEnumRet == ERROR_SUCCESS)
		{
			if (szKeyName.StartsWithIgnoreCase(szValueName))
			{
				szFullKey.Init(szJoystickRegPath);
				szFullKey.Append("\\");
				szFullKey.Append(szValueName);
				lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szFullKey.Text(), 0L, KEY_READ , &hKey2);
				szJoystickName[0] = '\0';
				if (lRet == ERROR_SUCCESS)
				{
					lRet = RegQueryValueEx(hKey2, "OEMName", NULL, NULL, (unsigned char*)szJoystickName, &dwLength);
					psRIDeviceDetail->eType = RDT_Joystick;
					iLen = (int)strlen(szJoystickName);
					if (iLen > 0)
					{
						psRIDeviceDetail->szFriendlyName.Set(szJoystickName);
					}
					iLen = (int)strlen(szValueName);
					psRIDeviceDetail->szAdditional.Set(szKeyName.Text(iLen));
					psRIDeviceDetail->szID.Clear();
					psRIDeviceDetail->AppendDescription(&psRIDeviceDetail->szID);
				}
				szFullKey.Kill();
				RegCloseKey(hKey2);
				break;
			}
			j++;
			lEnumRet = RegEnumKey(hKey, j, szValueName, dwLength);
		}
	}

	RegCloseKey(hKey);
	szKeyName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::AddDeviceDetails(void* pDevice)
{
	UINT				uiReturn;
	unsigned int		uiLength;
	unsigned int		uiSize;
	char				szDeviceName[1024];
	RID_DEVICE_INFO		sRidInfo;
	CRawInputDeviceDetail*	psRIDeviceDetail;
	RAWINPUTDEVICELIST*	pRawInputDeviceList;

	pRawInputDeviceList = (RAWINPUTDEVICELIST*)pDevice;

	//Length is returned in uiLength.
	uiReturn = GetRawInputDeviceInfo(pRawInputDeviceList->hDevice, RIDI_DEVICENAME, NULL, &uiLength);  

	//Get the device name into acName.
	uiReturn = GetRawInputDeviceInfo(pRawInputDeviceList->hDevice, RIDI_DEVICENAME, szDeviceName, &uiLength);

	uiSize = sizeof(sRidInfo);
	sRidInfo.cbSize = uiSize;
	uiReturn = GetRawInputDeviceInfo(pRawInputDeviceList->hDevice, RIDI_DEVICEINFO, &sRidInfo, &uiSize);

	if ((pRawInputDeviceList->dwType == RIM_TYPEKEYBOARD) || (pRawInputDeviceList->dwType == RIM_TYPEMOUSE))
	{
		AddRawDeviceDetails(pDevice, szDeviceName, &sRidInfo);
	}
	else
	{
		psRIDeviceDetail = AddRawDeviceDetails(pDevice, szDeviceName, &sRidInfo);
		UpdateJoystickDetails(pDevice, psRIDeviceDetail, szDeviceName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRawInputDeviceDetail* CWinRawInput::GetDetailForName(char* szName)
{
	int					i;
	CRawInputDeviceDetail*	psRIDeviceDetail;

	for (i = 0; i < masRIDeviceDetail.NumElements(); i++)
	{
		psRIDeviceDetail = masRIDeviceDetail.Get(i);
		if (psRIDeviceDetail->szFriendlyName.Equals(szName))
		{
			return psRIDeviceDetail;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRawInputDeviceDetail* CWinRawInput::GetDetailForHandle(ERawDeviceType eType, HANDLE hDevice)
{
	int					i;
	CRawInputDeviceDetail*	psRIDeviceDetail;

	for (i = 0; i < masRIDeviceDetail.NumElements(); i++)
	{
		psRIDeviceDetail = masRIDeviceDetail.Get(i);
		if ((psRIDeviceDetail->eType == eType) && (psRIDeviceDetail->hDevice == hDevice))
		{
			return psRIDeviceDetail;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::AddDevicesTo(CInputDevices* pcDevices)
{
	int						i;
	CRawInputDeviceDetail*	pcDetail;
	CInputDevice*			pcInputDevice;
	CInputDeviceDesc*		pcDeviceDesc;

	for (i = 0; i < masRIDeviceDetail.NumElements(); i++)
	{
		pcDetail = masRIDeviceDetail.Get(i);
		if ((pcDetail->eType == RDT_Mouse) || (pcDetail->eType == RDT_Keyboard))
		{
			pcDeviceDesc = pcDevices->GetDescription(pcDetail->szID.Text());
			if (!pcDeviceDesc)
			{
				if (pcDetail->eType == RDT_Mouse) 
				{
					pcDeviceDesc = GetStandardMouseDesc(pcDevices, pcDetail->uID.sMouseID.dwNumberOfButtons);
				}
				else if (pcDetail->eType == RDT_Keyboard)
				{
					pcDeviceDesc = GetStandardKeyboardDesc(pcDevices);
				}
			}
			if (pcDeviceDesc)
			{
				pcInputDevice = pcDevices->CreateDevice(pcDeviceDesc);
				pcInputDevice->GenerateRestState();
				pcDetail->SetUniqueID(pcInputDevice->GetDeviceID());
			}
			else
			{
				CChars::Dump("Unknown device: ");
				pcDetail->Dump();
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceDesc* CWinRawInput::GetStandardMouseDesc(CInputDevices* pcDevices, int iNumButtons)
{
	if (iNumButtons >= 5)
	{
		return pcDevices->GetDescription("WinRAW Mouse: Standard 5 Button Mouse");
	}
	else if (iNumButtons == 4)
	{
		return pcDevices->GetDescription("WinRAW Mouse: Standard 4 Button Mouse");
	}
	else if (iNumButtons == 3)
	{
		return pcDevices->GetDescription("WinRAW Mouse: Standard 3 Button Mouse");
	}
	else
	{
		return pcDevices->GetDescription("WinRAW Mouse: Standard 2 Button Mouse");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceDesc* CWinRawInput::GetStandardKeyboardDesc(CInputDevices* pcDevices)
{
	return pcDevices->GetDescription("WinRAW Keyboard: Standard 104 Button Keyboard");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::Update(CInputDevices* pcDevices, unsigned int uiSequence)
{
	int						i;
	SRawInputEvent*			psEvent;
	CRawInputDeviceDetail*	pcRIDeviceDetail;
	CInputDevice*			pcDevice;
	SRawMouseEvent*			psRawMouse;
	SRawKeyboardEvent*		psRawKeyboard;

	for (i = 0; i < masRawEvents.NumElements(); i++)
	{
		psEvent = masRawEvents.Get(i);
		pcRIDeviceDetail = GetDetailForHandle(psEvent->eType, psEvent->hDevice);
		if ((pcRIDeviceDetail != NULL) && (pcRIDeviceDetail->miUniqueID != -1))
		{
			pcDevice = pcDevices->GetDevice(pcRIDeviceDetail->miUniqueID);

			if (psEvent->eType == RDT_Mouse)
			{
				psRawMouse = &psEvent->uEvent.sMouse;
				pcDevice->Process(psRawMouse, uiSequence);
			}
			else if (psEvent->eType == RDT_Keyboard)
			{
				psRawKeyboard = &psEvent->uEvent.sKeyboard;
				pcDevice->Process(psRawKeyboard, uiSequence);

				//gcKeyboardKeyList.Key(psEvent->uEvent.sKeyboard.usVKey, psEvent->uEvent.sKeyboard.usFlags);
			}
		}
	}
	masRawEvents.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRawInput::DumpDetails(void)
{
	int					i;
	CRawInputDeviceDetail*	psRIDeviceDetail;

	for (i = 0; i < masRIDeviceDetail.NumElements(); i++)
	{
		psRIDeviceDetail = masRIDeviceDetail.Get(i);
		psRIDeviceDetail->Dump();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void WinRawInputEvent(CWinRawInput* pcWinRaw, void* pv)
{
	SRawInputEvent*		psRawEvent;
	RAWKEYBOARD*		psKeyboard;
	RAWMOUSE*			psMouse;
	RAWINPUT*			pRawInput;

	pRawInput = (RAWINPUT*)pv;
	if (pRawInput->header.dwType == RIM_TYPEMOUSE)
	{
		psMouse = &pRawInput->data.mouse;
		psRawEvent = pcWinRaw->masRawEvents.Add();
		psRawEvent->eType = RDT_Mouse;
		psRawEvent->hDevice = pRawInput->header.hDevice;
		psRawEvent->uEvent.sMouse.lLastX = psMouse->lLastX;
		psRawEvent->uEvent.sMouse.lLastY = psMouse->lLastY;
		psRawEvent->uEvent.sMouse.ulButtons = psMouse->ulButtons;
		psRawEvent->uEvent.sMouse.ulExtraInformation = psMouse->ulExtraInformation;
		psRawEvent->uEvent.sMouse.ulRawButtons= psMouse->ulRawButtons;
		psRawEvent->uEvent.sMouse.usFlags = psMouse->usFlags;
	}
	else if (pRawInput->header.dwType == RIM_TYPEKEYBOARD)
	{
		psKeyboard = &pRawInput->data.keyboard;
		psRawEvent = pcWinRaw->masRawEvents.Add();
		psRawEvent->eType = RDT_Keyboard;
		psRawEvent->hDevice = pRawInput->header.hDevice;
		psRawEvent->uEvent.sKeyboard.usMakeCode = psKeyboard->MakeCode;
		psRawEvent->uEvent.sKeyboard.usFlags = psKeyboard->Flags;
		psRawEvent->uEvent.sKeyboard.usReserved = psKeyboard->Reserved;
		psRawEvent->uEvent.sKeyboard.usVKey = psKeyboard->VKey;
		psRawEvent->uEvent.sKeyboard.uiMessage = psKeyboard->Message;
		psRawEvent->uEvent.sKeyboard.uiExtraInformation = psKeyboard->ExtraInformation;
	}
	else if (pRawInput->header.dwType == RIM_TYPEHID)
	{
		//CChars	sz;
		//RAWHID* psHID = &pRawInput->data.hid;
		//int		i;

		//sz.Init("----- HIDevice (");
		//sz.AppendHexHiLo(&pRawInput->header.hDevice, sizeof(pRawInput->header.hDevice));
		//sz.Append(") -----\n");

		//for (i = 0; i < (int)psHID->dwCount; i++)
		//{
		//	sz.AppendHexLoHi(&psHID->bRawData[i*psHID->dwSizeHid], psHID->dwSizeHid);
		//	sz.AppendNewLine();
		//}

		//sz.Dump();
		//sz.Kill();
	}
}

