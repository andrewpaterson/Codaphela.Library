/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __DATA_MACRO_H__
#define __DATA_MACRO_H__
#include "Define.h"
#include "PointerRemapper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class T, class M>
M* HeaderGetData(T* pv)
{
	if (pv == NULL)
	{
		return NULL;
	}
	return (M*)RemapSinglePointer(pv, sizeof(T));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class T, class M>
T* DataGetHeader(M* pv)
{
	if (pv == NULL)
	{
		return NULL;
	}
	return (T*)RemapSinglePointer(pv, -(ptrdiff_t)sizeof(T));
}


#endif // !__DATA_MACRO_H__

