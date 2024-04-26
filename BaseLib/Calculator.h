/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__
#include "Define.h"
#include "Number.h"
#include "ArrayChars.h"
#include "ArrayInt.h"
#include "ArrayIntAndPointer.h"
#include "TextParser.h"
#include "CalcOperator.h"
#include "CalcBinaryExpression.h"
#include "CalcUnaryExpression.h"
#include "CalcConstExpression.h"
#include "CalcParentheses.h"


class CCalculator
{
public:
	CArrayChars		mszOperators;
	CTextParser		mcParser;
	CArrayInt		maiPrecedence;
	CChars			mszError;
	bool			mbUseUserError;

	void		Init(void);
	void		Init(bool bUseUserError);
	void		Kill(void);
	CNumber		Eval(char* szText);
	bool		Expression(CCalcExpression** ppcExpression);
	bool		Operand(CCalcExpression** ppcExpression);
	bool		Operator(CCalcOperator** pcOperator);
	bool		Parentheses(CCalcParentheses** ppcParentheses);
	bool		Value(CCalcConstExpression** ppcConst);
	bool		Identifier(CCalcConstExpression** ppcConst);
	bool		BuildExpression(CCalcExpression** ppcExpression, CArrayIntAndPointer* pcArray);
	int			GetMinPrecedence(CArrayIntAndPointer* pcArray);
	void		ResolveAmbiguity(CCalcOperator* pcOperator, bool bIsUnary);
	bool		SetError(CChars* pszFirst, CArrayIntAndPointer* pcArray, CCalcExpression** ppcExpression, char* szLeft, char* szMiddle, char* szRight);
	void		SetError(char* szError);
	void		Print(CChars* psz, CArrayIntAndPointer* pcArray);
	void		Dump(CArrayIntAndPointer* pcArray);
	bool		HasError(void);
	char*		GetError(void);
};


#endif // __CALCULATOR_H__

