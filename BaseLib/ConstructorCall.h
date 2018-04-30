/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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


template<class M>
M* NewMalloc(void)
{
	M*	pv;

	pv = (M*)malloc(sizeof(M));
	memset(pv, 0, sizeof(M));
	new(pv) M;

	return pv;
}


template<class M>
class CPostMalloc
{
public:
	M* 		PostMalloc(M* pv);
	M* 		PostMalloc(M* pv, int iNumElements, int iStride);

	BOOL 	RequiresVirtualFunctionTable(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CPostMalloc<M>::PostMalloc(M* pv)
{
	if (!std::is_trivially_default_constructible<M>())
	{
		new(pv) M;
	}
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CPostMalloc<M>::PostMalloc(M* pv, int iNumElements, int iStride)
{
	int		i;

	if (RequiresVirtualFunctionTable())
	{
		for (i = 0; i < iNumElements; i++)
		{
			new(pv) M;
			pv = (M*)RemapSinglePointer(pv, iStride);
		}
	}
	return pv;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CPostMalloc<M>::RequiresVirtualFunctionTable(void)
{
	return !std::is_trivially_default_constructible<M>();
}


#endif // __CONSTRUCTOR_CALL_H__

