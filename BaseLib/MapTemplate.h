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
#ifndef __MAP_TEMPLATE_H__
#define __MAP_TEMPLATE_H__
#include <stdlib.h>
#include "SystemAllocator.h"
#include "MapBlock.h"
#include "PointerRemapper.h"
#include "FileReader.h"
#include "FileWriter.h"


template<class M, class D>
class CMapTemplate : public CMapBlock
{
public:
	D*		Get(M* psKey);

	D*		Put(M* psKey);
	bool	Put(M* psKey, D* psData);

	bool	HasKey(M* psKey);

	bool	Remove(M* psKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CMapTemplate<M, D>::Get(M* psKey)
{
	void*	pvData;

	pvData =  CMapBlock::Get(psKey, sizeof(M));
	return (D*)pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
D* CMapTemplate<M, D>::Put(M* psKey)
{
	return (D*)CMapBlock::Put(psKey, sizeof(M), sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CMapTemplate<M, D>::Put(M* psKey, D* psData)
{
	return CMapBlock::Put(psKey, sizeof(M), psData, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CMapTemplate<M, D>::HasKey(M* psKey)
{
	return CMapBlock::HasKey(psKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class D>
bool CMapTemplate<M, D>::Remove(M* psKey)
{
	return CMapBlock::Remove(psKey, sizeof(M));
}


#endif // __MAP_TEMPLATE_H__

