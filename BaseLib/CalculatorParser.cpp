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
#include "ErrorHandler.h"
#include "StackMemory.h"
#include "CalculatorParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorParser::Init(CCalculator* pcCalculator)
{
	memset(this, 0, sizeof(CCalculatorParser));
	mpcCalculator = pcCalculator;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCalculatorParser::Kill(void)
{
	mpcCalculator = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcObject* CCalculatorParser::Parse(char* szText)
{
	CCalcExpression*			pcExpression;
	CChars						sz;
	CCalcVariable*				pcIdentifier;
	TRISTATE					tAssignment;
	CCalculatorOperator*		pcAssignment;
	CCalcVariableDefinition*	pcVariableDefinition;

	mcParser.Init(szText);

	pcIdentifier = Identifier();
	if (pcIdentifier)
	{
		pcAssignment = mpcCalculator->GetAssignment();
		tAssignment = mcParser.GetExactCharacterSequence(pcAssignment->GetSymbol(), true);

		if (tAssignment != TRITRUE)
		{
			sz.Init("Cannot evaluate expression [");
			sz.Append(mcParser.Start());
			sz.Append("]\n");
			mpcCalculator->SetError(sz.Text());
			sz.Kill();

			mcParser.Kill();
			return NULL;
		}
	}

	pcExpression = Expression();
	if (!pcExpression)
	{
		sz.Init("Cannot evaluate expression [");
		sz.Append(mcParser.Start());
		sz.Append("]\n");
		mpcCalculator->SetError(sz.Text());
		sz.Kill();

		mcParser.Kill();
		return NULL;
	}

	if (pcIdentifier)
	{
		pcVariableDefinition = NewMalloc<CCalcVariableDefinition>();
		pcVariableDefinition->Set(pcIdentifier, pcExpression);

		mcParser.Kill();
		return pcVariableDefinition;
	}
	else
	{
		mcParser.Kill();
		return pcExpression;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcExpression* CCalculatorParser::Expression(void)
{
	bool					bFirst;
	CCalcOperator*			pcOperator;
	CCalcExpression*		pcOperand;
	CCalcExpression*		pcExpression;
	CCalcObjectArray	apcExpressions;

	apcExpressions.Init();
	bFirst = true;
	for (;;)
	{
		for (;;)
		{
			pcOperator = Operator();
			if (!pcOperator)
			{
				break;
			}
			apcExpressions.Add(pcOperator);
		}

		pcOperand = Operand();
		if (mpcCalculator->HasError())
		{
			return NULL;
		}

		if (!pcOperand && !pcOperator)
		{
			if (bFirst)
			{
				apcExpressions.Kill();
				return NULL;
			}
			else
			{
				pcExpression = mpcCalculator->BuildExpression(&apcExpressions);
				if (pcExpression)
				{
					apcExpressions.RemoveAll();
					apcExpressions.Kill();
					return pcExpression;
				}
				else
				{
					apcExpressions.Kill();
					return NULL;
				}
			}
		}
		apcExpressions.Add(pcOperand);
		bFirst = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcExpression* CCalculatorParser::Operand(void)
{
	CCalcParentheses*		pcParentheses;
	CCalcConstExpression*	pcConst;
	CCalcVariable*			pcIdentifier;

	pcParentheses = Parentheses();
	if (pcParentheses)
	{
		return pcParentheses;
	}

	pcConst = Value();
	if (pcConst)
	{
		return pcConst;
	}

	pcIdentifier = Identifier();
	if (pcIdentifier)
	{
		return pcIdentifier;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcVariable* CCalculatorParser::Identifier(void)
{
	TRISTATE				tResult;
	size					iLength;
	char*					sz;
	CCalcVariable*			pcIdentifier;
	CStackMemory<256>		cStack;
	CCalculatorVariables*	pcVariables;

	mcParser.PushPosition();
	tResult = mcParser.GetIdentifier(NULL, &iLength);
	mcParser.PopPosition();
	if (tResult == TRITRUE)
	{
		pcVariables = mpcCalculator->GetVariables();
		sz = (char*)cStack.Init(iLength + 1);
		mcParser.GetIdentifier(sz);
		pcIdentifier = NewMalloc<CCalcVariable>();
		pcIdentifier->Init(mpcCalculator->GetErrors());
		pcIdentifier->Set(sz, pcVariables);
		cStack.Kill();
		return pcIdentifier;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcConstExpression* CCalculatorParser::Value(void)
{
	TRISTATE				tResult;
	CNumber					cNumber;
	uint64					ulli;
	CCalcConstExpression*	pcConst;

	tResult = mcParser.GetHexadecimal(&ulli);
	if (tResult == TRITRUE)
	{
		pcConst = NewMalloc<CCalcConstExpression>();
		pcConst->Init(mpcCalculator->GetErrors());
		pcConst->SetValue(cNumber.Init((int32)ulli));  //This needs to properly take an int64.
		return pcConst;
	}

	tResult = mcParser.GetNumber(&cNumber);
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter('L', false);
		if (tResult == false)
		{
			tResult = mcParser.GetExactCharacter('l', false);
		}
		pcConst = NewMalloc<CCalcConstExpression>();
		pcConst->Init(mpcCalculator->GetErrors());
		pcConst->SetValue(&cNumber);
		return pcConst;
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcOperator* CCalculatorParser::Operator(void)
{
	TRISTATE					tResult;
	size						i;
	char*						szSimpleOp;
	ECalcOperator				eOp;
	CArrayCalculatorOperators*	pacOperators;
	size						uiNumOperators;
	CCalcOperator*				pcOperator;
	CCalculatorOperator*		pcDefinition;

	mcParser.PushPosition();

	pacOperators = mpcCalculator->GetOperators();
	uiNumOperators = pacOperators->NumElements();

	for (i = 0; i < uiNumOperators; i++)
	{
		pcDefinition = pacOperators->Get(i);
		szSimpleOp = pcDefinition->GetSymbol();
		tResult = mcParser.GetExactCharacterSequence(szSimpleOp);
		if (tResult == TRITRUE)
		{
			eOp = pcDefinition->GetOperator();
			mcParser.PassPosition();
			pcOperator = NewMalloc<CCalcOperator>();
			pcOperator->Init(mpcCalculator->GetErrors());
			pcOperator->Set(eOp);
			return pcOperator;
		}
		else if (tResult == TRIERROR)
		{
			mcParser.PopPosition();
			return NULL;
		}
	}

	mcParser.PopPosition();
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCalcParentheses* CCalculatorParser::Parentheses(void)
{
	TRISTATE			tResult;
	CCalcParentheses*	pcParentheses;
	CCalcExpression*	pcExpression;

	mcParser.PushPosition();
	tResult = mcParser.GetExactCharacter('(');
	if (tResult == TRITRUE)
	{
		tResult = mcParser.GetExactCharacter(')');
		if (tResult == TRITRUE)
		{
			mcParser.PassPosition();
			pcParentheses = NewMalloc<CCalcParentheses>();
			pcParentheses->Init(mpcCalculator->GetErrors());
			pcParentheses->SetExpression(NULL);
			return pcParentheses;
		}
		else
		{
			pcExpression = Expression();
			if (pcExpression)
			{
				tResult = mcParser.GetExactCharacter(')');
				if (tResult == TRITRUE)
				{
					mcParser.PassPosition();
					pcParentheses = NewMalloc<CCalcParentheses>();
					pcParentheses->Init(mpcCalculator->GetErrors());
					pcParentheses->SetExpression(pcExpression);
					return pcParentheses;
				}
			}
			SafeKill(pcExpression);
			mcParser.PopPosition();
			return NULL;
		}
	}
	else
	{
		mcParser.PopPosition();
		return NULL;
	}
}


