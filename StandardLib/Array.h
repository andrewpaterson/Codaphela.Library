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
#ifndef __ARRAY_H__
#define __ARRAY_H__
#include "ArrayUnknown.h"
#include "ArrayCommonObject.h"
#include "PointerObject.h"


class CArray : public CArrayCommonObject
{
BASE_FUNCTIONS(CArray);
public:
	void 			Init(int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);
	void 			Kill(void);

	void			Insert(int iIndex, CPointerObject pObject);
	CPointerObject	Get(int iIndex);
	void			Set(int iIndex, CPointerObject pObject);

	template<class M>
	CPointer<M>		Get(int iIndex);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CPointer<M> CArray::Get(int iIndex)
{
	return (M*)Get(iIndex);
}


#endif // __ARRAY_H__

