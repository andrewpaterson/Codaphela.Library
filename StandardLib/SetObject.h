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
#include "Pointer.h"


class CSetObject : public CArrayCommonObject
{
CONSTRUCTABLE(CSetObject)
DESTRUCTABLE(CSetObject)
public:
						Ptr<CSetObject>		Init(bool bSortPointers);

						CPointer			Get(size iIndex);

						void				RemoveDuringIteration(SSetIterator* psIter);

	template<class M>	Ptr<M>				StartIteration(SSetIterator* psIter);
	template<class M>	Ptr<M>				Iterate(SSetIterator* psIter);

						void				TouchAll(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CSetObject::StartIteration(SSetIterator* psIter)
{
	bool	bExists;

	return (M*)StartIteration(psIter, &bExists);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CSetObject::Iterate(SSetIterator* psIter)
{
	bool	bExists;

	return (M*)Iterate(psIter, &bExists);
}


#endif // __SET_OBJECT_H__

