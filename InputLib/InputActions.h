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
#include "InputDeviceVariableListener.h"


class CInputDevices;
class CInputActions : public CInputDeviceVariableListener
{
public:
	CActions			mcActions;
	CInputDevices*		mpcInputDevices;

	void 		Init(CInputDevices* pcInputDevices);
	void 		Kill(void);
	CAction*	GetAction(char* szName);

	template <class M>
	CAction*	AddAction(char* szName, void(M::*ActionFunc)(CUnknown*, void*));

	template <class M>
	CAction*	AddActionX(char* szName, void(M::*ActionFunc)(CUnknown*, void*), M* pcListener);

	void		VariableAction(CUnknown* pcSource, void* pvContext);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CAction* CInputActions::AddAction(char* szName, void(M::*ActionFunc)(CUnknown*, void*))
{
	return mcActions.AddAction(szName, ActionFunc);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CAction* CInputActions::AddActionX(char* szName, void(M::*ActionFunc)(CUnknown*, void*), M* pcListener)
{
	CAction*	pcAction;

	pcAction = mcActions.AddAction(szName, ActionFunc);
	pcAction->AddListener(pcListener);
	return pcAction;
}


#endif // __INPUT_ACTIONS_H__

