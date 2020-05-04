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
#ifndef __SET_UNKNOWN_H__
#define __SET_UNKNOWN_H__
#include "ArrayCommonUnknown.h"


class CSetUnknown : public CArrayCommonUnknown
{
BASE_FUNCTIONS(CSetUnknown);
public:
	void 		Init(void);
	void 		Init(int iChunkSize);
	void 		Kill(void);

	template<class M>
	M*			Add(void);
	void		Add(CUnknown* pcUnknown);

	BOOL		Remove(CUnknown* pcUnknown);
	void		RemoveDuringIteration(SSetIterator* psIter);

	CUnknown*	StartIteration(SSetIterator* psIter);
	CUnknown*	Iterate(SSetIterator* psIter);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CSetUnknown::Add(void)
{
	M*	pv;

	pv = gcUnknowns.Add<M>();
	Add(pv);
	return pv;
}


#endif // __SET_UNKNOWN_H__

