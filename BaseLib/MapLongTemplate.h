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
#ifndef __MAP_LONG_TEMPLATE_H__
#define __MAP_LONG_TEMPLATE_H__
#include "PrimitiveTypes.h"
#include "MapPrimitiveTemplate.h"
#include "IntegerHelper.h"


template<class D>
class CMapLongTemplate : public CMapPrimitiveTemplate<int64, D>
{
public:
	void	Init(bool bOverwrite);
	void	Init(CMallocator* pcMalloc, bool bOverwrite);

	bool	Read(CFileReader* pcFileReader);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapLongTemplate<D>::Init(bool bOverwrite)
{
	Init(&gcSystemAllocator, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapLongTemplate<D>::Init(CMallocator* pcMalloc, bool bOverwrite)
{
	CMapTemplate<int64, D>::Init(pcMalloc, &CompareLong, bOverwrite);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
bool CMapLongTemplate<D>::Read(CFileReader* pcFileReader)
{
	return CMapTemplate<int64, D>::Read(pcFileReader, &CompareLong);
}

#endif // __MAP_LONG_TEMPLATE_H__

