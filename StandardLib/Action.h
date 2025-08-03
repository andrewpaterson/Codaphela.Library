/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ACTION_H__
#define __ACTION_H__
#include "Unknown.h"
#include "ClassFunctionsCall.h"


//Remember, an action can only be invoked by one specific listener class.
//For instance: A timer alarm cannot invoke an action that expects an input event.
class CAction : public CUnknown, public CListener
{
CONSTRUCTABLE(CAction);
protected:
	CClassFunctionsCall		mcFunction;
	CChars					mszName;

public:
	template <class M>
	void 	Init(char* szName, void(M::*ActionFunc)(CUnknown*, void*));
	void 	Kill(void);
	bool	Is(char* szName);
	void	Call(void *pvContext);

	template <class M>
	bool	AddListener(M* pcListener);
	char*	GetName(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CAction::Init(char* szName, void(M::*ActionFunc)(CUnknown*, void*))
{
	mcFunction.Init<M>();
	mcFunction.SetFunction(ActionFunc);
	mszName.Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
bool CAction::AddListener(M* pcListener)
{
	return mcFunction.AddListener(pcListener);
}


#endif // __ACTION_H__

