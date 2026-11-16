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
#include "Objects.h"
#include "ArrayWeakObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::Init(void)
{
	PreInit();
	mapcObjects.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::Class(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Taken off its objects' weak froms (any freed with this have NULLed
// their entries already).
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::Free(void)
{
	RemoveAll();
	mapcObjects.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayWeakObject::Save(CObjectWriter* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayWeakObject::Load(CObjectReader* pcFile)
{
	mapcObjects.Init();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::ClearWeak(size uiSlot)
{
	mapcObjects.SetPtr(uiSlot, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::RemapWeak(size uiSlot, CEmbeddedObject* pcObject)
{
	mapcObjects.SetPtr(uiSlot, pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CArrayWeakObject::GetWeak(size uiSlot)
{
	return (uiSlot < mapcObjects.NumElements()) ? mapcObjects.GetPtr(uiSlot) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayWeakObject::Add(CEmbeddedObject* pcObject)
{
	size	uiIndex;

	if (pcObject == NULL)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	uiIndex = mapcObjects.NumElements();
	mapcObjects.Add(pcObject);
	pcObject->GetObjectsOrDefault()->AddWeakFrom(pcObject, this, uiIndex);
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayWeakObject::Add(CPointer& pObject)
{
	return Add(pObject.Object());
}


//////////////////////////////////////////////////////////////////////////
//
// The last entry moved into uiTo, and its weak from with it.
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::Move(size uiFrom, size uiTo)
{
	CEmbeddedObject*	pcObject;

	pcObject = mapcObjects.GetPtr(uiFrom);
	if (pcObject)
	{
		pcObject->GetObjectsOrDefault()->RemoveWeakFrom(pcObject, this, uiFrom);
		pcObject->GetObjectsOrDefault()->AddWeakFrom(pcObject, this, uiTo);
	}
	mapcObjects.SetPtr(uiTo, pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::RemoveAt(size uiIndex)
{
	CEmbeddedObject*	pcObject;
	size				uiLast;

	pcObject = mapcObjects.GetPtr(uiIndex);
	if (pcObject)
	{
		pcObject->GetObjectsOrDefault()->RemoveWeakFrom(pcObject, this, uiIndex);
	}
	uiLast = mapcObjects.NumElements() - 1;
	if (uiIndex != uiLast)
	{
		Move(uiLast, uiIndex);
	}
	mapcObjects.RemoveTail();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayWeakObject::Remove(CEmbeddedObject* pcObject)
{
	size	i;

	if (pcObject == NULL)
	{
		return false;
	}
	for (i = 0; i < mapcObjects.NumElements(); i++)
	{
		if (mapcObjects.GetPtr(i) == pcObject)
		{
			RemoveAt(i);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// From the end, so an entry moved into a gap has been looked at.
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::Compact(void)
{
	size	i;

	for (i = mapcObjects.NumElements(); i > 0; i--)
	{
		if (mapcObjects.GetPtr(i - 1) == NULL)
		{
			RemoveAt(i - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayWeakObject::RemoveAll(void)
{
	size				i;
	CEmbeddedObject*	pcObject;

	for (i = 0; i < mapcObjects.NumElements(); i++)
	{
		pcObject = mapcObjects.GetPtr(i);
		if (pcObject)
		{
			pcObject->GetObjectsOrDefault()->RemoveWeakFrom(pcObject, this, i);
		}
	}
	mapcObjects.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayWeakObject::NumElements(void)
{
	return mapcObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<> CArrayWeakObject::Get(size uiIndex)
{
	return Ptr<>(mapcObjects.GetPtr(uiIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayWeakObject::Contains(CEmbeddedObject* pcObject)
{
	size	i;

	for (i = 0; i < mapcObjects.NumElements(); i++)
	{
		if (mapcObjects.GetPtr(i) == pcObject)
		{
			return true;
		}
	}
	return false;
}

