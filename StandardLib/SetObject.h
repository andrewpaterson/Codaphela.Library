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
#ifndef __SET_OBJECT_H__
#define __SET_OBJECT_H__
#include "SetUnknown.h"
#include "ArrayCommonObject.h"
#include "PointerObject.h"
#include "Pointer.h"


class CSetObject : public CArrayCommonObject
{
BASE_FUNCTIONS(CSetObject);
public:
						Ptr<CSetObject> Init(int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);

						CPointer		Get(int iIndex);

						void			RemoveDuringIteration(SSetIterator* psIter);

						CPointer		StartIteration(SSetIterator* psIter);
						CPointer		Iterate(SSetIterator* psIter);

	template<class M>	Ptr<M>			StartIteration(SSetIterator* psIter);
	template<class M>	Ptr<M>			Iterate(SSetIterator* psIter);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CSetObject::StartIteration(SSetIterator* psIter)
{
	return (M*)StartIteration(psIter);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CSetObject::Iterate(SSetIterator* psIter)
{
	return (M*)Iterate(psIter);
}


#endif // __SET_OBJECT_H__

