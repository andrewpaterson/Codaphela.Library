#ifndef __ARRAY_CHAR_H__
#define __ARRAY_CHAR_H__
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
#include "ArrayTemplatePrimitive.h"


//CArrayChar (characters counted in size, a CArrayTemplatePrimitive<char>) and CArrayLargeChar (counted in lsize, a
//CArrayLargeTemplatePrimitive<char>) are this, written once: PRIMITIVE is the class each derives from.
template<class PRIMITIVE>
class CArrayCharBase : public PRIMITIVE
{
public:
	void	AddList(char iStop, ...);
};


//ArrayChar.cpp instantiates them; no other unit compiles their functions.
extern template class CArrayCharBase<CArrayTemplatePrimitive<char> >;
extern template class CArrayCharBase<CArrayLargeTemplatePrimitive<char> >;


//The names the arrays are known by: a CArrayChar is a CArrayTemplatePrimitive<char>.
class CArrayChar : public CArrayCharBase<CArrayTemplatePrimitive<char> > {};
class CArrayLargeChar : public CArrayCharBase<CArrayLargeTemplatePrimitive<char> > {};


#endif // __ARRAY_CHAR_H__

