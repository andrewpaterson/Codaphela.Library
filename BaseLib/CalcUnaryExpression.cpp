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
#include "PointerFunctions.h"
#include "CalculatorError.h"
#include "CalcUnaryExpression.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcUnaryExpression::Init(CCalculatorError* pcError)
{
	mpcError = pcError;
	mpcExpression = NULL;
	mpcOp = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcUnaryExpression::Kill(void)
{
	SafeKill(mpcExpression);
	SafeKill(mpcOp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber	CCalcUnaryExpression::Evaluate(void)
{
	CNumber	cNumber;
	CNumber cAnswer;

	cNumber = mpcExpression->Evaluate();

	mpcOp->Evaluate(&cAnswer, &cNumber);

	return cAnswer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcUnaryExpression::Set(CCalcExpression* pcExpression, CCalcOperator* pcOp)
{
	mpcExpression = pcExpression;
	mpcOp = pcOp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalcUnaryExpression::IsUnaryExpression(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcUnaryExpression::Print(CChars* psz)
{
	psz->Append("(");
	mpcOp->Print(psz);
	mpcExpression->Print(psz);
	psz->Append(")");
}

