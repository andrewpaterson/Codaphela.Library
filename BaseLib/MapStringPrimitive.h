/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __MAP_STRING_PRIMITIVE_H__
#define __MAP_STRING_PRIMITIVE_H__
#include "MapStringTemplate.h"


template<class D>
class CMapStringPrimitive : public CMapStringTemplate<D>
{
public:
	D		Get(char* szKey, D sNullValue = -1);
	D		Get(const char* szKey, D sNullValue = -1);
	bool	Put(char* szKey, D sData);
	bool	Put(const char* szKey, D sData);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D CMapStringPrimitive<D>::Get(char* szKey, D sNullValue)
{
	D* psData;

	psData = (CMapStringTemplate<D>::Get(szKey));
	if (psData)
	{
		return *psData;
	}
	else
	{
		return sNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D CMapStringPrimitive<D>::Get(const char* szKey, D sNullValue)
{
	D* psData;

	psData = (CMapStringTemplate<D>::Get(szKey));
	if (psData)
	{
		return *psData;
	}
	else
	{
		return sNullValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CMapStringPrimitive<D>::Put(char* szKey, D sData)
{
	return CMapStringTemplate<D>::Put(szKey, &sData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CMapStringPrimitive<D>::Put(const char* szKey, D sData)
{
	return CMapStringTemplate<D>::Put(szKey, &sData);
}


#endif // __MAP_STRING_INT_H__

