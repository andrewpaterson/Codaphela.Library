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
#include "CoreLib/GuidClass.h"
#include "WinDirectInput.h"
#define XINPUT_USE_9_1_0
#include <XInput.h>
#include "WinInput.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInputDetail::Init(GUID guidInstance, GUID guidProduct, char* szInstanceName,  char* szProductName, GUID guidFFDriver, WORD wUsagePage, WORD wUsage)
{
	CNativeDetail::Init();
	this->guidFFDriver = guidFFDriver;
	this->guidInstance = guidInstance;
	this->guidProduct = guidProduct;
	this->szInstanceName.Init(szInstanceName);
	this->szProductName.Init(szProductName);
	this->wUsage = wUsage;
	this->wUsagePage = wUsagePage;
	bXInput = FALSE;
	lpDIDevice = NULL;

	szID.Init();
	AppendDescription(&szID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInputDetail::Kill(void)
{
	szInstanceName.Kill();
	szProductName.Kill();
	szID.Kill();
	ReleaseDevice();
	CNativeDetail::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInputDetail::AppendDescription(CChars* sz)
{
	CGuidClass	cGuid;

	sz->Append("DirectInput Joystick:");

	sz->Append(" Name(");
	sz->Append(szProductName);
	sz->Append("), GUID(");
	cGuid.Init(guidProduct.Data1, guidProduct.Data2, guidProduct.Data3, guidProduct.Data4);
	cGuid.Append(sz);
	sz->Append("), UsagePage(");
	sz->Append(wUsagePage);
	sz->Append("), Usage(");
	sz->Append(wUsage);
	sz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInputDetail::ReleaseDevice(void)
{
	if (lpDIDevice != NULL)
	{
		lpDIDevice->Unacquire();
		lpDIDevice->Release();
		lpDIDevice = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInputDetail::Dump(void)
{
	CChars		sz;

	sz.Init();
	sz.Append("Joystick ");
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
void CDirectInput::Init(CWinInput* pcWinInput)
{
	HRESULT 	hRet;

	mpcWinInput = pcWinInput;
	hRet = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&mpDInput, NULL);
	if (FAILED(hRet))
	{
		return;
	}

	masDIJoystickDetail.Init(1);
	ResetDetails();
	CreateDevices();

	DumpDetaills();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::Kill(void)
{
	masDIJoystickDetail.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL FAR PASCAL EnumGameControllerDevices(LPCDIDEVICEINSTANCE pcdiDeviceInstance, LPVOID pVoid)
{
	CDirectInput*	pcThis;

	pcThis = (CDirectInput*)pVoid;
	pcThis->AddDetails((DIDEVICEINSTANCE*)pcdiDeviceInstance);
	return DIENUM_CONTINUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::CreateDevices(void)
{
	int						i;
	CDirectInputDetail*		psJoystickDetail;
	HRESULT					hResult;

	for (i = 0; i < masDIJoystickDetail.NumElements(); i++)
	{
		psJoystickDetail = masDIJoystickDetail.Get(i);
		if (!IsXInputDevice(psJoystickDetail))
		{
			psJoystickDetail->bXInput = FALSE;
			hResult = mpDInput->CreateDevice(psJoystickDetail->guidInstance, &psJoystickDetail->lpDIDevice, NULL);
			psJoystickDetail->lpDIDevice->SetDataFormat(&c_dfDIJoystick2);
			if (mpcWinInput->mbExclusive)
			{
				hResult = psJoystickDetail->lpDIDevice->SetCooperativeLevel(mpcWinInput->mhWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
			}
			else
			{
				hResult = psJoystickDetail->lpDIDevice->SetCooperativeLevel(mpcWinInput->mhWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
			}
			psJoystickDetail->lpDIDevice->Acquire();
		}
		else
		{
			psJoystickDetail->bXInput = TRUE;
			psJoystickDetail->lpDIDevice = NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::ReleaseDevices(void)
{
	int						i;
	CDirectInputDetail*		pcDetail;

	for (i = 0; masDIJoystickDetail.NumElements(); i++)
	{
		pcDetail = masDIJoystickDetail.Get(i);
		pcDetail->ReleaseDevice();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::ResetDetails(void)
{
	int						i;
	CDirectInputDetail*		pcDetail;

	for (i = 0; i < masDIJoystickDetail.NumElements(); i++)
	{
		pcDetail = masDIJoystickDetail.Get(i);
		pcDetail->Kill();
	}
	masDIJoystickDetail.Kill();

	mpDInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumGameControllerDevices, this, DIEDFL_ALLDEVICES);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::Update(void)
{
	DIDEVICEOBJECTDATA		sData[256];	
	DWORD					dwItems;
	int						iJoystick;
	DWORD					dwItem;
	HRESULT					hResult;
	DIJOYSTATE2				cJoyState;
	CDirectInputDetail*		pcDetail;

	for (iJoystick = 0; iJoystick > masDIJoystickDetail.NumElements(); iJoystick++)
	{
		pcDetail = masDIJoystickDetail.Get(iJoystick);
		hResult = pcDetail->lpDIDevice->GetDeviceState(sizeof(DIJOYSTATE2), &cJoyState);
		if (FAILED(hResult)) 
		{ 
			//May be unplugged, not just tabbed out.
			pcDetail->lpDIDevice->Unacquire();
			pcDetail->lpDIDevice->Acquire();
		}

		dwItems = 256;
		hResult = pcDetail->lpDIDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), sData, &dwItems, 0);
		if (SUCCEEDED(hResult)) 
		{ 
			for (dwItem = 0; dwItem < dwItems; dwItem++)
			{
				//Er?
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::AddDetails(DIDEVICEINSTANCE* pcdiDeviceInstance)
{
	CDirectInputDetail*	psDetail;

	psDetail = masDIJoystickDetail.Add();

	psDetail->Init(pcdiDeviceInstance->guidInstance,
		pcdiDeviceInstance->guidProduct,
		pcdiDeviceInstance->tszInstanceName,
		pcdiDeviceInstance->tszProductName,
		pcdiDeviceInstance->guidFFDriver,
		pcdiDeviceInstance->wUsagePage,
		pcdiDeviceInstance->wUsage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::AddDevicesTo(CInputDevices* pcDevices)
{
	int						i;
	CDirectInputDetail*		pcDetail;
	CInputDeviceDesc*		pcDeviceDesc;
	CInputDevice*			pcInputDevice;
	DIDEVCAPS				diDevCaps;

	for (i = 0; i < masDIJoystickDetail.NumElements(); i++)
	{
		pcDetail = masDIJoystickDetail.Get(i);
		if (pcDetail->lpDIDevice)
		{
			pcDeviceDesc = pcDevices->GetDescription(pcDetail->szID.Text());
			if (!pcDeviceDesc)
			{
				diDevCaps.dwSize = sizeof(DIDEVCAPS);
				pcDetail->lpDIDevice->GetCapabilities(&diDevCaps);
				pcDeviceDesc = AddStandardJoystick(pcDevices, diDevCaps.dwAxes, diDevCaps.dwButtons, diDevCaps.dwPOVs);
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
CInputDeviceDesc* CDirectInput::AddStandardJoystick(CInputDevices* pcDevices, int iAxes, int iButtons, int iPOVs)
{
	if (iAxes <= 2)
	{
		if (iPOVs >= 1)
		{
			if (iButtons <= 2)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 2 Button 1 POV Joystick");
			}
			else if ((iButtons >= 3) && (iButtons <= 4))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 4 Button 1 POV Joystick");
			}
			else if ((iButtons >= 5) && (iButtons <= 6))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 6 Button 1 POV Joystick");
			}
			else if ((iButtons >= 7) && (iButtons <= 8))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 8 Button 1 POV Joystick");
			}
			else if ((iButtons >= 9) && (iButtons <= 10))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 10 Button 1 POV Joystick");
			}
			else if (iButtons >= 11)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 12 Button 1 POV Joystick");
			}
		}
		else
		{
			if (iButtons <= 2)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 2 Button Joystick");
			}
			else if ((iButtons >= 3) && (iButtons <= 4))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 4 Button Joystick");
			}
			else if ((iButtons >= 5) && (iButtons <= 6))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 6 Button Joystick");
			}
			else if ((iButtons >= 7) && (iButtons <= 8))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 8 Button Joystick");
			}
			else if ((iButtons >= 9) && (iButtons <= 10))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 10 Button Joystick");
			}
			else if (iButtons >= 11)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 2 Axis 12 Button Joystick");
			}
		}
	}
	else if (iAxes == 3)
	{
		if (iPOVs >= 1)
		{
			if (iButtons <= 2)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 2 Button 1 POV Joystick");
			}
			else if ((iButtons >= 3) && (iButtons <= 4))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 4 Button 1 POV Joystick");
			}
			else if ((iButtons >= 5) && (iButtons <= 6))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 6 Button 1 POV Joystick");
			}
			else if ((iButtons >= 7) && (iButtons <= 8))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 8 Button 1 POV Joystick");
			}
			else if ((iButtons >= 9) && (iButtons <= 10))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 10 Button 1 POV Joystick");
			}
			else if (iButtons >= 11)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 12 Button 1 POV Joystick");
			}
		}
		else
		{
			if (iButtons <= 2)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 2 Button Joystick");
			}
			else if ((iButtons >= 3) && (iButtons <= 4))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 4 Button Joystick");
			}
			else if ((iButtons >= 5) && (iButtons <= 6))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 6 Button Joystick");
			}
			else if ((iButtons >= 7) && (iButtons <= 8))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 8 Button Joystick");
			}
			else if ((iButtons >= 9) && (iButtons <= 10))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 10 Button Joystick");
			}
			else if (iButtons >= 11)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 3 Axis 12 Button Joystick");
			}
		}
	}
	else if (iAxes >= 3)
	{
		if (iPOVs >= 1)
		{
			if (iButtons <= 2)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 2 Button 1 POV Joystick");
			}
			else if ((iButtons >= 3) && (iButtons <= 4))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 4 Button 1 POV Joystick");
			}
			else if ((iButtons >= 5) && (iButtons <= 6))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 6 Button 1 POV Joystick");
			}
			else if ((iButtons >= 7) && (iButtons <= 8))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 8 Button 1 POV Joystick");
			}
			else if ((iButtons >= 9) && (iButtons <= 10))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 10 Button 1 POV Joystick");
			}
			else if (iButtons >= 11)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 12 Button 1 POV Joystick");
			}
		}
		else
		{
			if (iButtons <= 2)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 2 Button Joystick");
			}
			else if ((iButtons >= 3) && (iButtons <= 4))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 4 Button Joystick");
			}
			else if ((iButtons >= 5) && (iButtons <= 6))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 6 Button Joystick");
			}
			else if ((iButtons >= 7) && (iButtons <= 8))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 8 Button Joystick");
			}
			else if ((iButtons >= 9) && (iButtons <= 10))
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 10 Button Joystick");
			}
			else if (iButtons >= 11)
			{
				return pcDevices->GetDescription("DirectInput Joystick: Standard 4 Axis 12 Button Joystick");
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::Update(CInputDevices* pcDevices, unsigned int uiSequence)
{
	int					i;
	CDirectInputDetail*	pcDetail;
	HRESULT				hr;
	DIJOYSTATE2			diJoyState2;
	CInputDevice*		pcDevice;

	for (i = 0; i < masDIJoystickDetail.NumElements(); i++)
	{
		pcDetail = masDIJoystickDetail.Get(i);
		if ((pcDetail->lpDIDevice) && (pcDetail->miUniqueID != -1))
		{
			pcDevice = pcDevices->GetDevice(pcDetail->miUniqueID);

			// Poll the device to read the current state
			hr = pcDetail->lpDIDevice->Poll();
			if (FAILED(hr))
			{
				hr = pcDetail->lpDIDevice->Acquire();
				continue;
			}
			hr = pcDetail->lpDIDevice->GetDeviceState(sizeof(DIJOYSTATE2), &diJoyState2);
			if (FAILED(hr))
			{
				continue;
			}

			pcDevice->Process(&diJoyState2, uiSequence);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDirectInput::DumpDetaills(void)
{
	int					i;
	CDirectInputDetail*	psDIJoystickDetail;

	for (i = 0; i < masDIJoystickDetail.NumElements(); i++)
	{
		psDIJoystickDetail = masDIJoystickDetail.Get(i);
		psDIJoystickDetail->Dump();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDirectInput::IsXInputDevice(CDirectInputDetail* psDetail)
{
	CRawInputDeviceDetail*	psRawDetail;

	psRawDetail = mpcWinInput->mcRaw.GetDetailForName(psDetail->szInstanceName.Text());
	if (psRawDetail)
	{
		if (psRawDetail->szAdditional.Contains("IG_"))
		{
			return TRUE;
		}
	}
	return FALSE;
}
