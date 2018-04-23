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
#ifndef __CONSTRUCTOR_CALL_H__
#define __CONSTRUCTOR_CALL_H__
#include <new> 
#include <stdlib.h>


template<class M>
M* NewMalloc(void)
{
	M*	pv;

	pv = (M*)malloc(sizeof(M));
	memset(pv, 0, sizeof(M));
	new(pv) M();

	return pv;
}


template<class M>
class CPostMalloc
{
public:
	M* PostMalloc(M* pv);
};


template<class M>
M* CPostMalloc<M>::PostMalloc(M* pv)
{
	memset(pv, 0, sizeof(M));
	new(pv) M();
	return pv;
}


#endif // __CONSTRUCTOR_CALL_H__

