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
#include "ListenerCall.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListener::CListener(void)
{
	mapcCalls.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListener::CListener(const CListener& cOther)
{
	//A copy is not in the calls the original is in.
	mapcCalls.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListener::~CListener(void)
{
	RemoveFromListenerCalls();
	mapcCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CListener& CListener::operator=(const CListener& cOther)
{
	//The listener stays in its own calls, not the other's.
	return *this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListener::RemoveFromListenerCalls(void)
{
	CListenerCall*	pcCall;

	//RemoveListener takes the call out of mapcCalls.
	while (mapcCalls.NumElements() > 0)
	{
		pcCall = mapcCalls.TailPtr();
		if (!pcCall->RemoveListener(this))
		{
			RemoveListenerCall(pcCall);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CListener::NumListenerCalls(void)
{
	return mapcCalls.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListener::AddListenerCall(CListenerCall* pcCall)
{
	if (mapcCalls.Find(pcCall) == ARRAY_ELEMENT_NOT_FOUND)
	{
		mapcCalls.Add(pcCall);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListener::RemoveListenerCall(CListenerCall* pcCall)
{
	size	uiIndex;

	uiIndex = mapcCalls.Find(pcCall);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		mapcCalls.RemoveAt(uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenerCall::Init(void)
{
	mcListeners.Init(1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenerCall::Kill(void)
{
	SMapIterator		sIter;
	void*				pvData;
	size				uiDataSize;
	bool				bContinue;
	CArrayListenerPtr*	pcArray;
	size				i;
	size				uiNumElements;

	bContinue = mcListeners.StartIteration(&sIter, NULL, NULL, &pvData, &uiDataSize);
	while (bContinue)
	{
		pcArray = (CArrayListenerPtr*)pvData;
		uiNumElements = pcArray->NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcArray->GetPtr(i)->RemoveListenerCall(this);
		}
		pcArray->Kill();
		bContinue = mcListeners.Iterate(&sIter, NULL, NULL, &pvData, &uiDataSize);
	}
	mcListeners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenerCall::AddAllowedClassName(const char* szName)
{
	CArrayListenerPtr*	pcArray;

	pcArray = mcListeners.Get(szName);
	if (!pcArray)
	{
		pcArray = mcListeners.Put(szName);
		pcArray->Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenerCall::RemoveListener(CListener* pcRemoved)
{
	size				uiIndex;
	CArrayListenerPtr*	pcArray;
	SMapIterator		sIter;
	bool				bContinue;
	bool				bRemoved;

	//A listener can be in the call as more than one listener class.  It's taken out of all of them.
	bRemoved = false;
	bContinue = mcListeners.StartIteration(&sIter, NULL, NULL, (void**)&pcArray, NULL);
	while (bContinue)
	{
		uiIndex = pcArray->Find(pcRemoved);
		if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			pcArray->RemoveAt(uiIndex);
			bRemoved = true;
		}
		bContinue = mcListeners.Iterate(&sIter, NULL, NULL, (void**)&pcArray, NULL);
	}

	if (bRemoved)
	{
		pcRemoved->RemoveListenerCall(this);
	}
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenerCall::IsListening(const char* szClassName, CListener* pcListener)
{
	CArrayListenerPtr*	pcArray;

	pcArray = mcListeners.Get(szClassName);
	if (pcArray)
	{
		return pcArray->Find(pcListener) != ARRAY_ELEMENT_NOT_FOUND;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CListenerCall::NumListeners(void)
{
	CArrayListenerPtr*	pcArray;
	SMapIterator		sIter;
	bool				bContinue;
	size				uiCount;

	uiCount = 0;
	bContinue = mcListeners.StartIteration(&sIter, NULL, NULL, (void**)&pcArray, NULL);
	while (bContinue)
	{
		uiCount += pcArray->NumElements();
		bContinue = mcListeners.Iterate(&sIter, NULL, NULL, (void**)&pcArray, NULL);
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CListenerCall::StartCall(const char* szClassName, CArrayEmbeddedListenerPtr* papcCalled)
{
	//The listeners are copied before any is called because a listener may add or remove listeners while it's called.
	CArrayListenerPtr*	pcArray;
	size				i;
	size				uiNumElements;

	pcArray = mcListeners.Get(szClassName);
	if (pcArray == NULL)
	{
		return false;
	}

	papcCalled->Init();
	uiNumElements = pcArray->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		papcCalled->Add(pcArray->GetPtr(i));
	}
	return true;
}

