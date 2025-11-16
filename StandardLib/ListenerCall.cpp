/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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

	bContinue = mcListeners.StartIteration(&sIter, NULL, NULL, &pvData, &uiDataSize);
	while (bContinue)
	{
		pcArray = (CArrayListenerPtr*)pvData;
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
	size				i;
	CArrayListenerPtr*	pcArray;
	CListener*			pcListener;
	SMapIterator		sIter;
	bool				bResult;
	size				uiNumElements;

	bResult = mcListeners.StartIteration(&sIter, NULL, NULL, (void**)&pcArray, NULL);
	while (bResult)
	{
		uiNumElements = pcArray->NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcListener = (*pcArray->Get(i));
			if (pcListener == pcRemoved)
			{
				pcArray->RemoveAt(i);
				return true;
			}
		}
		bResult = mcListeners.Iterate(&sIter, NULL, NULL, (void**)&pcArray, NULL);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenerCall::CallListenersName(char* szClassName, void(CListener::*ListenerFunc)(CUnknown*, void*), CUnknown* pcSource, void* pvContext)
{
	size				i;
	CListener*			pcListener;
	CArrayListenerPtr*	pcArray;
	size				uiNumElements;

	pcArray = mcListeners.Get(szClassName);
	if (pcArray)
	{
		uiNumElements = pcArray->NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcListener = *pcArray->Get(i);
			(pcListener->*ListenerFunc)(pcSource, pvContext);
		}
	}
}

