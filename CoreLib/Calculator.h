/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__
#include "BaseLib/Define.h"
#include "BaseLib/Number.h"
#include "BaseLib/ArrayString.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayPointer.h"
#include "TextParser.h"
#include "CalcOperator.h"
#include "CalcBinaryExpression.h"
#include "CalcUnaryExpression.h"
#include "CalcConstExpression.h"
#include "CalcParentheses.h"


class CCalculator
{
public:
	CArrayString	mszOperators;
	CTextParser		mcParser;
	CArrayInt		maiPrecedence;

	void		Init(void);
	void		Kill(void);
	CNumber		Eval(char* szText);
	BOOL		Expression(CCalcExpression** ppcExpression);
	BOOL		Operand(CCalcExpression** ppcExpression);
	BOOL		Operator(CCalcOperator** pcOperator);
	BOOL		Parentheses(CCalcParentheses** ppcParentheses);
	BOOL		Value(CCalcConstExpression** ppcConst);
	BOOL		Identifier(CCalcConstExpression** ppcConst);
	void		BuildExpression(CCalcExpression** ppcExpression, CArrayPointer* pcArray);
	int			GetMinPrecedence(CArrayPointer* pcArray);
	void		ResolveAmbiguity(CCalcOperator* pcOperator, BOOL bIsUnary);
};


#endif // __CALCULATOR_H__

