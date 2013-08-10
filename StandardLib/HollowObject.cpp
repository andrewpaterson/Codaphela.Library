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
#include "Objects.h"
#include "HollowEmbeddedObject.h"
#include "HollowObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHollowObject::CHollowObject(void)
{
	miFlags &= ~OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::Init(unsigned short iNumEmbedded)
{
	SetFlagNumEmbedded(iNumEmbedded);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::Kill(void)
{
	CBaseObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CHollowObject::Dehollow(void)
{
	return mpcObjectsThisIn->Dehollow(moi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObject::IsHollow(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObject::Save(CObjectSerialiser* pcFile)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObject::Load(CObjectDeserialiser* pcFile)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObject::IsCollection(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CHollowObject::IsObject(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::SetDistToRoot(int iDistToRoot)
{
	if (miDistToRoot == iDistToRoot)
	{
		return;
	}

	miDistToRoot = iDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::GetTos(CArrayEmbeddedObjectPtr* papcTos)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowObject::NumTos(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowObject::RemapTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::CollectPointedToToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::KillToPointers(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::KillData(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::RemoveTo(CEmbeddedObject* pcTo)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowObject::GetNumEmbedded(void)
{
	return GetNumEmbeddedFromFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CHollowObject::GetEmbeddedObject(int iIndex)
{
	if (iIndex >= GetNumEmbeddedFromFlags())
	{
		return NULL;
	}

	if (iIndex == 0)
	{
		return this;
	}

	return GetRemappedEmbeddedObject(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
{
	CHollowEmbeddedObject*	pcCurrent;
	int						i;

	if (pcEmbedded == this)
	{
		return 0;
	}
	else
	{
		for (i = 1; i < GetNumEmbedded(); i++)
		{
			pcCurrent = (CHollowEmbeddedObject*)GetEmbeddedObject(i);
			if (pcCurrent == pcEmbedded)
			{
				return i;
			}
		}

		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CHollowObject::GetRemappedEmbeddedObject(int iIndex)
{
	CEmbeddedObject*	pcEmbedded;

	pcEmbedded = (CEmbeddedObject*)RemapSinglePointer(this, sizeof(CHollowObject) + sizeof(CHollowEmbeddedObject)*(iIndex-1));
	return pcEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CHollowObject::NumFroms(void)
{
	CHollowEmbeddedObject*	pcEmbedded;
	int						iCount;	
	int						i;

	iCount = mapFroms.NumElements();
	for (i = 1; i < GetNumEmbedded(); i++)
	{
		pcEmbedded = (CHollowEmbeddedObject*)GetEmbeddedObject(i);
		iCount += pcEmbedded->NumFroms();
	}
	return iCount;
}

