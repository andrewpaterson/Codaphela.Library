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
#include "BaseLib/MapStringInt.h"
#include "ClassDefines.h"
#include "Objects.h"
#include "Orderer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderer::Init(void)
{
	PreInit();
	mpHolders = OMalloc<CArray<COrdererHolder>>();
	mcHolderIndices.Init(true);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderer::Free(void)
{
	mcHolderIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderer::Class(void)
{
	M_Pointer(mpHolders);
	U_Data(CMapLongSize, mcHolderIndices);
}


//////////////////////////////////////////////////////////////////////////
//
// The holders are saved through mpHolders and the index is rebuilt.
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::Save(CObjectWriter* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::Load(CObjectReader* pcFile)
{
	mcHolderIndices.Init(true);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An object to order, for one that neither fulfils nor requires anything.
// Adding a fulfilment or a requirement adds the object too.
//
//////////////////////////////////////////////////////////////////////////
void COrderer::Add(Ptr<CBaseObject> pObject)
{
	GetOrAddHolder(pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderer::AddFulfilment(Ptr<CBaseObject> pObject, const char* szName)
{
	GetOrAddHolder(pObject)->AddFulfilment(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderer::AddRequirement(Ptr<CBaseObject> pObject, const char* szName)
{
	GetOrAddHolder(pObject)->AddRequirement(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::Order(Ptr<CArray<CBaseObject>> pOrdered)
{
	return Order(pOrdered, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the objects to pOrdered so that each comes after the objects that
// fulfil its requirements.  Returns false if they can't all be ordered;
// pOrdered then has those that could be and pUnordered, if not NULL, the
// rest in the order they were added.
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::Order(Ptr<CArray<CBaseObject>> pOrdered, Ptr<CArray<CBaseObject>> pUnordered)
{
	CArrayTemplate<size>	auiToProcess;
	CArrayTemplate<size>	auiRemaining;
	CMapStringInt			cFulfilled;
	size					i;
	size					j;
	size					uiHolder;
	Ptr<COrdererHolder>		pHolder;
	CArrayChars*			paszFulfilments;
	bool					bResult;

	auiToProcess.Init();
	auiRemaining.Init();
	cFulfilled.Init(true);
	for (i = 0; i < mpHolders->NumElements(); i++)
	{
		auiToProcess.Add(&i);
	}

	bResult = true;
	while (auiToProcess.NumElements() > 0)
	{
		//Objects ordered earlier in a pass fulfil requirements of objects later in it.
		for (i = 0; i < auiToProcess.NumElements(); i++)
		{
			uiHolder = *auiToProcess.Get(i);
			pHolder = mpHolders->Get(uiHolder);
			if (AreRequirementsFulfilled(pHolder, &cFulfilled))
			{
				pOrdered->Add(pHolder->GetObject());
				paszFulfilments = pHolder->GetFulfilments();
				for (j = 0; j < paszFulfilments->NumElements(); j++)
				{
					cFulfilled.Put(paszFulfilments->GetText(j), 1);
				}
			}
			else
			{
				auiRemaining.Add(&uiHolder);
			}
		}

		if (auiRemaining.NumElements() == auiToProcess.NumElements())
		{
			bResult = false;
			break;
		}

		auiToProcess.ReInit();
		auiToProcess.Copy(&auiRemaining);
		auiRemaining.ReInit();
	}

	if (!bResult && pUnordered.IsNotNull())
	{
		for (i = 0; i < auiRemaining.NumElements(); i++)
		{
			pHolder = mpHolders->Get(*auiRemaining.Get(i));
			pUnordered->Add(pHolder->GetObject());
		}
	}

	cFulfilled.Kill();
	auiRemaining.Kill();
	auiToProcess.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::AreRequirementsFulfilled(Ptr<COrdererHolder> pHolder, CMapStringInt* pcFulfilled)
{
	CArrayChars*	paszRequirements;
	size			i;

	paszRequirements = pHolder->GetRequirements();
	for (i = 0; i < paszRequirements->NumElements(); i++)
	{
		if (!pcFulfilled->HasKey(paszRequirements->GetText(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<COrdererHolder> COrderer::GetOrAddHolder(Ptr<CBaseObject> pObject)
{
	Ptr<COrdererHolder>	pHolder;
	OIndex				oi;
	size				uiIndex;

	pHolder = GetHolder(pObject);
	if (pHolder.IsNull())
	{
		pHolder = OMalloc<COrdererHolder>(pObject);
		uiIndex = mpHolders->NumElements();
		mpHolders->Add(pHolder);
		oi = pObject.GetIndex();
		mcHolderIndices.Put(oi, &uiIndex);
	}
	return pHolder;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the object hasn't been added.
//
//////////////////////////////////////////////////////////////////////////
Ptr<COrdererHolder> COrderer::GetHolder(Ptr<CBaseObject> pObject)
{
	size	uiIndex;

	uiIndex = GetHolderIndex(pObject);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return NULL;
	}
	return mpHolders->Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size COrderer::GetHolderIndex(Ptr<CBaseObject> pObject)
{
	OIndex	oi;
	size*	puiIndex;

	if (mcHolderIndices.NumElements() != mpHolders->NumElements())
	{
		IndexHolders();
	}

	oi = pObject.GetIndex();
	puiIndex = mcHolderIndices.Get(oi);
	if (puiIndex == NULL)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	return *puiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrderer::IndexHolders(void)
{
	size				i;
	Ptr<COrdererHolder>	pHolder;
	OIndex				oi;

	mcHolderIndices.ReInit();
	for (i = 0; i < mpHolders->NumElements(); i++)
	{
		pHolder = mpHolders->Get(i);
		oi = pHolder->GetObject().GetIndex();
		mcHolderIndices.Put(oi, &i);
	}
}


size COrderer::NumObjects(void) { return mpHolders->NumElements(); }

