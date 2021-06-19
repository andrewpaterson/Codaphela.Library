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
#ifndef __CALC_OPERATOR_H__
#define __CALC_OPERATOR_H__
#include "BaseLib/Define.h"
#include "BaseLib/Number.h"
#include "CalcObject.h"


enum ECalcOperator
{
	CO_Increment,
	CO_Decrement,
	CO_EqualTo,
	CO_NotEqualTo,
	CO_GreaterThanEqualTo,
	CO_LessThanEqualTo,
	CO_LogicalOr,
	CO_LogicalAnd,
	CO_LeftShift,
	CO_RightShift,
	CO_Add,
	CO_Subtract,
	CO_Multiply,
	CO_Divide,
	CO_Modulus,
	CO_LogicalNot,
	CO_BitwiseAnd,
	CO_BitwiseOr,
	CO_BitwiseXor,
	CO_LessThan,
	CO_GreaterThan,
	CO_BitwiseNot,
	CO_UnaryAdd,
	CO_UnarySubtract,
	CO_invalid,
};


class CCalcOperator : public CCalcObject
{
public:
	ECalcOperator	meOp;

	void Init(void);
	void Kill(void);
	void Set(ECalcOperator eOp);
	void Evaluate(CNumber* pcAnswer, CNumber* pcNumberLeft, CNumber* pcNumberRight);
	void Evaluate(CNumber* pcAnswer, CNumber* pcNumber);
	BOOL IsExpression(void);
	BOOL IsOperator(void);
	BOOL IsUnary(void);
	BOOL IsBinary(void);
	BOOL IsAmbiguous(void);
};


#endif // !__CALC_ROOT_H__


