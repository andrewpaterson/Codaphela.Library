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
public:
	CONSTRUCTABLE(CInputDevices);

	CSetInputDevice					mlcDevices;
	CSetInputDeviceDesc				mlcDeviceDescs;
	CInputCategories				mcCategories;
	CInputDataFormats				mcDataFormats;
	CChars							mszXMLError;
	CChars							mszDirectory;
	CSetInputVirtualDevice			mlcVirtuals;
	CSetInputVirtualDeviceDesc		mlcVirtualsDescs;
	BOOL							mbCommonalityDirty;
	CInputActions*					mpcInputActions;

	void						Init(CInputActions* pcInputActions);
	BOOL						Read(char* szDirectory);
	void						Kill(void);

	CInputDeviceDesc*			CreateDeviceDescription(void);

	CInputDataFormat*			AddFormat(char* szName);
	CInputDevice*				CreateDevice(CInputDeviceDesc* pcDesc);
	CInputCategory*				AddCategory(char* szName);
	CInputCategory*				GetCategory(char* szName);

	CInputDeviceDesc*			CreateDescription(char* szID, char* szFriendlyName, CInputCategory* pcCategory, BOOL bPhysical);
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
	BOOL						CreateVirtualDevices(CArrayInputDeviceVirtualDevice* pacCreatedVirtualVevices, CInputVirtualDeviceDesc* pcVirtualDesc);  //pcVirtualDesc must be device agnostic.
	BOOL						AddChords(CInputVirtualDevice* pcVirtual, CInputVirtualDeviceDesc* pcVirtualDesc, CInputDevice* pcDevice);
	BOOL						AddChords(CInputVirtualDevice* pcVirtual, CInputChordDescs* pcChordDescs, CInputDevice* pcDevice);

	CInputVirtualDeviceDesc*	CreateVirtualDeviceDescription(char* szName, BOOL bDeviceAgnostic);

	void						UpdateCommonality(void);
	void						DirtyCommonality(void);
	void						DumpDevicesToFile(void);
	
	void						UpdateVirtualDevicesEvents(void);

	CInputVirtualDevice*		GetFirstKeyboard(void);
	CInputVirtualDevice*		GetFirstVirtualDeviceInCategoryKeyboard(char* szCategory, BOOL bPhysical = TRUE);

private:
	BOOL RecurseAddCriteria(CInputChordCriteriaDesc* pcChordCriteriaDesc, UInputChordCriteria* puChordCriteria, CInputChords* pcChords, CInputDevice* pcDevice);
	BOOL AddActiveOrInactiveCriteria(CInputChordActiveDesc* pcChordCriteriaDesc, UInputChordCriteria* puChordCriteria, CInputSourceEvaluator* pcEvaluator, CInputDevice* pcDevice);
	BOOL AddGroupCriteria(CInputChordCollectiveDesc* pcChordCriteriaDesc, CGroupInputChordCriteria* pcGroupCriteria, CInputChords* pcChords, CInputDevice* pcDevice);
	BOOL AddOrderedCriteria(CInputChordCollectiveDesc* pcChordCriteriaDesc, COrderedInputChordCriteria* pcGroupCriteria, CInputChords* pcChords, CInputDevice* pcDevice);
};


#endif // __INPUT_DEVICES_H__

