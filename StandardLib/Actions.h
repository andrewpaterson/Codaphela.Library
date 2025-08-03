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
#ifndef __ACTIONABLE_H__
#define __ACTIONABLE_H__
#include "SetType.h"
#include "Action.h"


typedef CSetType<CAction>	CSetActions;


class CActions
{
public:
	CSetActions		mlcActions;

	void 		Init(void);
	void 		Kill(void);

	template <class M>
	CAction*	AddAction(char* szName, void(M::*ActionFunc)(CUnknown*, void*));
	CAction*	GetAction(char* szName);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
CAction* CActions::AddAction(char* szName, void(M::*ActionFunc)(CUnknown*, void*))
{
	CAction*		pcAction;

	pcAction = mlcActions.Add();
	pcAction->Init(szName, ActionFunc);
	return pcAction;
}


#endif // __ACTIONABLE_H__

