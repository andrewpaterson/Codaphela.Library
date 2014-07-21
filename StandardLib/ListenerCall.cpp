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
	mcListeners.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenerCall::AddAllowedClassName(char* szName)
{
	CArrayListenerPtr*	pcArray;

	pcArray = mcListeners.Get(szName);
	if (!pcArray)
	{
		pcArray = mcListeners.Put(szName);
		pcArray->Init(1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CListenerCall::RemoveListener(CListener* pcRemoved)
{
	int					i;
	CArrayListenerPtr*	pcArray;
	CListener*			pcListener;
	SMapIterator		sIter;
	BOOL				bResult;

	bResult = mcListeners.StartIteration(&sIter, NULL, (void**)&pcArray);
	while (bResult)
	{
		for (i = 0; i < pcArray->NumElements(); i++)
		{
			pcListener = (*pcArray->Get(i));
			if (pcListener == pcRemoved)
			{
				pcArray->RemoveAt(i);
				return TRUE;
			}
		}
		bResult = mcListeners.Iterate(&sIter, NULL, (void**)&pcArray);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CListenerCall::CallListenersName(char* szClassName, void(CListener::*ListenerFunc)(CUnknown*, void*), CUnknown* pcSource, void* pvContext)
{
	int					i;
	CListener*			pcListener;
	CArrayListenerPtr*	pcArray;

	pcArray = mcListeners.Get(szClassName);
	if (pcArray)
	{
		for (i = 0; i < pcArray->NumElements(); i++)
		{
			pcListener = *pcArray->Get(i);
			(pcListener->*ListenerFunc)(pcSource, pvContext);
		}
	}
}
