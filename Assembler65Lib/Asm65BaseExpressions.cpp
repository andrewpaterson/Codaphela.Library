/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "Asm65BaseAssembler.h"


//////////////////////////////////////////////////////////////////////////
//
// Expressions are 64 bit and C like, lowest precedence first:
//   |   ^   &   << >>   + -   * / %   unary - + ~ !
// A primary is a number, a character, a symbol, C (the program counter, only in
// branch, bra and brsr targets) or a bracketed expression.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Expression(SAsm65Value* psValue)
{
	SAsm65Token*	psToken;
	bool			bResult;

	psToken = Peek();
	miExpressionDepth++;
	bResult = BitwiseOr(psValue);
	miExpressionDepth--;

	//Counts the section addresses a statement must use (see UseTerm).
	if (bResult && (miExpressionDepth == 0) && psValue->bResolved && (psValue->iTerms != 0))
	{
		muiStatementTerms++;
		if (mpsTermToken == NULL)
		{
			mpsTermToken = psToken;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65BaseAssembler::ClearValue(SAsm65Value* psValue)
{
	psValue->iValue = 0;
	psValue->bResolved = true;
	psValue->bLabel = false;
	psValue->iProgramCounter = 0;
	psValue->iTerms = 0;
	psValue->iSection = ASM65_SECTION_ABSOLUTE;
	psValue->uiTermToken = 0;
	psValue->uiForm = ASM65RF_Plain;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::BitwiseOr(SAsm65Value* psValue)
{
	SAsm65Value		sRight;
	SAsm65Token*	psOperator;

	if (!BitwiseXor(psValue))
	{
		return false;
	}
	for (psOperator = Peek(); Accept(AP65_Or); psOperator = Peek())
	{
		if (!BitwiseXor(&sRight))
		{
			return false;
		}
		if (!Combine(psValue, &sRight, psOperator))
		{
			return false;
		}
		psValue->iValue |= sRight.iValue;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::BitwiseXor(SAsm65Value* psValue)
{
	SAsm65Value		sRight;
	SAsm65Token*	psOperator;

	if (!BitwiseAnd(psValue))
	{
		return false;
	}
	for (psOperator = Peek(); Accept(AP65_Xor); psOperator = Peek())
	{
		if (!BitwiseAnd(&sRight))
		{
			return false;
		}
		if (!Combine(psValue, &sRight, psOperator))
		{
			return false;
		}
		psValue->iValue ^= sRight.iValue;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::BitwiseAnd(SAsm65Value* psValue)
{
	SAsm65Value		sRight;
	SAsm65Token*	psOperator;

	if (!ShiftExpression(psValue))
	{
		return false;
	}
	for (psOperator = Peek(); Accept(AP65_And); psOperator = Peek())
	{
		if (!ShiftExpression(&sRight))
		{
			return false;
		}
		if (FormTerm(psValue, &sRight, 0xFFFF, ASM65RF_Low16))
		{
			continue;
		}
		if (!Combine(psValue, &sRight, psOperator))
		{
			return false;
		}
		psValue->iValue &= sRight.iValue;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Shifts by 64 or more give 0 (or -1 shifting a negative value right).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::ShiftExpression(SAsm65Value* psValue)
{
	SAsm65Value		sRight;
	SAsm65Token*	psOperator;
	bool			bLeft;

	if (!Additive(psValue))
	{
		return false;
	}
	for (;;)
	{
		psOperator = Peek();
		if (Accept(AP65_ShiftLeft))
		{
			bLeft = true;
		}
		else if (Accept(AP65_ShiftRight))
		{
			bLeft = false;
		}
		else
		{
			return true;
		}

		if (!Additive(&sRight))
		{
			return false;
		}
		if (!bLeft && FormTerm(psValue, &sRight, 16, ASM65RF_Bank))
		{
			continue;
		}
		if (!Combine(psValue, &sRight, psOperator))
		{
			return false;
		}
		if ((sRight.iValue < 0) || (sRight.iValue >= 64))
		{
			psValue->iValue = (bLeft || (psValue->iValue >= 0)) ? 0 : -1;
		}
		else if (bLeft)
		{
			psValue->iValue = (int64)((uint64)psValue->iValue << sRight.iValue);
		}
		else
		{
			psValue->iValue = psValue->iValue >> sRight.iValue;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Stops before "+ register" so addresses like label + X can follow.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Additive(SAsm65Value* psValue)
{
	SAsm65Value		sRight;
	SAsm65Token*	psOperator;
	bool			bAdd;

	if (!Multiplicative(psValue))
	{
		return false;
	}
	for (;;)
	{
		if (Peek()->Is(AP65_Plus))
		{
			if (IsRegisterName(Peek(1)))
			{
				return true;
			}
			bAdd = true;
		}
		else if (Peek()->Is(AP65_Minus))
		{
			bAdd = false;
		}
		else
		{
			return true;
		}

		psOperator = Next();
		if (!Multiplicative(&sRight))
		{
			return false;
		}
		if ((psValue->uiForm != ASM65RF_Plain) || (sRight.uiForm != ASM65RF_Plain))
		{
			return Error(psOperator, "Nothing can be added to an address's bank or low 16 bits; add to the address first");
		}
		if (!AddTerms(psValue, &sRight, bAdd, psOperator))
		{
			return false;
		}
		psValue->iValue = bAdd ? (psValue->iValue + sRight.iValue) : (psValue->iValue - sRight.iValue);
		psValue->iProgramCounter = bAdd ? (psValue->iProgramCounter + sRight.iProgramCounter) : (psValue->iProgramCounter - sRight.iProgramCounter);
		psValue->bResolved &= sRight.bResolved;
		psValue->bLabel |= sRight.bLabel;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Multiplicative(SAsm65Value* psValue)
{
	SAsm65Value			sRight;
	SAsm65Token*		psOperator;

	if (!Unary(psValue))
	{
		return false;
	}
	for (;;)
	{
		psOperator = Peek();
		if (!psOperator->Is(AP65_Multiply) && !psOperator->Is(AP65_Divide) && !psOperator->Is(AP65_Modulus))
		{
			return true;
		}

		Next();
		if (!Unary(&sRight))
		{
			return false;
		}
		if (!Combine(psValue, &sRight, psOperator))
		{
			return false;
		}
		if (psOperator->Is(AP65_Multiply))
		{
			psValue->iValue *= sRight.iValue;
		}
		else if (sRight.iValue == 0)
		{
			if (sRight.bResolved)
			{
				return Error(psOperator, "Division by zero");
			}
			psValue->iValue = 0;
		}
		else if (psOperator->Is(AP65_Divide))
		{
			psValue->iValue /= sRight.iValue;
		}
		else
		{
			psValue->iValue %= sRight.iValue;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Unary(SAsm65Value* psValue)
{
	SAsm65Token*	psOperator;

	if (Accept(AP65_Minus))
	{
		if (!Unary(psValue))
		{
			return false;
		}
		if (psValue->uiForm != ASM65RF_Plain)
		{
			return Error(mcTokeniser.Get(muiToken - 1), "An address's bank or low 16 bits cannot be negated");
		}
		psValue->iValue = -psValue->iValue;
		psValue->iProgramCounter = -psValue->iProgramCounter;
		psValue->iTerms = -psValue->iTerms;
		return true;
	}
	else if (Accept(AP65_Plus))
	{
		return Unary(psValue);
	}
	else if (Accept(AP65_Not))
	{
		psOperator = mcTokeniser.Get(muiToken - 1);
		if (!Unary(psValue))
		{
			return false;
		}
		if (psValue->iProgramCounter != 0)
		{
			return Error(psOperator, "C can only have an offset added or subtracted");
		}
		if (psValue->iTerms != 0)
		{
			return Error(psOperator, "A section address can only have a constant added or subtracted");
		}
		psValue->iValue = ~psValue->iValue;
		return true;
	}
	else if (Accept(AP65_LogicalNot))
	{
		psOperator = mcTokeniser.Get(muiToken - 1);
		if (!Unary(psValue))
		{
			return false;
		}
		if (psValue->iProgramCounter != 0)
		{
			return Error(psOperator, "C can only have an offset added or subtracted");
		}
		if (psValue->iTerms != 0)
		{
			return Error(psOperator, "A section address can only have a constant added or subtracted");
		}
		psValue->iValue = !psValue->iValue;
		return true;
	}
	return Primary(psValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Primary(SAsm65Value* psValue)
{
	SAsm65Token*	psToken;
	SAsm65Symbol*	psSymbol;

	psToken = Peek();
	ClearValue(psValue);

	if (psToken->eType == AT65_Number)
	{
		Next();
		psValue->iValue = psToken->iValue;
		return true;
	}
	else if (psToken->Is(AP65_LeftRound))
	{
		Next();
		if (!Expression(psValue))
		{
			return false;
		}
		return Expect(AP65_RightRound, "')'");
	}
	else if (psToken->IsIdentifier("C"))
	{
		if (!mbRelative)
		{
			return Error(psToken, "C is only allowed in branch, bra and brsr targets");
		}
		Next();
		psValue->iProgramCounter = 1;
		return true;
	}
	else if (psToken->eType == AT65_Identifier)
	{
		if (IsRegisterName(psToken))
		{
			return Error(psToken, "A register cannot be used in an expression");
		}
		Next();
		psValue->bLabel = true;
		psSymbol = mcSymbols.Get(psToken->szText, psToken->uiLength);
		if ((psSymbol == NULL) || !psSymbol->bResolved)
		{
			psValue->bResolved = false;
			if (mbFinal)
			{
				return Error(psToken, "Undefined symbol");
			}
			if (mbObject)
			{
				mcUndefined.Add((int)TokenIndex(psToken));
			}
			return true;
		}
		psValue->iValue = psSymbol->iValue;
		if (psSymbol->bLabel && (psSymbol->uiPass < muiPass) && (psSymbol->iSection == (mbObject ? (int32)muiSection : ASM65_SECTION_ABSOLUTE)))
		{
			//A label further on has moved at least as far as the last one bound, as instructions only grow.
			psValue->iValue += miLabelShift;
		}
		if (psSymbol->iSection != ASM65_SECTION_ABSOLUTE)
		{
			psValue->iTerms = 1;
			psValue->iSection = psSymbol->iSection;
			psValue->uiTermToken = psSymbol->uiTermToken;
		}
		return true;
	}
	return Error(psToken, "Expected a value");
}


//////////////////////////////////////////////////////////////////////////
//
// A section address >> 16 (its bank) or & $FFFF (its offset in the bank) is a
// relocation of that form.  False, changing nothing, if this isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::FormTerm(SAsm65Value* psValue, SAsm65Value* psRight, int64 iOperand, uint8 uiForm)
{
	if ((psValue->iTerms != 1) || (psValue->uiForm != ASM65RF_Plain) || (psValue->iProgramCounter != 0) ||
		(psRight->iTerms != 0) || !psRight->bResolved || (psRight->iProgramCounter != 0) || (psRight->iValue != iOperand))
	{
		return false;
	}
	psValue->uiForm = uiForm;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Operators other than + and - cannot use C, section addresses or a formed address.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::Combine(SAsm65Value* psValue, SAsm65Value* psRight, SAsm65Token* psOperator)
{
	if ((psValue->iProgramCounter != 0) || (psRight->iProgramCounter != 0))
	{
		return Error(psOperator, "C can only have an offset added or subtracted");
	}
	if ((psValue->iTerms != 0) || (psRight->iTerms != 0))
	{
		return Error(psOperator, "A section address can only have a constant added or subtracted");
	}
	psValue->bResolved &= psRight->bResolved;
	psValue->bLabel |= psRight->bLabel;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds or subtracts psRight's section addresses.  Addresses in one section subtract
// to a constant; addresses in different sections cannot be combined.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::AddTerms(SAsm65Value* psValue, SAsm65Value* psRight, bool bAdd, SAsm65Token* psOperator)
{
	int	iRight;

	if (psRight->iTerms == 0)
	{
		return true;
	}

	iRight = bAdd ? psRight->iTerms : -psRight->iTerms;
	if (psValue->iTerms == 0)
	{
		psValue->iTerms = iRight;
		psValue->iSection = psRight->iSection;
		psValue->uiTermToken = psRight->uiTermToken;
		return true;
	}
	if (!SameTerm(psValue, psRight))
	{
		return Error(psOperator, "Addresses in different sections cannot be combined");
	}
	psValue->iTerms += iRight;
	if (psValue->iTerms == 0)
	{
		psValue->iSection = ASM65_SECTION_ABSOLUTE;
	}
	else if ((psValue->iTerms == 1) && (iRight == 1))
	{
		psValue->uiTermToken = psRight->uiTermToken;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The same section, or the same external.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::SameTerm(SAsm65Value* psValue, SAsm65Value* psRight)
{
	SAsm65Token*	psLeftTerm;
	SAsm65Token*	psRightTerm;

	if (psValue->iSection != psRight->iSection)
	{
		return false;
	}
	if (psValue->iSection != ASM65_SECTION_UNDEFINED)
	{
		return true;
	}
	psLeftTerm = mcTokeniser.Get(psValue->uiTermToken);
	psRightTerm = mcTokeniser.Get(psRight->uiTermToken);
	return (psLeftTerm->uiLength == psRightTerm->uiLength) && (memcmp(psLeftTerm->szText, psRightTerm->szText, psLeftTerm->uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// A relative target is a label (an address), C + offset or a raw offset.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::RelativeTarget(SAsm65Value* psValue, SAsm65Token* psToken)
{
	if ((psValue->iProgramCounter == 0) || ((psValue->iProgramCounter == 1) && !psValue->bLabel))
	{
		return true;
	}
	return Error(psToken, "A target using C must be C + offset or C - offset");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::ExpressionValue(int64* piValue)
{
	SAsm65Value	sValue;

	if (!Expression(&sValue))
	{
		return false;
	}
	*piValue = sValue.iValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Registers, C (the program counter) and P (the status) cannot be symbols.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65BaseAssembler::IsReservedName(SAsm65Token* psToken)
{
	return IsRegisterName(psToken) || psToken->IsIdentifier("C") || psToken->IsIdentifier("P");
}

