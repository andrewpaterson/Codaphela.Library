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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "BaseObject.h"
#include "WeakFroms.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakFroms::Init(void)
{
	mcObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject's weak froms, NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
CArrayWeakFrom* CWeakFroms::Find(CEmbeddedObject* pcObject)
{
	CArrayWeakFrom**	ppacFroms;

	ppacFroms = mcObjects.Get(&pcObject);
	return (ppacFroms != NULL) ? *ppacFroms : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing should be weakly held by now: every object has been freed,
// which cleared its weak froms.
//
//////////////////////////////////////////////////////////////////////////
void CWeakFroms::Kill(void)
{
	size			i;
	CArrayWeakFrom*	pacFroms;

	for (i = 0; i < mcObjects.NumElements(); i++)
	{
		pacFroms = *mcObjects.GetValue(i);
		SafeKill(pacFroms);
	}
	mcObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakFroms::Add(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot)
{
	CArrayWeakFrom*	pacFroms;
	SWeakFrom*		psFrom;

	pacFroms = Find(pcObject);
	if (pacFroms == NULL)
	{
		pacFroms = NewMalloc<CArrayWeakFrom>();
		pacFroms->Init();
		mcObjects.Put(&pcObject, &pacFroms);
	}
	psFrom = pacFroms->Add();
	psFrom->pcHolder = pcHolder;
	psFrom->uiSlot = uiSlot;
}


//////////////////////////////////////////////////////////////////////////
//
// False if pcObject hadn't that weak from.
//
//////////////////////////////////////////////////////////////////////////
bool CWeakFroms::Remove(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot)
{
	CArrayWeakFrom*	pacFroms;
	SWeakFrom*		psFrom;
	size			i;

	pacFroms = Find(pcObject);
	if (pacFroms == NULL)
	{
		return false;
	}
	for (i = 0; i < pacFroms->NumElements(); i++)
	{
		psFrom = pacFroms->Get(i);
		if ((psFrom->pcHolder == pcHolder) && (psFrom->uiSlot == uiSlot))
		{
			pacFroms->RemoveAt(i, false);
			if (pacFroms->NumElements() == 0)
			{
				mcObjects.Remove(&pcObject);
				SafeKill(pacFroms);
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Taken out of the map first, so a holder's ClearWeak can't reach it.
//
//////////////////////////////////////////////////////////////////////////
void CWeakFroms::Clear(CEmbeddedObject* pcObject)
{
	CArrayWeakFrom*	pacFroms;
	SWeakFrom*		psFrom;
	size			i;

	pacFroms = Find(pcObject);
	if (pacFroms == NULL)
	{
		return;
	}
	mcObjects.Remove(&pcObject);
	for (i = 0; i < pacFroms->NumElements(); i++)
	{
		psFrom = pacFroms->Get(i);
		psFrom->pcHolder->ClearWeak(psFrom->uiSlot);
	}
	SafeKill(pacFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakFroms::Move(CEmbeddedObject* pcFrom, CEmbeddedObject* pcTo)
{
	CArrayWeakFrom*	pacFroms;
	SWeakFrom*		psFrom;
	size			i;

	pacFroms = Find(pcFrom);
	if ((pacFroms == NULL) || (pcFrom == pcTo))
	{
		return;
	}
	mcObjects.Remove(&pcFrom);
	for (i = 0; i < pacFroms->NumElements(); i++)
	{
		psFrom = pacFroms->Get(i);
		psFrom->pcHolder->RemapWeak(psFrom->uiSlot, pcTo);
		Add(pcTo, psFrom->pcHolder, psFrom->uiSlot);
	}
	SafeKill(pacFroms);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CWeakFroms::NumWeakFroms(CEmbeddedObject* pcObject)
{
	CArrayWeakFrom*	pacFroms;

	pacFroms = Find(pcObject);
	return (pacFroms != NULL) ? pacFroms->NumElements() : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SWeakFrom* CWeakFroms::GetWeakFrom(CEmbeddedObject* pcObject, size uiIndex)
{
	CArrayWeakFrom*	pacFroms;

	pacFroms = Find(pcObject);
	return ((pacFroms != NULL) && (uiIndex < pacFroms->NumElements())) ? pacFroms->Get(uiIndex) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CWeakFroms::NumObjects(void)
{
	return mcObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWeakFroms::Contains(CEmbeddedObject* pcObject, CWeakHolder* pcHolder, size uiSlot)
{
	size		i;
	SWeakFrom*	psFrom;

	for (i = 0; i < NumWeakFroms(pcObject); i++)
	{
		psFrom = GetWeakFrom(pcObject, i);
		if ((psFrom->pcHolder == pcHolder) && (psFrom->uiSlot == uiSlot))
		{
			return true;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakFroms::GetObjects(CArrayEmbeddedObjectPtr* papcObjects)
{
	size	i;

	for (i = 0; i < mcObjects.NumElements(); i++)
	{
		papcObjects->Add(*mcObjects.GetKey(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWeakFroms::ValidateConsistency(void)
{
	CEmbeddedObject*	pcObject;
	CArrayWeakFrom*		pacFroms;
	SWeakFrom*			psFrom;
	CBaseObject*		pcContainer;
	size				uiObject;
	size				i;
	CChars				szObject;

	for (uiObject = 0; uiObject < mcObjects.NumElements(); uiObject++)
	{
		pcObject = *mcObjects.GetKey(uiObject);
		pacFroms = *mcObjects.GetValue(uiObject);
		pcContainer = pcObject->GetEmbeddingContainer();
		if (((pcContainer->GetFlags() & OBJECT_FLAGS_WEAKLY_HELD) == 0) || (pcContainer->GetFlags() & OBJECT_FLAGS_FREED))
		{
			szObject.Init();
			pcObject->PrintObject(&szObject);
			gcLogger.Error2(__METHOD__, " Weakly held object [", szObject.Text(), "] isn't flagged weakly held, or is freed.", NULL);
			szObject.Kill();
		}
		for (i = 0; i < pacFroms->NumElements(); i++)
		{
			psFrom = pacFroms->Get(i);
			if (psFrom->pcHolder->GetWeak(psFrom->uiSlot) != pcObject)
			{
				szObject.Init();
				pcObject->PrintObject(&szObject);
				gcLogger.Error2(__METHOD__, " A weak reference to [", szObject.Text(), "] doesn't point at it.", NULL);
				szObject.Kill();
			}
		}
	}
}

