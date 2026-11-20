/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __C_P_P_FUNCTION_PACKS_H__
#define __C_P_P_FUNCTION_PACKS_H__
#include "BaseLib/ArrayTemplate.h"
#include "CPPSymbol.h"


//A function parameter pack (Args... args, a generic lambda's auto... args) and its elements' symbols: a variable for
//each argument it has in the function being written (CCPPUnit::FunctionPackElement).
struct SCPPFunctionPack
{
	CCPPSymbol*			pcPack;
	CArrayCPPSymbols	apcElements;
};


typedef CArrayTemplate<SCPPFunctionPack>	CArrayCPPFunctionPacks;


//The function parameter packs that can be expanded where a function is being written: its own and, in a lambda's
//body, those of the functions the lambda is in, whose elements it captures (CCPPUnit::AddParameters).
//[temp.variadic] 5, [expr.prim.lambda] 24
class CCPPFunctionPacks
{
protected:
	CArrayCPPFunctionPacks	masPacks;

public:
	void				Init(void);
	void				Kill(void);
	void				ReInit(void);

	SCPPFunctionPack*	Add(CCPPSymbol* pcPack);
	void				AddAll(CCPPFunctionPacks* pcPacks);
	SCPPFunctionPack*	Find(CCPPSymbol* pcPack);
};


#endif // __C_P_P_FUNCTION_PACKS_H__

