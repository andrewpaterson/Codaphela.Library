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
#ifndef __SET_H__
#define __SET_H__
#include "Pointer.h"
#include "SetObject.h"
#include "BaseObject.h"


template<class M = CBaseObjec>
class CSet : public CSetObject
{
	BASE_FUNCTIONS(CSet);
public:
	Ptr<CSet<M>>	Init(int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);
	void 			Kill(void);

	void			Add(Ptr<M> pObject);
	void			AddAll(Ptr<CArrayCommonObject> pcSet);
	Ptr<M>			Get(int iIndex);
	BOOL			Remove(Ptr<M> pObject);

	void			RemoveDuringIteration(SSetIterator* psIter);

	Ptr<M>			StartIteration(SSetIterator* psIter);
	Ptr<M>			Iterate(SSetIterator* psIter);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<CSet<M>> CSet<M>::Init(int iChunkSize)
{
	CSetObject::Init(iChunkSize);
	return Ptr<CSet<M>>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSet<M>::Kill(void)
{
	CSetObject::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSet<M>::Add(Ptr<M> pObject)
{
	CSetObject::Add(pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSet<M>::AddAll(Ptr<CArrayCommonObject> pcSet)
{
	pcSet->ClassName();
	CSetObject::AddAll((CArrayCommonObject*)pcSet.Dereference());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CSet<M>::Get(int iIndex)
{
	return CSetObject::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CSet<M>::Remove(Ptr<M> pObject)
{
	return CSetObject::Remove(pObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSet<M>::RemoveDuringIteration(SSetIterator* psIter)
{
	CSetObject::RemoveDuringIteration(psIter);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CSet<M>::StartIteration(SSetIterator* psIter)
{
	return CSetObject::StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CSet<M>::Iterate(SSetIterator* psIter)
{
	return CSetObject::Iterate(psIter);
}


#endif // __SET_H__

