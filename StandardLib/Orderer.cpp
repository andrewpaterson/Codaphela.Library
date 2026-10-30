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
	CArrayTemplate<size>	auiOrdered;
	CArrayTemplate<size>	auiUnordered;
	size					i;
	bool					bResult;
	size					uiNumElements;

	auiOrdered.Init();
	auiUnordered.Init();
	bResult = OrderHolders(&auiOrdered, &auiUnordered, ARRAY_ELEMENT_NOT_FOUND, NULL);

	uiNumElements = auiOrdered.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pOrdered->Add(mpHolders->Get(*auiOrdered.Get(i))->GetObject());
	}
	if (pUnordered.IsNotNull())
	{
		uiNumElements = auiUnordered.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pUnordered->Add(mpHolders->Get(*auiUnordered.Get(i))->GetObject());
		}
	}

	auiUnordered.Kill();
	auiOrdered.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the objects could still all be ordered if pObject also required
// szName, without adding the requirement.  False if they can't be ordered
// already.  An object that hasn't been added could require szName if the
// objects can be ordered and one of them fulfils it.
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::CanRequire(Ptr<CBaseObject> pObject, const char* szName)
{
	CArrayTemplate<size>	auiOrdered;
	CArrayTemplate<size>	auiUnordered;
	size					uiHolder;
	bool					bResult;

	auiOrdered.Init();
	auiUnordered.Init();

	uiHolder = GetHolderIndex(pObject);
	if (uiHolder == ARRAY_ELEMENT_NOT_FOUND)
	{
		bResult = OrderHolders(&auiOrdered, &auiUnordered, ARRAY_ELEMENT_NOT_FOUND, NULL);
		if (bResult)
		{
			bResult = IsFulfilled(szName);
		}
	}
	else
	{
		bResult = OrderHolders(&auiOrdered, &auiUnordered, uiHolder, szName);
	}

	auiUnordered.Kill();
	auiOrdered.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Orders the holders by index.  The holder at uiExtraHolder also requires
// szExtraRequirement as if it had been added.
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::OrderHolders(CArrayTemplate<size>* pauiOrdered, CArrayTemplate<size>* pauiUnordered, size uiExtraHolder, const char* szExtraRequirement)
{
	CArrayTemplate<size>	auiToProcess;
	CMapStringInt			cFulfilled;
	size					i;
	size					j;
	size					uiHolder;
	Ptr<COrdererHolder>		pHolder;
	CArrayChars*			paszFulfilments;
	const char*				szRequirement;
	bool					bResult;
	size					uiNumElements;
	size					uiNumFulfilments;

	auiToProcess.Init();
	cFulfilled.Init(true);
	uiNumElements = mpHolders->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		auiToProcess.Add(&i);
	}

	bResult = true;
	while (auiToProcess.NumElements() > 0)
	{
		//Objects ordered earlier in a pass fulfil requirements of objects later in it.
		pauiUnordered->ReInit();
		uiNumElements = auiToProcess.NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			uiHolder = *auiToProcess.Get(i);
			pHolder = mpHolders->Get(uiHolder);
			szRequirement = (uiHolder == uiExtraHolder) ? szExtraRequirement : NULL;
			if (AreRequirementsFulfilled(pHolder, szRequirement, &cFulfilled))
			{
				pauiOrdered->Add(&uiHolder);
				paszFulfilments = pHolder->GetFulfilments();
				uiNumFulfilments = paszFulfilments->NumElements();
				for (j = 0; j < uiNumFulfilments; j++)
				{
					cFulfilled.Put(paszFulfilments->GetText(j), 1);
				}
			}
			else
			{
				pauiUnordered->Add(&uiHolder);
			}
		}

		if (pauiUnordered->NumElements() == auiToProcess.NumElements())
		{
			bResult = false;
			break;
		}

		auiToProcess.ReInit();
		auiToProcess.Copy(pauiUnordered);
	}

	cFulfilled.Kill();
	auiToProcess.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The holder's requirements and szExtraRequirement, if not NULL.
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::AreRequirementsFulfilled(Ptr<COrdererHolder> pHolder, const char* szExtraRequirement, CMapStringInt* pcFulfilled)
{
	CArrayChars*	paszRequirements;
	size			i;
	size			uiNumElements;

	if ((szExtraRequirement != NULL) && !pcFulfilled->HasKey(szExtraRequirement))
	{
		return false;
	}

	paszRequirements = pHolder->GetRequirements();
	uiNumElements = paszRequirements->NumElements();
	for (i = 0; i < uiNumElements; i++)
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
// Whether any object fulfils szName.
//
//////////////////////////////////////////////////////////////////////////
bool COrderer::IsFulfilled(const char* szName)
{
	size				i;
	Ptr<COrdererHolder>	pHolder;
	size				uiNumElements;

	uiNumElements = mpHolders->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pHolder = mpHolders->Get(i);
		if (pHolder->GetFulfilments()->GetIndex(szName) != ARRAY_ELEMENT_NOT_FOUND)
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
	size				uiNumElements;

	mcHolderIndices.ReInit();
	uiNumElements = mpHolders->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pHolder = mpHolders->Get(i);
		oi = pHolder->GetObject().GetIndex();
		mcHolderIndices.Put(oi, &i);
	}
}


size COrderer::NumObjects(void) { return mpHolders->NumElements(); }

