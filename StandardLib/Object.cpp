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
void CObject::Kill(void)
{
	CBaseObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::KillInternalData(void)
{
	CBaseObject::KillInternalData();
	mapEmbedded.Kill();
	mapPointers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::KillDontFree(void)
{
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	int					i;

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->KillDontFree();
	}

	CBaseObject::KillDontFree();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::UnsafeNumEmbeddedObjectTos(void)
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
int CObject::NumTos(void)
{
	int					i;
	int					iNumEmbedded;
	CBaseObject*		pcEmbedded;
	int					iCount;

	iCount = UnsafeNumEmbeddedObjectTos();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		iCount += pcEmbedded->UnsafeNumEmbeddedObjectTos();
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
void CObject::SetPointedTosExpectedDistToRoot(int iDistToRoot)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	SetPointedTosDistToRoot(iDistToRoot);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->SetPointedTosExpectedDistToRoot(iDistToRoot);
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
		gcLogger.Error2(__METHOD__, "Don't know how to set dist to root to [", IntToString(iDistToRoot), "].", NULL);
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
void CObject::GetTos(CArrayEmbeddedObjectPtr* papcTos)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	UnsafeGetEmbeddedObjectTos(papcTos);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->GetTos(papcTos);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::ContainsTo(CEmbeddedObject* pcEmbedded)
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
void CObject::CollectAndClearTosInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	int					iNumPointers;
	int					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;
	CBaseObject*		pcContainer;
	int					iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;

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

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->CollectAndClearTosInvalidDistToRootObjects(pcParameters);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::CollectEmbeddedObjectDetachedFroms(CDistCalculatorParameters* pcParameters)
{
	int					iNumEmbedded;
	int					i;
	CEmbeddedObject*	pcEmbedded;
	int					iNumWithStackPointers;

	iNumWithStackPointers = CBaseObject::CollectEmbeddedObjectDetachedFroms(pcParameters);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		iNumWithStackPointers += pcEmbedded->CollectEmbeddedObjectDetachedFroms(pcParameters);
	}

	return iNumWithStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::UnsafeGetEmbeddedObjectTos(CArrayEmbeddedObjectPtr* papcTos)
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
void CObject::UnsafeRemoveEmbeddedObjectAllTos(void)
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
		RemoveToFrom(pcPointedTo);
		(*ppPointer)->UnsafeClearObject();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::UnsafeRemoveAllTos(void)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	UnsafeRemoveEmbeddedObjectAllTos();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->UnsafeRemoveAllTos();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::UpdateAttachedEmbeddedObjectTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
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
		AddExpectedDistToRoot(pcPointedTo, iExpectedDist+1, pcParameters);
	}

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->UpdateAttachedEmbeddedObjectTosDistToRoot(pcParameters, iExpectedDist);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveTo(CEmbeddedObject* pcTo)
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
CPointer* CObject::Pointer(CPointer* pcPointer)
{
	pcPointer->SetEmbedding(this);
	mapPointers.Add(&pcPointer);
	return pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Embedded(CBaseObject* pcObject)
{
	pcObject->Class();
	pcObject->mpcEmbedded = this;
	mapEmbedded.Add(&pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
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
void CObject::GetHeapFroms(CArrayEmbeddedBaseObjectPtr* papcFroms)
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
int CObject::GetNumEmbedded(void)
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
CEmbeddedObject* CObject::GetEmbeddedObject(int iIndex)
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
int CObject::GetFieldPointerIndex(CPointer* pcFieldPointer)
{
	int		iIndex;

	iIndex = 0;
	if (RecurseGetFieldPointerIndex(pcFieldPointer, &iIndex))
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
BOOL CObject::RecurseGetFieldPointerIndex(CPointer* pcTest, int* piIndex)
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
			if (pcObject->RecurseGetFieldPointerIndex(pcTest, piIndex))
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
int CObject::GetNumFieldPointers(void)
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
			iCount += pcObject->GetNumFieldPointers();
		}
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CObject::GetFieldPointer(int iIndex)
{
	int			iCount;
	CPointer*	pcReturned;

	iCount = 0;
	pcReturned = RecurseGetFieldPointer(iIndex, &iCount);
	return pcReturned;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CObject::RecurseGetFieldPointer(int iIndex, int* piCount)
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
			pcReturned = pcObject->RecurseGetFieldPointer(iIndex, piCount);
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
void CObject::GetStackFroms(CArrayPointerPtr* papcFroms)
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
void CObject::ValidateEmbeddedObjectTos(void)
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
			ValidateTo(pcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::ValidateTos(void)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	ValidateEmbeddedObjectTos();

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->ValidateEmbeddedObjectTos();
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

