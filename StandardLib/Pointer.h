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
#include "BaseLib/Logger.h"
#include "BaseLib/Define.h"
#include "PointerObject.h"
#include "EmbeddedObject.h"


class CObject;

template<class M = CEmbeddedObject>
class Ptr : public CPointer
{
public:
			Ptr();
			Ptr(CEmbeddedObject* pcObject);
			Ptr(CPointer& cPointer);

	void	operator = (CEmbeddedObject* pcObject);
	void	operator = (CPointer& pcPointer);
	M*		operator -> ();
	M*		operator & ();
};




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::Ptr()
{
	CPointer::CPointer();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::Ptr(CEmbeddedObject* pcObject)
{
	CPointer::CPointer(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::Ptr(CPointer& cPointer)
{
	CPointer::CPointer(cPointer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void Ptr<M>::operator = (CEmbeddedObject* pcObject)
{
	LOG_POINTER_DEBUG();

	PointTo(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void Ptr<M>::operator = (CPointer& pcPointer)
{
	LOG_POINTER_DEBUG();

	PointTo(pcPointer.mpcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* Ptr<M>::operator -> ()
{
	return (M*)Dereference();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* Ptr<M>::operator & ()
{
	return (M*)Dereference();
}


#endif // __POINTER_H__

