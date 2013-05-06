/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ARRAY_UNKNOWN_H__
#define __ARRAY_UNKNOWN_H__
#include "ArrayCommonUnknown.h"


class CArrayUnknown : public CArrayCommonUnknown
{
BASE_FUNCTIONS(CArrayUnknown);
public:
	void 		Init(int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);
	void 		Kill(void);

	template<class M>
	M*			Add(void);
	BOOL		Add(CUnknown* pcUnknown);

	template<class M>
	M*			Insert(int iIndex);
	BOOL		Insert(int iIndex, CUnknown* pcUnknown);

	CUnknown*	Get(int iIndex);
	void		Set(int iIndex, CUnknown* pcUnknown);

	void		Remove(int iIndex);
	BOOL		Remove(CUnknown* pcUnknown);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknown::Add(void)
{
	M*	pv;

	pv = gcUnknowns.Add<M>();
	CArrayCommonUnknown::Add(pv);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknown::Insert(int iIndex)
{
	M*	pv;

	pv = gcUnknowns.Add<M>();
	CArrayCommonUnknown::Insert(iIndex, pv);
	return pv;
}


#endif // __ARRAY_UNKNOWN_H__

