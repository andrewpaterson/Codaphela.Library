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


#define CONSTRUCT(mem, type)	{ type _o_; memcpy(mem, &_o_, sizeof(type)); }


template<class M>
M* NewMalloc(void)
{
	M*	pv;
	M	v;

	pv = (M*)malloc(sizeof(M));

	memcpy(pv, &v, sizeof(M));

	return pv;
}


#define Malloc(type)	NewMalloc<type>();


#endif // __CONSTRUCTOR_CALL_H__

