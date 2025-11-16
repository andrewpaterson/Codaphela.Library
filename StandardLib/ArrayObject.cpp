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
#include "ArrayObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayObject> CArrayObject::Init(void)
{
	CArrayCommonObject::Init(false, false, true);
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
bool CArrayObject::Insert(size iIndex, CPointer& pObject)
{
	return CArrayCommonObject::InsertAt(iIndex, pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CArrayObject::Get(size iIndex)
{
	CBaseObject*	pcObject;
	CPointer		pObject;

	if (iIndex < mcArray.UnsafeNumElements())
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
bool CArrayObject::Remove(size iIndex)
{
	return CArrayCommonObject::RemoveAt(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CArrayObject::Remove(CPointer& pObject)
{
	return CArrayCommonObject::Remove(pObject);
}

