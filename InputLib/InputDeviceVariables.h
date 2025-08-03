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
#ifndef __INPUT_DEVICE_VARIABLES_H__
#define __INPUT_DEVICE_VARIABLES_H__
#include "InputDeviceVariable.h"
#include "InputDeviceValue.h"


class CInputDeviceVariables
{
public:
	CSetInputDeviceVariable		mlcVariables;
	CInputChords*				mpcChords;
	bool						mbChordThisFrame;

	void 						Init(void);
	void 						Kill(void);
	void						SetChords(CInputChords* pcChords);

	void						ClearEvents(void);

	void 						AddVariablesFromDesc(CInputDeviceDesc* pcDesc);
	void 						AddVariableValuesFromDesc(CInputDeviceVariableDesc* pcVariableDesc, CInputDeviceVariable* pcVariable);
	void 						AddVariableConditionsFromDescription(CInputDeviceVariableValueDesc* pcValueDesc, CInputDeviceVariableValue* pcValue);
	CInputChord*				GetChordForDesc(CInputChordDesc* pcChordDesc);
	CInputDeviceVariableValue*	GetValueForDesc(CInputDeviceVariableValueDesc* pcValueDesc);

	void						VariableAction(CInputChord* pcChord);
	void						VariableAction(CInputDeviceVariable* pcVariable, CInputChord* pcChord, CArrayInputDeviceVariableValuePtr* papcVariableValues);
	void						MatchVariableValue(CInputDeviceVariableValue* pcValue);
	bool						UpdateChordVariableValues(CArrayInputDeviceVariableValuePtr* papcVariableValues);
	bool						UpdateNonChordVariableValues(void);
	bool						UpdateNonChordVariableValues(CInputDeviceVariable* pcVariable);
};


#endif // __INPUT_DEVICE_VARIABLES_H__

