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
#ifndef __CALC_OPERATOR_H__
#define __CALC_OPERATOR_H__
#include "Define.h"
#include "Number.h"
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

	void	Init(void);
	void	Kill(void);
	void	Set(ECalcOperator eOp);
	void	Evaluate(CNumber* pcAnswer, CNumber* pcNumberLeft, CNumber* pcNumberRight);
	void	Evaluate(CNumber* pcAnswer, CNumber* pcNumber);
	bool	IsExpression(void);
	bool	IsOperator(void);
	bool	IsUnary(void);
	bool	IsBinary(void);
	bool	IsAmbiguous(void);
	void	Print(CChars* psz);
};


#endif // __CALC_ROOT_H__

