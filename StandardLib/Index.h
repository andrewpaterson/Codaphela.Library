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
#ifndef __INDEX_H__
#define __INDEX_H__
#include "Pointer.h"
#include "IndexObject.h"
#include "BaseObject.h"


template<class M = CBaseObject>
class CIndex final : public CIndexObject
{
CONSTRUCTABLE(CIndex)
public:
	Ptr<CIndex<M>>	Init(EIndexKeyReverse eKeyReverse = IKR_No);

	bool			Put(char* szKey, Ptr<M>& pObject);
	bool			Put(uint8* pvKey, size iKeySize, Ptr<M>& pObject);

	bool			Remove(char* szKey);
	bool			Remove(uint8* pvKey, size iKeySize);

	Ptr<M>			Get(char* szKey);
	Ptr<M>			Get(uint8* pvKey, size iKeySize);

	Ptr<M>			StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists = NULL);
	Ptr<M>			Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists = NULL);

	Ptr<M>			StartIterationPointer(SIndexTreeMemoryIterator* psIterator);
	Ptr<M>			IteratePointer(SIndexTreeMemoryIterator* psIterator);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<CIndex<M>> CIndex<M>::Init(EIndexKeyReverse eKeyReverse)
{
	CIndexObject::Init(eKeyReverse);
	return Ptr<CIndex<M>>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndex<M>::Put(char* szKey, Ptr<M>& pObject)
{
	return CIndexObject::Put(szKey, pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndex<M>::Put(uint8* pvKey, size iKeySize, Ptr<M>& pObject)
{
	return CIndexObject::Put(pvKey, iKeySize, pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndex<M>::Remove(char* szKey)
{
	return CIndexObject::Remove(szKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CIndex<M>::Remove(uint8* pvKey, size iKeySize)
{
	return CIndexObject::Remove(szKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndex<M>::Get(char* szKey)
{
	return CIndexObject::Get(szKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndex<M>::Get(uint8* pvKey, size iKeySize)
{
	return CIndexObject::Get(pvKey, iKeySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndex<M>::StartIteration(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists)
{
	return CIndexObject::StartIteration(psIterator, pvKey, piKeySize, iMaxKeySize, bExists);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndex<M>::Iterate(SIndexTreeMemoryIterator* psIterator, uint8* pvKey, size* piKeySize, size iMaxKeySize, bool* bExists)
{
	return CIndexObject::Iterate(psIterator, pvKey, piKeySize, iMaxKeySize, bExists);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndex<M>::StartIterationPointer(SIndexTreeMemoryIterator* psIterator)
{
	return CIndexObject::StartIterationPointer(psIterator);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CIndex<M>::IteratePointer(SIndexTreeMemoryIterator* psIterator)
{
	return CIndexObject::IteratePointer(psIterator);
}


#endif // __INDEX_H__

