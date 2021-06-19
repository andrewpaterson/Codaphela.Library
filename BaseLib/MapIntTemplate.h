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
#ifndef __MAP_INT_TEMPLATE_H__
#define __MAP_INT_TEMPLATE_H__
#include "MapPrimitiveTemplate.h"
#include "IntegerHelper.h"


template<class D>
class CMapIntTemplate : public CMapPrimitiveTemplate<int, D>
{
public:
	void	Init(BOOL bOverwrite = TRUE);
	void	Init(CMallocator* pcMalloc, BOOL bOverwrite = TRUE);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapIntTemplate<D>::Init(BOOL bOverwrite)
{
	Init(&gcSystemAllocator, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapIntTemplate<D>::Init(CMallocator* pcMalloc, BOOL bOverwrite)
{
	CMapTemplate<int, D>::Init(pcMalloc, &CompareInt, bOverwrite);
};


#endif // !__MAP_INT_TEMPLATE_H__

