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
#include "ArrayObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayObject> CArrayObject::Init(void)
{
	CArrayCommonObject::Init(FALSE, FALSE, TRUE);
	return Ptr<CArrayObject>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayObject::Add(CPointer& pObject)
{
	CArrayCommonObject::Add(pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayObject::AddAll(Ptr<CArrayObject> pcArray)
{
	CArrayCommonObject::AddAll((CArrayCommonObject*)pcArray.Dereference());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayObject::Insert(int iIndex, CPointer& pObject)
{
	mcArray.Insert(iIndex, pObject.Object());
	pObject.AddHeapFrom(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CArrayObject::Get(int iIndex)
{
	CBaseObject*	pcObject;
	CPointer		pObject;

	if ((iIndex >=0) && (iIndex < mcArray.UnsafeNumElements()))
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(iIndex);
		pObject.AssignObject(pcObject);
		return pObject;
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayObject::Set(int iIndex, CPointer& pObject)
{
	CBaseObject*	pcObject;

	if ((iIndex >=0) && (iIndex < mcArray.UnsafeNumElements()))
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(iIndex);
		
		CPointer	pTemp;
		pTemp.AssignObject(pcObject);
		pTemp = pObject;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayObject::Remove(CPointer& pObject)
{
	return CArrayCommonObject::Remove(pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayObject::RemoveAt(int iIndex)
{
	CPointer pObject;

	pObject = Get(iIndex);
	if (pObject.IsNotNull())
	{
		if (mcArray.Remove(iIndex))
		{
			pObject->RemoveHeapFrom(this, TRUE);
			return TRUE;
		}
	}
	return FALSE;
}

