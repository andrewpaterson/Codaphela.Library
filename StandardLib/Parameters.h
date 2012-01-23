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
#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__


struct SParams
{
	int		_iParamSize;
};


#define PARAM1(f, t1, a1) \
struct f : SParams { \
	t1 a1; \
	f::f(t1 a1) \
{ _iParamSize=sizeof(f); this->a1 = a1; } \
};


#define PARAM2(f, t1, a1, t2, a2) \
struct f : SParams { \
	t1 a1; t2 a2; \
	f::f(t1 a1, t2 a2) \
{ _iParamSize=sizeof(f); this->a1 = a1; this->a2 = a2; } \
};


#define PARAM3(f, t1, a1, t2, a2, t3, a3) \
struct f : SParams { \
	t1 a1; t2 a2; t3 a3; \
	f::f(t1 a1, t2 a2, t3 a3) \
{ _iParamSize=sizeof(f); this->a1 = a1; this->a2 = a2; this->a3 = a3; } \
};


#define PARAM4(f, t1, a1, t2, a2, t3, a3, t4, a4) \
struct f : SParams { \
	t1 a1; t2 a2; t3 a3; t4 a4; \
	f::f(t1 a1, t2 a2, t3 a3, t4 a4) \
{ _iParamSize=sizeof(f); this->a1 = a1; this->a2 = a2; this->a3 = a3; this->a4 = a4; } \
};



#endif // __PARAMETERS_H__

