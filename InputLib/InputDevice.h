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
#ifndef __INPUT_DEVICE_H__
#define __INPUT_DEVICE_H__
#include "BaseLib/ArrayChars.h"
#include "StandardLib/SetType.h"
#include "InputDeviceDesc.h"
#include "InputCategory.h"
#include "InputDeviceState.h"
#include "InputVirtualDevice.h"
#include "InputDeviceVariables.h"


class CInputActions;
class CInputChord;
class CInputDevice : public CUnknown
{
CONSTRUCTABLE(CInputDevice);
protected:
	CInputDeviceDesc*			mpcDesc;
	CChars						mszFriendlyName;
	CInputDeviceState			mcState;
	bool						mbResting;
	CInputDeviceValues			mcInputValues;
	CInputDeviceVariables		mcVariables;
	int							miDeviceID;  //Unique across all devices in the system.  Matches the CInputDevice to it's OS specific detail.  Eg: CRawInputDeviceDetail.
	EInputDevice				meDescriptionID;  //Hopefully the same between sessions but not guaranteed.
	CInputVirtualDevice*		mpcDefaultVirtual;
	CInputVirtualDevice*		mpcVariableVirtual;

public:
	void 						Init(CInputDeviceDesc* pcDesc, int iDeviceID, EInputDevice iDescriptionID, CInputActions* pcInputActions);
	void 						Kill(void);
	CInputDevices*				GetInputDevices(void);
	void 						GenerateRestState(void);
	void 						Process(void* pvInput, unsigned int uiSequence);
	void 						ClearStateFlags(void);
	void 						UpdateRestEvents(unsigned int uiSequence);
	void 						GetSources(CArrayInputSourceDescPtr* pcActions, CInputCategoryGeneric* pcGeneric);
	void 						ClearEvents(void);
	void 						SortEvents(void);
	int							GetDeviceID(void);
	CInputSourceDesc*			GetSource(char* szFriendlyName);
	CInputVirtualDevice*		GetDefaultVirtualDevice(void);
	CInputVirtualDevice*		CreateDefaultVirtualDeviceFromThis(char* szName = NULL);
	bool						SetDefaultVirtualDevice(CInputVirtualDevice* pcVirtual);
	CInputDeviceVariableValue*	GetValueForDesc(CInputDeviceVariableValueDesc* pcValueDesc);
	CInputChord*				GetChordForDesc(CInputChordDesc* pcChordDesc);
	CInputVirtualDevice*		GetVariableVirtualDevice(CInputActions* pcInputActions);
	CInputVirtualDevice*		CreateVariableVirtualDeviceFromThis(CInputActions* pcInputActions);
	CInputCategoryGeneric*		GetCategoryGeneric(char* szGeneric);
	void						VariableAction(CInputChord* pcChord);
	EInputDevice				GetDescriptionID(void);
	CInputDeviceValues*			GetInputValues(void);
	CInputDeviceDesc*			GetDesc(void);
	void						DumpEvents(void);
};


typedef CSetType<CInputDevice>	CSetInputDevice;
typedef CSetType<CInputDeviceDesc> CSetInputDeviceDesc;


#endif // !__INPUT_DEVICE_H__

