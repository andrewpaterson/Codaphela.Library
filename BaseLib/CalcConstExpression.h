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
#ifndef __CALC_CONST_EXPRESSION_H__
#define __CALC_CONST_EXPRESSION_H__
#include "CalculatorError.h"
#include "CalcExpression.h"


class CCalcConstExpression : public CCalcExpression
{
public:
	CNumber		mcValue;

	void		Init(CCalculatorError* pcError);
	void		Kill(void);
	CNumber		Evaluate(void);
	void		SetValue(CNumber* pcNumber);
	void		Print(CChars* psz);
	bool		IsConstExpression(void);
	CNumber*	GetNumber(void);
};


#endif // __CALC_CONST_EXPRESSION_H__

