/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTString.h"
#include "CTIdentifier.h"
#include "CTUserLiteral.h"
#include "CppTokeniser.h"
#include "CPP65IRFrontEnd.h"
#include "CPPSyntaxTree.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"
#include "CPPUnit.h"
#include "CPPWarnings.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if pcExpression is an integer constant that an implicit conversion to
// psTo truncates: it fits neither psTo's signed nor its unsigned range (so -1
// to an unsigned type and 0xFFFF to a 16 bit signed one are bit patterns, not
// warned about).  89037124 passed to an int parameter is.  With
// SetWarnNarrowing a non constant integer converted to a narrower type is
// warned about too: a 32 bit index stored in a 16 bit one.  A multiply or
// left shift converted to a wider type is warned about (WideningArithmetic).
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::ConversionOverflow(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	SCPP65Type*	psFrom;
	int64		lliValue;
	bool		bConstant;
	CChars		szMessage;

	if (!psTo || !psTo->IsInteger() || (psTo->eKind == CPP65TK_Bool) || !mpcUnit->mcContext.EnterWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	psFrom = mpcUnit->GetTypeChecker()->TypeOf(pcExpression);
	mpcUnit->mcContext.LeaveWorking();
	if (!psFrom || !psFrom->IsInteger())
	{
		return;
	}

	//Most expressions aren't constant; looking must not leave an error.
	bConstant = mpcUnit->GetFolder()->QuietConstant(pcExpression, &lliValue);
	if (!bConstant)
	{
		WideningArithmetic(pcExpression, psFrom, psTo);
		if (mpcUnit->mpcOptions->mbWarnNarrowing && (psFrom->eKind == CPP65TK_Integer) && (psFrom->iBytes > psTo->iBytes))
		{
			szMessage.Init("Integer conversion narrows: a ");
			szMessage.Append(psFrom->iBytes * 8)->Append(" bit value is converted to the ")->Append(psTo->iBytes * 8)->Append(" bit type and may be truncated.");
			mpcUnit->mcDiagnostics.Warning(pcExpression, szMessage.Text());
			szMessage.Kill();
		}
		return;
	}
	if (psTo->iBytes >= 8)
	{
		return;
	}

	//It fits the type's width, as signed or as unsigned.
	if ((lliValue >= CPPIntegerLeast(psTo->iBytes, true)) && (lliValue <= CPPIntegerMost(psTo->iBytes, false)))
	{
		return;
	}

	szMessage.Init("Integer conversion overflow: the constant ");
	szMessage.Append(lliValue)->Append(" doesn't fit the ")->Append(psTo->iBytes * 8)->Append(" bit type it is converted to and is truncated.");
	mpcUnit->mcDiagnostics.Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if pcExpression (of type psFrom) is a multiply or a left shift done
// in an integer type narrower than psTo, which it is implicitly converted
// to: the result may have wrapped before it was widened.  iKey = ui * 0x39e7
// with a 16 bit ui and a 64 bit iKey multiplies in 16 bits.  Converting an
// operand first ((int64)ui * 0x39e7) does the arithmetic in the wider type.
// An explicit cast of the result ((int64)(ui * 0x39e7)) is warned about too:
// it doesn't make the multiply wider.  Constants are ConstantOverflow's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::WideningArithmetic(CCPPSyntax* pcExpression, SCPP65Type* psFrom, SCPP65Type* psTo)
{
	CCPPSyntaxBinaryExpression*	pcBinary;
	CChars						szMessage;
	int64						lliValue;
	bool						bConstant;

	if ((psFrom->eKind != CPP65TK_Integer) || (psTo->eKind != CPP65TK_Integer) || (psTo->iBytes <= psFrom->iBytes) || mpcUnit->mcContext.IsWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	pcBinary = NarrowMultiply(pcExpression, psFrom);
	if (!pcBinary)
	{
		return;
	}

	bConstant = mpcUnit->GetFolder()->QuietConstant(pcBinary, &lliValue);
	if (bConstant)
	{
		return;
	}

	szMessage.Init("Integer arithmetic widened after it may have wrapped: the ");
	szMessage.Append(psFrom->iBytes * 8)->Append(" bit result of '")->Append((pcBinary->meOperator == CT_Multiply) ? "*" : "<<");
	szMessage.Append("' is converted to the ")->Append(psTo->iBytes * 8)->Append(" bit type.  Convert an operand first to do it in ")->Append(psTo->iBytes * 8)->Append(" bits.");
	mpcUnit->mcDiagnostics.Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a shift's count is a constant not less than iBits, the width of
// the (promoted) value shifted: the result is undefined.  On the W65C816 an
// int is 16 bits, so a uint8 or int promoted to it and shifted by 16 or more
// ((uint32)(b << 16) with a uint8 b) is zero, not the 32 bit value meant.
// Converting the operand first ((uint32)b << 16) shifts in the wider type.
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::ShiftCount(CCPPSyntaxBinaryExpression* pcBinary, int iBits)
{
	CChars	szMessage;
	int64	lliCount;
	bool	bConstant;

	if (mpcUnit->mcContext.IsWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	bConstant = mpcUnit->GetFolder()->QuietConstant(pcBinary->mpcRight, &lliCount);
	if (!bConstant || (lliCount < iBits))
	{
		return;
	}

	szMessage.Init("Shift count ");
	szMessage.Append(lliCount)->Append(" is not less than the ")->Append(iBits)->Append(" bit width of the value shifted, so the result is undefined.  Convert the operand first to shift in a wider type.");
	mpcUnit->mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The multiply or left shift WideningArithmetic warns about in pcExpression:
// the expression itself, or one under +, -, |, ^ or & that is done in the
// same type (psType): y * uiRowStride + x * uiPixelStride with 16 bit
// operands made a 16 bit offset before it was widened to the pointer's
// ptrdiff.  NULL if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxBinaryExpression* CCPPWarnings::NarrowMultiply(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxBinaryExpression*	pcFound;
	SCPP65Type*					psSide;
	int							i;

	pcInner = CPPSkipParentheses(pcExpression);
	if (!pcInner || (pcInner->GetKind() != CPPS_BinaryExpression))
	{
		return NULL;
	}
	pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
	if ((pcBinary->meOperator == CT_Multiply) || (pcBinary->meOperator == CT_LeftShift))
	{
		return pcBinary;
	}
	if ((pcBinary->meOperator != CT_Add) && (pcBinary->meOperator != CT_Subtract) && (pcBinary->meOperator != CT_BitwiseOr) &&
		(pcBinary->meOperator != CT_BitwiseXor) && (pcBinary->meOperator != CT_BitwiseAnd))
	{
		return NULL;
	}

	//Only a side already the type the whole is done in: a wider side would have made the whole wider.
	for (i = 0; i < 2; i++)
	{
		mpcUnit->mcContext.EnterWorking(CPPWK_Warning, NULL);
		psSide = mpcUnit->GetTypeChecker()->TypeOf((i == 0) ? pcBinary->mpcLeft : pcBinary->mpcRight);
		mpcUnit->mcContext.LeaveWorking();
		if (!psSide || (psSide->eKind != CPP65TK_Integer) || (psSide->iBytes != psType->iBytes))
		{
			continue;
		}
		pcFound = NarrowMultiply((i == 0) ? pcBinary->mpcLeft : pcBinary->mpcRight, psType);
		if (pcFound)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a for loop's counter (a variable its increment steps with ++,
// --, += or -=) is compared in its condition with a wider integer that isn't
// a constant: for (size i = 0; i < aoi.NumElements(); i++) with a 16 bit i
// and a 32 bit count wraps i at 65535 and goes round forever once the count
// passes it.  Converting the bound to the counter's type ((size)uiCount) says
// it fits.
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::LoopCounterWidth(CCPPSyntaxForStatement* pcFor)
{
	CArrayVoidPtr	apcCounters;

	if (!pcFor->mpcCondition || !pcFor->mpcIncrement || mpcUnit->mcContext.IsWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	apcCounters.Init();
	LoopCounters(pcFor->mpcIncrement, &apcCounters);
	if (apcCounters.NumElements() > 0)
	{
		LoopCounterComparisons(pcFor->mpcCondition, &apcCounters);
	}
	apcCounters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The variables pcIncrement steps (through commas): i++, --i, i += 2.
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::LoopCounters(CCPPSyntax* pcIncrement, CArrayVoidPtr* papcCounters)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CCPPSymbol*					pcSymbol;

	pcInner = CPPSkipParentheses(pcIncrement);
	if (!pcInner)
	{
		return;
	}
	pcSymbol = NULL;
	if (pcInner->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcInner;
		if ((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement))
		{
			pcSymbol = LoopCounterSymbol(pcUnary->mpcOperand);
		}
	}
	else if (pcInner->GetKind() == CPPS_BinaryExpression)
	{
		pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
		if (pcBinary->meOperator == CT_Comma)
		{
			LoopCounters(pcBinary->mpcLeft, papcCounters);
			LoopCounters(pcBinary->mpcRight, papcCounters);
		}
		else if ((pcBinary->meOperator == CT_AssignmentAdd) || (pcBinary->meOperator == CT_AssignmentSubtract))
		{
			pcSymbol = LoopCounterSymbol(pcBinary->mpcLeft);
		}
	}
	if (pcSymbol)
	{
		papcCounters->Add(pcSymbol);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The comparisons in pcCondition (through && and ||) of a counter with a
// wider integer that isn't a constant.
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::LoopCounterComparisons(CCPPSyntax* pcCondition, CArrayVoidPtr* papcCounters)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntax*					pcCounter;
	CCPPSyntax*					pcBound;
	CCPPSymbol*					pcSymbol;
	SCPP65Type*					psCounter;
	SCPP65Type*					psBound;
	int64						lliValue;
	bool						bConstant;
	CChars						szMessage;
	ECppTokens					eOperator;

	pcInner = CPPSkipParentheses(pcCondition);
	if (!pcInner || (pcInner->GetKind() != CPPS_BinaryExpression))
	{
		return;
	}
	pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
	eOperator = pcBinary->meOperator;
	if ((eOperator == CT_LogicalAnd) || (eOperator == CT_LogicalOr))
	{
		LoopCounterComparisons(pcBinary->mpcLeft, papcCounters);
		LoopCounterComparisons(pcBinary->mpcRight, papcCounters);
		return;
	}
	if ((eOperator != CT_LessThan) && (eOperator != CT_LessThanEqualTo) && (eOperator != CT_GreaterThan) &&
		(eOperator != CT_GreaterThanEqualTo) && (eOperator != CT_NotEqualTo))
	{
		return;
	}

	pcSymbol = LoopCounterSymbol(pcBinary->mpcLeft);
	if (pcSymbol && (papcCounters->Find(pcSymbol) != ARRAY_ELEMENT_NOT_FOUND))
	{
		pcCounter = pcBinary->mpcLeft;
		pcBound = pcBinary->mpcRight;
	}
	else
	{
		pcSymbol = LoopCounterSymbol(pcBinary->mpcRight);
		if (!pcSymbol || (papcCounters->Find(pcSymbol) == ARRAY_ELEMENT_NOT_FOUND))
		{
			return;
		}
		pcCounter = pcBinary->mpcRight;
		pcBound = pcBinary->mpcLeft;
	}

	mpcUnit->mcContext.EnterWorking(CPPWK_Warning, NULL);
	psCounter = mpcUnit->GetTypeChecker()->TypeOf(pcCounter);
	psBound = mpcUnit->GetTypeChecker()->TypeOf(pcBound);
	mpcUnit->mcContext.LeaveWorking();
	if (!psCounter || !psBound || (psCounter->eKind != CPP65TK_Integer) || (psBound->eKind != CPP65TK_Integer) || (psBound->iBytes <= psCounter->iBytes))
	{
		return;
	}

	//A constant bound is ComparisonRange's, and a choice of constants (b ? 7 : 3) is two of them.
	mpcUnit->mcDiagnostics.BeginTrial();
	pcInner = CPPSkipParentheses(pcBound);
	if (pcInner && (pcInner->GetKind() == CPPS_ConditionalExpression))
	{
		bConstant = mpcUnit->mcFolder.Constant(((CCPPSyntaxConditionalExpression*)pcInner)->mpcTrue, &lliValue) &&
					mpcUnit->mcFolder.Constant(((CCPPSyntaxConditionalExpression*)pcInner)->mpcFalse, &lliValue);
	}
	else
	{
		bConstant = mpcUnit->mcFolder.Constant(pcBound, &lliValue);
	}
	mpcUnit->mcDiagnostics.EndTrial();
	if (bConstant || mpcUnit->mcDiagnostics.HasWarning(pcBinary, "Loop counter"))
	{
		return;
	}

	szMessage.Init("Loop counter narrower than its bound: the ");
	szMessage.Append(psCounter->iBytes * 8)->Append(" bit counter is compared with a ")->Append(psBound->iBytes * 8);
	szMessage.Append(" bit value and wraps before it can pass one that doesn't fit.  Widen the counter, or convert the bound if it always fits.");
	mpcUnit->mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The variable pcExpression names (i, or a member m), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPWarnings::LoopCounterSymbol(CCPPSyntax* pcExpression)
{
	CCPPSyntax*				pcInner;
	CCPPSyntaxIdExpression*	pcId;

	pcInner = CPPSkipParentheses(pcExpression);
	if (!pcInner || (pcInner->GetKind() != CPPS_IdExpression))
	{
		return NULL;
	}
	pcId = (CCPPSyntaxIdExpression*)pcInner;
	if (!pcId->mpcName)
	{
		return NULL;
	}
	return pcId->mpcName->mpcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if one side of an integer comparison is a constant outside the range
// of the other side's type, so the comparison is always true or always false:
// a 16 bit size compared with 0xFFFFFFFF (a 32 bit "not found").  The constant
// is converted to the comparison's type first, so -1 compared with a 16 bit
// unsigned int is 0xFFFF and fine.  A side the comparison's type can't hold
// every value of (signed compared as unsigned, or a 32 bit unsigned enum
// compared as a 32 bit long) wraps, so it isn't looked at.
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::ComparisonRange(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*		psLeft;
	SCPP65Type*		psRight;
	SCPP65Type*		psVariable;
	SCPP65Type*		psCommon;
	CCPPSyntax*		pcConstant;
	int64			lliValue;
	int64			lliMin;
	int64			lliMax;
	bool			bConstant;
	bool			bAlways;
	CChars			szMessage;

	if (!mpcUnit->mcContext.EnterWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	psLeft = mpcUnit->GetTypeChecker()->TypeOf(pcBinary->mpcLeft);
	psRight = mpcUnit->GetTypeChecker()->TypeOf(pcBinary->mpcRight);
	mpcUnit->mcContext.LeaveWorking();
	if (!psLeft || !psRight || (psLeft->eKind != CPP65TK_Integer) || (psRight->eKind != CPP65TK_Integer))
	{
		return;
	}

	//Most comparisons have no constant side; looking must not leave an error.
	mpcUnit->mcDiagnostics.BeginTrial();
	pcConstant = pcBinary->mpcRight;
	psVariable = psLeft;
	bConstant = mpcUnit->mcFolder.Constant(pcConstant, &lliValue);
	if (!bConstant)
	{
		pcConstant = pcBinary->mpcLeft;
		psVariable = psRight;
		bConstant = mpcUnit->mcFolder.Constant(pcConstant, &lliValue) && !mpcUnit->mcFolder.Constant(pcBinary->mpcRight, &lliMin);
	}
	else if (mpcUnit->mcFolder.Constant(pcBinary->mpcLeft, &lliMin))
	{
		bConstant = false;
	}
	mpcUnit->mcDiagnostics.EndTrial();
	if (!bConstant)
	{
		return;
	}

	psCommon = mpcUnit->mcTypes.Arithmetic(psLeft, psRight);
	if (!psCommon || (psCommon->iBytes >= 8) || (psVariable->bSigned && !psCommon->bSigned) || (!psVariable->bSigned && psCommon->bSigned && (psCommon->iBytes <= psVariable->iBytes)))
	{
		return;
	}

	//The constant as the comparison's type sees it.
	lliValue = mpcUnit->mcFolder.ConstantOfType(psCommon, lliValue);

	lliMin = psVariable->Least();
	lliMax = psVariable->Most();
	if ((lliValue >= lliMin) && (lliValue <= lliMax))
	{
		ComparisonAtLimit(pcBinary, pcConstant, psVariable, lliValue, lliMin, lliMax);
		return;
	}

	//Out of range: the result is the same for every value of the variable side.
	switch (pcBinary->meOperator)
	{
	case CT_EqualTo:				bAlways = false;	break;
	case CT_NotEqualTo:				bAlways = true;		break;
	case CT_LessThan:
	case CT_LessThanEqualTo:		bAlways = (lliValue > lliMax) == (pcConstant == pcBinary->mpcRight);	break;
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:		bAlways = (lliValue < lliMin) == (pcConstant == pcBinary->mpcRight);	break;
	default:						return;
	}

	szMessage.Init("Comparison is always ");
	szMessage.Append(bAlways ? "true" : "false")->Append(": the constant ")->Append(lliValue)->Append(" is out of the range of the ")->Append(psVariable->iBytes * 8)->Append(" bit ")->Append(psVariable->bSigned ? "signed" : "unsigned")->Append(" value it is compared with.");
	mpcUnit->mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a <= (or >=) comparison's constant is the largest value the
// variable side's type holds (or a signed type's smallest), so it is always
// true: for (i = -32768; i <= 32767; i++) with a 16 bit int never ends.
// Always false comparisons at a limit (ui > MAX_UINT32, a guard for wider
// builds) and an unsigned value compared with 0 (u >= 0, written to say what
// is expected) aren't warned about.
//
//////////////////////////////////////////////////////////////////////////
void CCPPWarnings::ComparisonAtLimit(CCPPSyntaxBinaryExpression* pcBinary, CCPPSyntax* pcConstant, SCPP65Type* psVariable, int64 lliValue, int64 lliMin, int64 lliMax)
{
	ECppTokens	eOperator;
	bool		bAlways;
	CChars		szMessage;

	//As variable OP constant.
	eOperator = pcBinary->meOperator;
	if (pcConstant == pcBinary->mpcLeft)
	{
		switch (eOperator)
		{
		case CT_LessThan:				eOperator = CT_GreaterThan;			break;
		case CT_LessThanEqualTo:		eOperator = CT_GreaterThanEqualTo;	break;
		case CT_GreaterThan:			eOperator = CT_LessThan;			break;
		case CT_GreaterThanEqualTo:		eOperator = CT_LessThanEqualTo;		break;
		default:						break;
		}
	}

	if ((lliValue == lliMax) && (eOperator == CT_LessThanEqualTo))
	{
		bAlways = true;
	}
	else if (psVariable->bSigned && (lliValue == lliMin) && (eOperator == CT_GreaterThanEqualTo))
	{
		bAlways = true;
	}
	else
	{
		return;
	}

	szMessage.Init("Comparison is always ");
	szMessage.Append(bAlways ? "true" : "false")->Append(": the constant ")->Append(lliValue)->Append(" is the ")->Append((lliValue == lliMax) ? "largest" : "smallest");
	szMessage.Append(" value of the ")->Append(psVariable->iBytes * 8)->Append(" bit ")->Append(psVariable->bSigned ? "signed" : "unsigned")->Append(" value it is compared with.");
	mpcUnit->mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}

