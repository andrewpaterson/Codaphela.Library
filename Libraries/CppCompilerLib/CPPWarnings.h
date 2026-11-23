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
#ifndef __C_P_P_WARNINGS_H__
#define __C_P_P_WARNINGS_H__
#include "BaseLib/ArrayVoidPtr.h"
#include "BaseLib/PrimitiveTypes.h"
#include "CPPSyntaxes.h"


class CCPPUnit;
class CCPPSymbol;
struct SCPP65Type;


//Warns of what is right where an int is 32 bits and wrong with the W65C816's 16 bit one (CCPPUnit::GetWarner): a
//conversion or a product that loses bits, arithmetic widened after it overflowed, a shift by the type's width or
//more, a comparison that is always or never true, and a loop counter narrower than what it is compared with.
class CCPPWarnings
{
protected:
	CCPPUnit*				mpcUnit;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	void					ConversionOverflow(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	void					WideningArithmetic(CCPPSyntax* pcExpression, SCPP65Type* psFrom, SCPP65Type* psTo);
	void					ShiftCount(CCPPSyntaxBinaryExpression* pcBinary, int iBits);
	CCPPSyntaxBinaryExpression*	NarrowMultiply(CCPPSyntax* pcExpression, SCPP65Type* psType);
	void					LoopCounterWidth(CCPPSyntaxForStatement* pcFor);
	void					LoopCounters(CCPPSyntax* pcIncrement, CArrayVoidPtr* papcCounters);
	void					LoopCounterComparisons(CCPPSyntax* pcCondition, CArrayVoidPtr* papcCounters);
	CCPPSymbol*				LoopCounterSymbol(CCPPSyntax* pcExpression);
	void					ComparisonRange(CCPPSyntaxBinaryExpression* pcBinary);
	void					ComparisonAtLimit(CCPPSyntaxBinaryExpression* pcBinary, CCPPSyntax* pcConstant, SCPP65Type* psVariable, int64 lliValue, int64 lliMin, int64 lliMax);
};


#endif // __C_P_P_WARNINGS_H__

