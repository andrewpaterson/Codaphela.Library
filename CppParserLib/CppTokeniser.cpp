/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BaseLib/Numbers.h"
#include "CppTokeniser.h"
#include "CTBoolean.h"
#include "CTChar.h"
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CTString.h"
#include "CTWideString.h"
#include "CTDouble.h"
#include "CTFloat.h"
#include "CTLongDouble.h"
#include "CTInt.h"
#include "CTLongLong.h"
#include "CTShort.h"
#include "PPText.h"
#include "CTNewLine.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::Init(void)
{
	mcStack.Init(128 KB);

	mcIdentifiers.Init();
	mcTokens.Init();

	mcReservedWords.Init();
	mcReservedWords.AddIndex("(");
	mcReservedWords.AddIndex(")");
	mcReservedWords.AddIndex("{");
	mcReservedWords.AddIndex("}");
	mcReservedWords.AddIndex("[");
	mcReservedWords.AddIndex("]");
	mcReservedWords.AddIndex("<<=");
	mcReservedWords.AddIndex(">>=");
	mcReservedWords.AddIndex("++");
	mcReservedWords.AddIndex("--");
	mcReservedWords.AddIndex("==");
	mcReservedWords.AddIndex("!=");
	mcReservedWords.AddIndex(">=");
	mcReservedWords.AddIndex("<=");
	mcReservedWords.AddIndex("||");
	mcReservedWords.AddIndex("&&");
	mcReservedWords.AddIndex("+=");
	mcReservedWords.AddIndex("-=");
	mcReservedWords.AddIndex("*=");
	mcReservedWords.AddIndex("/=");
	mcReservedWords.AddIndex("%=");
	mcReservedWords.AddIndex("&=");
	mcReservedWords.AddIndex("|=");
	mcReservedWords.AddIndex("^=");
	mcReservedWords.AddIndex("<<");
	mcReservedWords.AddIndex(">>");
	mcReservedWords.AddIndex("+");
	mcReservedWords.AddIndex("-");
	mcReservedWords.AddIndex("*");
	mcReservedWords.AddIndex("/");
	mcReservedWords.AddIndex("%");
	mcReservedWords.AddIndex("=");
	mcReservedWords.AddIndex("!");
	mcReservedWords.AddIndex("&");
	mcReservedWords.AddIndex("|");
	mcReservedWords.AddIndex("^");
	mcReservedWords.AddIndex(">");
	mcReservedWords.AddIndex("<");
	mcReservedWords.AddIndex("~");
	mcReservedWords.AddIndex("::*");
	mcReservedWords.AddIndex("::");
	mcReservedWords.AddIndex("?");
	mcReservedWords.AddIndex(":");
	mcReservedWords.AddIndex(";");
	mcReservedWords.AddIndex("->");
	mcReservedWords.AddIndex(".");
	mcReservedWords.AddIndex("->*");
	mcReservedWords.AddIndex(".*");
	mcReservedWords.AddIndex("...");
	mcReservedWords.AddIndex(",");
	mcReservedWords.AddIndex("constauto");
	mcReservedWords.AddIndex("double");
	mcReservedWords.AddIndex("float");
	mcReservedWords.AddIndex("int");
	mcReservedWords.AddIndex("short");
	mcReservedWords.AddIndex("struct");
	mcReservedWords.AddIndex("unsigned");
	mcReservedWords.AddIndex("break");
	mcReservedWords.AddIndex("continue");
	mcReservedWords.AddIndex("else");
	mcReservedWords.AddIndex("for");
	mcReservedWords.AddIndex("long");
	mcReservedWords.AddIndex("signed");
	mcReservedWords.AddIndex("switch");
	mcReservedWords.AddIndex("void");
	mcReservedWords.AddIndex("case");
	mcReservedWords.AddIndex("default");
	mcReservedWords.AddIndex("enum");
	mcReservedWords.AddIndex("goto");
	mcReservedWords.AddIndex("register");
	mcReservedWords.AddIndex("sizeof");
	mcReservedWords.AddIndex("typedef");
	mcReservedWords.AddIndex("volatile");
	mcReservedWords.AddIndex("char");
	mcReservedWords.AddIndex("do");
	mcReservedWords.AddIndex("extern");
	mcReservedWords.AddIndex("if");
	mcReservedWords.AddIndex("return");
	mcReservedWords.AddIndex("static");
	mcReservedWords.AddIndex("union");
	mcReservedWords.AddIndex("while");
	mcReservedWords.AddIndex("asm");
	mcReservedWords.AddIndex("dynamic_cast");
	mcReservedWords.AddIndex("namespace");
	mcReservedWords.AddIndex("reinterpret_cast");
	mcReservedWords.AddIndex("try");
	mcReservedWords.AddIndex("bool");
	mcReservedWords.AddIndex("explicit");
	mcReservedWords.AddIndex("new");
	mcReservedWords.AddIndex("static_cast");
	mcReservedWords.AddIndex("typeid");
	mcReservedWords.AddIndex("catch");
	mcReservedWords.AddIndex("false");
	mcReservedWords.AddIndex("operator");
	mcReservedWords.AddIndex("template");
	mcReservedWords.AddIndex("typename");
	mcReservedWords.AddIndex("class");
	mcReservedWords.AddIndex("friend");
	mcReservedWords.AddIndex("private");
	mcReservedWords.AddIndex("this");
	mcReservedWords.AddIndex("using");
	mcReservedWords.AddIndex("const_cast");
	mcReservedWords.AddIndex("inline");
	mcReservedWords.AddIndex("public");
	mcReservedWords.AddIndex("throw");
	mcReservedWords.AddIndex("virtual");
	mcReservedWords.AddIndex("delete");
	mcReservedWords.AddIndex("mutable");
	mcReservedWords.AddIndex("protected");
	mcReservedWords.AddIndex("true");
	mcReservedWords.AddIndex("wchar_t");
	mcReservedWords.AddIndex("and");
	mcReservedWords.AddIndex("bitand");
	mcReservedWords.AddIndex("compl");
	mcReservedWords.AddIndex("not_eq");
	mcReservedWords.AddIndex("or_eq");
	mcReservedWords.AddIndex("xor_eq");
	mcReservedWords.AddIndex("and_eq");
	mcReservedWords.AddIndex("bitor");
	mcReservedWords.AddIndex("not");
	mcReservedWords.AddIndex("or");
	mcReservedWords.AddIndex("xor");

	if (mcReservedWords.mcWords.NumElements() != NUM_CPP_TOKENS)
	{
		gcUserError.Set("Reserved words is out of synch!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::Kill(void)
{
	int				i;
	CCppToken*		pcToken;

	for (i = 0; i < mcTokens.NumElements(); i++)
	{
		pcToken = *mcTokens.Get(i);
		pcToken->Kill();
	}
	mcTokens.Kill();
	mcStack.Kill();

	mcIdentifiers.Kill();
	mcReservedWords.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::Line(void)
{
	BOOL		bResult;
	int			iTokenNum;

	iTokenNum = -1;
	while (mcParser.HasToken())
	{
		iTokenNum++;
		bResult = Literal();
		if (bResult)
		{
			continue;
		}

		bResult = ReservedWord();
		if (bResult)
		{
			continue;
		}

		bResult = Identifier();
		if (bResult)
		{
			continue;
		}
		
		if (!mcParser.HasToken())
		{
			//The last token must have been white space.
			continue;
		}

		//If there are still tokens left then we could not parse this line.
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::Tokenise(CPPTokenHolder* pcSource)
{
	int			i;
	BOOL		bResult;
	CPPToken*	pcToken;
	CPPLine*	pcLine;
	CCTNewLine*	pcNewLine;

	mcTokens.Kill();
	mcTokens.Init();

	bResult = TRUE;
	mcParser.Init();
	for (i = 0; i < pcSource->mcArray.NumElements(); i++)
	{
		pcToken = *pcSource->mcArray.Get(i);
		if (pcToken->IsLine())
		{
			pcLine = (CPPLine*)pcToken;
			mcParser.Kill();
			mcParser.Init(pcLine);
			bResult = Line();
			if (!bResult)
			{
				break;
			}
			pcNewLine = CCTNewLine::Construct(mcStack.Add(sizeof(CCTNewLine)));
			AddToken(pcNewLine);
		}
	}

	if (bResult)
	{
		mcParser.Kill();
		return TRUE;
	}
	else
	{
		PrintError(pcLine);
		mcParser.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::PrintError(CPPLine* pcLine)
{
	CChars		sz;

	sz.Init("Tokenising Broke (");
	sz.Append(mcParser.Line()+1);
	sz.Append(", ");
	sz.Append(mcParser.Column()+1);
	sz.Append("): ");

	sz.Append("\n");
	sz.Append("Line[");
	pcLine->AppendNoNewline(&sz);
	sz.Append("]\nRemain[");
	mcParser.AppendRemaining(&sz);
	sz.Append("]\n");
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::PrintTokens(void)
{
	int			j;
	CCppToken*	pcToken;
	CChars		sz;
	BOOL		bPrevNewLine;

	bPrevNewLine = FALSE;
	sz.Init();
	for (j = 0; j < mcTokens.NumElements(); j++)
	{
		pcToken = *mcTokens.Get(j);
		if (!pcToken->IsHelper())
		{
			pcToken->Append(&sz, this);
			sz.Append(" ");
			bPrevNewLine = FALSE;
		}
		else
		{
			if (!bPrevNewLine)
			{
				pcToken->Append(&sz, this);
			}
			bPrevNewLine = TRUE;
		}
		if (sz.Length() > 9950)
		{
			sz.Dump();
			sz.Kill();
			sz.Init();
		}
	}
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::Literal(void)
{
	BOOL		bReturn;

	bReturn = BooleanLiteral();
	if (bReturn)
	{
		return bReturn;
	}

	bReturn = SingleQuotedLiteral(FALSE);
	if (bReturn)
	{
		return bReturn;
	}

	bReturn = ShortLiteral();
	if (bReturn)
	{
		return bReturn;
	}

	bReturn = StringLiteral();
	if (bReturn)
	{
		return bReturn;
	}

	bReturn = IntegerLiteral();
	if (bReturn)
	{
		return bReturn;
	}

	//Float and Double.  *Must* come after Integer
	bReturn = DoubleLiteral();
	if (bReturn)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::ReservedWord(void)
{
	int					iIndex;
	CCTReservedWord*	pcReservedWord;
	CChars*				pszWord;
	CPPText*			pcText;
	char				szTemp[1024];
	int					iLen;
	CPPToken*			pcToken;

	mcParser.SkipWhiteSpace();

	if (!mcParser.HasToken())
	{
		return FALSE;
	}

	pcToken = mcParser.GetToken();
	if (pcToken->IsText())
	{
		pcText = (CPPText*)pcToken;
		if (pcText->meType != PPT_Decorator)
		{
			iIndex = mcReservedWords.GetIndex(pcText->mcText.msz, pcText->mcText.EndInclusive(), FALSE);
		}
		else
		{
			mcParser.ToDecortatorString(szTemp, 1024, &iLen);
			iIndex = mcReservedWords.GetIndex(szTemp, szTemp+(iLen-1), FALSE);
		}
		if (iIndex != -1)
		{
			pszWord = mcReservedWords.mcWords.Get(iIndex);

			//Move the parser on.
			if (pcText->meType != PPT_Decorator)
			{
				//Just skip one token.
				mcParser.NextToken();
			}
			else
			{
				//Each letter was a single decorator.  Skip them all.
				mcParser.NextToken(pszWord->Length());
			}

			pcReservedWord = CCTReservedWord::Construct(mcStack.Add(sizeof(CCTReservedWord)));
			pcReservedWord->Init(iIndex);
			AddToken(pcReservedWord);
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::Identifier(void)
{
	int					iIndex;
	CCTIdentifier*		pcIdentifier;
	CExternalString		cIdentifier;
	BOOL				bResult;

	bResult = mcParser.GetIdentifier(&cIdentifier);
	if (bResult)
	{
		iIndex = mcIdentifiers.AddOrGet(cIdentifier.msz, cIdentifier.EndInclusive());
		if (iIndex != -1)
		{
			pcIdentifier = CCTIdentifier::Construct(mcStack.Add(sizeof(CCTIdentifier)));
			pcIdentifier->Init(iIndex);
			AddToken(pcIdentifier);
			return TRUE;
		}
		return FALSE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::DoubleLiteral(void)
{
	long double				f;
	unsigned long long int	ulliExponent;
	long double				fTen;
	BOOL					bResult;

	mcParser.PushPosition();

	bResult = mcParser.GetFloat(&f);
	if (bResult)
	{
		bResult = mcParser.GetExactIdentifier("e", FALSE, FALSE);
		if (bResult)
		{
			bResult = mcParser.GetInteger(&ulliExponent, FALSE, TRUE);
			if (bResult)
			{
				fTen = 10.0;
				f = pow(fTen, (int)ulliExponent) * f;
				DoubleType(f);
				mcParser.PassPosition();
				return TRUE;
			}
			else
			{
				mcParser.PopPosition();
				return FALSE;
			}
		}
		else
		{
			DoubleType(f);
			mcParser.PassPosition();
			return TRUE;
		}
	}
	else
	{
		mcParser.PopPosition();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::BooleanLiteral(void)
{
	BOOL			bResult;
	CCTBoolean* 	pcBoolean;

	bResult = mcParser.GetExactIdentifier("true", TRUE, TRUE);
	if (bResult)
	{
		pcBoolean = CCTBoolean::Construct(mcStack.Add(sizeof(CCTBoolean)));
		pcBoolean->Init(true);
		AddToken(pcBoolean);
		return TRUE;
	}
	else
	{
		bResult = mcParser.GetExactIdentifier("false", TRUE, TRUE);
		if (bResult)
		{
			pcBoolean = CCTBoolean::Construct(mcStack.Add(sizeof(CCTBoolean)));
			pcBoolean->Init(false);
			AddToken(pcBoolean);
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::SingleQuotedLiteral(BOOL bShort)
{
	BOOL			bResult;
	CCTChar*		pcChar;
	CCTInt*			pcInt;
	CCTShort*		pcShort;
	CExternalString	cText;

	mcParser.PushPosition();
	bResult = mcParser.GetStringSingleQuoted(&cText, !bShort);
	if (bResult)
	{
		if ((cText.miLen >= 5) || (bShort && (cText.miLen >= 3)))
		{
			Error("Too many character in character literal");
			mcParser.PopPosition();
			return FALSE;
		}
		
		if (cText.miLen == 0)
		{
			Error("Empty character literal");
			mcParser.PopPosition();
			return FALSE;
		}

		if (cText.miLen == 1)
		{
			if (!bShort)
			{
				pcChar = CCTChar::Construct(mcStack.Add(sizeof(CCTChar)));
				pcChar->Init(cText.msz[0]);
				AddToken(pcChar);
				mcParser.PassPosition();
				return TRUE;
			}
			else
			{
				pcShort = CCTShort::Construct(mcStack.Add(sizeof(CCTShort)));
				pcShort->Init(cText.msz[0]);
				AddToken(pcShort);
				mcParser.PassPosition();
				return TRUE;
			}
		}

		if (cText.miLen == 2)
		{
			if (!bShort)
			{
				pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
				pcInt->Init(cText.msz[1] + (cText.msz[0]<<8));
				AddToken(pcInt);
				mcParser.PassPosition();
				return TRUE;
			}
			else
			{
				pcShort = CCTShort::Construct(mcStack.Add(sizeof(CCTShort)));
				pcShort->Init(cText.msz[1] + (cText.msz[0]<<8));
				AddToken(pcShort);
				mcParser.PassPosition();
				return TRUE;
			}
		}

		if (cText.miLen == 3)
		{
			pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
			pcInt->Init(cText.msz[2] + (cText.msz[1]<<8) + (cText.msz[0]<<16));
			AddToken(pcInt);
			mcParser.PassPosition();
			return TRUE;
		}

		if (cText.miLen == 4)
		{
			pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
			pcInt->Init(cText.msz[3] + (cText.msz[2]<<8) + (cText.msz[1]<<16) + (cText.msz[0]<<24));
			AddToken(pcInt);
			mcParser.PassPosition();
			return TRUE;
		}

		gcUserError.Set("Should never fall through to this statement.");
		mcParser.PopPosition();
		return FALSE;
	}
	else
	{
		mcParser.PopPosition();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::StringLiteral(void)
{
	BOOL				bResult;
	CCTString*			pcString;
	CExternalString		cString;

	bResult = mcParser.GetStringDoubleQuoted(&cString);
	if (bResult)
	{
		pcString = CCTString::Construct(mcStack.Add(sizeof(CCTString)));
		pcString->Init(cString.msz, cString.EndInclusive());
		AddToken(pcString);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::WideStringLiteral(void)
{
	BOOL				bResult;
	CCTWideString*		pcString;
	CExternalString		cString;

	bResult = mcParser.GetStringDoubleQuoted(&cString);
	if (bResult)
	{
		pcString = CCTWideString::Construct(mcStack.Add(sizeof(CCTWideString)));
		pcString->Init(cString.msz, cString.EndInclusive());
		AddToken(pcString);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::DecimalInteger(void)
{
	BOOL					bResult;
	unsigned long long int  ulli;

	mcParser.PushPosition();

	bResult  = mcParser.GetInteger(&ulli, TRUE, FALSE);
	if (bResult)
	{
		IntegerType(ulli);

		mcParser.PassPosition();
		return TRUE;
	}
	mcParser.PopPosition();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::IntegerLiteral(void)
{
	unsigned long long int	ulliNumber;
	int						iNumDigits;
	BOOL					bReturn;

	bReturn = DecimalInteger();
	if (bReturn)
	{
		return TRUE;
	}
	else
	{
		bReturn = mcParser.GetHexadecimal(&ulliNumber, &iNumDigits);
		if (bReturn)
		{
			IntegerType(ulliNumber);
			return TRUE;
		}
		else
		{
			bReturn = mcParser.GetOctal(&ulliNumber, &iNumDigits);
			if (bReturn)
			{
				IntegerType(ulliNumber);
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTokeniser::ShortLiteral(void)
{
	BOOL			bResult;

	mcParser.PushPosition();

	bResult = mcParser.GetExactIdentifier("L", TRUE, TRUE);
	if (bResult)
	{
		bResult = SingleQuotedLiteral(TRUE);
		if (bResult)
		{
			mcParser.PassPosition();
			return TRUE;
		}
		bResult = WideStringLiteral();
		if (bResult)
		{
			mcParser.PassPosition();
			return TRUE;
		}
		mcParser.PopPosition();
		return FALSE;
	}
	else
	{
		mcParser.PopPosition();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::IntegerType(unsigned long long int ulliValue)
{
	BOOL			bResult;
	BOOL			bUnsigned;
	BOOL			bLong;
	BOOL			bLongLong;
	CCTInt*			pcInt;
	CCTLongLong*	pcLongLong;

	/*
	U
	L
	UL
	LL
	ULL
	*/

	bUnsigned = FALSE;
	bLongLong = FALSE;
	bLong = FALSE;

	bResult = mcParser.GetExactIdentifier("ULL", FALSE, FALSE);
	if (bResult)
	{
		bLongLong = TRUE;
		bUnsigned = TRUE;
	}

	bResult = mcParser.GetExactIdentifier("UL", FALSE, FALSE);
	if (bResult)
	{
		bLong = TRUE;
		bUnsigned = TRUE;
	}


	bResult = mcParser.GetExactIdentifier("LL", FALSE, FALSE);
	if (bResult)
	{
		bLongLong = TRUE;
	}


	bResult = mcParser.GetExactIdentifier("L", FALSE, FALSE);
	if (bResult)
	{
		bLong = TRUE;
	}

	bResult = mcParser.GetExactIdentifier("U", FALSE, FALSE);
	if (bResult)
	{
		bUnsigned = TRUE;
	}


	if (bLongLong)
	{
		pcLongLong = CCTLongLong::Construct(mcStack.Add(sizeof(CCTLongLong)));
		pcLongLong->Init(ulliValue);
		pcLongLong->mbSigned = !bUnsigned;
		AddToken(pcLongLong);
	}
	else
	{
		pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
		pcInt->Init((unsigned int)ulliValue);
		pcInt->mbSigned = !bUnsigned;
		AddToken(pcInt);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::DoubleType(long double ldValue)
{
	BOOL			bResult;
	CCTFloat*		pcFloat;
	CCTDouble*		pcDouble;
	CCTLongDouble*	pcLongDouble;

	bResult = mcParser.GetExactIdentifier("L", FALSE, FALSE);
	if (bResult)
	{
		pcLongDouble = CCTLongDouble::Construct(mcStack.Add(sizeof(CCTLongDouble)));
		pcLongDouble->Init(ldValue);
		AddToken(pcLongDouble);
	}
	else
	{
		bResult = mcParser.GetExactIdentifier("F", FALSE, FALSE);
		if (bResult)
		{
			pcFloat = CCTFloat::Construct(mcStack.Add(sizeof(CCTFloat)));
			pcFloat->Init((float)ldValue);
			AddToken(pcFloat);
		}
		else
		{
			pcDouble = CCTDouble::Construct(mcStack.Add(sizeof(CCTDouble)));
			pcDouble->Init((double)ldValue);
			AddToken(pcDouble);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::AddToken(CCppToken* pcToken)
{
	CCppToken**	ppcToken;

	ppcToken = mcTokens.Add();
	*ppcToken = pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTokeniser::Error(char* szError)
{
	CChars::Dump(szError);
	CChars::Dump("\n");
}
