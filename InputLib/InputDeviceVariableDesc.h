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
#ifndef __INPUT_DEVICE_VARIABLE_DESC_H__
#define __INPUT_DEVICE_VARIABLE_DESC_H__
#include "BaseLib/Chars.h"
#include "StandardLib/SetType.h"
#include "InputDeviceVariableValueDesc.h"


class CInputDeviceDesc;
class CInputDeviceCopyContext;
class CInputDeviceVariableDesc : public CUnknown
{
CONSTRUCTABLE(CInputDeviceVariableDesc);
protected:
	CChars								mszName;
	CSetInputDeviceVariableValueDesc	mlcValues;
	CInputDeviceDesc*					mpcDeviceDesc;

public:
	void 							Init(char* szName, CInputDeviceDesc* pcDeviceDesc);
	void 							Kill(void);
	void							Copy(CInputDeviceVariableDesc* pcSourceDesc, CInputDeviceCopyContext* pcContext);

	bool 							Equals(char* szName);
	CInputDeviceVariableValueDesc*	AddValue(char* szName);
	CInputDeviceVariableValueDesc*	GetValue(char* szName);

	void							ToString(CChars* psz);
	CInputDeviceVariableValueDesc*	StartValuesIteration(SSetIterator* psIter);
	CInputDeviceVariableValueDesc*	IterateValues(SSetIterator* psIter);

	char*								GetName(void);
	CInputDeviceDesc*					GetDeviceDesc(void);
};


typedef CSetType<CInputDeviceVariableDesc>	CSetInputDeviceVariableDesc;


#endif // !__INPUT_DEVICE_VARIABLE_DESC_H__

