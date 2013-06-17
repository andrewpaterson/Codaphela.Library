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
#include "Array.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArray::Init(int iChunkSize)
{
	CArrayCommonObject::Init(FALSE, FALSE, TRUE, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArray::Kill(void)
{
	CArrayCommonObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArray::Insert(int iIndex, CPointer pObject)
{
	mcArray.Insert(iIndex, pObject.mpcObject);
	pObject->AddFrom(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CArray::Get(int iIndex)
{
	CBaseObject*	pcObject;
	CPointer	pObject;

	if ((iIndex >=0) && (iIndex < mcArray.UnsafeNumElements()))
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(iIndex);
		pObject.mpcObject = pcObject;
		return pObject;
	}
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArray::Set(int iIndex, CPointer pObject)
{
	CBaseObject*	pcObject;
	CPointer	pTemp;

	if ((iIndex >=0) && (iIndex < mcArray.UnsafeNumElements()))
	{
		pcObject = (CBaseObject*)mcArray.UnsafeGet(iIndex);
		pTemp.mpcObject = pcObject;
		pTemp = pObject;
	}
}

