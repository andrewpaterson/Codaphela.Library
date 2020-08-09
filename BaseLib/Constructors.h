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
#ifndef __CONSTRUCTORS_H__
#define __CONSTRUCTORS_H__
#include "ConstructorCall.h"
#include "MapStringBlock.h"


class CConstructors
{
protected:
	CMapStringBlock		mcConstructors;

public:
	void	Init(void);
	void	Kill(void);

	template<class M>
	void	Add(char* szConstructorName);
	template<class M>
	void	Add(void);

	void*	Construct(const char* szName, CMallocator* pcMalloc);
	int		NumConstructors(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CConstructors::Add(char* szConstructorName)
{
	M*		pvM;
	int		iSize;

	iSize = sizeof(M);
	pvM = (M*)mcConstructors.Put(szConstructorName, iSize);
	memset(pvM, 0, iSize);
	new(pvM) M();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CConstructors::Add(void)
{
	M*		pvM;
	int		iSize;
	M		m;

	iSize = sizeof(M);
	pvM = (M*)mcConstructors.Put(m.ClassName(), iSize);
	memset(pvM, 0, iSize);
	new(pvM) M();
}


#endif // __CONSTRUCTORS_H__

