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
#ifndef __CALCULATOR_H__
#define __CALCULATOR_H__
#include "Define.h"
#include "Number.h"
#include "ArrayChars.h"
#include "TextParser.h"
#include "CalcOperator.h"
#include "CalcBinaryExpression.h"
#include "CalcUnaryExpression.h"
#include "CalcConstExpression.h"
#include "CalcParentheses.h"
#include "CalcVariable.h"
#include "CalculatorOperator.h"
#include "CalcObjectArray.h"


class CCalculator
{
public:
	CArrayCalculatorOperators	macOperators;
	CCalculatorOperator			mcAssignment;

	CChars						mszError;
	bool						mbUseUserError;

	void						Init(void);
	void						Init(bool bUseUserError);
	void						Kill(void);

	CNumber						Eval(CCalcExpression* pcExpression);

	CCalcExpression*			BuildExpression(CCalcuObjectArray* pcArray);
	size						GetMinPrecedence(CCalcuObjectArray* papcExpressions);
	ECalcOperator				ResolveAmbiguity(ECalcOperator eOperator, bool bIsUnary);
	bool						SetError(CChars* pszFirst, CCalcuObjectArray* papcExpressions, char* szLeft, char* szMiddle, char* szRight);
	void						SetError(char* szError);
	void						Print(CChars* psz, CCalcuObjectArray* papcExpressions);
	void						Print(CChars* psz, CCalcObject* pcExpression);
	bool						HasError(void);
	char*						GetError(void);

	CArrayCalculatorOperators*	GetOperators(void);
	CCalculatorOperator*		GetAssignment(void);

	void						Dump(CCalcuObjectArray* papcExpressions);
	void						Dump(CCalcObject* pcExpression);
};


#endif // __CALCULATOR_H__

