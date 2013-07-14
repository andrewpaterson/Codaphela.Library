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
void CObject::Kill(void)
{
	CBaseObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::KillToPointers(void)
{
	mapEmbedded.Kill();
	mapPointers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Free(void)
{
	CBaseObject::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::NumTos(void)
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
void CObject::CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	int					i;
	CBaseObject*		pcPointedTo;
	CPointer**			ppPointer;

	MarkForKilling(papcKilled);

	for (i = 0; i < mapPointers.NumElements(); i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->BaseObject();
		if (pcPointedTo)
		{
			if (pcPointedTo->miDistToRoot != UNATTACHED_DIST_TO_ROOT)
			{
				if (!pcPointedTo->CanFindRoot())
				{
					pcPointedTo->CollectThoseToBeKilled(papcKilled);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::SetDistToRoot(int iDistToRoot)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	if (miDistToRoot != iDistToRoot)
	{
		PrivateSetDistToRoot(iDistToRoot);
	}

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);
		pcEmbedded->SetDistToRoot(iDistToRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::PrivateSetDistToRoot(int iDistToRoot)
{
	int				i;
	CBaseObject*	pcPointedTo;
	CPointer**		ppPointer;
	int				iNumPointers;

	miDistToRoot = iDistToRoot;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (**ppPointer).BaseObject();
		if (pcPointedTo)
		{
			PotentiallySetDistToRoot(pcPointedTo, iDistToRoot+1);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::GetTos(CArrayEmbeddedObjectPtr* papcTos)
{
	int					iNumPointers;
	int					i;
	CPointer**			ppPointer;
	CEmbeddedObject*	pcPointedTo;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = &(**ppPointer);
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
void CObject::RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
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
		RemoveToFrom(pcPointedTo, papcFromsChanged);
		(*ppPointer)->ClearObject();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::RemoveTo(CEmbeddedObject* pcTo)
{
	int				iNumPointers;
	int				i;
	CPointer**		ppPointer;
	CEmbeddedObject*	pcPointedTo;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
	
		if (pcPointedTo == pcTo)
		{
			(*ppPointer)->ClearObject();	
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer* CObject::Pointer(CPointer* pcPointer)
{
	pcPointer->Init(this);
	mapPointers.Add(&pcPointer);
	return pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CObject::Embedded(CBaseObject* pcObject)
{
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
void CObject::RecurseGetFroms(CArrayEmbeddedBaseObjectPtr* papcFroms)
{
	int				i;
	int				iNumEmbedded;
	CBaseObject*	pcEmbedded;

	CBaseObject::RecurseGetFroms(papcFroms);

	iNumEmbedded = mapEmbedded.NumElements();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = *mapEmbedded.Get(i);

		pcEmbedded->RecurseGetFroms(papcFroms);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CObject::IsEmbeddedDirty(void)
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
			if (pcEmbedded->IsEmbeddedDirty())
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
int CObject::GetEmbeddedIndex(CBaseObject* pcEmbedded)
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
BOOL CObject::RecurseGetEmbeddedIndex(CBaseObject* pcTest, int* piIndex)
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
	SetNumEmbeddedFlag(iNumEmbedded);
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
CBaseObject* CObject::Dehollow(void)
{
	return this;
}

