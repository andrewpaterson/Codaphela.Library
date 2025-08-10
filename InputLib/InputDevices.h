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
#ifndef __INPUT_DEVICES_H__
#define __INPUT_DEVICES_H__
#include "StandardLib/Action.h"
#include "InputDevice.h"
#include "InputCategories.h"
#include "InputDataFormats.h"
#include "InputDeviceVirtualDeviceArray.h"


class CInputActions;
class CInputDevices : public CListener
{
CONSTRUCTABLE(CInputDevices); 
public:
	CSetInputDevice					mlcDevices;
	CSetInputDeviceDesc				mlcDeviceDescs;
	CInputCategories				mcCategories;
	CInputDataFormats				mcDataFormats;
	CChars							mszXMLError;
	CChars							mszDirectory;
	CSetInputVirtualDevice			mlcVirtuals;
	CSetInputVirtualDeviceDesc		mlcVirtualsDescs;
	bool							mbCommonalityDirty;
	CInputActions*					mpcInputActions;

	void						Init(CInputActions* pcInputActions);
	bool						Read(char* szDirectory);
	void						Kill(void);

	CInputDeviceDesc*			CreateDeviceDescription(void);

	CInputDataFormat*			AddFormat(char* szName);
	CInputDevice*				CreateDevice(CInputDeviceDesc* pcDesc);
	CInputCategory*				AddCategory(char* szName);
	CInputCategory*				GetCategory(char* szName);

	CInputDeviceDesc*			CreateDescription(char* szID, char* szFriendlyName, CInputCategory* pcCategory, bool bPhysical);
	CInputDeviceDesc*			GetDescription(char* szID);
	CInputDeviceDesc*			GetDescriptionForVirtualDesc(CInputVirtualDeviceDesc* pcVirtualDesc);

	CInputDevice*				GetDevice(int iDeviceID);
	void						GetDevicesForCategory(CArrayInputDevicePtr* pcDevices, char* szCategory);
	void						GetDevicesForDeviceDesc(CArrayInputDevicePtr* pcDevices, CInputDeviceDesc* pcDesc);
	CInputDevice*				GetDeviceForVirtualSourceDesc(CInputVirtualSourceDesc* pcVirtualSourceDesc);
	CInputDevice*				GetDeviceForSourceDesc(CInputSourceDesc* pcSourceDesc, int iDescID);
	void						GetDevices(CArrayInputDevicePtr* pcDevices);

	CInputVirtualDevice*		CreateVirtualDevice(char* szName);
	CInputVirtualDevice*		CreateVirtualDevice(CInputVirtualDeviceDesc* pcVirtualDesc);  //pcVirtualDesc may not be device agnostic.
	bool						CreateVirtualDevices(CArrayInputDeviceVirtualDevice* pacCreatedVirtualVevices, CInputVirtualDeviceDesc* pcVirtualDesc);  //pcVirtualDesc must be device agnostic.
	bool						AddChords(CInputVirtualDevice* pcVirtual, CInputVirtualDeviceDesc* pcVirtualDesc, CInputDevice* pcDevice);
	bool						AddChords(CInputVirtualDevice* pcVirtual, CInputChordDescs* pcChordDescs, CInputDevice* pcDevice);

	CInputVirtualDeviceDesc*	CreateVirtualDeviceDescription(char* szName, bool bDeviceAgnostic);

	void						UpdateCommonality(void);
	void						DirtyCommonality(void);
	void						DumpDevicesToFile(void);
	
	void						UpdateVirtualDevicesEvents(void);

	CInputVirtualDevice*		GetFirstKeyboard(void);
	CInputVirtualDevice*		GetFirstVirtualDeviceInCategoryKeyboard(char* szCategory, bool bPhysical = true);

private:
	bool RecurseAddCriteria(CInputChordCriteriaDesc* pcChordCriteriaDesc, UInputChordCriteria* puChordCriteria, CInputChords* pcChords, CInputDevice* pcDevice);
	bool AddActiveOrInactiveCriteria(CInputChordActiveDesc* pcChordCriteriaDesc, UInputChordCriteria* puChordCriteria, CInputSourceEvaluator* pcEvaluator, CInputDevice* pcDevice);
	bool AddGroupCriteria(CInputChordCollectiveDesc* pcChordCriteriaDesc, CGroupInputChordCriteria* pcGroupCriteria, CInputChords* pcChords, CInputDevice* pcDevice);
	bool AddOrderedCriteria(CInputChordCollectiveDesc* pcChordCriteriaDesc, COrderedInputChordCriteria* pcGroupCriteria, CInputChords* pcChords, CInputDevice* pcDevice);
};


#endif // __INPUT_DEVICES_H__

