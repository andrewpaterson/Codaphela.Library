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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "CPP65DebugWatch.h"


//A binary operator and how tightly it binds (the higher the tighter), as C++'s grammar has them.
struct SCPP65WatchOperator
{
	const char*	szOperator;
	int			iPrecedence;
};


static const SCPP65WatchOperator gasCPP65WatchOperators[] =
{
	{ "||", 1 }, { "&&", 2 }, { "|", 3 }, { "^", 4 }, { "&", 5 }, { "==", 6 }, { "!=", 6 }, { "<", 7 }, { ">", 7 }, { "<=", 7 }, { ">=", 7 },
	{ "<<", 8 }, { ">>", 8 }, { "+", 9 }, { "-", 9 }, { "*", 10 }, { "/", 10 }, { "%", 10 }, { NULL, 0 }
};


//The fundamental types as the W65C816 has them (an int is 16 bits), for a cast to one, or a result the records have
//no type for: their names are their keys (CCPP65DebugWriter's).
struct SCPP65WatchBuiltIn
{
	const char*	szName;
	uint8		uiKind;		//EAsm65DebugType.
	uint8		uiFlags;
	uint32		uiBytes;
};


static const SCPP65WatchBuiltIn gasCPP65WatchBuiltIns[] =
{
	{ "bool", ADT_Bool, 0, 1 }, { "char", ADT_Integer, ADTF_Signed | ADTF_Character, 1 }, { "signed char", ADT_Integer, ADTF_Signed | ADTF_Character, 1 },
	{ "unsigned char", ADT_Integer, ADTF_Character, 1 }, { "int", ADT_Integer, ADTF_Signed, 2 }, { "unsigned int", ADT_Integer, 0, 2 },
	{ "short", ADT_Integer, ADTF_Signed, 2 }, { "unsigned short", ADT_Integer, 0, 2 }, { "long", ADT_Integer, ADTF_Signed, 4 },
	{ "unsigned long", ADT_Integer, 0, 4 }, { "long long", ADT_Integer, ADTF_Signed, 8 }, { "unsigned long long", ADT_Integer, 0, 8 },
	{ "float", ADT_Float, 0, 4 }, { "double", ADT_Float, 0, 8 }, { "void", ADT_Void, 0, 0 }, { NULL, 0, 0, 0 }
};


