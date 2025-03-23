/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
	muiFlags &= ~OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::Init(size iNumEmbedded)
{
	PreInit();
	SetFlagNumEmbedded(iNumEmbedded);
	PostInit();
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
void CHollowObject::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::EmbedFields(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CHollowObject::Dehollow(void)
{
	if (IsNamed())
	{
		return mpcObjectsThisIn->Dehollow(mon.Text());
	}
	else
	{
		return mpcObjectsThisIn->Dehollow(moi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowObject::IsHollow(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowObject::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowObject::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowObject::IsCollection(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHollowObject::IsObject(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::SetPointerTosExpectedDistToRoot(int iDistToRoot)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::GetPointerTos(CArrayTemplateEmbeddedObjectPtr* papcTos)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CHollowObject::NumPointerTos(void)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CHollowObject::RemapPointerTos(CEmbeddedObject* pcOld, CEmbeddedObject* pcNew)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::RemoveAllPointerTosDontKill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::RemoveAllPointerTos(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::UpdateAttachedEmbeddedObjectPointerTosDistToRoot(CDistCalculatorParameters* pcParameters, int iExpectedDist)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::FreePointers(void)
{
	CBaseObject::FreePointers();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::RemovePointerTo(CEmbeddedObject* pcTo)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CHollowObject::GetNumEmbedded(void)
{
	return GetNumEmbeddedFromFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CHollowObject::GetEmbeddedObject(size iIndex)
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
size CHollowObject::GetEmbeddedIndex(CEmbeddedObject* pcEmbedded)
{
	CHollowEmbeddedObject*	pcCurrent;
	size					i;
	size					uiNumEmbedded;

	if (pcEmbedded == this)
	{
		return 0;
	}
	else
	{
		uiNumEmbedded = GetNumEmbedded();
		for (i = 1; i < uiNumEmbedded; i++)
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
CEmbeddedObject* CHollowObject::GetRemappedEmbeddedObject(size iIndex)
{
	CEmbeddedObject*	pcEmbedded;

	pcEmbedded = (CEmbeddedObject*)RemapSinglePointer(this, sizeof(CHollowObject) + (sizeof(CHollowEmbeddedObject) * (iIndex - 1)));
	return pcEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CHollowObject::NumHeapFroms(void)
{
	CHollowEmbeddedObject*	pcHollowEmbedded;
	size					i;
	size					iCount;	
	size					uiNumEmbedded;

	iCount = CEmbeddedObject::NumHeapFroms();
	uiNumEmbedded = GetNumEmbedded();
	for (i = 1; i < uiNumEmbedded; i++)
	{
		pcHollowEmbedded = (CHollowEmbeddedObject*)GetEmbeddedObject(i);
		iCount += pcHollowEmbedded->NumHeapFroms();
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CHollowObject::NumStackFroms(void)
{
	CHollowEmbeddedObject*	pcHollowEmbedded;
	size					i;
	size					iCount;	
	size					uiNumEmbedded;

	iCount = CEmbeddedObject::NumStackFroms();
	uiNumEmbedded = GetNumEmbedded();
	for (i = 1; i < uiNumEmbedded; i++)
	{
		pcHollowEmbedded = (CHollowEmbeddedObject*)GetEmbeddedObject(i);
		iCount += pcHollowEmbedded->NumStackFroms();
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::BaseValidatePointerTos(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHollowObject::ValidateConsistency(void)
{
	ValidateEmbeddedConsistency();
	ValidateCanFindRoot();
}

