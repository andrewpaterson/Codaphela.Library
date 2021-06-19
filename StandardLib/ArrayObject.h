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
#ifndef __ARRAY_OBJECT_H__
#define __ARRAY_OBJECT_H__
#include "ArrayUnknown.h"
#include "ArrayCommonObject.h"
#include "Pointer.h"


class CArrayObject : public CArrayCommonObject
{
CONSTRUCTABLE(CArrayObject);
DESTRUCTABLE(CArrayObject);
public:
	Ptr<CArrayObject>	Init(void);

	void				Add(CPointer& pObject);
	void				AddAll(Ptr<CArrayObject> pcArray);
	void				Insert(int iIndex, CPointer& pObject);
	CPointer			Get(int iIndex);
	void				Set(int iIndex, CPointer& pObject);
	BOOL				Remove(CPointer& pObject);
	BOOL				RemoveAt(int iIndex);

	template<class M>
	Ptr<M>				Get(int iIndex);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> CArrayObject::Get(int iIndex)
{
	return (M*)Get(iIndex);
}


#endif // !__ARRAY_OBJECT_H__

