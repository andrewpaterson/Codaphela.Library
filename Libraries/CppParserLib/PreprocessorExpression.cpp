/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/StringHelper.h"
#include "LiteralHelper.h"
#include "PreprocessorExpression.h"


enum EPPOperator
{
	PPO_None,
	PPO_Multiply,
	PPO_Divide,
	PPO_Remainder,
	PPO_Add,
	PPO_Subtract,
	PPO_ShiftLeft,
	PPO_ShiftRight,
	PPO_Less,
	PPO_LessEqual,
	PPO_Greater,
	PPO_GreaterEqual,
	PPO_Equal,
	PPO_NotEqual,
	PPO_BitAnd,
	PPO_BitXor,
	PPO_BitOr,
	PPO_LogicalAnd,
	PPO_LogicalOr,
};


//Indexed by EPPOperator.  Higher binds tighter.
static int giPPOperatorPrecedence[] = { 0, 10, 10, 10, 9, 9, 8, 8, 7, 7, 7, 7, 6, 6, 5, 4, 3, 2, 1 };


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPPValue::Init(uint64 ulliValue, bool bUnsigned)
{
	this->ulliValue = ulliValue;
	this->bUnsigned = bUnsigned;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool SPPValue::IsTrue(void)
{
	return ulliValue != 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 SPPValue::Signed(void)
{
	return (int64)ulliValue;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CPreprocessorExpression::Evaluate(char* szText, CChars* pszError)
{
	CPreprocessorExpression		cExpression;
	TRISTATE					tResult;

	cExpression.Init(szText, pszError);
	tResult = cExpression.Evaluate();
	cExpression.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorExpression::Init(char* szText, CChars* pszError)
{
	mszStart = szText;
	mszPos = szText;
	mpszError = pszError;
	mbError = false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorExpression::Kill(void)
{
	mszStart = NULL;
	mszPos = NULL;
	mpszError = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CPreprocessorExpression::Evaluate(void)
{
	SPPValue	sValue;

	SkipWhitespace();
	if (*mszPos == '\0')
	{
		Error("Expected an expression");
		return TRIERROR;
	}

	sValue = Conditional(true);
	if (mbError)
	{
		return TRIERROR;
	}

	SkipWhitespace();
	if (*mszPos != '\0')
	{
		Error("Unexpected text");
		return TRIERROR;
	}

	return sValue.IsTrue() ? TRITRUE : TRIFALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Conditional(bool bEvaluate)
{
	SPPValue	sCondition;
	SPPValue	sTrue;
	SPPValue	sFalse;
	SPPValue	sResult;

	sCondition = Binary(1, bEvaluate);
	if (mbError)
	{
		return sCondition;
	}

	if (!GetCharacter('?'))
	{
		return sCondition;
	}

	sTrue = Conditional(bEvaluate && sCondition.IsTrue());
	if (mbError)
	{
		return sTrue;
	}

	if (!GetCharacter(':'))
	{
		return Error("Expected ':'");
	}

	sFalse = Conditional(bEvaluate && !sCondition.IsTrue());
	if (mbError)
	{
		return sFalse;
	}

	//The usual arithmetic conversions apply to the second and third operands.
	sResult = sCondition.IsTrue() ? sTrue : sFalse;
	sResult.bUnsigned = sTrue.bUnsigned || sFalse.bUnsigned;
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Binary(int iMinPrecedence, bool bEvaluate)
{
	SPPValue	sLeft;
	SPPValue	sRight;
	int			iOperator;
	int			iLength;
	int			iPrecedence;
	bool		bEvaluateRight;

	sLeft = Unary(bEvaluate);
	if (mbError)
	{
		return sLeft;
	}

	for (;;)
	{
		iOperator = PeekBinaryOperator(&iLength);
		if (iOperator == PPO_None)
		{
			return sLeft;
		}

		iPrecedence = giPPOperatorPrecedence[iOperator];
		if (iPrecedence < iMinPrecedence)
		{
			return sLeft;
		}
		mszPos += iLength;

		//&& and || do not evaluate their right operand when the left decides the result.
		bEvaluateRight = bEvaluate;
		if (iOperator == PPO_LogicalAnd)
		{
			bEvaluateRight = bEvaluate && sLeft.IsTrue();
		}
		else if (iOperator == PPO_LogicalOr)
		{
			bEvaluateRight = bEvaluate && !sLeft.IsTrue();
		}

		sRight = Binary(iPrecedence + 1, bEvaluateRight);
		if (mbError)
		{
			return sRight;
		}

		sLeft = Apply(iOperator, sLeft, sRight, bEvaluate);
		if (mbError)
		{
			return sLeft;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Apply(int iOperator, SPPValue sLeft, SPPValue sRight, bool bEvaluate)
{
	SPPValue	sResult;
	bool		bUnsigned;
	uint64		ulliLeft;
	uint64		ulliRight;
	int64		lliLeft;
	int64		lliRight;

	bUnsigned = sLeft.bUnsigned || sRight.bUnsigned;
	ulliLeft = sLeft.ulliValue;
	ulliRight = sRight.ulliValue;
	lliLeft = sLeft.Signed();
	lliRight = sRight.Signed();

	switch (iOperator)
	{
	case PPO_Multiply:
		sResult.Init(ulliLeft * ulliRight, bUnsigned);
		return sResult;

	case PPO_Divide:
	case PPO_Remainder:
		if (ulliRight == 0)
		{
			if (bEvaluate)
			{
				return Error("Division by zero");
			}
			sResult.Init(0, bUnsigned);
			return sResult;
		}
		if (bUnsigned)
		{
			sResult.Init((iOperator == PPO_Divide) ? (ulliLeft / ulliRight) : (ulliLeft % ulliRight), true);
		}
		else if ((lliRight == -1) && (ulliLeft == 0x8000000000000000ULL))
		{
			//INT64_MIN / -1 overflows; the remainder is zero.
			sResult.Init((iOperator == PPO_Divide) ? ulliLeft : 0, false);
		}
		else
		{
			sResult.Init((uint64)((iOperator == PPO_Divide) ? (lliLeft / lliRight) : (lliLeft % lliRight)), false);
		}
		return sResult;

	case PPO_Add:
		sResult.Init(ulliLeft + ulliRight, bUnsigned);
		return sResult;

	case PPO_Subtract:
		sResult.Init(ulliLeft - ulliRight, bUnsigned);
		return sResult;

	case PPO_ShiftLeft:
	case PPO_ShiftRight:
		//The result has the type of the left operand.
		if (ulliRight >= 64)
		{
			sResult.Init(((iOperator == PPO_ShiftRight) && !sLeft.bUnsigned && (lliLeft < 0)) ? (uint64)-1 : 0, sLeft.bUnsigned);
		}
		else if (iOperator == PPO_ShiftLeft)
		{
			sResult.Init(ulliLeft << ulliRight, sLeft.bUnsigned);
		}
		else if (sLeft.bUnsigned)
		{
			sResult.Init(ulliLeft >> ulliRight, true);
		}
		else
		{
			sResult.Init((uint64)(lliLeft >> ulliRight), false);
		}
		return sResult;

	case PPO_Less:
		sResult.Init(bUnsigned ? (ulliLeft < ulliRight) : (lliLeft < lliRight), false);
		return sResult;

	case PPO_LessEqual:
		sResult.Init(bUnsigned ? (ulliLeft <= ulliRight) : (lliLeft <= lliRight), false);
		return sResult;

	case PPO_Greater:
		sResult.Init(bUnsigned ? (ulliLeft > ulliRight) : (lliLeft > lliRight), false);
		return sResult;

	case PPO_GreaterEqual:
		sResult.Init(bUnsigned ? (ulliLeft >= ulliRight) : (lliLeft >= lliRight), false);
		return sResult;

	case PPO_Equal:
		sResult.Init(ulliLeft == ulliRight, false);
		return sResult;

	case PPO_NotEqual:
		sResult.Init(ulliLeft != ulliRight, false);
		return sResult;

	case PPO_BitAnd:
		sResult.Init(ulliLeft & ulliRight, bUnsigned);
		return sResult;

	case PPO_BitXor:
		sResult.Init(ulliLeft ^ ulliRight, bUnsigned);
		return sResult;

	case PPO_BitOr:
		sResult.Init(ulliLeft | ulliRight, bUnsigned);
		return sResult;

	case PPO_LogicalAnd:
		sResult.Init(sLeft.IsTrue() && sRight.IsTrue(), false);
		return sResult;

	case PPO_LogicalOr:
		sResult.Init(sLeft.IsTrue() || sRight.IsTrue(), false);
		return sResult;
	}

	return Error("Unknown operator");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Unary(bool bEvaluate)
{
	SPPValue	sValue;

	SkipWhitespace();
	if (GetCharacter('!'))
	{
		sValue = Unary(bEvaluate);
		sValue.Init(!sValue.IsTrue(), false);
		return sValue;
	}
	else if (GetCharacter('~'))
	{
		sValue = Unary(bEvaluate);
		sValue.ulliValue = ~sValue.ulliValue;
		return sValue;
	}
	else if ((mszPos[0] == '-') && (mszPos[1] != '-'))
	{
		mszPos++;
		sValue = Unary(bEvaluate);
		sValue.ulliValue = (uint64)0 - sValue.ulliValue;
		return sValue;
	}
	else if ((mszPos[0] == '+') && (mszPos[1] != '+'))
	{
		mszPos++;
		return Unary(bEvaluate);
	}
	return Primary(bEvaluate);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Primary(bool bEvaluate)
{
	SPPValue	sValue;
	char*		szIdentifier;
	size		iLength;

	SkipWhitespace();
	if (GetCharacter('('))
	{
		sValue = Conditional(bEvaluate);
		if (mbError)
		{
			return sValue;
		}
		if (!GetCharacter(')'))
		{
			return Error("Expected ')'");
		}
		return sValue;
	}

	if (((*mszPos >= '0') && (*mszPos <= '9')) || ((*mszPos == '.') && (mszPos[1] >= '0') && (mszPos[1] <= '9')))
	{
		return Number();
	}

	if (*mszPos == '\'')
	{
		return Character();
	}

	if (IsIdentifierCharacter(*mszPos))
	{
		szIdentifier = mszPos;
		while (IsIdentifierCharacter(*mszPos))
		{
			mszPos++;
		}
		iLength = mszPos - szIdentifier;

		//Character literal prefixes.
		if ((*mszPos == '\'') && (((iLength == 1) && ((*szIdentifier == 'L') || (*szIdentifier == 'u') || (*szIdentifier == 'U'))) || ((iLength == 2) && (szIdentifier[0] == 'u') && (szIdentifier[1] == '8'))))
		{
			return Character();
		}

		//Identifiers left after macro expansion are 0, except for true and false.
		if ((iLength == 4) && (memcmp(szIdentifier, "true", 4) == 0))
		{
			sValue.Init(1, false);
		}
		else
		{
			sValue.Init(0, false);
		}
		return sValue;
	}

	return Error("Expected a value");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Number(void)
{
	SPPValue			sValue;
	SIntegerLiteral		sLiteral;
	char*				szStart;
	char				c;
	bool				bHex;

	//The pp-number's extent: a sign belongs to it after e (not in hex) or p.  [lex.ppnumber]
	szStart = mszPos;
	bHex = (mszPos[0] == '0') && ((mszPos[1] == 'x') || (mszPos[1] == 'X'));
	for (;;)
	{
		c = *mszPos;
		if (IsIdentifierCharacter(c) || (c == '.') || (c == '\''))
		{
			mszPos++;
		}
		else if (((c == '+') || (c == '-')) && (((!bHex) && ((mszPos[-1] == 'e') || (mszPos[-1] == 'E'))) || (bHex && ((mszPos[-1] == 'p') || (mszPos[-1] == 'P')))))
		{
			mszPos++;
		}
		else
		{
			break;
		}
	}

	switch (ParseIntegerLiteral(szStart, mszPos - szStart, &sLiteral))
	{
	case ILR_Integer:
		break;
	case ILR_Floating:
		return Error("Floating point in a preprocessor expression");
	case ILR_InvalidDigit:
		return Error("Invalid digit in integer");
	case ILR_TooLarge:
		return Error("Integer is too large");
	default:
		return Error("Invalid integer suffix");
	}

	//A literal too large for intmax_t is uintmax_t.
	sValue.Init(sLiteral.ulliValue, sLiteral.bUnsigned || (sLiteral.ulliValue > 0x7FFFFFFFFFFFFFFFULL));
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Character(void)
{
	SPPValue	sValue;
	uint64		ulliValue;
	uint32		uiCharacter;
	int			iCount;
	char		c;
	const char*	sz;

	mszPos++;  //Skip the opening '.
	ulliValue = 0;
	iCount = 0;
	while (*mszPos != '\'')
	{
		c = *mszPos;
		if ((c == '\0') || (c == '\n'))
		{
			return Error("Unterminated character literal");
		}

		if (c != '\\')
		{
			uiCharacter = (unsigned char)c;
			mszPos++;
		}
		else
		{
			sz = mszPos + 1;
			if (!DecodeEscape(&sz, sz + StrLen(sz), &uiCharacter))
			{
				return Error("Unknown escape sequence");
			}
			mszPos = (char*)sz;
		}

		//Multi-character literals, as in 'ABCD', are int.
		ulliValue = (ulliValue << 8) | (uiCharacter & 0xFF);
		iCount++;
	}
	mszPos++;  //Skip the closing '.

	if (iCount == 0)
	{
		return Error("Empty character literal");
	}

	sValue.Init(ulliValue, false);
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CPreprocessorExpression::PeekBinaryOperator(int* piLength)
{
	char	c;
	char	c1;

	SkipWhitespace();
	c = mszPos[0];
	if (c == '\0')
	{
		return PPO_None;
	}
	c1 = mszPos[1];

	*piLength = 2;
	if ((c == '<') && (c1 == '<'))	return PPO_ShiftLeft;
	if ((c == '>') && (c1 == '>'))	return PPO_ShiftRight;
	if ((c == '<') && (c1 == '='))	return PPO_LessEqual;
	if ((c == '>') && (c1 == '='))	return PPO_GreaterEqual;
	if ((c == '=') && (c1 == '='))	return PPO_Equal;
	if ((c == '!') && (c1 == '='))	return PPO_NotEqual;
	if ((c == '&') && (c1 == '&'))	return PPO_LogicalAnd;
	if ((c == '|') && (c1 == '|'))	return PPO_LogicalOr;

	*piLength = 1;
	switch (c)
	{
	case '*':	return PPO_Multiply;
	case '/':	return PPO_Divide;
	case '%':	return PPO_Remainder;
	case '+':	return PPO_Add;
	case '-':	return PPO_Subtract;
	case '<':	return PPO_Less;
	case '>':	return PPO_Greater;
	case '&':	return PPO_BitAnd;
	case '^':	return PPO_BitXor;
	case '|':	return PPO_BitOr;
	}
	return PPO_None;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorExpression::GetCharacter(char c)
{
	SkipWhitespace();
	if (*mszPos == c)
	{
		mszPos++;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorExpression::SkipWhitespace(void)
{
	while ((*mszPos == ' ') || (*mszPos == '\t') || (*mszPos == '\r') || (*mszPos == '\n'))
	{
		mszPos++;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorExpression::IsIdentifierCharacter(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPValue CPreprocessorExpression::Error(char* szError)
{
	SPPValue	sValue;

	if (!mbError)
	{
		mbError = true;
		mpszError->Append(szError);
		mpszError->Append(" at column ");
		mpszError->Append((int)(mszPos - mszStart) + 1);
		mpszError->Append(".");
	}
	sValue.Init(0, false);
	return sValue;
}