static const char*	gaszCPP65WatchPunctuators[] = { "->", "::", "<<", ">>", "<=", ">=", "==", "!=", "&&", "||", NULL };
static const char	gszCPP65WatchEscapes[] = "n\nt\tr\r0\0\\\\''\"\"";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWatch::Init(CCPP65DebugValues* pcValues, CAsm65StackWalker* pcWalker)
{
	mpcValues = pcValues;
	mpcWalker = pcWalker;
	mpsFrame = NULL;
	mpcRecords = NULL;
	muiLine = 0;
	memset(&msToken, 0, sizeof(msToken));
	mszError.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWatch::Kill(void)
{
	mszError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szExpression's value in psFrame stopped at uiLine of its function's
// file (0: every local is in scope).  False, with GetError saying why, if
// it can't be.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Evaluate(SAsm65StackFrame* psFrame, uint32 uiLine, const char* szExpression, SCPP65DebugValue* psResult)
{
	mpsFrame = psFrame;
	mpcRecords = psFrame->pcRecords;
	muiLine = uiLine;
	mszError.Clear();
	if (!mpcRecords)
	{
		return Error("The code stopped in has no debug records");
	}
	msToken.szNext = szExpression;
	Next();
	if (!Expression(1, psResult) || !Suffix(psResult))
	{
		return false;
	}
	if (msToken.eToken != CPP65WT_End)
	{
		return Error("Unexpected", true);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65DebugWatch::GetError(void)
{
	return mszError.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// Binary operators binding at least iLeast tightly, after a unary
// expression.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Expression(int iLeast, SCPP65DebugValue* psValue)
{
	const SCPP65WatchOperator*	psOperator;
	SCPP65DebugValue			sRight;

	if (!Unary(psValue))
	{
		return false;
	}
	for (;;)
	{
		for (psOperator = gasCPP65WatchOperators; psOperator->szOperator && !Is(psOperator->szOperator); psOperator++)
		{
		}
		if (!psOperator->szOperator || (psOperator->iPrecedence < iLeast))
		{
			break;
		}
		Next();
		if (!Expression(psOperator->iPrecedence + 1, &sRight) || !Binary(psOperator->szOperator, psValue, &sRight))
		{
			return false;
		}
	}
	if (Is("="))
	{
		return Error("Assignments aren't evaluated");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// - + ! ~ * & sizeof and casts, then a postfix expression.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Unary(SCPP65DebugValue* psValue)
{
	SCPP65WatchPosition	sStart;
	SAsm65DebugType*	psType;
	uint32				uiType;
	uint32				uiCount;
	char				cOperator;

	if (Is("-") || Is("+") || Is("!") || Is("~"))
	{
		cOperator = msToken.szStart[0];
		Next();
		return Unary(psValue) && Negate(cOperator, psValue);
	}
	if (Accept("*"))
	{
		return Unary(psValue) && Dereference(psValue);
	}
	if (Accept("&"))
	{
		return Unary(psValue) && AddressOf(psValue);
	}
	if ((msToken.eToken == CPP65WT_Identifier) && (msToken.uiLength == 6) && (strncmp(msToken.szStart, "sizeof", 6) == 0))
	{
		Next();
		sStart = msToken;
		uiCount = 1;
		if (!(Accept("(") && TypeName(&uiType) && Accept(")")))
		{
			msToken = sStart;
			if (!Unary(psValue))
			{
				return false;
			}
			uiType = psValue->uiType;
			uiCount = (psValue->uiArrayCount != 0) ? psValue->uiArrayCount : 1;
		}
		psType = mpcRecords->GetType(uiType);
		psValue->Init(mpcRecords, IntegerType(2, false), (uint64)(psType ? psType->uiBytes : 0) * uiCount);
		return true;
	}
	if (Is("("))
	{
		//A cast if a type the records have is in the brackets, else a bracketed expression.
		sStart = msToken;
		Next();
		if (TypeName(&uiType) && Accept(")"))
		{
			return Unary(psValue) && Cast(uiType, psValue);
		}
		msToken = sStart;
	}
	return Postfix(psValue);
}


//////////////////////////////////////////////////////////////////////////
//
// A primary expression followed by . -> and [].
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Postfix(SCPP65DebugValue* psValue)
{
	SCPP65DebugValue	sMember;
	CChars				szName;
	bool				bArrow;

	if (!Primary(psValue))
	{
		return false;
	}
	for (;;)
	{
		bArrow = Is("->");
		if (bArrow || Is("."))
		{
			Next();
			if (msToken.eToken != CPP65WT_Identifier)
			{
				return Error("Expected a member's name", true);
			}
			if (bArrow && !Dereference(psValue))
			{
				return false;
			}
			szName.Init();
			TokenText(&szName);
			Next();
			if (Is("("))
			{
				szName.Kill();
				return Error("Functions aren't called");
			}
			if (!mpcValues->Member(psValue, szName.Text(), &sMember))
			{
				Error("No member named ");
				mszError.Append(&szName);
				szName.Kill();
				return false;
			}
			szName.Kill();
			*psValue = sMember;
		}
		else if (Accept("["))
		{
			if (!Expression(1, &sMember) || !Expect("]") || !Index(psValue, &sMember))
			{
				return false;
			}
		}
		else if (Is("("))
		{
			return Error("Functions aren't called");
		}
		else
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A literal, true, false, nullptr, a (qualified) name, or an expression in
// brackets.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Primary(SCPP65DebugValue* psValue)
{
	CChars	szName;
	bool	bResult;

	switch (msToken.eToken)
	{
	case CPP65WT_Integer:
	case CPP65WT_Float:
	case CPP65WT_Character:
		return Literal(psValue);
	case CPP65WT_Identifier:
		szName.Init();
		TokenText(&szName);
		Next();
		while (Accept("::"))
		{
			if (msToken.eToken != CPP65WT_Identifier)
			{
				szName.Kill();
				return Error("Expected a name", true);
			}
			szName.Append("::");
			TokenText(&szName);
			Next();
		}
		if (szName.Equals("true") || szName.Equals("false"))
		{
			psValue->Init(mpcRecords, BuiltIn("bool", ADT_Bool, 0, 1), szName.Equals("true") ? 1 : 0);
			bResult = true;
		}
		else if (szName.Equals("nullptr"))
		{
			psValue->Init(mpcRecords, PointerTo(BuiltIn("void", ADT_Void, 0, 0)), 0);
			bResult = true;
		}
		else
		{
			bResult = Name(szName.Text(), psValue);
		}
		szName.Kill();
		return bResult;
	default:
		if (Accept("("))
		{
			return Expression(1, psValue) && Expect(")");
		}
		return Error("Expected an expression", true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// ,x ,d ,s or ,n after the expression: how its value is shown.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Suffix(SCPP65DebugValue* psValue)
{
	SAsm65DebugType*	psType;
	char*				szEnd;
	uint32				uiCount;

	if (!Accept(","))
	{
		return true;
	}
	if ((msToken.eToken == CPP65WT_Identifier) && (msToken.uiLength == 1))
	{
		switch (msToken.szStart[0])
		{
		case 'x':
		case 'X':
		case 'h':
			psValue->uiFormat = CPP65DF_Hexadecimal;
			break;
		case 'd':
			psValue->uiFormat = CPP65DF_Decimal;
			break;
		case 's':
			psValue->uiFormat = CPP65DF_String;
			break;
		default:
			return Error("Unknown format", true);
		}
		Next();
		return true;
	}
	if (msToken.eToken != CPP65WT_Integer)
	{
		return Error("Unknown format", true);
	}

	//p,n: the n things p points at, as an array.
	uiCount = (uint32)strtoul(msToken.szStart, &szEnd, 0);
	Next();
	psType = GetType(psValue);
	if (!psType || (psType->uiKind != ADT_Pointer) || !Dereference(psValue) || (uiCount == 0))
	{
		return Error("Only a pointer is shown as an array");
	}
	psValue->uiArrayCount = uiCount;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A local in scope at the line (the innermost), a member of this, a
// static of the function's, a global, or an enumerator.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Name(const char* szName, SCPP65DebugValue* psValue)
{
	SAsm65DebugFunction*	psFunction;
	SAsm65DebugVariable*	psVariable;
	SAsm65DebugVariable*	psFound;
	SAsm65DebugVariable*	psThis;
	SAsm65DebugGlobal*		psGlobal;
	SAsm65DebugType*		psType;
	SAsm65DebugMember*		psMember;
	SCPP65DebugValue		sThis;
	size					ui;
	size					uiMember;

	psFunction = mpsFrame->psFunction;
	psFound = NULL;
	psThis = NULL;
	for (ui = 0; psFunction && (ui < psFunction->uiNumVariables); ui++)
	{
		psVariable = mpcRecords->GetVariable(psFunction, ui);
		if ((muiLine != 0) && ((muiLine < psVariable->uiFirstLine) || (muiLine > psVariable->uiLastLine)))
		{
			continue;
		}
		if (psVariable->szName.Equals(szName) && (!psFound || (psVariable->uiFirstLine >= psFound->uiFirstLine)))
		{
			psFound = psVariable;
		}
		psThis = psVariable->szName.Equals("this") ? psVariable : psThis;
	}
	if (psFound)
	{
		if (!mpcValues->Variable(mpcWalker, mpsFrame, psFound, psValue))
		{
			Error(szName);
			mszError.Append(" isn't kept anywhere here (optimised away)");
			return false;
		}
		return true;
	}
	if (psThis && mpcValues->Variable(mpcWalker, mpsFrame, psThis, &sThis) && mpcValues->Dereference(&sThis, &sThis) && mpcValues->Member(&sThis, szName, psValue))
	{
		return true;
	}

	psGlobal = psFunction ? mpcRecords->FindGlobal(szName, psFunction) : NULL;
	psGlobal = psGlobal ? psGlobal : mpcRecords->FindGlobal(szName, NULL);
	if (psGlobal)
	{
		mpcValues->Global(mpcRecords, psGlobal, (EAsm65StackSpace)mpsFrame->uiSpace, psValue);
		return true;
	}
	for (ui = 0; ui < mpcRecords->NumTypes(); ui++)
	{
		psType = mpcRecords->GetType((uint32)ui);
		for (uiMember = 0; (psType->uiKind == ADT_Enum) && (uiMember < psType->uiNumMembers); uiMember++)
		{
			psMember = mpcRecords->GetMember(psType, uiMember);
			if (psMember->szName.Equals(szName))
			{
				psValue->Init(mpcRecords, (uint32)ui, (uint64)psMember->iValue);
				return true;
			}
		}
	}
	Error(szName);
	mszError.Append(" is undefined");
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// An integer (an int, else a long, else a long long), a floating point
// number (a double, or with f a float) or a character.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Literal(SCPP65DebugValue* psValue)
{
	const char*	szText;
	char*		szEnd;
	uint64		uiValue;
	float		f;
	double		d;
	size		ui;

	szText = msToken.szStart;
	switch (msToken.eToken)
	{
	case CPP65WT_Float:
		d = strtod(szText, &szEnd);
		if ((*szEnd == 'f') || (*szEnd == 'F'))
		{
			f = (float)d;
			uiValue = 0;
			memcpy(&uiValue, &f, sizeof(f));
			psValue->Init(mpcRecords, BuiltIn("float", ADT_Float, 0, 4), uiValue);
		}
		else
		{
			memcpy(&uiValue, &d, sizeof(d));
			psValue->Init(mpcRecords, BuiltIn("double", ADT_Float, 0, 8), uiValue);
		}
		break;
	case CPP65WT_Character:
		uiValue = (uint8)szText[1];
		if (szText[1] == '\\')
		{
			for (ui = 0; (ui < sizeof(gszCPP65WatchEscapes) - 1) && (gszCPP65WatchEscapes[ui] != szText[2]); ui += 2)
			{
			}
			if (ui >= sizeof(gszCPP65WatchEscapes) - 1)
			{
				return Error("Unknown escape", true);
			}
			uiValue = (uint8)gszCPP65WatchEscapes[ui + 1];
		}
		psValue->Init(mpcRecords, BuiltIn("char", ADT_Integer, ADTF_Signed | ADTF_Character, 1), uiValue);
		break;
	default:
		uiValue = strtoull(szText, &szEnd, 0);
		psValue->Init(mpcRecords, (uiValue <= 0x7FFF) ? IntegerType(2, true) : ((uiValue <= 0x7FFFFFFF) ? IntegerType(4, true) : IntegerType(8, uiValue <= 0x7FFFFFFFFFFFFFFFull)), uiValue);
		break;
	}
	psValue->uiSpace = mpsFrame->uiSpace;
	Next();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A type the records have (or a fundamental one), and its *'s.  False,
// without an error, if there isn't one here.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::TypeName(uint32* puiType)
{
	CChars	szName;
	uint32	uiType;

	//Words that name a type together (unsigned long), or a class's (qualified) name; const and volatile change nothing.
	szName.Init();
	while (msToken.eToken == CPP65WT_Identifier)
	{
		if (!(msToken.uiLength == 5 && strncmp(msToken.szStart, "const", 5) == 0) && !(msToken.uiLength == 8 && strncmp(msToken.szStart, "volatile", 8) == 0))
		{
			szName.Append(szName.Empty() ? "" : " ");
			TokenText(&szName);
		}
		Next();
		while (Accept("::") && (msToken.eToken == CPP65WT_Identifier))
		{
			szName.Append("::");
			TokenText(&szName);
			Next();
		}
	}
	if (szName.Equals("unsigned"))
	{
		szName.Append(" int");
	}
	uiType = szName.Empty() ? ASM65_DEBUG_NONE : FindTypeNamed(szName.Text());
	szName.Kill();
	if (uiType == ASM65_DEBUG_NONE)
	{
		return false;
	}
	while (Accept("*"))
	{
		uiType = PointerTo(uiType);
	}
	*puiType = uiType;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// psLeft = psLeft szOperator psRight: integers as C++ promotes them (to at
// least an int), floats as doubles, a pointer plus or less an integer, a
// pointer less a pointer, comparisons and && || as bools.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Binary(const char* szOperator, SCPP65DebugValue* psLeft, SCPP65DebugValue* psRight)
{
	SAsm65DebugType*	psLeftType;
	SAsm65DebugType*	psRightType;
	SAsm65DebugType*	psPointee;
	SAsm65DebugType*	psInteger;
	SAsm65DebugType*	apsIntegers[2];
	size				ui;
	int64				iLeft;
	int64				iRight;
	int64				iResult;
	double				dLeft;
	double				dRight;
	double				dResult;
	bool				bLeftFloat;
	bool				bRightFloat;
	bool				bSigned;
	uint32				uiBytes;
	uint32				uiSize;
	uint32				uiBool;
	char				c;

	if (!Decay(psLeft) || !Decay(psRight) || !Number(psLeft, &iLeft, &dLeft, &bLeftFloat) || !Number(psRight, &iRight, &dRight, &bRightFloat))
	{
		return false;
	}
	psLeftType = GetType(psLeft);
	psRightType = GetType(psRight);
	c = szOperator[0];
	uiBool = BuiltIn("bool", ADT_Bool, 0, 1);

	if ((strcmp(szOperator, "&&") == 0) || (strcmp(szOperator, "||") == 0))
	{
		iResult = (c == '&') ? ((bLeftFloat ? (dLeft != 0) : (iLeft != 0)) && (bRightFloat ? (dRight != 0) : (iRight != 0))) :
							   ((bLeftFloat ? (dLeft != 0) : (iLeft != 0)) || (bRightFloat ? (dRight != 0) : (iRight != 0)));
		psLeft->Init(mpcRecords, uiBool, (uint64)iResult);
		return true;
	}
	if ((c == '<') || (c == '>') || (c == '=') || (c == '!'))
	{
		if ((strcmp(szOperator, "<<") != 0) && (strcmp(szOperator, ">>") != 0))
		{
			dLeft = bLeftFloat ? dLeft : (double)iLeft;
			dRight = bRightFloat ? dRight : (double)iRight;
			if (!bLeftFloat && !bRightFloat)
			{
				iResult = (strcmp(szOperator, "<") == 0) ? (iLeft < iRight) : (strcmp(szOperator, ">") == 0) ? (iLeft > iRight) : (strcmp(szOperator, "<=") == 0) ? (iLeft <= iRight) :
						  (strcmp(szOperator, ">=") == 0) ? (iLeft >= iRight) : (strcmp(szOperator, "==") == 0) ? (iLeft == iRight) : (iLeft != iRight);
			}
			else
			{
				iResult = (strcmp(szOperator, "<") == 0) ? (dLeft < dRight) : (strcmp(szOperator, ">") == 0) ? (dLeft > dRight) : (strcmp(szOperator, "<=") == 0) ? (dLeft <= dRight) :
						  (strcmp(szOperator, ">=") == 0) ? (dLeft >= dRight) : (strcmp(szOperator, "==") == 0) ? (dLeft == dRight) : (dLeft != dRight);
			}
			psLeft->Init(mpcRecords, uiBool, (uint64)iResult);
			return true;
		}
	}

	//A pointer and an integer (or two pointers less each other): in what it points at.
	if ((psLeftType->uiKind == ADT_Pointer) || (psRightType->uiKind == ADT_Pointer))
	{
		if ((c != '+') && (c != '-'))
		{
			return Error("Only + and - work on pointers");
		}
		psPointee = mpcRecords->GetType((psLeftType->uiKind == ADT_Pointer) ? psLeftType->uiTarget : psRightType->uiTarget);
		uiSize = (psPointee && (psPointee->uiBytes != 0)) ? psPointee->uiBytes : 1;
		if ((psLeftType->uiKind == ADT_Pointer) && (psRightType->uiKind == ADT_Pointer))
		{
			if (c != '-')
			{
				return Error("Pointers can't be added");
			}
			psLeft->Init(mpcRecords, IntegerType(4, true), (uint64)((iLeft - iRight) / (int64)uiSize));
			psLeft->uiSpace = mpsFrame->uiSpace;
			return true;
		}
		if (psLeftType->uiKind == ADT_Pointer)
		{
			iResult = iLeft + ((c == '+') ? iRight : -iRight) * (int64)uiSize;
			psLeft->Init(mpcRecords, psLeft->uiType, (uint64)iResult & 0xFFFFFF);
		}
		else
		{
			if (c != '+')
			{
				return Error("A pointer can't be taken from a number");
			}
			psLeft->Init(mpcRecords, psRight->uiType, (uint64)(iRight + iLeft * (int64)uiSize) & 0xFFFFFF);
		}
		psLeft->uiSpace = mpsFrame->uiSpace;
		return true;
	}

	if (bLeftFloat || bRightFloat)
	{
		dLeft = bLeftFloat ? dLeft : (double)iLeft;
		dRight = bRightFloat ? dRight : (double)iRight;
		switch (c)
		{
		case '+':	dResult = dLeft + dRight;	break;
		case '-':	dResult = dLeft - dRight;	break;
		case '*':	dResult = dLeft * dRight;	break;
		case '/':	dResult = dLeft / dRight;	break;
		default:
			return Error("That operator needs integers");
		}
		memcpy(&iResult, &dResult, sizeof(dResult));
		psLeft->Init(mpcRecords, BuiltIn("double", ADT_Float, 0, 8), (uint64)iResult);
		return true;
	}

	//Integers: as wide as the wider, at least an int (a bit field narrower than one is an int); unsigned if the widest is.
	uiBytes = 2;
	bSigned = true;
	apsIntegers[0] = (psLeftType->uiKind == ADT_Enum) ? mpcRecords->GetType(psLeftType->uiTarget) : psLeftType;
	apsIntegers[1] = (psRightType->uiKind == ADT_Enum) ? mpcRecords->GetType(psRightType->uiTarget) : psRightType;
	for (ui = 0; ui < 2; ui++)
	{
		psInteger = apsIntegers[ui];
		if (psInteger && (psInteger->uiKind == ADT_Integer) && (psInteger->uiBytes >= uiBytes) && (((ui == 0) ? psLeft->uiBits : psRight->uiBits) == 0))
		{
			bSigned = (psInteger->uiBytes > uiBytes) ? ((psInteger->uiFlags & ADTF_Signed) != 0) : (bSigned && ((psInteger->uiFlags & ADTF_Signed) || (psInteger->uiBytes < 2)));
			uiBytes = psInteger->uiBytes;
		}
	}
	if (((c == '/') || (c == '%')) && (iRight == 0))
	{
		return Error("Divided by zero");
	}
	switch (c)
	{
	case '+':	iResult = iLeft + iRight;	break;
	case '-':	iResult = iLeft - iRight;	break;
	case '*':	iResult = iLeft * iRight;	break;
	case '/':	iResult = bSigned ? (iLeft / iRight) : (int64)((uint64)iLeft / (uint64)iRight);	break;
	case '%':	iResult = bSigned ? (iLeft % iRight) : (int64)((uint64)iLeft % (uint64)iRight);	break;
	case '&':	iResult = iLeft & iRight;	break;
	case '|':	iResult = iLeft | iRight;	break;
	case '^':	iResult = iLeft ^ iRight;	break;
	case '<':	iResult = (int64)((uint64)iLeft << (iRight & 63));	break;
	default:	iResult = bSigned ? (iLeft >> (iRight & 63)) : (int64)((uint64)iLeft >> (iRight & 63));	break;
	}
	psLeft->Init(mpcRecords, IntegerType(uiBytes, bSigned), (uint64)iResult);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// - + ~ on a number (an integer promoted to an int), ! as a bool.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Negate(char cOperator, SCPP65DebugValue* psValue)
{
	SCPP65DebugValue	sZero;

	if (cOperator == '!')
	{
		sZero.Init(mpcRecords, IntegerType(2, true), 0);
		return Binary("==", psValue, &sZero);
	}
	if (cOperator == '~')
	{
		sZero.Init(mpcRecords, IntegerType(2, true), (uint64)-1);
		return Binary("^", psValue, &sZero);
	}
	sZero.Init(mpcRecords, IntegerType(2, true), 0);
	if (cOperator == '+')
	{
		return Binary("+", psValue, &sZero);
	}
	if (!Binary("-", &sZero, psValue))
	{
		return false;
	}
	*psValue = sZero;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// (type)value: a number or pointer made the type's; an object in memory
// seen as another type there.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Cast(uint32 uiType, SCPP65DebugValue* psValue)
{
	SAsm65DebugType*	psType;
	int64				iValue;
	double				d;
	float				f;
	bool				bFloat;
	uint64				uiValue;
	uint64				uiMask;

	psType = mpcRecords->GetType(uiType);
	if ((psType->uiKind == ADT_Class) || (psType->uiKind == ADT_Array) || (psType->uiKind == ADT_Reference))
	{
		if (!psValue->bInMemory)
		{
			return Error("Only an object in memory can be seen as another type");
		}
		psValue->uiType = (psType->uiKind == ADT_Reference) ? psType->uiTarget : uiType;
		psValue->uiArrayCount = 0;
		return true;
	}
	if (!Decay(psValue) || !Number(psValue, &iValue, &d, &bFloat))
	{
		return false;
	}
	switch (psType->uiKind)
	{
	case ADT_Float:
		d = bFloat ? d : (double)iValue;
		uiValue = 0;
		if (psType->uiBytes == sizeof(float))
		{
			f = (float)d;
			memcpy(&uiValue, &f, sizeof(f));
		}
		else
		{
			memcpy(&uiValue, &d, sizeof(d));
		}
		break;
	case ADT_Bool:
		uiValue = (bFloat ? (d != 0) : (iValue != 0)) ? 1 : 0;
		break;
	default:
		uiValue = bFloat ? (uint64)(int64)d : (uint64)iValue;
		uiMask = (psType->uiBytes >= 8) ? ~(uint64)0 : ((uint64)1 << (psType->uiBytes * 8)) - 1;
		uiValue &= uiMask;
		break;
	}
	psValue->Init(mpcRecords, uiType, uiValue);
	psValue->uiSpace = mpsFrame->uiSpace;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// psValue[psIndex]: an element of an array, or what a pointer points at
// that many along.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Index(SCPP65DebugValue* psValue, SCPP65DebugValue* psIndex)
{
	SAsm65DebugType*	psType;
	SAsm65DebugType*	psPointee;
	int64				iIndex;
	int64				iAddress;
	double				d;
	bool				bFloat;

	if (!Number(psIndex, &iIndex, &d, &bFloat) || bFloat || !Decay(psValue))
	{
		return Error("An index must be an integer");
	}
	psType = GetType(psValue);
	if (!psType || (psType->uiKind != ADT_Pointer) || !Number(psValue, &iAddress, &d, &bFloat))
	{
		return Error("Only an array or a pointer is indexed");
	}
	psPointee = mpcRecords->GetType(psType->uiTarget);
	if (!psPointee || (psPointee->uiBytes == 0))
	{
		return Error("What it points at has no size");
	}
	psValue->Init(mpcRecords, psType->uiTarget, (EAsm65StackSpace)mpsFrame->uiSpace, (uint32)(iAddress + iIndex * (int64)psPointee->uiBytes) & 0xFFFFFF);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// *psValue: what a pointer points at, an array's first element.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Dereference(SCPP65DebugValue* psValue)
{
	SCPP65DebugValue	sTarget;

	if (!Decay(psValue) || !mpcValues->Dereference(psValue, &sTarget))
	{
		return Error("Only a pointer to something (not null) is dereferenced");
	}
	*psValue = sTarget;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// &psValue: a pointer to something in memory.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::AddressOf(SCPP65DebugValue* psValue)
{
	uint32	uiAddress;
	uint32	uiType;

	if (!psValue->bInMemory || (psValue->uiBits != 0))
	{
		return Error("Only something in memory has an address");
	}
	uiAddress = psValue->uiAddress;
	uiType = psValue->uiType;
	psValue->Init(mpcRecords, PointerTo(uiType), uiAddress);
	psValue->uiSpace = mpsFrame->uiSpace;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An array (or one a watch shows) as a pointer to its first element; any
// other value as it is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Decay(SCPP65DebugValue* psValue)
{
	SAsm65DebugType*	psType;
	uint32				uiElement;
	uint8				uiSpace;

	psType = GetType(psValue);
	if (!psType)
	{
		return Error("Has no type");
	}
	if ((psValue->uiArrayCount == 0) && (psType->uiKind != ADT_Array))
	{
		return true;
	}
	if (!psValue->bInMemory)
	{
		return Error("An array not in memory");
	}
	uiElement = (psValue->uiArrayCount != 0) ? psValue->uiType : psType->uiTarget;
	uiSpace = psValue->uiSpace;
	psValue->Init(mpcRecords, PointerTo(uiElement), psValue->uiAddress);
	psValue->uiSpace = uiSpace;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A value's number: an integer (bool, char, enum, a pointer's address) or
// a float's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Number(SCPP65DebugValue* psValue, int64* piValue, double* pdValue, bool* pbFloat)
{
	SAsm65DebugType*	psType;
	uint64				uiValue;
	float				f;

	psType = GetType(psValue);
	if (!psType || (psType->uiKind == ADT_Class) || (psType->uiKind == ADT_Void) || (psType->uiKind == ADT_Function) || (psType->uiKind == ADT_Array))
	{
		return Error("Not a number");
	}
	if (!mpcValues->Read(psValue, &uiValue))
	{
		return Error("Its memory can't be read");
	}
	*pbFloat = psType->uiKind == ADT_Float;
	*piValue = (int64)uiValue;
	if (psType->uiKind == ADT_Pointer)
	{
		*piValue = (int64)(uiValue & 0xFFFFFF);
	}
	else if (*pbFloat && (psType->uiBytes == sizeof(float)))
	{
		memcpy(&f, &uiValue, sizeof(f));
		*pdValue = f;
	}
	else if (*pbFloat)
	{
		memcpy(pdValue, &uiValue, sizeof(*pdValue));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// int, long or long long (the first of the fundamental types so wide),
// unsigned or not.
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65DebugWatch::IntegerType(size uiBytes, bool bSigned)
{
	const SCPP65WatchBuiltIn*	psBuiltIn;

	for (psBuiltIn = gasCPP65WatchBuiltIns; psBuiltIn->szName; psBuiltIn++)
	{
		if ((psBuiltIn->uiKind == ADT_Integer) && (psBuiltIn->uiBytes == uiBytes) && (((psBuiltIn->uiFlags & ADTF_Signed) != 0) == bSigned) && !(psBuiltIn->uiFlags & ADTF_Character))
		{
			return BuiltIn(psBuiltIn->szName, (EAsm65DebugType)psBuiltIn->uiKind, psBuiltIn->uiFlags, psBuiltIn->uiBytes);
		}
	}
	return ASM65_DEBUG_NONE;
}


//////////////////////////////////////////////////////////////////////////
//
// The records' type named (and keyed) szName, added if they haven't it.
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65DebugWatch::BuiltIn(const char* szName, EAsm65DebugType eKind, uint8 uiFlags, uint32 uiBytes)
{
	uint32	uiType;

	uiType = mpcRecords->FindTypeIndex(szName);
	if (uiType == ASM65_DEBUG_NONE)
	{
		mpcRecords->AddType(szName, szName, eKind, uiFlags, uiBytes, ASM65_DEBUG_NONE, 0);
		uiType = (uint32)mpcRecords->NumTypes() - 1;
	}
	return uiType;
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer to uiType, keyed as the compiler keys one (*int).
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65DebugWatch::PointerTo(uint32 uiType)
{
	SAsm65DebugType*	psType;
	CChars				szKey;
	CChars				szName;
	uint32				uiPointer;

	psType = mpcRecords->GetType(uiType);
	szKey.Init("*")->Append(&psType->szKey);
	uiPointer = mpcRecords->FindTypeIndex(szKey.Text());
	if (uiPointer == ASM65_DEBUG_NONE)
	{
		szName.Init(psType->szName)->Append('*');
		mpcRecords->AddType(szKey.Text(), szName.Text(), ADT_Pointer, 0, 4, uiType, 0);
		szName.Kill();
		uiPointer = (uint32)mpcRecords->NumTypes() - 1;
	}
	szKey.Kill();
	return uiPointer;
}


//////////////////////////////////////////////////////////////////////////
//
// The type named szName: the records' (a complete one before one only
// declared), or a fundamental one.  ASM65_DEBUG_NONE if there is none.
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPP65DebugWatch::FindTypeNamed(const char* szName)
{
	const SCPP65WatchBuiltIn*	psBuiltIn;
	SAsm65DebugType*			psType;
	uint32						uiFound;
	size						ui;

	uiFound = ASM65_DEBUG_NONE;
	for (ui = 0; ui < mpcRecords->NumTypes(); ui++)
	{
		psType = mpcRecords->GetType((uint32)ui);
		if (psType->szName.Equals(szName) && ((uiFound == ASM65_DEBUG_NONE) || !(psType->uiFlags & ADTF_Incomplete)))
		{
			uiFound = (uint32)ui;
		}
	}
	for (psBuiltIn = gasCPP65WatchBuiltIns; psBuiltIn->szName && (uiFound == ASM65_DEBUG_NONE); psBuiltIn++)
	{
		if (strcmp(psBuiltIn->szName, szName) == 0)
		{
			uiFound = BuiltIn(psBuiltIn->szName, (EAsm65DebugType)psBuiltIn->uiKind, psBuiltIn->uiFlags, psBuiltIn->uiBytes);
		}
	}
	return uiFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65DebugType* CCPP65DebugWatch::GetType(SCPP65DebugValue* psValue)
{
	return (psValue->uiType == ASM65_DEBUG_NONE) ? NULL : mpcRecords->GetType(psValue->uiType);
}


//////////////////////////////////////////////////////////////////////////
//
// The next token: a name, a number, a character, a punctuator.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWatch::Next(void)
{
	const char*		sz;
	const char**	pszPunctuator;
	bool			bFloat;

	sz = msToken.szNext;
	while ((*sz == ' ') || (*sz == '\t'))
	{
		sz++;
	}
	msToken.szStart = sz;
	msToken.eToken = CPP65WT_Punctuator;
	if (*sz == '\0')
	{
		msToken.eToken = CPP65WT_End;
	}
	else if (isalpha((uint8)*sz) || (*sz == '_'))
	{
		msToken.eToken = CPP65WT_Identifier;
		while (isalnum((uint8)*sz) || (*sz == '_'))
		{
			sz++;
		}
	}
	else if (isdigit((uint8)*sz) || ((*sz == '.') && isdigit((uint8)sz[1])))
	{
		bFloat = false;
		if ((sz[0] == '0') && ((sz[1] == 'x') || (sz[1] == 'X')))
		{
			for (sz += 2; isxdigit((uint8)*sz); sz++)
			{
			}
		}
		else
		{
			for (; isdigit((uint8)*sz) || (*sz == '.'); sz++)
			{
				bFloat |= *sz == '.';
			}
			if ((*sz == 'e') || (*sz == 'E'))
			{
				bFloat = true;
				sz += ((sz[1] == '+') || (sz[1] == '-')) ? 2 : 1;
				while (isdigit((uint8)*sz))
				{
					sz++;
				}
			}
		}
		for (; (*sz == 'u') || (*sz == 'U') || (*sz == 'l') || (*sz == 'L') || (*sz == 'f') || (*sz == 'F'); sz++)
		{
			bFloat |= bFloat || (*sz == 'f') || (*sz == 'F');
		}
		msToken.eToken = bFloat ? CPP65WT_Float : CPP65WT_Integer;
	}
	else if ((sz[0] == '\'') && (sz[1] != '\0'))
	{
		sz += (sz[1] == '\\') ? 3 : 2;
		if (*sz == '\'')
		{
			msToken.eToken = CPP65WT_Character;
			sz++;
		}
	}
	else
	{
		for (pszPunctuator = gaszCPP65WatchPunctuators; *pszPunctuator && (strncmp(sz, *pszPunctuator, 2) != 0); pszPunctuator++)
		{
		}
		sz += *pszPunctuator ? 2 : 1;
	}
	msToken.uiLength = (size)(sz - msToken.szStart);
	msToken.szNext = sz;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Is(const char* szPunctuator)
{
	return (msToken.eToken == CPP65WT_Punctuator) && (msToken.uiLength == strlen(szPunctuator)) && (strncmp(msToken.szStart, szPunctuator, msToken.uiLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Accept(const char* szPunctuator)
{
	if (!Is(szPunctuator))
	{
		return false;
	}
	Next();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Expect(const char* szPunctuator)
{
	if (Accept(szPunctuator))
	{
		return true;
	}
	Error("Expected ");
	mszError.Append(szPunctuator);
	return Error("", true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWatch::TokenText(CChars* pszDest)
{
	pszDest->AppendSubString(msToken.szStart, msToken.szStart + msToken.uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// Notes why the expression can't be evaluated (with the token it is at
// when bToken).  Returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWatch::Error(const char* szError, bool bToken)
{
	mszError.Append(szError);
	if (bToken)
	{
		mszError.Append(mszError.Empty() ? "'" : " '");
		if (msToken.eToken == CPP65WT_End)
		{
			mszError.Append("end of expression");
		}
		else
		{
			TokenText(&mszError);
		}
		mszError.Append('\'');
	}
	return false;
}

