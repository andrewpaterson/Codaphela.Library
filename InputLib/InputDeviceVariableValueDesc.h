/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __INPUT_DEVICE_VARIABLE_VALUE_DESC_H__
#define __INPUT_DEVICE_VARIABLE_VALUE_DESC_H__
#include "BaseLib/Chars.h"
#include "StandardLib/SetType.h"
#include "StandardLib/Unknown.h"
#include "InputDeviceVariableValueConditionDesc.h"
#include "InputDeviceVariableCondition.h"


class CInputDeviceVariableDesc;
class CInputDeviceVariableValueDesc : public CUnknown
{
CONSTRUCTABLE(CInputDeviceVariableValueDesc);
protected:
	CChars											mszName;
	CInputDeviceVariableDesc*						mpcVariableDesc;
	CArrayInputDeviceVariableValueConditionDesc		mapcConditions;
	BOOL											mbInitial;

public:
	void 											Init(char* szName, CInputDeviceVariableDesc* pcVariableDesc);
	void 											Kill(void);
	void 											Copy(CInputDeviceVariableValueDesc* pcSource, CInputDeviceCopyContext* pcContext);
	BOOL 											Equals(char* szName);
	void 											AddCondition(CInputChordDesc* pcChordDesc);
	void 											AddCondition(CInputDeviceVariableValueDesc* pcVariableValueDesc);
	char*											GetName(void);
	BOOL											IsInitial(void);
	CArrayInputDeviceVariableValueConditionDesc*	GetConditions(void);
	CInputDeviceVariableDesc*						GetVariableDesc(void);
	void											SetInitial(BOOL bInitial);
};


typedef CSetType<CInputDeviceVariableValueDesc>	CSetInputDeviceVariableValueDesc;


#endif // __INPUT_DEVICE_VARIABLE_VALUE_DESC_H__

