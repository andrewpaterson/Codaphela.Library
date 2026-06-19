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
			Ptr() : CPointer() {};
			Ptr(CEmbeddedObject* pcObject) : CPointer(pcObject) {};
			Ptr(CPointer& cPointer) : CPointer(cPointer) {};
			Ptr(const CPointer& cPointer) : CPointer(cPointer) {};
			Ptr(Ptr& cPointer) : CPointer(cPointer) {};
			Ptr(const Ptr& cPointer) : CPointer(cPointer) {};

			using CPointer::operator =;
	
	M*		operator -> ();
	M*		operator & ();

	Ptr<M>	operator + (Ptr<M> obj);
	Ptr<M>	operator + (char* sz);

	explicit operator bool() const;

	const char*		TemplateClassName(void);
};


bool IsValidPointerClassAllowNull(char* szParameter, const char* szInstanceClassName, const char* szExpectedClassName);
bool FailValidPointerClassAllowNull(char* szMethod, char* szParameter, const char* szInstanceClassName, const char* szExpectedClassName);
#define ValidatePtr(p) if (!IsValidPointerClassAllowNull(#p, p.IsNotNull() ? p->ClassName() : NULL, p.TemplateClassName())) FailValidPointerClassAllowNull(__METHOD__, #p, p.IsNotNull() ? p->ClassName() : NULL, p.TemplateClassName());


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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
const char* Ptr<M>::TemplateClassName(void)
{
	const char* szClassName;
	M			m;

	szClassName = m.ClassName();
	return szClassName;
}

#endif // __POINTER_H__


