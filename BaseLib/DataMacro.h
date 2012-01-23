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


#define HeaderAndDataAllocate(SHeaderType, SDataType) \
	HeaderAllocateMacro<SHeaderType, SDataType>()

#define HeaderGetData(SHeaderType, SDataType, Header) \
	HeaderGetDataMacro<SHeaderType, SDataType>(Header)

#define DataGetHeader(SHeaderType, SDataType, Data) \
	DataGetHeaderMacro<SHeaderType, SDataType>(Data)

#define HeaderAndDataFree(SHeaderType, SDataType, Data) \
	HeaderFreeMacro<SHeaderType, SDataType>(Data)


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class T, class M>
M* HeaderAllocateMacro()
{
	void*	pv;

	pv = malloc(sizeof(T) + sizeof(M));
	if (pv == NULL)
	{
		gcUserError.Set("Not enough memory to allocate unknown type");
		return NULL;
	}
	return (M*)((ENGINE_SIZE_T) ((int)((ENGINE_SIZE_T) pv) + sizeof(T)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class T, class M>
M* HeaderGetDataMacro(T* pv)
{
	if (pv == NULL)
	{
		return NULL;
	}
	return (M*)((ENGINE_SIZE_T) ((int)((ENGINE_SIZE_T) pv) + sizeof(T)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class T, class M>
T* DataGetHeaderMacro(M* pv)
{
	if (pv == NULL)
	{
		return NULL;
	}
	return (T*)((ENGINE_SIZE_T) ((int)((ENGINE_SIZE_T) pv) - sizeof(T)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class T, class M>
void HeaderFreeMacro(M* pv)
{
	T* pvHeader;

	pvHeader = DataGetHeaderMacro<T, M>(pv);
	if (pvHeader)
	{
		free(pvHeader);
	}
}


#endif // __DATA_MACRO_H__