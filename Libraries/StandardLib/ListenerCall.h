/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/ArrayTemplatePtr.h"

//I'm not sure where the following defines belong.  I suspect they are what CListenerCall should have been had C++ not been retarded.
//Rewrite this to use variable args templates.
#define CALL_LISTENERS_0(listeners, func) for (size iListenerNumber = 0; iListenerNumber < listeners.NumElements(); iListenerNumber++) { (*listeners.Get(iListenerNumber))->func(); }
#define CALL_LISTENERS_1(listeners, func, param1) for (size iListenerNumber = 0; iListenerNumber < listeners.NumElements(); iListenerNumber++) { (*listeners.Get(iListenerNumber))->func(param1); }
#define CALL_LISTENERS_2(listeners, func, param1, param2) for (size iListenerNumber = 0; iListenerNumber < listeners.NumElements(); iListenerNumber++) { (*listeners.Get(iListenerNumber))->func(param1, param2); }


typedef CArrayTemplatePtr<CListener>			CArrayListenerPtr;
typedef CArrayTemplateEmbeddedPtr<CListener, 8>	CArrayEmbeddedListenerPtr;


//Calls its listeners, by the listener class they were added as.  A listener is added once per class and is told which
//calls it is in so that it can remove itself when it is destroyed; Kill removes the call from the listeners still in
//it.  A listener may add or remove listeners while it is being called; one removed before its turn is not called.
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
	bool			AddListener(M* pcListener);
	template <class M>
	void			CallListeners(void(M::* ListenerFunc)(CUnknown*, void*), CUnknown* pcSource, void* pvContext);
	bool			RemoveListener(CListener* pcRemoved);
	bool			IsListening(const char* szClassName, CListener* pcListener);
	size			NumListeners(void);

protected:
	bool			StartCall(const char* szClassName, CArrayEmbeddedListenerPtr* papcCalled);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CListenerCall::AddAllowedClass(void)
{
	AddAllowedClassName(M::StaticClassName());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
bool CListenerCall::AddListener(M* pcListener)
{
	CListener*			pcCast;
	CArrayListenerPtr*	pcArray;

	pcCast = pcListener;
	pcArray = mcListeners.Get(M::StaticClassName());
	if ((pcArray == NULL) || (pcCast == NULL))
	{
		return false;
	}
	if (pcArray->Find(pcCast) != ARRAY_ELEMENT_NOT_FOUND)
	{
		//Already listening.  A listener added twice would be called twice for every event.
		return false;
	}

	pcArray->Add(pcCast);
	pcCast->AddListenerCall(this);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template <class M>
void CListenerCall::CallListeners(void(M::*ListenerFunc)(CUnknown*, void*), CUnknown* pcSource, void* pvContext)
{
	size						i;
	M*							pcListener;
	CListener*					pcCalled;
	const char*					szName;
	CArrayEmbeddedListenerPtr	apcCalled;
	size						uiNumElements;

	szName = M::StaticClassName();
	if (StartCall(szName, &apcCalled))
	{
		uiNumElements = apcCalled.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcCalled = apcCalled.GetPtr(i);
			if (IsListening(szName, pcCalled))
			{
				pcListener = (M*)pcCalled;
				(pcListener->*ListenerFunc)(pcSource, pvContext);
			}
		}
		apcCalled.Kill();
	}
}


#endif // __LISTENER_CALL_H__

