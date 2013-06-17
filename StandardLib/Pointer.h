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
#ifndef __POINTER_H__
#define __POINTER_H__
#include "PointerObject.h"
#include "BaseObject.h"

class CObject;

template<class M = CBaseObject>
class CPointer : public CPointerObject
{
public:
			CPointer();
			CPointer(CPointerObject cPointer);
	void 	Init(CObject* pcEmbedding);
	void	operator = (M* ptr);
	void	operator = (CPointer<M> pcPointer);
	void	operator = (CPointerObject pcPointer);
	M*		operator -> ();
	M*		operator & ();
};




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CPointer<M>::CPointer()
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
CPointer<M>::CPointer(CPointerObject cPointer)
{
	CPointerObject::Construct(cPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CPointer<M>::Init(CObject* pcEmbedding)
{
	CPointerObject::Init(pcEmbedding);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CPointer<M>::operator = (M* ptr)
{
	PointTo(ptr);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CPointer<M>::operator = (CPointer<M> pcPointer)
{
	PointTo(pcPointer.mpcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CPointer<M>::operator = (CPointerObject pcPointer)
{
	PointTo(pcPointer.mpcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CPointer<M>::operator -> ()
{
	return (M*)Dereference();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CPointer<M>::operator & ()
{
	return (M*)Dereference();
}


#endif // __POINTER_H__

