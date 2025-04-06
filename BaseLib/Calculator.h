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
#include "CalcObjectArray.h"
#include "CalculatorVariables.h"
#include "CalculatorSymbols.h"
#include "CalculatorError.h"


class CCalculator
{
public:
	CCalculatorSymbols*			mpcSymbols;

	CCalculatorVariables		mcVariables;
	CCalculatorError			mcErrors;

	bool						mbSkipWhitespace;

	void						Init(CCalculatorSymbols* pcSymbols);
	void						Init(CCalculatorSymbols* pcSymbols, bool bUseUserError);
	void						Init(CCalculatorSymbols* pcSymbols, bool bSkipWhitespace, bool bUseUserError);
	void						Kill(void);

	CNumber						Eval(CCalcExpression* pcExpression);
	CNumber						Eval(void);

	CCalcExpression*			BuildExpression(CCalcObjectArray* pcArray);
	CCalcExpression*			CleanExpression(CCalcExpression* pcExpression);
	void						RecurseCleanExpression(CCalcExpression* pcExpression);
	void						StripParentheses(CCalcExpression** ppcExpression);
	void						ConvertToUnaryOperators(CCalcObjectArray* papcExpressions);
	size						GetMinPrecedence(CCalcObjectArray* papcExpressions);
	bool						SetError(CChars* pszFirst, CCalcObjectArray* papcExpressions, char* szLeft, char* szMiddle, char* szRight);
	void						SetError(char* szError);
	void						Print(CChars* psz, CCalcObjectArray* papcExpressions);
	void						Print(CChars* psz, CCalcObject* pcExpression);
	void						Print(CChars* psz);
	bool						HasError(void);
	char*						GetError(void);
	void						ClearError(void);

	CArrayCalculatorOperators*	GetOperators(void);
	CCalculatorOperator*		GetAssignment(void);
	CCalculatorVariables*		GetVariables(void);
	CCalculatorError*			GetErrors(void);
	CCalculatorSymbols*			GetSymbols(void);
	bool						IsSkipWhitespace(void);

	void						Add(CCalcVariableDefinition* pcVariableDefinition);
	void						Add(CCalcExpression* pcExpression);
	void						Add(CCalcObject* pcObject);

	void						Dump(CCalcObjectArray* papcExpressions);
	void						Dump(CCalcObject* pcExpression);
};


#endif // __CALCULATOR_H__

