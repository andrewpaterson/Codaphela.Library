/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __CONSTRUCTOR_UNKNOWN_H__
#define __CONSTRUCTOR_UNKNOWN_H__
#include "BaseLib/ConstructorCall.h"
#include "Unknown.h"
#include "MapStringUnknown.h"


class CConstructorUnknown : public CUnknown
{
BASE_FUNCTIONS(CConstructorUnknown);
protected:
	CMapStringUnknown	mcConstructorObjects;

public:
	void		Init(void);
	void		Kill(void);

	void		AddUnknown(CUnknown* pcUnknown);
	BOOL		Construct(CUnknown* pcDest, char* szName);
	CUnknown*	GetUnknown(char* szName);  //The only thing initialised on this returned Unknown is the virtual function table.

	template<class M>
	void		Add(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CConstructorUnknown::Add(void)
{
	M* pvM = NewMalloc<M>();

	AddUnknown(pvM);
}


#endif // __CONSTRUCTOR_UNKNOWN_H__

