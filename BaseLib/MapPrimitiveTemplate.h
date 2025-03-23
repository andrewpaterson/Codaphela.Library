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
#ifndef __MAP_PRIMITIVE_TEMPLATE_H__
#define __MAP_PRIMITIVE_TEMPLATE_H__
#include "MapTemplate.h"


template<class M, class D>
class CMapPrimitiveTemplate : public CMapTemplate<M , D>
{
public:
	void	Init(bool bOverwrite);
	void	Init(CMallocator* pcMalloc, bool bOverwrite);
	D*		Get(M iKey);
	D*		Put(M iKey);
	bool	Put(M iKey, D* psData);
	bool	HasKey(M iKey);
	bool	Remove(M iKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
void CMapPrimitiveTemplate<M, D>::Init(bool bOverwrite)
{
	Init(&gcSystemAllocator, bOverwrite);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
void CMapPrimitiveTemplate<M, D>::Init(CMallocator* pcMalloc, bool bOverwrite)
{
	CMapTemplate<M, D>::Init(pcMalloc, &CompareInt, bOverwrite);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CMapPrimitiveTemplate<M, D>::Get(M iKey)
{
	return CMapTemplate<M, D>::Get(&iKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CMapPrimitiveTemplate<M, D>::Put(M iKey)
{
	return CMapTemplate<M, D>::Put(&iKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CMapPrimitiveTemplate<M, D>::Put(M iKey, D* psData)
{
	return CMapTemplate<M, D>::Put(&iKey, psData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CMapPrimitiveTemplate<M, D>::HasKey(M iKey)
{
	return CMapTemplate<M, D>::HasKey(&iKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CMapPrimitiveTemplate<M, D>::Remove(M iKey)
{
	return CMapTemplate<M, D>::Remove(&iKey);
}


#endif // __MAP_PRIMITIVE_TEMPLATE_H__

