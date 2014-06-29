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
#include "MapTemplate.h"
#include "IntegerHelper.h"


template<class D>
class CMapIntTemplate : public CMapTemplate<int, D>
{
public:
	void	Init(int iChunkSize);
	D*		GetWithKey(int iKey);
	void	GetAtIndex(int iIndex, int** ppiKey, D** ppsData);
	D*		Put(int iKey);
	void	Put(int iKey, D* psData);
	BOOL	Remove(int iKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapIntTemplate<D>::Init(int iChunkSize)
{
	CMapTemplate<int, D>::Init(iChunkSize, &CompareInt);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapIntTemplate<D>::GetWithKey(int iKey)
{
	return CMapTemplate<int, D>::GetWithKey(&iKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapIntTemplate<D>::GetAtIndex(int iIndex, int** ppiKey, D** ppsData)
{
	CMapTemplate<int, D>::GetAtIndex(iIndex, ppiKey, ppsData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapIntTemplate<D>::Put(int iKey)
{
	return CMapTemplate<int, D>::Put(&iKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapIntTemplate<D>::Put(int iKey, D* psData)
{
	return CMapTemplate<int, D>::Put(&iKey, psData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CMapIntTemplate<D>::Remove(int iKey)
{
	return CMapTemplate<int, D>::Remove(&iKey);
}


#endif //__MAP_INT_TEMPLATE_H__

