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
#include "PointerObject.h"
#include "Objects.h"
#include "Object.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObject::CObject()
{
	mapPointers.Init();
	mapEmbedded.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObject::~CObject()
{
	//Only called for an object on the stack.
	if (!DestructOnStack(__METHOD__, false))
	{
		mapPointers.Kill();
		mapEmbedded.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Allocate(CObjects* pcObjects)
{
	size	i;
	size	uiNumEmbedded;

	CBaseObject::Allocate(pcObjects);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		(*mapEmbedded.Get(i))->CBaseObject::Allocate(NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Kill(void)
{
	if (muiFlags != OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		CBaseObject::Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::EmbedFields(void)
{
	EmbedPointerFields();
	EmbedEmbeddedObjectFields();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::EmbedPointerFields(void)
{
	size				uiNumFields;
	CPointerField**		ppacPointerFields;
	CArrayVoidPtr*		papv;
	size				i;
	CPointer*			pcPointer;

	papv = mpcClass->GetPointerFields();
	ppacPointerFields = (CPointerField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPointer = ppacPointerFields[i]->GetPointer(this);
		pcPointer->SetEmbedding(this);
		mapPointers.AddPtr(pcPointer);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::EmbedEmbeddedObjectFields(void)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	size					j;
	CBaseObject*			pcEmbeddedObject;
	CObject*				pcEmbeddedContainer;
	size					uiNumNested;

	//Each embedded object is followed by the ones embedded in it: PreClass has listed them in it already.
	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		pcEmbeddedObject->EmbedIn(this);
		pcEmbeddedObject->PreClass();
		mapEmbedded.AddPtr(pcEmbeddedObject);

		if (pcEmbeddedObject->IsObject())
		{
			pcEmbeddedContainer = (CObject*)pcEmbeddedObject;
			uiNumNested = pcEmbeddedContainer->mapEmbedded.NumElements();
			for (j = 0; j < uiNumNested; j++)
			{
				mapEmbedded.AddPtr(*pcEmbeddedContainer->mapEmbedded.Get(j));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The objects this is made of, for the loops over them: this (0), then
// each object embedded in it at any depth (1 on, mapEmbedded's order).
// There are CountNumEmbedded of them.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObject::GetContained(size uiIndex)
{
	return (uiIndex == 0) ? this : *mapEmbedded.Get(uiIndex - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// The next object embedded directly in this, from *puiIndex in
// mapEmbedded; NULL after the last.  mapEmbedded holds every object
// embedded in this, at any depth, each followed by those embedded in
// it, which are stepped over.  Moves *puiIndex on before returning, so
// the object returned may free its own list.
//
// No virtual call is made on the embedded objects: ~CObject runs after
// the members embedded in a sub-class have been destroyed.
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObject::IterateDirectlyEmbedded(size* puiIndex)
{
	CBaseObject*	pcEmbedded;
	size			uiNumEmbedded;

	uiNumEmbedded = mapEmbedded.NumElements();
	if (*puiIndex >= uiNumEmbedded)
	{
		return NULL;
	}

	pcEmbedded = *mapEmbedded.Get(*puiIndex);
	(*puiIndex)++;
	while ((*puiIndex < uiNumEmbedded) && ((*mapEmbedded.Get(*puiIndex))->GetEmbedding() != this))
	{
		(*puiIndex)++;
	}
	return pcEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::FreePointers(void)
{
	CBaseObject*	pcEmbedded;
	size			uiIndex;

	uiIndex = 0;
	pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	while (pcEmbedded)
	{
		pcEmbedded->FreePointers();
		pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	}
	CBaseObject::FreePointers();

	mapEmbedded.Kill();
	mapPointers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::FreeInternal(void)
{
	CBaseObject*	pcEmbedded;
	size			uiIndex;

	uiIndex = 0;
	pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	while (pcEmbedded)
	{
		pcEmbedded->FreeInternal();
		pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	}

	CBaseObject::FreeInternal();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::DoneKill(bool bAllocatedInObjects)
{
	CBaseObject*	pcEmbedded;
	size			uiIndex;

	uiIndex = 0;
	pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	while (pcEmbedded)
	{
		pcEmbedded->DoneKill(bAllocatedInObjects);
		pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	}

	CBaseObject::DoneKill(bAllocatedInObjects);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::BaseNumPointerTos(void)
{
	size				i;
	CEmbeddedObject*	pcPointedTo;
	CPointer**			ppPointer;
	size				uiCount;
	size				uiNumElements;

	uiCount = 0;
	uiNumElements = mapPointers.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		if (pcPointedTo)
		{
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::NumPointerTos(void)
{
	size	i;
	size	uiNumContained;
	size	uiCount;

	uiCount = 0;
	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		uiCount += GetContained(i)->BaseNumPointerTos();
	}
	return uiCount;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::IsObject(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::IsCollection(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveAllHeapFroms(void)
{
	size	i;
	size	uiNumContained;

	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		GetContained(i)->CBaseObject::RemoveAllHeapFroms();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveAllStackFroms(void)
{
	size	i;
	size	uiNumContained;

	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		GetContained(i)->CBaseObject::RemoveAllStackFroms();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::GetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
{
	size	i;
	size	uiNumContained;

	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		GetContained(i)->BaseGetPointerTos(papcTos);
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::BaseGetPointerTos(CArrayEmbeddedObjectPtr* papcTos)
{
	size					uiNumPointers;
	size					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		if (pcPointedTo)
		{
			papcTos->Add(pcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::ClearPointerTos(bool bFree)
{
	size				i;
	size				uiIndex;
	CBaseObject*		pcEmbedded;
	size				uiNumPointers;
	CPointer*			pcPointer;
	bool				bResult;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		pcPointer = *mapPointers.Get(i);
		if (bFree)
		{
			pcPointer->PointToNull();
		}
		else
		{
			//An object freed with this drops its froms anyway: unlinking from a hub (one search of its froms for each
			//object freed with it) was the cost of freeing them together.  Others, embedded hollow objects too, keep living.
			if (pcPointer->Object() && !pcPointer->Object()->IsBeingFreed())
			{
				pcPointer->Object()->UnlinkHeapFrom(this);
			}
			pcPointer->UnsafeClearObject();
		}
	}

	bResult = true;
	uiIndex = 0;
	pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	while (pcEmbedded)
	{
		bResult &= pcEmbedded->ClearPointerTos(bFree);
		pcEmbedded = IterateDirectlyEmbedded(&uiIndex);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
	size				uiNumPointers;
	size				i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
	
		if (pcPointedTo == pcTo)
		{
			(*ppPointer)->UnsafeClearObject();	
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::GetPointerFieldsTo(CEmbeddedObject* pcTo, CArrayHeapFromPointer* pacPointers)
{
	size				i;
	size				uiNumPointers;
	CPointer**			ppPointer;
	SHeapFromPointer*	psPointer;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		if ((*ppPointer)->Object() == pcTo)
		{
			psPointer = pacPointers->Add();
			psPointer->pcFrom = this;
			psPointer->pcTo = pcTo;
			psPointer->pcPointer = *ppPointer;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	size				uiNumPointers;
	size				i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;
	size				uiCount;

	uiCount = 0;
	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (**ppPointer).Object();
		if (pcPointedTo == pcOld)
		{
			(**ppPointer).UnsafePointTo(pcNew);
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
{
	size	i;
	size	uiNumEmbedded;

	if (pcEmbedded == this)
	{
		return 0;
	}

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		if (*mapEmbedded.Get(i) == pcEmbedded)
		{
			return i + 1;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::NumEmbedded(void)
{
	size			uiNumEmbedded;

	uiNumEmbedded = GetNumEmbeddedFromFlags();
	if (uiNumEmbedded != 0)
	{
		return (uint16)uiNumEmbedded;
	}

	uiNumEmbedded = CountNumEmbedded();

	SetFlagNumEmbedded(uiNumEmbedded);
	return uiNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::CountNumEmbedded(void)
{
	return 1 + mapEmbedded.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CObject::GetEmbeddedObject(size iIndex)
{
	if (iIndex == 0)
	{
		return this;
	}
	if (iIndex <= mapEmbedded.NumElements())
	{
		return *mapEmbedded.Get(iIndex - 1);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::GetFieldPointerToIndex(CPointer* pcFieldPointer)
{
	size			uiIndex;
	size			i;
	size			uiNumContained;
	CBaseObject*	pcContained;
	CObject*		pcObject;
	size			uiNumPointers;
	size			j;

	//This object's pointers, then those of each object embedded in it, in GetEmbeddedObject's order.
	uiIndex = 0;
	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		pcContained = GetContained(i);
		if (pcContained->IsObject())
		{
			pcObject = (CObject*)pcContained;
			uiNumPointers = pcObject->mapPointers.NumElements();
			for (j = 0; j < uiNumPointers; j++)
			{
				if (*pcObject->mapPointers.Get(j) == pcFieldPointer)
				{
					return uiIndex;
				}
				uiIndex++;
			}
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::GetNumFieldPointerTos(void)
{
	size			i;
	size			uiCount;
	size			uiNumContained;
	CBaseObject*	pcContained;

	uiCount = 0;
	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		pcContained = GetContained(i);
		if (pcContained->IsObject())
		{
			uiCount += ((CObject*)pcContained)->mapPointers.NumElements();
		}
	}
	return uiCount;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CObject::GetFieldPointerTo(size iIndex)
{
	size			i;
	size			uiNumContained;
	CBaseObject*	pcContained;
	CObject*		pcObject;
	size			uiNumPointers;

	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		pcContained = GetContained(i);
		if (pcContained->IsObject())
		{
			pcObject = (CObject*)pcContained;
			uiNumPointers = pcObject->mapPointers.NumElements();
			if (iIndex < uiNumPointers)
			{
				return *pcObject->mapPointers.Get(iIndex);
			}
			iIndex -= uiNumPointers;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObject::Dehollow(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::ValidateEmbeddedConsistency(void)
{
	size	i;
	size	uiNumContained;

	uiNumContained = CountNumEmbedded();
	for (i = 0; i < uiNumContained; i++)
	{
		GetContained(i)->CBaseObject::ValidateEmbeddedConsistency();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::ValidateConsistency(void)
{
	if (IsEmbedded())
	{
		FailNotExpectedToBeEmbedded(__METHOD__);
		return;
	}

	ValidateObjectIdentifiers();
	ValidateEmbeddedConsistency();
	ValidateCanFindRoot();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::ClassNotImplemented(void)
{
	gcLogger.Error2(__METHOD__, " Class [", ClassName(), "] not yet implemented.", NULL);
}
