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
CONSTRUCTABLE(CArrayUnknown);
public:
	void 		Init(void);
	void 		Kill(void);

	template<class M>
	M*			Add(void);
	bool		Add(CUnknown* pcUnknown);

	template<class M>
	M*			Insert(size iIndex);
	bool		Insert(size iIndex, CUnknown* pcUnknown);

	CUnknown*	Get(size iIndex);
	void		Set(size iIndex, CUnknown* pcUnknown);

	void		Remove(size iIndex);
	bool		Remove(CUnknown* pcUnknown);
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
M* CArrayUnknown::Insert(size iIndex)
{
	M*	pv;

	pv = gcUnknowns.Add<M>();
	CArrayCommonUnknown::Insert(iIndex, pv);
	return pv;
}


#endif // __ARRAY_UNKNOWN_H__

