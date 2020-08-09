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
#ifndef __INPUT_DEVICE_VARIABLE_VALUE_CONDITION_DESC_H__
#define __INPUT_DEVICE_VARIABLE_VALUE_CONDITION_DESC_H__
#include "StandardLib/Unknown.h"
#include "StandardLib/ArrayType.h"


class CInputChordDesc;
class CInputDeviceVariableValueDesc;
class CInputDeviceVariableValueConditionDesc : public CUnknown
{
CONSTRUCTABLE(CInputDeviceVariableValueConditionDesc);
public:
	void Init(void);
	void Kill(void);

	virtual BOOL IsChord(void) =0;
	virtual BOOL IsVariable(void) =0;
};


class CInputDeviceVariableValueConditionChordDesc : public CInputDeviceVariableValueConditionDesc
{
public:
	CONSTRUCTABLE(CInputDeviceVariableValueConditionChordDesc);

	CInputChordDesc* mpcChordDesc;

	void Init(CInputChordDesc* pcChordDesc);
	void Kill(void);

	BOOL IsChord(void);
	BOOL IsVariable(void);
};


class CInputDeviceVariableValueConditionVariableDesc : public CInputDeviceVariableValueConditionDesc
{
public:
	CONSTRUCTABLE(CInputDeviceVariableValueConditionVariableDesc);

	CInputDeviceVariableValueDesc* mpcVariableValueDesc;

	void Init(CInputDeviceVariableValueDesc* pcVariableValueDesc);
	void Kill(void);

	BOOL IsChord(void);
	BOOL IsVariable(void);
};


typedef CArrayUnknown /*Type<CInputDeviceVariableValueConditionDesc>*/	CArrayInputDeviceVariableValueConditionDesc;


#endif // __INPUT_DEVICE_VARIABLE_VALUE_CONDITION_DESC_H__
