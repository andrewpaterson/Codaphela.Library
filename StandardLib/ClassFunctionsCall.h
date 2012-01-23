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
#ifndef __CLASS_FUNCTIONS_CALL_H__
#define __CLASS_FUNCTIONS_CALL_H__
#include "ListenerCall.h"


class CClassFunctionsCall : public CListenerCall
{
public:
	ListenerFunc					mFunction;
	CChars							mszClassName;

	template<class M>
	void 	Init(void);
	void 	Init(char* szClassName);
	void 	Kill(void);

	template <class M>
	void	SetFunction(void(M::*ActionFunc)(CUnknown*, void*));
	void	CallListeners(CUnknown* pcSource, void* pvContext);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CClassFunctionsCall::Init(void)
{
	M	cTemp;
	
	mFunction = NULL;
	CClassFunctionsCall::Init(cTemp.ClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CClassFunctionsCall::SetFunction(void(M::*ActionFunc)(CUnknown*, void*))
{
	mFunction = (ListenerFunc)ActionFunc;
}


#endif // __CLASS_FUNCTIONS_CALL_H__

