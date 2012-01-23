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
#include "WinXInput.h"
#include "WinInput.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXInputDetail::Init(unsigned char	ucType, unsigned char ucSubType, unsigned short	usFlags,  unsigned char ucBattery)
{
	CNativeDetail::Init();
	this->ucType = ucType;
	this->ucSubType = ucSubType;
	this->usFlags = usFlags;
	this->ucBattery = ucBattery;

	szID.Init();
	AppendDescription(&szID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXInputDetail::Kill(void)
{
	Disconnect();
	CNativeDetail::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXInputDetail::AppendDescription(CChars* sz)
{
	sz->Append("XInput ");

	switch (ucSubType)
	{
	case XINPUT_DEVSUBTYPE_GAMEPAD:
		sz->Append("Gamepad ");
		break;
	case XINPUT_DEVSUBTYPE_WHEEL:
		sz->Append("Wheel ");
		break;
	case XINPUT_DEVSUBTYPE_ARCADE_STICK:
		sz->Append("Arcade Stick ");
		break;
	case XINPUT_DEVSUBTYPE_FLIGHT_SICK:
		sz->Append("Flight Stick ");
		break;
	case XINPUT_DEVSUBTYPE_DANCE_PAD:
		sz->Append("Dance Pad ");
		break;
	case XINPUT_DEVSUBTYPE_GUITAR:
		sz->Append("Guitar ");
		break;
	case XINPUT_DEVSUBTYPE_DRUM_KIT:
		sz->Append("Drum Kit ");
		break;
	}
	
	sz->Append(": Battery(");
	switch (ucBattery)
	{
	case BATTERY_TYPE_WIRED:
		sz->Append("Wired");
		break;
	case BATTERY_TYPE_ALKALINE:
		sz->Append("Alkaline");
		break;
	case BATTERY_TYPE_NIMH:
		sz->Append("NiMH");
		break;
	case BATTERY_TYPE_UNKNOWN:
		sz->Append("Unknown");
		break;
	}
	sz->Append(")");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXInputDetail::Disconnect(void)
{
	if (bConnected)
	{
		szID.Kill();
		memset(this, 0, sizeof(CXInputDetail));
		bConnected = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CXInputDetail::Dump(void)
{
	CChars		sz;

	if (bConnected)
	{
		sz.Init();
		sz.Append("XInput ");
		sz.Append(": ");
		sz.Append(szID);
		sz.AppendNewLine();
		sz.Dump();
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinXInput::Init(CWinInput* pcWinInput)
{
	mpcWinInput = pcWinInput;
	XInputEnable(TRUE);
	ResetDetails();

	DumpDetails();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinXInput::Kill(void)
{
	XInputEnable(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinXInput::ResetDetails(void)
{
	int							dwResult;
	XINPUT_STATE				sXState;
	int							i;
	XINPUT_CAPABILITIES			sXCaps;
	XINPUT_BATTERY_INFORMATION	sXBattery;

	for (i = 0; i < MAX_XINPUT_CONTROLLERS; i++)
	{
		dwResult = XInputGetState(i, &sXState);
		memset(&masXInputDetail[i], 0, sizeof(CXInputDetail));
		if (dwResult == ERROR_SUCCESS)
		{
			masXInputDetail[i].bConnected = TRUE;

			XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &sXCaps);
			XInputGetBatteryInformation(i, sXCaps.Type, &sXBattery);
			
			masXInputDetail[i].Init(sXCaps.Type, sXCaps.SubType, sXCaps.Flags, sXBattery.BatteryType);
		}
		else
		{
			masXInputDetail[i].bConnected = FALSE;
		}
		masXInputDetail[i].SetUniqueID(-1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinXInput::AddDevicesTo(CInputDevices* pcDevices)
{
	int						i;
	CXInputDetail*			pcDetail;
	CInputDeviceDesc*		pcDeviceDesc;
	CInputDevice*			pcInputDevice;

	for (i = 0; i < MAX_XINPUT_CONTROLLERS; i++)
	{
		pcDetail = &masXInputDetail[i];
		if (pcDetail->bConnected)
		{
			pcDeviceDesc = pcDevices->GetDescription(pcDetail->szID.Text());
			if (!pcDeviceDesc)
			{
				pcDeviceDesc = AddStandardGamepad(pcDevices);
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
CInputDeviceDesc* CWinXInput::AddStandardGamepad(CInputDevices* pcDevices)
{
	return pcDevices->GetDescription("XInput Gamepad: Standard XBox 360 Gamepad");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinXInput::Update(CInputDevices* pcDevices, unsigned int uiSequence)
{
	int					dwResult;
	XINPUT_STATE		sXState;
	int					i;
	CInputDevice*		pcDevice;

	//for (i = 0; i < MAX_XINPUT_CONTROLLERS; i++)
	//{
	//	dwResult = XInputGetState(i, &sXState);
	//	if ( dwResult == ERROR_SUCCESS )
	//	{
	//		masXInputDetail[i].bConnected = TRUE;
	//	}
	//	else
	//	{
	//		masXInputDetail[i].Disconnect();
	//	}
	//}

	for (i = 0; i < MAX_XINPUT_CONTROLLERS; i++)
	{
		if ((masXInputDetail[i].miUniqueID != -1) && (masXInputDetail[i].bConnected))
		{
			pcDevice = pcDevices->GetDevice(masXInputDetail[i].miUniqueID);

			dwResult = XInputGetState(i, &sXState);
			if (dwResult == ERROR_SUCCESS)
			{
				pcDevice->Process(&sXState.Gamepad, uiSequence);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinXInput::DumpDetails(void)
{
	int		i;

	for (i = 0; i < MAX_XINPUT_CONTROLLERS; i++)
	{	
		if (masXInputDetail[i].bConnected)
		{
			masXInputDetail[i].Dump();
		}
	}
}
