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
void CObject::Allocate(CObjects* pcObjects)
{
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;
	size			i;

	CBaseObject::Allocate(pcObjects);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->Allocate(NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Kill(void)
{
	CBaseObject::Kill();
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
	size					uiNumFields;
	CPointerField**			ppacPointerFields;
	CArrayVoidPtr*			papv;
	size					i;
	CPointer*				pcPointer;

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
	CBaseObject*			pcEmbeddedObject;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		pcEmbeddedObject->SetEmbedding(this);
		pcEmbeddedObject->PreClass();
		mapEmbedded.AddPtr(pcEmbeddedObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::FreePointers(void)
{
	CBaseObject::FreePointers();
	mapEmbedded.Kill();
	mapPointers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::FreeInternal(bool bAllocatedInObjects)
{
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;
	size			i;

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->FreeInternal(bAllocatedInObjects);
	}

	CBaseObject::FreeInternal(bAllocatedInObjects);
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
	size				i;
	size				uiNumEmbedded;
	CBaseObject*		pcEmbedded;
	size				uiCount;

	uiCount = BaseNumPointerTos();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		uiCount += pcEmbedded->NumPointerTos();
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
	size				i;
	size				uiNumEmbedded;
	CBaseObject*		pcEmbedded;

	CEmbeddedObject::RemoveAllHeapFroms();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->RemoveAllHeapFroms();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveAllStackFroms(void)
{
	size				i;
	size				uiNumEmbedded;
	CBaseObject*		pcEmbedded;

	CEmbeddedObject::RemoveAllStackFroms();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->RemoveAllStackFroms();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::SetPointerTosExpectedDistToRoot(int iDistToRoot)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	SetPointedTosDistToRoot(iDistToRoot);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->SetPointerTosExpectedDistToRoot(iDistToRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::SetDistToRoot(int iDistToRoot)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;
	bool			bAnyChange;

	bAnyChange = CBaseObject::SetDistToRoot(iDistToRoot);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		bAnyChange |= pcEmbedded->SetDistToRoot(iDistToRoot);
	}

	return bAnyChange;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::SetDistToStack(int iDistToStack)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::SetDistToStack(iDistToStack);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->SetDistToStack(iDistToStack);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::SetPointedTosDistToRoot(int iDistToRoot)
{
	size			i;
	CBaseObject*	pcPointedTo;
	CPointer**		ppPointer;
	size			uiNumPointers;
	CBaseObject*	pcContainer;

	if (iDistToRoot >= ROOT_DIST_TO_ROOT)
	{
		uiNumPointers = mapPointers.NumElements();
		for (i = 0; i < uiNumPointers; i++)
		{
			ppPointer = mapPointers.Get(i);
			pcPointedTo = (**ppPointer).BaseObject();
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetExpectedDistToRoot(iDistToRoot + 1);
			}
		}
	}
	else if (iDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		uiNumPointers = mapPointers.NumElements();
		for (i = 0; i < uiNumPointers; i++)
		{
			ppPointer = mapPointers.Get(i);
			pcPointedTo = (**ppPointer).BaseObject();
			if (pcPointedTo)
			{
				pcContainer = pcPointedTo->GetEmbeddingContainer();
				pcContainer->SetCalculatedDistToRoot();
			}
		}
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::CalculateDistToRootFromPointedFroms(int iDistToRoot)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	iDistToRoot = CBaseObject::CalculateDistToRootFromPointedFroms(iDistToRoot);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		iDistToRoot = pcEmbedded->CalculateDistToRootFromPointedFroms(iDistToRoot);
	}

	return iDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObject::GetClosestFromToRoot(void)
{
	size				i;
	size				uiNumEmbedded;
	CBaseObject*		pcEmbedded;
	CEmbeddedObject*	pcNearesetPointedFrom;
	CEmbeddedObject*	pcEmbeddedNearesetPointedFrom;

	pcNearesetPointedFrom = CEmbeddedObject::GetClosestFromToRoot();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbeddedNearesetPointedFrom = pcEmbedded->GetClosestFromToRoot();

		if (pcNearesetPointedFrom == NULL)
		{
			pcNearesetPointedFrom = pcEmbeddedNearesetPointedFrom;
		}
		else
		{
			if (pcEmbeddedNearesetPointedFrom != NULL)
			{
				if (pcEmbeddedNearesetPointedFrom->GetDistToRoot() < pcNearesetPointedFrom->GetDistToRoot())
				{
					pcNearesetPointedFrom = pcEmbeddedNearesetPointedFrom;
				}
			}
		}
	}

	return (CBaseObject*)pcNearesetPointedFrom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObject::GetClosestFromForCanFindRoot(void)
{
	size				i;
	size				uiNumEmbedded;
	CBaseObject*		pcEmbedded;
	CEmbeddedObject*	pcNearesetPointedFrom;
	CEmbeddedObject*	pcEmbeddedNearesetPointedFrom;

	pcNearesetPointedFrom = CEmbeddedObject::GetClosestFromForCanFindRoot();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbeddedNearesetPointedFrom = pcEmbedded->GetClosestFromForCanFindRoot();

		if (pcNearesetPointedFrom == NULL)
		{
			pcNearesetPointedFrom = pcEmbeddedNearesetPointedFrom;
		}
		else
		{
			if (pcEmbeddedNearesetPointedFrom != NULL)
			{
				if (pcEmbeddedNearesetPointedFrom->GetDistToRoot() < pcNearesetPointedFrom->GetDistToRoot())
				{
					pcNearesetPointedFrom = pcEmbeddedNearesetPointedFrom;
				}
			}
		}
	}

	return (CBaseObject*)pcNearesetPointedFrom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CObject::GetClosestFromToStack(void)
{
	size				i;
	size				uiNumEmbedded;
	CBaseObject*		pcEmbedded;
	CEmbeddedObject*	pcNearesetPointedFrom;
	CEmbeddedObject*	pcEmbeddedNearesetPointedFrom;

	pcNearesetPointedFrom = CEmbeddedObject::GetClosestFromToStack();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbeddedNearesetPointedFrom = pcEmbedded->GetClosestFromToStack();

		if (pcNearesetPointedFrom == NULL)
		{
			pcNearesetPointedFrom = pcEmbeddedNearesetPointedFrom;
		}
		else
		{
			if (pcEmbeddedNearesetPointedFrom != NULL)
			{
				if (pcEmbeddedNearesetPointedFrom->GetDistToRoot() < pcNearesetPointedFrom->GetDistToRoot())
				{
					pcNearesetPointedFrom = pcEmbeddedNearesetPointedFrom;
				}
			}
		}
	}

	return (CBaseObject*)pcNearesetPointedFrom;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	BaseGetPointerTos(papcTos);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->GetPointerTos(papcTos);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
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
		if (pcPointedTo)
		{
			if (pcPointedTo == pcEmbedded)
			{
				return true;
			}
		}
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::BaseCollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	size				uiNumPointers;
	size				i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;
	CBaseObject*		pcContainer;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		if (pcPointedTo)
		{
			pcContainer = pcPointedTo->GetEmbeddingContainer();
			pcContainer->CollectAndClearInvalidDistToRootObjects(pcParameters);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::CollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	size				i;
	size				uiNumEmbedded;
	CEmbeddedObject*	pcEmbedded;

	BaseCollectAndClearPointerTosInvalidDistToRootObjects(pcParameters);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->CollectAndClearPointerTosInvalidDistToRootObjects(pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::CollectDetachedFroms(CDistCalculatorParameters* pcParameters)
{
	size				uiNumEmbedded;
	size				i;
	CEmbeddedObject*	pcEmbedded;
	size				iNumWithStackPointers;

	iNumWithStackPointers = CBaseObject::CollectDetachedFroms(pcParameters);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		iNumWithStackPointers += pcEmbedded->CollectDetachedFroms(pcParameters);
	}

	return iNumWithStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::BaseGetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
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
			papcTos->Add(&pcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveAllPointerTosDontKill(void)
{
	size				i;
	size				uiNumEmbedded;
	CBaseObject*		pcEmbedded;
	size				uiNumPointers;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		RemoveToFrom(pcPointedTo);
		(*ppPointer)->UnsafeClearObject();
	}

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->RemoveAllPointerTosDontKill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveAllPointerTos(void)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;
	size			uiNumPointers;
	CPointer**		ppPointer;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		(*ppPointer)->PointTo(NULL, true);
	}

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->RemoveAllPointerTos();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::BaseUpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
	size				i;
	size				uiNumPointers;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		AddExpectedDistToRoot(pcPointedTo, iExpectedDist + 1, pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	BaseUpdateAttachedEmbeddedObjectPointerTosDistToRoot(pcParameters, iExpectedDist);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->UpdateAttachedEmbeddedObjectPointerTosDistToRoot(pcParameters, iExpectedDist);
	}
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
void CObject::Pointer(CPointer* pcPointer, char* szFieldName)
{
	mpcClass->Pointer(this, pcPointer, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Embedded(CBaseObject* pcObject, char* szFieldName)
{
	mpcClass->Embedded(this, pcObject, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Primitive(Int8* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt8* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Int16* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt16* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Int32* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt32* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Int64* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt64* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Bool* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Float32* pcPrimitive, char* szFieldName)	{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Float64* pcPrimitive, char* szFieldName)	{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Char8* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Char16* pcPrimitive, char* szFieldName)		{ mpcClass->Primitive(this, pcPrimitive, szFieldName); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::UnmanagedInt8(int8* pc, char* szFieldName)						{ mpcClass->UnmanagedInt8(this, pc, szFieldName); };
void CObject::UnmanagedInt8(int8* pac, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedInt8(this, pac, uiLength, szFieldName); };
void CObject::UnmanagedInt16(int16* pi, char* szFieldName)						{ mpcClass->UnmanagedInt16(this, pi, szFieldName); };
void CObject::UnmanagedInt16(int16* pai, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedInt16(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedInt32(int32* pi, char* szFieldName)						{ mpcClass->UnmanagedInt32(this, pi, szFieldName); };
void CObject::UnmanagedInt32(int32* pai, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedInt32(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedInt64(int64* pi, char* szFieldName)						{ mpcClass->UnmanagedInt64(this, pi, szFieldName); };
void CObject::UnmanagedInt64(int64* pai, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedInt64(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedSInt(int* pi, char* szFieldName)							{ mpcClass->UnmanagedSInt(this, pi, szFieldName); };
void CObject::UnmanagedSInt(int* pai, size uiLength, char* szFieldName)			{ mpcClass->UnmanagedSInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedUInt8(uint8* pc, char* szFieldName)						{ mpcClass->UnmanagedUInt8(this, pc, szFieldName); };
void CObject::UnmanagedUInt8(uint8* pac, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedUInt8(this, pac, uiLength, szFieldName); };
void CObject::UnmanagedUInt16(uint16* pi, char* szFieldName)					{ mpcClass->UnmanagedUInt16(this, pi, szFieldName); };
void CObject::UnmanagedUInt16(uint16* pai, size uiLength, char* szFieldName)	{ mpcClass->UnmanagedUInt16(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedUInt32(uint32* pi, char* szFieldName)					{ mpcClass->UnmanagedUInt32(this, pi, szFieldName); };
void CObject::UnmanagedUInt32(uint32* pai, size uiLength, char* szFieldName)	{ mpcClass->UnmanagedUInt32(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedUInt64(uint64* pi, char* szFieldName)					{ mpcClass->UnmanagedUInt64(this, pi, szFieldName); };
void CObject::UnmanagedUInt64(uint64* pai, size uiLength, char* szFieldName)	{ mpcClass->UnmanagedUInt64(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedSUInt(uint* pi, char* szFieldName)						{ mpcClass->UnmanagedSUInt(this, pi, szFieldName); };
void CObject::UnmanagedSUInt(uint* pai, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedSUInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedSize(size* pi, char* szFieldName)						{ mpcClass->UnmanagedSize(this, pi, szFieldName); };
void CObject::UnmanagedSize(size* pai, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedSize(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedFloat(float32* pf, char* szFieldName)					{ mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CObject::UnmanagedFloat(float32* paf, size uiLength, char* szFieldName)	{ mpcClass->UnmanagedFloat(this, paf, uiLength,szFieldName); }
void CObject::UnmanagedFloat(float64* pf, char* szFieldName)					{ mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CObject::UnmanagedFloat(float64* paf, size uiLength, char* szFieldName)	{ mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CObject::UnmanagedChar(char8* pc, char* szFieldName)						{ mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CObject::UnmanagedChar(char8* pac, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
void CObject::UnmanagedChar(char16* pc, char* szFieldName)						{ mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CObject::UnmanagedChar(char16* pac, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
void CObject::UnmanagedBool(bool* pb, char* szFieldName)						{ mpcClass->UnmanagedBool(this, pb, szFieldName); }
void CObject::UnmanagedBool(bool* pab, size uiLength, char* szFieldName)		{ mpcClass->UnmanagedBool(this, pab, uiLength, szFieldName); }
	 
void CObject::UnmanagedEnum(void* pe, char* szFieldName) { mpcClass->UnmanagedEnum(this, pe, szFieldName); }

void CObject::UnmanagedString(CChars* pcChars, char* szFieldName) { mpcClass->UnmanagedString(this, pcChars, szFieldName); };
void CObject::UnmanagedNumber(CNumber* pcNumber, char* szFieldName) { mpcClass->UnmanagedNumber(this, pcNumber, szFieldName); };
void CObject::UnmanagedDate(CDate* pcDate, char* szFieldName) { mpcClass->UnmanagedDate(this, pcDate, szFieldName); };
	 
void CObject::UnmanagedData(void* pv, size uiSizeof, char* szFieldName) { mpcClass->UnmanagedData(this, pv, uiSizeof, szFieldName); };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	size					uiNumPointers;
	size					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;
	size					uiCount;

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
void  CObject::GetHeapFroms(CArrayTemplateEmbeddedBaseObjectPtr* papcFroms)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::GetHeapFroms(papcFroms);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->GetHeapFroms(papcFroms);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::IsDirty(void)
{
	size				i;
	CBaseObject*	pcEmbedded;
	size			uiNumEmbedded;

	if (muiFlags & OBJECT_FLAGS_DIRTY)
	{
		return true;
	}
	else
	{
		uiNumEmbedded = mapEmbedded.NumElements();
		for (i = 0; i < uiNumEmbedded; i++)
		{
			pcEmbedded = *mapEmbedded.Get(i);
			if (pcEmbedded->IsDirty())
			{
				return true;
			}
		}
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
{
	size		iIndex;

	if (pcEmbedded == this)
	{
		return 0;
	}
	else
	{
		iIndex = 0;
		if (RecurseGetEmbeddedIndex(pcEmbedded, &iIndex))
		{
			return iIndex;
		}
		else
		{
			return -1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::RecurseGetEmbeddedIndex(CEmbeddedObject* pcTest, size* piIndex)
{
	size			i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;
	size			uiNumEmbedded;

	if (pcTest == this)
	{
		return true;
	}
	else
	{
		uiNumEmbedded = mapEmbedded.NumElements();
		for (i = 0; i < uiNumEmbedded; i++)	
		{
			pcEmbedded = *mapEmbedded.Get(i);
			(*piIndex)++;

			if (pcEmbedded->IsObject())
			{
				pcObject = (CObject*)pcEmbedded;
				if (pcObject->RecurseGetEmbeddedIndex(pcTest, piIndex))
				{
					return true;
				}
			}
			else
			{
				if (pcEmbedded == pcTest)
				{
					return true;
				}
			}
		}
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::GetNumEmbedded(void)
{
	size			i;
	CBaseObject*	pcEmbedded;
	size			uiCount;
	size			uiNumEmbedded;

	uiNumEmbedded = GetNumEmbeddedFromFlags();
	if (uiNumEmbedded != 0)
	{
		return (uint16)uiNumEmbedded;
	}

	uiCount = 1;
	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)	
	{
		pcEmbedded = *mapEmbedded.Get(i);

		uiCount += pcEmbedded->GetNumEmbedded();
	}

	SetFlagNumEmbedded(uiCount);
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CObject::GetEmbeddedObject(size iIndex)
{
	size				uiCount;
	CEmbeddedObject*	pcReturned;

	uiCount = 0;
	pcReturned = RecurseGetEmbeddedObject(iIndex, &uiCount);
	return pcReturned;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CObject::RecurseGetEmbeddedObject(size iIndex, size* piCount)
{
	size				i;
	CBaseObject*		pcEmbedded;
	CObject*			pcObject;
	CEmbeddedObject*	pcReturned;
	size				uiNumEmbedded;

	if (iIndex == *piCount)
	{
		return this;
	}
	else
	{
		uiNumEmbedded = mapEmbedded.NumElements();
		for (i = 0; i < uiNumEmbedded; i++)	
		{
			pcEmbedded = *mapEmbedded.Get(i);
			(*piCount)++;

			if (pcEmbedded->IsObject())
			{
				pcObject = (CObject*)pcEmbedded;
				pcReturned = pcObject->RecurseGetEmbeddedObject(iIndex, piCount);
				if (pcReturned)
				{
					return pcReturned;
				}
			}
			else
			{
				if (iIndex == *piCount)
				{
					return pcEmbedded;
				}
			}
		}
		return NULL;
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::GetFieldPointerToIndex(CPointer* pcFieldPointer)
{
	size	iIndex;

	iIndex = 0;
	if (RecurseGetFieldPointerToIndex(pcFieldPointer, &iIndex))
	{
		return iIndex;
	}
	else
	{
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::RecurseGetFieldPointerToIndex(CPointer* pcTest, size* piIndex)
{
	size			i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;
	CPointer*		pcFieldPointer;
	size			uiNumPointers;
	size			uiNumEmbedded;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		pcFieldPointer = *mapPointers.Get(i);

		if (pcFieldPointer == pcTest)
		{
			return true;
		}
		(*piIndex)++;
	}

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);

		if (pcEmbedded->IsObject())
		{
			pcObject = (CObject*)pcEmbedded;
			if (pcObject->RecurseGetFieldPointerToIndex(pcTest, piIndex))
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::GetNumFieldPointerTos(void)
{
	size			i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;
	size			uiCount;
	size			uiNumEmbedded;

	uiCount = mapPointers.NumElements();
	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		if (pcEmbedded->IsObject())
		{
			pcObject = (CObject*)pcEmbedded;
			uiCount += pcObject->GetNumFieldPointerTos();
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
	size		uiCount;
	CPointer*	pcReturned;

	uiCount = 0;
	pcReturned = RecurseGetFieldPointerTo(iIndex, &uiCount);
	return pcReturned;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CObject::RecurseGetFieldPointerTo(size iIndex, size* piCount)
{
	size			i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;
	CPointer*		pcReturned;
	CPointer*		pcFieldPointer;
	size			uiNumPointers;
	size			uiNumEmbedded;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		pcFieldPointer = *mapPointers.Get(i);
		if (*piCount == iIndex)
		{
			return pcFieldPointer;
		}
		(*piCount)++;
	}

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);

		if (pcEmbedded->IsObject())
		{
			pcObject = (CObject*)pcEmbedded;
			pcReturned = pcObject->RecurseGetFieldPointerTo(iIndex, piCount);
			if (pcReturned)
			{
				return pcReturned;
			}
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
size CObject::NumHeapFroms(void)
{
	CBaseObject*	pcBaseObject;
	size			i;
	size			uiCount;
	size			uiNumEmbedded;

	uiCount = CEmbeddedObject::NumHeapFroms();
	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcBaseObject = *mapEmbedded.Get(i);
		uiCount += pcBaseObject->NumHeapFroms();
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CObject::NumStackFroms(void)
{
	CBaseObject*	pcBaseObject;
	size			i;
	size			uiCount;
	size			uiNumEmbedded;

	uiCount = CEmbeddedObject::NumStackFroms();
	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcBaseObject = *mapEmbedded.Get(i);
		uiCount += pcBaseObject->NumStackFroms();
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::SetFlag(uint16 iFlag, uint16 iFlagValue)
{
	size			i;
	CBaseObject*	pcBaseObject;
	size			uiNumEmbedded;

	::SetFlagShort(&muiFlags, iFlag, iFlagValue);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcBaseObject = *mapEmbedded.Get(i);
		pcBaseObject->SetFlag(iFlag, iFlagValue);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::GetStackFroms(CArrayTypedPointerPtr* papcFroms)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::GetStackFroms(papcFroms);

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);

		pcEmbedded->GetStackFroms(papcFroms);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CObject::IsDistToRootValid(void)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	if (CBaseObject::IsDistToRootValid())
	{
		return true;
	}

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		if (pcEmbedded->IsDistToRootValid())
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
void CObject::BaseValidatePointerTos(void)
{
	size				i;
	CEmbeddedObject*	pcPointedTo;
	CPointer**			ppPointer;
	size				uiNumPointers;

	uiNumPointers = mapPointers.NumElements();
	for (i = 0; i < uiNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		if (pcPointedTo)
		{
			ValidatePointerTo(pcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::ValidatePointerTos(void)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	BaseValidatePointerTos();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->BaseValidatePointerTos();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::ValidateEmbeddedConsistency(void)
{
	size			i;
	size			uiNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::ValidateEmbeddedConsistency();

	uiNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < uiNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->ValidateEmbeddedConsistency();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::ValidateConsistency(void)
{
	ValidateNotEmbedded(__METHOD__);

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

