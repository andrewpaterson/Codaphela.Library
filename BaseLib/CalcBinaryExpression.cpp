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
#include "CalcBinaryExpression.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcBinaryExpression::Init(CCalculatorError* pcError)
{
	mpcError = pcError;
	mpcLeft = NULL;
	mpcOp = NULL;
	mpcRight = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcBinaryExpression::Kill(void)
{
	SafeKill(mpcLeft);
	SafeKill(mpcOp);
	SafeKill(mpcRight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNumber	CCalcBinaryExpression::Evaluate(void)
{
	CNumber	cNumberLeft;
	CNumber	cNumberRight;
	CNumber cAnswer;

	cNumberLeft = mpcLeft->Evaluate();
	cNumberRight = mpcRight->Evaluate();

	//To do short circuiting you actually need to do some sort of partial evaluation here.
	mpcOp->Evaluate(&cAnswer, &cNumberLeft, &cNumberRight);

	return cAnswer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcBinaryExpression::Set(CCalcExpression* pcLeft, CCalcOperator* pcOp, CCalcExpression* pcRight, bool bPrintSpace)
{
	mpcLeft = pcLeft;
	mpcOp = pcOp;
	mpcRight = pcRight;
	mbPrintSpace = bPrintSpace;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCalcBinaryExpression::IsBinaryExpression(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalcBinaryExpression::Print(CChars* psz)
{
	psz->Append("(");
	mpcLeft->Print(psz);
	if (mbPrintSpace)
		psz->Append(" ");
	mpcOp->Print(psz);
	if (mbPrintSpace)
		psz->Append(" ");
	mpcRight->Print(psz);
	psz->Append(")");
}

