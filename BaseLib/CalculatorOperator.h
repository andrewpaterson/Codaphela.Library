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
#ifndef __CALCULATOR_OPERATOR_H__
#define __CALCULATOR_OPERATOR_H__
#include "Define.h"
#include "Chars.h"
#include "CalcOperator.h"
#include "ArrayTemplate.h"


class CCalculatorOperator
{
protected:
	CChars			mszSymbol;
	ECalcOperator	meOperator;
	uint16			miPrecedence;

public:
	void Init(char* szSymbol, ECalcOperator eOperator, uint16 iPrecedence);
	void Init(void);
	void Kill(void);

	char*			GetSymbol(void);
	ECalcOperator	GetOperator(void);
	uint16			GetPrecedence(void);
};


typedef CArrayTemplate<CCalculatorOperator> __CArrayCalculatorOperators;


class CArrayCalculatorOperators : public __CArrayCalculatorOperators
{
public:
	void					Kill(void);

	CCalculatorOperator*	Add(char* szSymbol, ECalcOperator eOperator, uint16 iPrecedence);
	void					Add(void);
};


#endif // __CALCULATOR_OPERATOR_H__


