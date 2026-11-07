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
#ifndef __INPUT_ACTIONS_H__
#define __INPUT_ACTIONS_H__
#include "StandardLib/Actions.h"


//The action whose chords change a device's variables (see CInputDevice::VariableAction).
#define INPUT_DEVICE_VARIABLE_ACTION "Variable Action"


class CInputDevices;
class CInputActions
{
public:
	CActions			mcActions;
	CInputDevices*		mpcInputDevices;

	void 		Init(CInputDevices* pcInputDevices);
	void 		Kill(void);
	CAction*	GetAction(const char* szName);

	CAction*	AddAction(const char* szName);
	template<class M, class O>
	CAction*	AddAction(const char* szName, O* pcObject, void(M::*fFunction)(CAction* pcAction, void* pvContext));

	void		VariableAction(CAction* pcAction, void* pvContext);
};


//////////////////////////////////////////////////////////////////////////
//
// An action that calls pcObject's fFunction.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
CAction* CInputActions::AddAction(const char* szName, O* pcObject, void(M::*fFunction)(CAction* pcAction, void* pvContext))
{
	CAction*	pcAction;

	pcAction = AddAction(szName);
	pcAction->Add(pcObject, fFunction);
	return pcAction;
}


#endif // __INPUT_ACTIONS_H__

