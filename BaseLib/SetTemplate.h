/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __SET_TEMPLATE_H__
#define __SET_TEMPLATE_H__
#include <stdlib.h>
#include "SystemAllocator.h"
#include "SetBlock.h"
#include "PointerRemapper.h"
#include "FileReader.h"
#include "FileWriter.h"


template<class M>
class CSetTemplate : public CSetBlock
{
public:
	M*		Get(M* psKey);

	M*		Put(M* psKey);

	bool	HasKey(M* psKey);

	bool	Remove(M* psKey);

	bool	PutInSorted(size iIndex, M* ppsKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CSetTemplate<M>::Get(M* psKey)
{
	void*	pvData;

	pvData =  CSetBlock::Get(psKey, sizeof(M));
	return (M*)pvData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CSetTemplate<M>::Put(M* psKey)
{
	return (M*)CSetBlock::Put(psKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CSetTemplate<M>::HasKey(M* psKey)
{
	return CSetBlock::HasKey(psKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CSetTemplate<M>::Remove(M* psKey)
{
	return CSetBlock::Remove(psKey, sizeof(M));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CSetTemplate<M>::PutInSorted(size iIndex, M* ppsKey)
{
	return CSetBlock::PutInSorted(iIndex, ppsKey, sizeof(M));
}


#endif // __SET_TEMPLATE_H__

