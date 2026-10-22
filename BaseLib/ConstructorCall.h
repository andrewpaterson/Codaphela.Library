/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __CONSTRUCTOR_CALL_H__
#define __CONSTRUCTOR_CALL_H__
#include <new>
#include <stdlib.h>
#include <string.h>
#include "Define.h"
#include "PointerRemapper.h"


//You probably want to create a CPostMalloc<Type> instance and call PostMalloc on it with your uninitialised variable.


//True if Class has no virtual table and no constructor that does anything.
//__is_trivially_constructible is a compiler intrinsic in MSVC, g++ and clang.
template<class Class>
constexpr bool IsTriviallyDefaultConstructible(void)
{
	return __is_trivially_constructible(Class);
}

template<class Class>
Class* NewMalloc(void)
{
	Class*	pv;

	pv = (Class*)malloc(sizeof(Class));
	memset(pv, 0, sizeof(Class));
	new(pv) Class;

	return pv;
}


template<class Class>
Class* New(Class* pv)
{
	memset(pv, 0, sizeof(Class));
	new(pv) Class;

	return pv;
}


template<class Class>
class CPostMalloc
{
public:
	Class* 		PostMalloc(Class* pv);
	Class* 		PostMalloc(Class* pv, size iNumElements, size iStride);

	template<class Subclass>
	Class* 		PostMalloc(Subclass* pv);
	template<class Subclass>
	Class* 		PostMalloc(Subclass* pv, int iNumElements, int iStride);

	bool 		RequiresVirtualFunctionTable(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
Class* CPostMalloc<Class>::PostMalloc(Class* pv)
{
	if (!IsTriviallyDefaultConstructible<Class>())
	{
		new(pv) Class;
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
Class* CPostMalloc<Class>::PostMalloc(Class* pv, size iNumElements, size iStride)
{
	size	i;

	if (RequiresVirtualFunctionTable())
	{
		for (i = 0; i < iNumElements; i++)
		{
			new(pv) Class;
			pv = (Class*)RemapSinglePointer(pv, iStride);
		}
	}
	return pv;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class> template<class Subclass>
Class* CPostMalloc<Class>::PostMalloc(Subclass* pv)
{
	if (!IsTriviallyDefaultConstructible<Subclass>())
	{
		new(pv) Subclass;
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class> template<class Subclass>
Class* CPostMalloc<Class>::PostMalloc(Subclass* pv, int iNumElements, int iStride)
{
	int		i;

	if (RequiresVirtualFunctionTable())
	{
		for (i = 0; i < iNumElements; i++)
		{
			new(pv) Subclass;
			pv = (Subclass*)RemapSinglePointer(pv, iStride);
		}
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
bool CPostMalloc<Class>::RequiresVirtualFunctionTable(void)
{
	return !IsTriviallyDefaultConstructible<Class>();
}


#endif // __CONSTRUCTOR_CALL_H__

