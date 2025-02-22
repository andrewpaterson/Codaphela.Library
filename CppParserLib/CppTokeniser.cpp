/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
void CCPPTokeniser::Init(void)
{
	mcStack.Init(16 KB);

	mcIdentifiers.Init();
	mcTokens.Init();

	mcReservedWords.Init();
	mcReservedWords.Add("(");
	mcReservedWords.Add(")");
	mcReservedWords.Add("{");
	mcReservedWords.Add("}");
	mcReservedWords.Add("[");
	mcReservedWords.Add("]");
	mcReservedWords.Add("<<=");
	mcReservedWords.Add(">>=");
	mcReservedWords.Add("++");
	mcReservedWords.Add("--");
	mcReservedWords.Add("==");
	mcReservedWords.Add("!=");
	mcReservedWords.Add(">=");
	mcReservedWords.Add("<=");
	mcReservedWords.Add("||");
	mcReservedWords.Add("&&");
	mcReservedWords.Add("+=");
	mcReservedWords.Add("-=");
	mcReservedWords.Add("*=");
	mcReservedWords.Add("/=");
	mcReservedWords.Add("%=");
	mcReservedWords.Add("&=");
	mcReservedWords.Add("|=");
	mcReservedWords.Add("^=");
	mcReservedWords.Add("<<");
	mcReservedWords.Add(">>");
	mcReservedWords.Add("+");
	mcReservedWords.Add("-");
	mcReservedWords.Add("*");
	mcReservedWords.Add("/");
	mcReservedWords.Add("%");
	mcReservedWords.Add("=");
	mcReservedWords.Add("!");
	mcReservedWords.Add("&");
	mcReservedWords.Add("|");
	mcReservedWords.Add("^");
	mcReservedWords.Add(">");
	mcReservedWords.Add("<");
	mcReservedWords.Add("~");
	mcReservedWords.Add("::*");
	mcReservedWords.Add("::");
	mcReservedWords.Add("?");
	mcReservedWords.Add(":");
	mcReservedWords.Add(";");
	mcReservedWords.Add("->");
	mcReservedWords.Add(".");
	mcReservedWords.Add("->*");
	mcReservedWords.Add(".*");
	mcReservedWords.Add("...");
	mcReservedWords.Add(",");
	mcReservedWords.Add("constauto");
	mcReservedWords.Add("double");
	mcReservedWords.Add("float");
	mcReservedWords.Add("int");
	mcReservedWords.Add("short");
	mcReservedWords.Add("struct");
	mcReservedWords.Add("unsigned");
	mcReservedWords.Add("break");
	mcReservedWords.Add("continue");
	mcReservedWords.Add("else");
	mcReservedWords.Add("for");
	mcReservedWords.Add("long");
	mcReservedWords.Add("signed");
	mcReservedWords.Add("switch");
	mcReservedWords.Add("void");
	mcReservedWords.Add("case");
	mcReservedWords.Add("default");
	mcReservedWords.Add("enum");
	mcReservedWords.Add("goto");
	mcReservedWords.Add("register");
	mcReservedWords.Add("sizeof");
	mcReservedWords.Add("typedef");
	mcReservedWords.Add("volatile");
	mcReservedWords.Add("char");
	mcReservedWords.Add("do");
	mcReservedWords.Add("extern");
	mcReservedWords.Add("if");
	mcReservedWords.Add("return");
	mcReservedWords.Add("static");
	mcReservedWords.Add("union");
	mcReservedWords.Add("while");
	mcReservedWords.Add("asm");
	mcReservedWords.Add("dynamic_cast");
	mcReservedWords.Add("namespace");
	mcReservedWords.Add("reinterpret_cast");
	mcReservedWords.Add("try");
	mcReservedWords.Add("bool");
	mcReservedWords.Add("explicit");
	mcReservedWords.Add("new");
	mcReservedWords.Add("static_cast");
	mcReservedWords.Add("typeid");
	mcReservedWords.Add("catch");
	mcReservedWords.Add("false");
	mcReservedWords.Add("operator");
	mcReservedWords.Add("template");
	mcReservedWords.Add("typename");
	mcReservedWords.Add("class");
	mcReservedWords.Add("friend");
	mcReservedWords.Add("private");
	mcReservedWords.Add("this");
	mcReservedWords.Add("using");
	mcReservedWords.Add("const_cast");
	mcReservedWords.Add("inline");
	mcReservedWords.Add("public");
	mcReservedWords.Add("throw");
	mcReservedWords.Add("virtual");
	mcReservedWords.Add("delete");
	mcReservedWords.Add("mutable");
	mcReservedWords.Add("protected");
	mcReservedWords.Add("true");
	mcReservedWords.Add("wchar_t");
	mcReservedWords.Add("and");
	mcReservedWords.Add("bitand");
	mcReservedWords.Add("compl");
	mcReservedWords.Add("not_eq");
	mcReservedWords.Add("or_eq");
	mcReservedWords.Add("xor_eq");
	mcReservedWords.Add("and_eq");
	mcReservedWords.Add("bitor");
	mcReservedWords.Add("not");
	mcReservedWords.Add("or");
	mcReservedWords.Add("xor");

	if (mcReservedWords.NumElements() != NUM_CPP_TOKENS)
	{
		gcUserError.Set("Reserved words is out of synch!");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::Kill(void)
{
	size			i;
	CCPPToken*		pcToken;

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
bool CCPPTokeniser::Line(void)
{
	bool		bResult;
	size		iTokenNum;

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
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Tokenise(CPPTokenList* pcSource)
{
	int				i;
	bool			bResult;
	CPPToken*		pcToken;
	CPPLine*		pcLine;
	CCTNewLine*		pcNewLine;
	int				iNumTokens;

	mcTokens.Kill();
	mcTokens.Init();

	bResult = true;
	pcLine = NULL;
	mcParser.Init();
	iNumTokens = pcSource->NumTokens();
	for (i = 0; i < iNumTokens; i++)
	{
		pcToken = pcSource->Get(i);
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
		return true;
	}
	else
	{
		PrintError(pcLine);
		mcParser.Kill();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::PrintError(CPPLine* pcLine)
{
	CChars		sz;

	sz.Init("Tokenising Broke (");
	sz.Append(mcParser.Line() + 1);
	sz.Append(", ");
	sz.Append(mcParser.Column()+1);
	sz.Append("): ");

	sz.Append("\n");
	sz.Append("Line[");
	pcLine->Print(&sz);
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
void CCPPTokeniser::PrintTokens(void)
{
	size		j;
	CCPPToken*	pcToken;
	CChars		sz;
	bool		bPrevNewLine;

	bPrevNewLine = false;
	sz.Init();
	for (j = 0; j < mcTokens.NumElements(); j++)
	{
		pcToken = *mcTokens.Get(j);
		if (!pcToken->IsHelper())
		{
			pcToken->Append(&sz, this);
			sz.Append(" ");
			bPrevNewLine = false;
		}
		else
		{
			if (!bPrevNewLine)
			{
				pcToken->Append(&sz, this);
			}
			bPrevNewLine = true;
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
bool CCPPTokeniser::Literal(void)
{
	bool		bReturn;

	bReturn = BooleanLiteral();
	if (bReturn)
	{
		return bReturn;
	}

	bReturn = SingleQuotedLiteral(false);
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
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::ReservedWord(void)
{
	int64				lliID;
	CCTReservedWord*	pcReservedWord;
	char*				szWord;
	CPPText*			pcText;
	char				szTemp[1024];
	int					iLen;
	CPPToken*			pcToken;

	mcParser.SkipWhiteSpace();

	if (!mcParser.HasToken())
	{
		return false;
	}

	pcToken = mcParser.GetToken();
	if (pcToken->IsText())
	{
		pcText = (CPPText*)pcToken;
		if (pcText->meType != PPT_Decorator)
		{
			lliID = mcReservedWords.Get(pcText->mcText.msz, pcText->mcText.EndInclusive(), false);
		}
		else
		{
			mcParser.ToDecortatorString(szTemp, 1024, &iLen);
			lliID = mcReservedWords.Get(szTemp, szTemp + (iLen - 1), false);
		}

		if (lliID != -1)
		{
			szWord = mcReservedWords.Get(lliID, &iLen);

			//Move the parser on.
			if (pcText->meType != PPT_Decorator)
			{
				//Just skip one token.
				mcParser.NextToken();
			}
			else
			{
				//Each letter was a single decorator.  Skip them all.
				mcParser.NextToken(iLen);
			}

			pcReservedWord = CCTReservedWord::Construct(mcStack.Add(sizeof(CCTReservedWord)));
			pcReservedWord->Init(lliID);
			AddToken(pcReservedWord);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Identifier(void)
{
	int64				lliID;
	CCTIdentifier*		pcIdentifier;
	CExternalString		cIdentifier;
	bool				bResult;

	bResult = mcParser.GetIdentifier(&cIdentifier);
	if (bResult)
	{
		lliID = mcIdentifiers.Get(cIdentifier.msz, cIdentifier.EndInclusive(), true);
		if (lliID != -1)
		{
			lliID = mcIdentifiers.Add(cIdentifier.msz, cIdentifier.EndInclusive());
			pcIdentifier = CCTIdentifier::Construct(mcStack.Add(sizeof(CCTIdentifier)));
			pcIdentifier->Init(lliID);
			AddToken(pcIdentifier);
			return true;
		}
		return false;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::DoubleLiteral(void)
{
	long double		f;
	uint64			ulliExponent;
	long double		fTen;
	bool			bResult;

	mcParser.PushPosition();

	bResult = mcParser.GetFloat(&f);
	if (bResult)
	{
		bResult = mcParser.GetExactIdentifier("e", false, false);
		if (bResult)
		{
			bResult = mcParser.GetInteger(&ulliExponent, false, true);
			if (bResult)
			{
				fTen = 10.0;
				f = pow(fTen, (int)ulliExponent) * f;
				DoubleType(f);
				mcParser.PassPosition();
				return true;
			}
			else
			{
				mcParser.PopPosition();
				return false;
			}
		}
		else
		{
			DoubleType(f);
			mcParser.PassPosition();
			return true;
		}
	}
	else
	{
		mcParser.PopPosition();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::BooleanLiteral(void)
{
	bool			bResult;
	CCTBoolean* 	pcBoolean;

	bResult = mcParser.GetExactIdentifier("true", true, true);
	if (bResult)
	{
		pcBoolean = CCTBoolean::Construct(mcStack.Add(sizeof(CCTBoolean)));
		pcBoolean->Init(true);
		AddToken(pcBoolean);
		return true;
	}
	else
	{
		bResult = mcParser.GetExactIdentifier("false", true, true);
		if (bResult)
		{
			pcBoolean = CCTBoolean::Construct(mcStack.Add(sizeof(CCTBoolean)));
			pcBoolean->Init(false);
			AddToken(pcBoolean);
			return true;
		}
		else
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::SingleQuotedLiteral(bool bShort)
{
	bool			bResult;
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
			return false;
		}
		
		if (cText.miLen == 0)
		{
			Error("Empty character literal");
			mcParser.PopPosition();
			return false;
		}

		if (cText.miLen == 1)
		{
			if (!bShort)
			{
				pcChar = CCTChar::Construct(mcStack.Add(sizeof(CCTChar)));
				pcChar->Init(cText.msz[0]);
				AddToken(pcChar);
				mcParser.PassPosition();
				return true;
			}
			else
			{
				pcShort = CCTShort::Construct(mcStack.Add(sizeof(CCTShort)));
				pcShort->Init(cText.msz[0]);
				AddToken(pcShort);
				mcParser.PassPosition();
				return true;
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
				return true;
			}
			else
			{
				pcShort = CCTShort::Construct(mcStack.Add(sizeof(CCTShort)));
				pcShort->Init(cText.msz[1] + (cText.msz[0]<<8));
				AddToken(pcShort);
				mcParser.PassPosition();
				return true;
			}
		}

		if (cText.miLen == 3)
		{
			pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
			pcInt->Init(cText.msz[2] + (cText.msz[1]<<8) + (cText.msz[0]<<16));
			AddToken(pcInt);
			mcParser.PassPosition();
			return true;
		}

		if (cText.miLen == 4)
		{
			pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
			pcInt->Init(cText.msz[3] + (cText.msz[2]<<8) + (cText.msz[1]<<16) + (cText.msz[0]<<24));
			AddToken(pcInt);
			mcParser.PassPosition();
			return true;
		}

		gcUserError.Set("Should never fall through to this statement.");
		mcParser.PopPosition();
		return false;
	}
	else
	{
		mcParser.PopPosition();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::StringLiteral(void)
{
	bool				bResult;
	CCTString*			pcString;
	CExternalString		cString;

	bResult = mcParser.GetStringDoubleQuoted(&cString);
	if (bResult)
	{
		pcString = CCTString::Construct(mcStack.Add(sizeof(CCTString)));
		pcString->Init(cString.msz, cString.EndInclusive());
		AddToken(pcString);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::WideStringLiteral(void)
{
	bool				bResult;
	CCTWideString*		pcString;
	CExternalString		cString;

	bResult = mcParser.GetStringDoubleQuoted(&cString);
	if (bResult)
	{
		pcString = CCTWideString::Construct(mcStack.Add(sizeof(CCTWideString)));
		pcString->Init(cString.msz, cString.EndInclusive());
		AddToken(pcString);
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::DecimalInteger(void)
{
	bool	bResult;
	uint64  ulli;

	mcParser.PushPosition();

	bResult  = mcParser.GetInteger(&ulli, true, false);
	if (bResult)
	{
		IntegerType(ulli);

		mcParser.PassPosition();
		return true;
	}
	mcParser.PopPosition();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::IntegerLiteral(void)
{
	uint64	ulliNumber;
	int		iNumDigits;
	bool	bReturn;

	bReturn = DecimalInteger();
	if (bReturn)
	{
		return true;
	}
	else
	{
		bReturn = mcParser.GetHexadecimal(&ulliNumber, &iNumDigits);
		if (bReturn)
		{
			IntegerType(ulliNumber);
			return true;
		}
		else
		{
			bReturn = mcParser.GetOctal(&ulliNumber, &iNumDigits);
			if (bReturn)
			{
				IntegerType(ulliNumber);
				return true;
			}
			else
			{
				return false;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::ShortLiteral(void)
{
	bool			bResult;

	mcParser.PushPosition();

	bResult = mcParser.GetExactIdentifier("L", true, true);
	if (bResult)
	{
		bResult = SingleQuotedLiteral(true);
		if (bResult)
		{
			mcParser.PassPosition();
			return true;
		}
		bResult = WideStringLiteral();
		if (bResult)
		{
			mcParser.PassPosition();
			return true;
		}
		mcParser.PopPosition();
		return false;
	}
	else
	{
		mcParser.PopPosition();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::IntegerType(uint64 ulliValue)
{
	bool			bResult;
	bool			bUnsigned;
	bool			bLong;
	bool			bLongLong;
	CCTInt*			pcInt;
	CCTLongLong*	pcLongLong;

	/*
	U
	L
	UL
	LL
	ULL
	*/

	bUnsigned = false;
	bLongLong = false;
	bLong = false;

	bResult = mcParser.GetExactIdentifier("ULL", false, false);
	if (bResult)
	{
		bLongLong = true;
		bUnsigned = true;
	}

	bResult = mcParser.GetExactIdentifier("UL", false, false);
	if (bResult)
	{
		bLong = true;
		bUnsigned = true;
	}


	bResult = mcParser.GetExactIdentifier("LL", false, false);
	if (bResult)
	{
		bLongLong = true;
	}


	bResult = mcParser.GetExactIdentifier("L", false, false);
	if (bResult)
	{
		bLong = true;
	}

	bResult = mcParser.GetExactIdentifier("U", false, false);
	if (bResult)
	{
		bUnsigned = true;
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
void CCPPTokeniser::DoubleType(long double ldValue)
{
	bool			bResult;
	CCTFloat*		pcFloat;
	CCTDouble*		pcDouble;
	CCTLongDouble*	pcLongDouble;

	bResult = mcParser.GetExactIdentifier("L", false, false);
	if (bResult)
	{
		pcLongDouble = CCTLongDouble::Construct(mcStack.Add(sizeof(CCTLongDouble)));
		pcLongDouble->Init(ldValue);
		AddToken(pcLongDouble);
	}
	else
	{
		bResult = mcParser.GetExactIdentifier("F", false, false);
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
void CCPPTokeniser::AddToken(CCPPToken* pcToken)
{
	CCPPToken**	ppcToken;

	ppcToken = mcTokens.Add();
	*ppcToken = pcToken;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::Error(char* szError)
{
	CChars::Dump(szError);
	CChars::Dump("\n");
}

