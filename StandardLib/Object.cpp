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
	CBaseObject*		pcPointedTo;
	CPointerObject**	ppPointer;
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
	CPointerObject**	ppPointer;

	MarkForKilling(papcKilled);

	for (i = 0; i < mapPointers.NumElements(); i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (*ppPointer)->Object();
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
	int					i;
	CBaseObject*		pcPointedTo;
	CPointerObject**	ppPointer;
	int					iNumPointers;

	miDistToRoot = iDistToRoot;

	iNumPointers = mapPointers.NumElements();
	for (i = 0; i < iNumPointers; i++)
	{
		ppPointer = mapPointers.Get(i);
		pcPointedTo = (**ppPointer).Object();
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
void CObject::GetTos(CArrayBaseObjectPtr* papcTos)
{
	int					iNumPointers;
	int					i;
	CPointerObject**	ppPointer;
	CBaseObject*		pcPointedTo;

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
	CPointerObject**	ppPointer;
	CBaseObject*		pcPointedTo;

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
void CObject::RemoveTo(CBaseObject* pcTo)
{
	int					iNumPointers;
	int					i;
	CPointerObject**	ppPointer;
	CBaseObject*		pcPointedTo;

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
CPointerObject* CObject::Pointer(CPointerObject* pcPointer)
{
	pcPointer->Init(this);
	mapPointers.Add(&pcPointer);
	return pcPointer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CObject::RemapTos(CBaseObject* pcOld, CBaseObject* pcNew)
{
	int					iNumPointers;
	int					i;
	CPointerObject**	ppPointer;
	CBaseObject*		pcPointedTo;
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

