/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifdef LINUX_GNU_32
#include <type_traits>
#endif // LINUX_GNU_32
#include "Define.h"
#include "PointerRemapper.h"


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
	Class* 		PostMalloc(Class* pv, int iNumElements, int iStride);

	template<class Subclass>
	Class* 		PostMalloc(Subclass* pv);
	template<class Subclass>
	Class* 		PostMalloc(Subclass* pv, int iNumElements, int iStride);

	BOOL 		RequiresVirtualFunctionTable(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
Class* CPostMalloc<Class>::PostMalloc(Class* pv)
{
	if (!std::is_trivially_default_constructible<Class>())
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
Class* CPostMalloc<Class>::PostMalloc(Class* pv, int iNumElements, int iStride)
{
	int		i;

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
	if (!std::is_trivially_default_constructible<Subclass>())
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
BOOL CPostMalloc<Class>::RequiresVirtualFunctionTable(void)
{
	return !std::is_trivially_default_constructible<Class>();
}


#endif // !__CONSTRUCTOR_CALL_H__

