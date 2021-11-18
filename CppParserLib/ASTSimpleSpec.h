/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __A_S_T_SIMPLE_SPEC_H__
#define __A_S_T_SIMPLE_SPEC_H__
#include "BaseLib/Define.h"
#include "BaseLib/PointerFunctions.h"
#include "ASTDeclarator.h"
#include "ASTSpecifier.h"


enum ESimpleTypes
{
	ST_int,
	ST_long,
	ST_short,
	ST_char,
	ST_double,
	ST_float,
	ST_bool,
	ST_unsigned,
	ST_signed,
	ST_void,
	NUM_SIMPLE_SPECIFIERS,
	ST_compoundSimple,
	ST_notsimple,
	ST_invalid
};


enum ESimpleModifiers
{

	SM_const,
	SM_volatile,
	SM_static,
	SM_extern,
	SM_inline,
	SM_register,
	NUM_SIMPLE_MODIFIERS,
	SM_compoundSimple,
	SM_notsimple,
	SM_invalid
};


class CASTSimpleSpec : public CASTSpecifier
{
public:
	AST_CONSTRUCT(CASTSimpleSpec)

	ESimpleTypes	meSpec;

	void Init(void);
	void Set(ESimpleTypes eSpec);
	void Kill(void);
};


#endif // !__A_S_T_SIMPLE_SPEC_H__

