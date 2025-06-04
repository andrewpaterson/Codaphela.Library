/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __CALCULATOR_PARSER_H__
#define __CALCULATOR_PARSER_H__
#include "Calculator.h"
#include "TextParser.h"


class CCalculatorParser
{
public:
	CCalculator*	mpcCalculator;
	CTextParser*	mpcParser;
	bool			mbSkipWhitespace;

public:
	void					Init(CCalculator* pcCalculator);
	void					Kill(void);

	CCalcObject*			Parse(char* szText);
	CCalcObject*			Parse(CTextParser* pcParser, bool bErrorOnBadExpression = true);

	CCalcExpression*		Expression(void);
	CCalcExpression*		Operand(void);
	CCalcOperator*			Operator(void);
	CCalcParentheses*		Parentheses(void);
	CCalcConstExpression*	Value(void);
	CCalcVariable*			Identifier(void);
};


#endif // __CALCULATOR_H__

