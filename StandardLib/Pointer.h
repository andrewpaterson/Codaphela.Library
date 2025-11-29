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
			~Ptr();

	void	operator = (CEmbeddedObject* pcObject);
	void	operator = (CPointer& pcPointer);
	void	operator = (Ptr& pcPointer);
	M*		operator -> ();
	M*		operator & ();

	Ptr<M>	operator + (Ptr<M> obj);
	Ptr<M>	operator + (char* sz);

	explicit operator bool() const;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::Ptr()
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::Ptr(CPointer& cPointer)
{
	mpcEmbedding = NULL;
	mpcObject = NULL;

	LOG_POINTER_DEBUG();

	PointTo(cPointer.mpcObject, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::Ptr(CEmbeddedObject* pcObject)
{
	LOG_POINTER_DEBUG();

	mpcEmbedding = NULL;
	mpcObject = NULL;

	PointTo(pcObject, false);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::~Ptr()
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void Ptr<M>::operator = (CEmbeddedObject* pcObject)
{
	LOG_POINTER_DEBUG();

	PointTo(pcObject, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void Ptr<M>::operator = (CPointer& pcPointer)
{
	LOG_POINTER_DEBUG();

	PointTo(pcPointer.mpcObject, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void Ptr<M>::operator = (Ptr& pcPointer)
{
	LOG_POINTER_DEBUG();

	PointTo(pcPointer.mpcObject, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* Ptr<M>::operator -> ()
{
	return (M*)DereferenceArrow();
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> Ptr<M>::operator + (Ptr<M> pObject)
{
	M*	p = (M*)DereferenceArrow();

	if (p)
	{
		return p->AddOperator(pObject);
	}
	else
	{
		return Ptr();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M> Ptr<M>::operator + (char* sz)
{
	M* p = (M*)DereferenceArrow();

	if (p)
	{
		return p->AddOperator(sz);
	}
	else
	{
		return Ptr();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
Ptr<M>::operator bool() const 
{
	return mpcObject != NULL;
}


#endif // __POINTER_H__


