/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ELEMENT_TYPES_H__
#define __ELEMENT_TYPES_H__
#include <math.h>


enum EElement
{
	ET_Error,
	ET_Unknown,
	ET_Text,
	ET_Identifier,
	ET_Char,
	ET_Node,
	ET_Definition,
	ET_PrimitiveType,
	ET_End,
	PT_IntegerType,
	PT_RealType,
	PT_StringType,
	PT_CharType,
	NT_Single,
	NT_Ordered,
	NT_Exclusive,
	NT_Inclusive,
	NM_Normal,
	NM_Inverse,
	NM_NotANode,
	NN_Normal,
	NN_Necessary,
	NN_Empty,
	NUM_ELEMENT_TYPES,
	EELEMENT_FOUR_BYTE = 0xffffffff
};


#endif //__ELEMENT_TYPES_H__