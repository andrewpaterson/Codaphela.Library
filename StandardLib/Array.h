/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2013 Andrew Paterson

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
#ifndef __ARRAY_H__
#define __ARRAY_H__
#include "Pointer.h"
#include "ArrayObject.h"
#include "BaseObject.h"


template<class M = CBaseObject>
class CArray : public CArrayObject
{
CONSTRUCTABLE(CArray);
public:
	Ptr<CArray<M>>	Init(void);

	void			Add(Ptr<M> pObject);
	void			AddAll(Ptr<CArrayCommonObject> pcArray);
	void			Insert(int iIndex, Ptr<M> pObject);
	Ptr<M>			Get(int iIndex);
	void			Set(int iIndex, Ptr<M> pObject);

	BOOL			RemoveAt(int iIndex);
	BOOL			Remove(Ptr<M> pObject);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<CArray<M>> CArray<M>::Init(void)
{
	CArrayObject::Init();
	return Ptr<CArray<M>>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArray<M>::Add(Ptr<M> pObject)
{
	CArrayObject::Add(pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArray<M>::AddAll(Ptr<CArrayCommonObject> pcArray)
{
	pcArray->ClassName();
	CArrayObject::AddAll((CArrayCommonObject*)pcArray.Dereference());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArray<M>::Insert(int iIndex, Ptr<M> pObject)
{
	CArrayObject::Insert(iIndex, pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArray<M>::Set(int iIndex, Ptr<M> pObject)
{
	CArrayObject::Set(iIndex, pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CArray<M>::Get(int iIndex)
{
	return CArrayObject::Get(iIndex);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArray<M>::RemoveAt(int iIndex)
{
	return CArrayObject::RemoveAt(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArray<M>::Remove(Ptr<M> pObject)
{
	return CArrayObject::Remove(pObject);
}


#endif // __ARRAY_H__

