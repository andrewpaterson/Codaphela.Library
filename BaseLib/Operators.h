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
#ifndef __OPERATORS_H__
#define __OPERATORS_H__
#include "OperatorPlus.h"
#include "OperatorMinus.h"
#include "OperatorIncrement.h"
#include "OperatorDecrement.h"
#include "OperatorMultiply.h"
#include "OperatorDivide.h"
#include "OperatorRemainder.h"
#include "OperatorPower.h"

#include "OperatorEqual.h"
#include "OperatorNotEqual.h"
#include "OperatorLessThan.h"
#include "OperatorLessThanOrEqual.h"
#include "OperatorGreaterThan.h"
#include "OperatorGreatherThanOrEqual.h"

#include "OperatorBitwiseAnd.h"
#include "OperatorLeftShift.h"
#include "OperatorBitwiseNot.h"
#include "OperatorBitwiseOr.h"
#include "OperatorRightShift.h"
#include "OperatorBitwiseXOr.h"

#include "OperatorLogicalAnd.h"
#include "OperatorLogicalNot.h"
#include "OperatorLogicalOr.h"

#include "OperatorAssignment.h"

//Arithmetic functions: + - * / ++ -- ^ %
//Relational functions: > < == >= <= !=
//Bitwise functions: << >> ~ & | xor
//Logical functions: ! && ||


class COperators
{
public:
	COperatorPlus		mcAdd;
	COperatorMinus		mcSub;
	COperatorIncrement	mcIncrement;
	COperatorDecrement	mcDecrement;
	COperatorMultiply	mcMultiply;
	COperatorDivide		mcDivide;
	COperatorRemainder	mcRemainder;
	COperatorPower		mcPower;

	COperatorEqual					mcEqual;
	COperatorNotEqual				mcNotEqual;
	COperatorLessThan				mcLess;
	COperatorLessThanOrEqual		mcLessOrEqual;
	COperatorGreaterThan			mcGreater;
	COperatorGreatherThanOrEqual	mcGreaterOrEqual;

	COperatorBitwiseAnd	mcBwAnd;
	COperatorLeftShift	mcLeftShift;
	COperatorBitwiseNot	mcBwNot;
	COperatorBitwiseOr	mcBwOr;
	COperatorRightShift	mcRightShift;
	COperatorBitwiseXOr	mcBwXOr;

	COperatorLogicalAnd	mcLgAnd;
	COperatorLogicalNot	mcLgNot;
	COperatorLogicalOr	mcLgOr;

	COperatorAssignment	mcAssignment;

	void Init(void);
	void Kill(void);
};


extern COperators gcOperators;


void OperatorsInit(void);
void OperatorsKill(void);


#endif // __OPERATORS_H__
