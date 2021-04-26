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
#ifndef __LISTENER_CALL_H__
#define __LISTENER_CALL_H__
#include "Unknown.h"
#include "Listener.h"

//I'm not sure where the following defines belong.  I suspect they are what CListenerCall should have been had C++ not been retarded.
//Rewrite this to use variable args templates.
#define CALL_LISTENERS_0(listeners, func) for (int iListenerNumber = 0; iListenerNumber < listeners.NumElements(); iListenerNumber++) { (*listeners.Get(iListenerNumber))->func(); }
#define CALL_LISTENERS_1(listeners, func, param1) for (int iListenerNumber = 0; iListenerNumber < listeners.NumElements(); iListenerNumber++) { (*listeners.Get(iListenerNumber))->func(param1); }
#define CALL_LISTENERS_2(listeners, func, param1, param2) for (int iListenerNumber = 0; iListenerNumber < listeners.NumElements(); iListenerNumber++) { (*listeners.Get(iListenerNumber))->func(param1, param2); }


typedef CArrayTemplate<CListener*>	CArrayListenerPtr;
typedef void (CListener::*ListenerFunc)(CUnknown*, void*);


class CListenerCall
{
protected:
	CMapStringTemplate<CArrayListenerPtr>	mcListeners;

public:
	void			Init(void);
	void			Kill(void);

	void			AddAllowedClassName(const char* szName);
	template <class M>
	void			AddAllowedClass(void);
	template <class M>
	BOOL			AddListener(M* pcListener);
	template <class M>
	void			CallListeners(void(M::*ListenerFunc)(CUnknown*, void*), CUnknown* pcSource, void* pvContext);
	void			CallListenersName(char* szClassName, void(CListener::*ListenerFunc)(CUnknown*, void*), CUnknown* pcSource, void* pvContext);
	BOOL			RemoveListener(CListener* pcRemoved);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CListenerCall::AddAllowedClass(void)
{
	const char*				szName;
	M					cTemp;

	szName = cTemp.ClassName();
	AddAllowedClassName(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
BOOL CListenerCall::AddListener(M* pcListener)
{
	CListener*			pcCast;
	CArrayListenerPtr*	pcArray;
	const char*			szName;
	M					cTemp;

	pcCast = pcListener;
	szName = cTemp.ClassName();
	pcArray = mcListeners.Get(szName);
	if (pcArray)
	{
		pcArray->Add(&pcCast);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CListenerCall::CallListeners(void(M::*ListenerFunc)(CUnknown*, void*), CUnknown* pcSource, void* pvContext)
{
	int					i;
	M*					pcListener;
	M					cTemp;
	const char*			szName;
	CArrayListenerPtr*	pcArray;

	szName = cTemp.ClassName();
	pcArray = mcListeners.Get(szName);
	if (pcArray)
	{
		for (i = 0; i < pcArray->NumElements(); i++)
		{
			pcListener = (M*)(*pcArray->Get(i));
			(pcListener->*ListenerFunc)(pcSource, pvContext);
		}
	}
}


#endif // __LISTENER_CALL_H__

