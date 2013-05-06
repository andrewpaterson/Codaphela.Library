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
#ifndef __INPUT_DEVICE_DESC_H__
#define __INPUT_DEVICE_DESC_H__
#include "BaseLib/Chars.h"
#include "BaseLib/MapStringString.h"
#include "InputSourceDesc.h"
#include "InputDataFormat.h"
#include "InputDeviceVariableDesc.h"
#include "InputDevicePointerArray.h"
#include "InputVirtualDeviceDesc.h"
#include "InputVirtualDeviceSwitch.h"


class CInputDevices;
class CInputCategory;
class CInputDeviceCopyContext;
class CInputVirtualDeviceDesc;
class CInputDeviceDesc : public CUnknown
{
BASE_FUNCTIONS(CInputDeviceDesc);
protected:
	CChars								mszID;
	CChars								mszFriendlyName;
	CInputCategory*						mpcCategory;
	CSetInputSourceDesc					mlcInputs;
	CInputDataFormat*					mpcDataFormat;
	CChars								mszComment;
	CSetInputDeviceVariableDesc			mlcVariables;
	CArrayInputVirtualDeviceSwitch		macSwitches;
	CArrayInputDevicePtr				mapcDevices;
	BOOL								mbPhysical;
	CInputChordDescs					mcVariableChordDescs;

public:
	void 						Init(char* szID, char* szFriendlyName, CInputCategory* pcCategory, BOOL bPhysical);
	void 						Kill(void);
	CInputDevices*				GetInputDevices(void);
	void						Copy(CInputDeviceDesc* pcSourceDesc);
	BOOL 						Is(char* szID);
	BOOL						HasTag(char* szTagName);
	CInputSourceDesc*			AddStateInput(char* szFriendlyName);
	CInputSourceDesc*			AddDeltaInput(char* szFriendlyName);
	CInputSourceDesc*			AddInput(EInputSourceType eType, char* szFriendlyName);
	void						CopySources(CInputDeviceCopyContext* pcContext);
	void						CopyVariables(CInputDeviceCopyContext* pcContext);
	void						SetDataFormat(CInputDataFormat* pcDataFormat);
	void						SetComment(char* szComment);
	BOOL						RenameSource(char* szOldName, char* szNewName);
	CInputDeviceVariableDesc*	GetVariable(char* szName);
	CInputDeviceVariableDesc*	AddVariable(char* szName);
	CInputSourceDesc*			GetSource(char* szFriendlyName);
	void						GetSources(CArrayPointer* apcDest, CInputCategoryGeneric* pcGeneric);
	void						GetSources(CArrayPointer* apcDest);
	CInputCategory*				GetCategory(void);
	CInputVirtualDeviceSwitch*	AddVariableSwitch(CInputDeviceVariableValue* pcVariableValue);
	void						ToString(CChars* psz);
	void						AddDevice(CInputDevice* pcDevice);
	void						RemoveDevice(CInputDevice* pcDevice);
	int							GetUnusedID(void);
	CInputVirtualDeviceDesc*	CreateDefaultVirtualDesc(void);
	CInputDeviceVariableDesc*	StartVariablesIteration(SSetIterator* psIter);
	CInputDeviceVariableDesc*	IterateVariables(SSetIterator* psIter);
	CInputSourceDesc*			StartInputsIteration(SSetIterator* psIter);
	CInputSourceDesc*			IterateInputs(SSetIterator* psIter);
	int							NumInputs(void);
	CInputDataFormat*			GetDataFormat(void);
	CInputChordDescs*			GetVariableChordDescs(void);
	BOOL						IsPhysical(void);
	char*						GetID(void);
	char*						GetFriendlyName(void);
	void						Dump(void);
};


#endif // __INPUT_DEVICE_DESC_H__

