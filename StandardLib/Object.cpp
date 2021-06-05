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
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	int					i;

	CBaseObject::Allocate(pcObjects);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->Allocate(NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Class(void)
{
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
	int						iNumFields;
	CPointerField**			ppacPointerFields;
	CArrayVoidPtr*			papv;
	int						i;
	CPointer*				pcPointer;

	papv = mpcClass->GetPointerFields();
	ppacPointerFields = (CPointerField**)papv->GetData();
	iNumFields = papv->NumElements();
	for (i = 0; i < iNumFields; i++)
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
	int						iNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	int						i;
	CBaseObject*			pcEmbeddedObject;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	iNumFields = papv->NumElements();
	for (i = 0; i < iNumFields; i++)
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
void CObject::InternalFree(void)
{
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	int					i;

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->InternalFree();
	}

	CBaseObject::InternalFree();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::BaseNumPointerTos(void)
{
	int					i;
	CEmbeddedObject*	pcPointedTo;
	CPointer**			ppPointer;
	int					iCount;

	iCount = 0;
	for (i = 0; i < mapPointers.NumElements(); i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		if (pcPointedTo)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::NumPointerTos(void)
{
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	int					iCount;

	iCount = BaseNumPointerTos();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		iCount += pcEmbedded->NumPointerTos();
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::IsObject(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::IsCollection(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveAllHeapFroms(void)
{
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;

	CEmbeddedObject::RemoveAllHeapFroms();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;

	CEmbeddedObject::RemoveAllStackFroms();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	SetPointedTosDistToRoot(iDistToRoot);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->SetPointerTosExpectedDistToRoot(iDistToRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::SetDistToRoot(int iDistToRoot)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;
	BOOL			bAnyChange;

	bAnyChange = CBaseObject::SetDistToRoot(iDistToRoot);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::SetDistToStack(iDistToStack);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int				i;
	CBaseObject*	pcPointedTo;
	CPointer**		ppPointer;
	int				iNumPointers;
	CBaseObject*	pcContainer;

	if (iDistToRoot >= ROOT_DIST_TO_ROOT)
	{
		iNumPointers = mapPointers.NumElements();

		for (i = 0; i < iNumPointers; i++)
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
		iNumPointers = mapPointers.NumElements();

		for (i = 0; i < iNumPointers; i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	iDistToRoot = CBaseObject::CalculateDistToRootFromPointedFroms(iDistToRoot);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	CEmbeddedObject*	pcNearesetPointedFrom;
	CEmbeddedObject*	pcEmbeddedNearesetPointedFrom;

	pcNearesetPointedFrom = CEmbeddedObject::GetClosestFromToRoot();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	CEmbeddedObject*	pcNearesetPointedFrom;
	CEmbeddedObject*	pcEmbeddedNearesetPointedFrom;

	pcNearesetPointedFrom = CEmbeddedObject::GetClosestFromForCanFindRoot();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	CEmbeddedObject*		pcNearesetPointedFrom;
	CEmbeddedObject*	pcEmbeddedNearesetPointedFrom;

	pcNearesetPointedFrom = CEmbeddedObject::GetClosestFromToStack();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	BaseGetPointerTos(papcTos);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->GetPointerTos(papcTos);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
{
	int					iNumPointers;
	int					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		if (pcPointedTo)
		{
			if (pcPointedTo == pcEmbedded)
			{
				return TRUE;
			}
		}
	}

	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::BaseCollectAndClearPointerTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	int					iNumPointers;
	int					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;
	CBaseObject*		pcContainer;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
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
	int					i;
	int					iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;

	BaseCollectAndClearPointerTosInvalidDistToRootObjects(pcParameters);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->CollectAndClearPointerTosInvalidDistToRootObjects(pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::CollectDetachedFroms(CDistCalculatorParameters* pcParameters)
{
	int					iNumEmbedded;
	int					i;
	CEmbeddedObject*	pcEmbedded;
	int					iNumWithStackPointers;

	iNumWithStackPointers = CBaseObject::CollectDetachedFroms(pcParameters);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int					iNumPointers;
	int					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
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
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	int					iNumPointers;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		RemoveToFrom(pcPointedTo);
		(*ppPointer)->UnsafeClearObject();
	}

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;
	int				iNumPointers;
	CPointer**		ppPointer;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		(*ppPointer)->PointTo(NULL, TRUE);
	}

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int					i;
	int					iNumPointers;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
		AddExpectedDistToRoot(pcPointedTo, iExpectedDist+1, pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;

	BaseUpdateAttachedEmbeddedObjectPointerTosDistToRoot(pcParameters, iExpectedDist);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int					iNumPointers;
	int					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
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
void CObject::Primitive(Int8* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt8* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Int16* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt16* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Int32* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt32* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Int64* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(UInt64* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Bool* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Float32* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Float64* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Char8* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CObject::Primitive(Char16* pcPrimitive, char* szFieldName) { mpcClass->Primitive(this, pcPrimitive, szFieldName); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::UnmanagedInt(int8* pc, char* szFieldName) { mpcClass->UnmanagedInt(this, pc, szFieldName); };
void CObject::UnmanagedInt(int8* pac, size_t uiLength, char* szFieldName) { mpcClass->UnmanagedInt(this, pac, uiLength, szFieldName); };
void CObject::UnmanagedInt(int16* pi, char* szFieldName) { mpcClass->UnmanagedInt(this, pi, szFieldName); };
void CObject::UnmanagedInt(int16* pai, size_t uiLength, char* szFieldName) { mpcClass->UnmanagedInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedInt(int32* pi, char* szFieldName) { mpcClass->UnmanagedInt(this, pi, szFieldName); };
void CObject::UnmanagedInt(int32* pai, size_t uiLength, char* szFieldName) { mpcClass->UnmanagedInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedInt(int64* pi, char* szFieldName) { mpcClass->UnmanagedInt(this, pi, szFieldName); };
void CObject::UnmanagedInt(int64* pai, size_t uiLength, char* szFieldName){ mpcClass->UnmanagedInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedInt(uint8* pc, char* szFieldName){ mpcClass->UnmanagedInt(this, pc, szFieldName); };
void CObject::UnmanagedInt(uint8* pac, size_t uiLength, char* szFieldName){ mpcClass->UnmanagedInt(this, pac, uiLength, szFieldName); };
void CObject::UnmanagedInt(uint16* pi, char* szFieldName){ mpcClass->UnmanagedInt(this, pi, szFieldName); };
void CObject::UnmanagedInt(uint16* pai, size_t uiLength, char* szFieldName){ mpcClass->UnmanagedInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedInt(uint32* pi, char* szFieldName){ mpcClass->UnmanagedInt(this, pi, szFieldName); };
void CObject::UnmanagedInt(uint32* pai, size_t uiLength, char* szFieldName){ mpcClass->UnmanagedInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedInt(uint64* pi, char* szFieldName){ mpcClass->UnmanagedInt(this, pi, szFieldName); };
void CObject::UnmanagedInt(uint64* pai, size_t uiLength, char* szFieldName){ mpcClass->UnmanagedInt(this, pai, uiLength, szFieldName); };
void CObject::UnmanagedFloat(float32* pf, char* szFieldName) { mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CObject::UnmanagedFloat(float32* paf, size_t uiLength, char* szFieldName) { mpcClass->UnmanagedFloat(this, paf, uiLength,szFieldName); }
void CObject::UnmanagedFloat(float64* pf, char* szFieldName) { mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CObject::UnmanagedFloat(float64* paf, size_t uiLength, char* szFieldName) { mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CObject::UnmanagedChar(char8* pc, char* szFieldName) { mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CObject::UnmanagedChar(char8* pac, size_t uiLength, char* szFieldName) { mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
void CObject::UnmanagedChar(char16* pc, char* szFieldName) { mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CObject::UnmanagedChar(char16* pac, size_t uiLength, char* szFieldName) { mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
	 
void CObject::UnmanagedString(CChars* pcChars, char* szFieldName) { mpcClass->UnmanagedString(this, pcChars, szFieldName); };
void CObject::UnmanagedNumber(CNumber* pcNumber, char* szFieldName) { mpcClass->UnmanagedNumber(this, pcNumber, szFieldName); };
void CObject::UnmanagedDate(CDate* pcDate, char* szFieldName) { mpcClass->UnmanagedDate(this, pcDate, szFieldName); };
	 
void CObject::UnmanagedData(void* pv, size_t uiSizeof, char* szFieldName) { mpcClass->UnmanagedData(this, pv, uiSizeof, szFieldName); };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	int					iNumPointers;
	int					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;
	int					iCount;

	iCount = 0;
	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (**ppPointer).Object();
		if (pcPointedTo == pcOld)
		{
			(**ppPointer).UnsafePointTo(pcNew);
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::GetHeapFroms(CArrayTemplateEmbeddedBaseObjectPtr* papcFroms)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::GetHeapFroms(papcFroms);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->GetHeapFroms(papcFroms);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::IsDirty(void)
{
	int				i;
	CBaseObject*	pcEmbedded;

	if (miFlags & OBJECT_FLAGS_DIRTY)
	{
		return TRUE;
	}
	else
	{
		for (i = 0; i < mapEmbedded.NumElements(); i++)	
		{
			pcEmbedded = *mapEmbedded.Get(i);
			if (pcEmbedded->IsDirty())
			{
				return TRUE;
			}
		}
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
{
	int		iIndex;

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
BOOL CObject::RecurseGetEmbeddedIndex(CEmbeddedObject* pcTest, int* piIndex)
{
	int				i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;

	if (pcTest == this)
	{
		return TRUE;
	}
	else
	{
		for (i = 0; i < mapEmbedded.NumElements(); i++)	
		{
			pcEmbedded = *mapEmbedded.Get(i);
			(*piIndex)++;

			if (pcEmbedded->IsObject())
			{
				pcObject = (CObject*)pcEmbedded;
				if (pcObject->RecurseGetEmbeddedIndex(pcTest, piIndex))
				{
					return TRUE;
				}
			}
			else
			{
				if (pcEmbedded == pcTest)
				{
					return TRUE;
				}
			}
		}
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CObject::GetNumEmbedded(void)
{
	int				i;
	CBaseObject*	pcEmbedded;
	int				iCount;
	int				iNumEmbedded;

	iNumEmbedded = GetNumEmbeddedFromFlags();
	if (iNumEmbedded != 0)
	{
		return iNumEmbedded;
	}

	iCount = 1;
	for (i = 0; i < mapEmbedded.NumElements(); i++)	
	{
		pcEmbedded = *mapEmbedded.Get(i);

		iCount += pcEmbedded->GetNumEmbedded();
	}

	iNumEmbedded = iCount;
	SetFlagNumEmbedded(iNumEmbedded);
	return iNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CObject::GetEmbeddedObject(uint16 iIndex)
{
	int					iCount;
	CEmbeddedObject*	pcReturned;

	iCount = 0;
	pcReturned = RecurseGetEmbeddedObject(iIndex, &iCount);
	return pcReturned;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CObject::RecurseGetEmbeddedObject(int iIndex, int* piCount)
{
	int					i;
	CBaseObject*		pcEmbedded;
	CObject*			pcObject;
	CEmbeddedObject*	pcReturned;

	if (iIndex == *piCount)
	{
		return this;
	}
	else
	{
		for (i = 0; i < mapEmbedded.NumElements(); i++)	
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
int CObject::GetFieldPointerToIndex(CPointer* pcFieldPointer)
{
	int		iIndex;

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
BOOL CObject::RecurseGetFieldPointerToIndex(CPointer* pcTest, int* piIndex)
{
	int				i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;
	CPointer*		pcFieldPointer;

	for (i = 0; i < mapPointers.NumElements(); i++)	
	{
		pcFieldPointer = *mapPointers.Get(i);

		if (pcFieldPointer == pcTest)
		{
			return TRUE;
		}
		(*piIndex)++;
	}

	for (i = 0; i < mapEmbedded.NumElements(); i++)	
	{
		pcEmbedded = *mapEmbedded.Get(i);

		if (pcEmbedded->IsObject())
		{
			pcObject = (CObject*)pcEmbedded;
			if (pcObject->RecurseGetFieldPointerToIndex(pcTest, piIndex))
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::GetNumFieldPointerTos(void)
{
	int				i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;
	int				iCount;

	iCount = mapPointers.NumElements();

	for (i = 0; i < mapEmbedded.NumElements(); i++)	
	{
		pcEmbedded = *mapEmbedded.Get(i);
		if (pcEmbedded->IsObject())
		{
			pcObject = (CObject*)pcEmbedded;
			iCount += pcObject->GetNumFieldPointerTos();
		}
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CObject::GetFieldPointerTo(int iIndex)
{
	int			iCount;
	CPointer*	pcReturned;

	iCount = 0;
	pcReturned = RecurseGetFieldPointerTo(iIndex, &iCount);
	return pcReturned;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CObject::RecurseGetFieldPointerTo(int iIndex, int* piCount)
{
	int				i;
	CBaseObject*	pcEmbedded;
	CObject*		pcObject;
	CPointer*		pcReturned;
	CPointer*		pcFieldPointer;

	for (i = 0; i < mapPointers.NumElements(); i++)	
	{
		pcFieldPointer = *mapPointers.Get(i);
		if (*piCount == iIndex)
		{
			return pcFieldPointer;
		}
		(*piCount)++;
	}

	for (i = 0; i < mapEmbedded.NumElements(); i++)	
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
int CObject::NumHeapFroms(void)
{
	CBaseObject*	pcBaseObject;
	int				i;
	int				iCount;

	iCount = CEmbeddedObject::NumHeapFroms();

	for (i = 0; i < mapEmbedded.NumElements(); i++)
	{
		pcBaseObject = *mapEmbedded.Get(i);
		iCount += pcBaseObject->NumHeapFroms();
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::NumStackFroms(void)
{
	CBaseObject*	pcBaseObject;
	int				i;
	int				iCount;

	iCount = CEmbeddedObject::NumStackFroms();

	for (i = 0; i < mapEmbedded.NumElements(); i++)
	{
		pcBaseObject = *mapEmbedded.Get(i);
		iCount += pcBaseObject->NumStackFroms();
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::SetFlag(int iFlag, int iFlagValue)
{
	int				i;
	CBaseObject*	pcBaseObject;

	::SetFlag(&miFlags, iFlag, iFlagValue);

	for (i = 0; i < mapEmbedded.NumElements(); i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::GetStackFroms(papcFroms);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);

		pcEmbedded->GetStackFroms(papcFroms);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::IsDistToRootValid(void)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	if (CBaseObject::IsDistToRootValid())
	{
		return TRUE;
	}

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		if (pcEmbedded->IsDistToRootValid())
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::BaseValidatePointerTos(void)
{
	int					i;
	CEmbeddedObject*	pcPointedTo;
	CPointer**			ppPointer;

	for (i = 0; i < mapPointers.NumElements(); i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	BaseValidatePointerTos();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::ValidateEmbeddedConsistency();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
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

