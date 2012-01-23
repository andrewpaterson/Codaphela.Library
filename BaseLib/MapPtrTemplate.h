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
#ifndef __MAP_PTR_TEMPLATE_H__
#define __MAP_PTR_TEMPLATE_H__
#include "MapTemplate.h"
#include "PointerFunctions.h"


template<class D>
class CMapPtrTemplate : public CMapTemplate<void*, D>
{
public:
	void	Init(int iChunkSize);
	D*		GetWithKey(void* pvKey);
	void	GetAtIndex(int iIndex, void*** pppvKey, D** ppsData);
	D*		Put(void* pvKey);
	void	Put(void* pvKey, D* psData);
	void	Remove(void* pvKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapPtrTemplate<D>::Init(int iChunkSize)
{
	CMapTemplate<void*, D>::Init(iChunkSize, &ComparePtrPtr);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapPtrTemplate<D>::GetWithKey(void* pvKey)
{
	return CMapTemplate<void*, D>::GetWithKey(&pvKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapPtrTemplate<D>::GetAtIndex(int iIndex, void*** pppvKey, D** ppsData)
{
	CMapTemplate<void*, D>::GetAtIndex(iIndex, pppvKey, ppsData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapPtrTemplate<D>::Put(void* pvKey)
{
	return CMapTemplate<void*, D>::Put(&pvKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapPtrTemplate<D>::Put(void* pvKey, D* psData)
{
	return CMapTemplate<void*, D>::Put(&pvKey, psData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapPtrTemplate<D>::Remove(void* pvKey)
{
	return CMapTemplate<void*, D>::Remove(&pvKey);
}


#endif //__MAP_PTR_TEMPLATE_H__

