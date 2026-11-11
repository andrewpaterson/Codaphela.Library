/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "BaseLib/ErrorHandler.h"
#include "CppTokeniser.h"
#include "SourceFile.h"
#include "CTBoolean.h"
#include "CTChar.h"
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CTString.h"
#include "CTDouble.h"
#include "CTFloat.h"
#include "CTLongDouble.h"
#include "CTInt.h"
#include "CTLongLong.h"
#include "CTShort.h"
#include "CPPPunctuatorList.h"


struct SCPPReservedWord
{
	const char*	szSpelling;
	ECppTokens	eToken;
};


//The first spelling of each token is in ECppTokens order and is how the token prints.  The alternative spellings follow.
static SCPPReservedWord gasCPPReservedWords[] =
{
#define CPP_RESERVED_SPELLING(sz, eToken)	{ sz, eToken },
	CPP_PUNCTUATORS(CPP_RESERVED_SPELLING)


	{ "alignas", CT_alignas },
	{ "alignof", CT_alignof },
	{ "asm", CT_asm },
	{ "auto", CT_auto },
	{ "bool", CT_bool },
	{ "break", CT_break },
	{ "case", CT_case },
	{ "catch", CT_catch },
	{ "char", CT_char },
	{ "char16_t", CT_char16_t },
	{ "char32_t", CT_char32_t },
	{ "class", CT_class },
	{ "const", CT_const },
	{ "constexpr", CT_constexpr },
	{ "const_cast", CT_const_cast },
	{ "continue", CT_continue },
	{ "decltype", CT_decltype },
	{ "default", CT_default },
	{ "delete", CT_delete },
	{ "do", CT_do },
	{ "double", CT_double },
	{ "dynamic_cast", CT_dynamic_cast },
	{ "else", CT_else },
	{ "enum", CT_enum },
	{ "explicit", CT_explicit },
	{ "export", CT_export },
	{ "extern", CT_extern },
	{ "false", CT_false },
	{ "float", CT_float },
	{ "for", CT_for },
	{ "friend", CT_friend },
	{ "goto", CT_goto },
	{ "if", CT_if },
	{ "inline", CT_inline },
	{ "int", CT_int },
	{ "long", CT_long },
	{ "mutable", CT_mutable },
	{ "namespace", CT_namespace },
	{ "new", CT_new },
	{ "noexcept", CT_noexcept },
	{ "nullptr", CT_nullptr },
	{ "operator", CT_operator },
	{ "private", CT_private },
	{ "protected", CT_protected },
	{ "public", CT_public },
	{ "register", CT_register },
	{ "reinterpret_cast", CT_reinterpret_cast },
	{ "return", CT_return },
	{ "short", CT_short },
	{ "signed", CT_signed },
	{ "sizeof", CT_sizeof },
	{ "static", CT_static },
	{ "static_assert", CT_static_assert },
	{ "static_cast", CT_static_cast },
	{ "struct", CT_struct },
	{ "switch", CT_switch },
	{ "template", CT_template },
	{ "this", CT_this },
	{ "thread_local", CT_thread_local },
	{ "throw", CT_throw },
	{ "true", CT_true },
	{ "try", CT_try },
	{ "typedef", CT_typedef },
	{ "typeid", CT_typeid },
	{ "typename", CT_typename },
	{ "union", CT_union },
	{ "unsigned", CT_unsigned },
	{ "using", CT_using },
	{ "virtual", CT_virtual },
	{ "void", CT_void },
	{ "volatile", CT_volatile },
	{ "wchar_t", CT_wchar_t },
	{ "while", CT_while },

	{ "__declspec", CT___declspec },
	{ "__cdecl", CT___cdecl },
	{ "__stdcall", CT___stdcall },
	{ "__fastcall", CT___fastcall },
	{ "__thiscall", CT___thiscall },
	{ "__vectorcall", CT___vectorcall },
	{ "__int8", CT___int8 },
	{ "__int16", CT___int16 },
	{ "__int32", CT___int32 },
	{ "__int64", CT___int64 },
	{ "__w64", CT___w64 },
	{ "__ptr32", CT___ptr32 },
	{ "__ptr64", CT___ptr64 },
	{ "__unaligned", CT___unaligned },
	{ "__restrict", CT___restrict },
	{ "__forceinline", CT___forceinline },
	{ "__inline", CT___inline },
	{ "__pragma", CT___pragma },
	{ "__uuidof", CT___uuidof },

	{ "__attribute__", CT___attribute__ },
	{ "__extension__", CT___extension__ },
	{ "__restrict__", CT___restrict__ },
	{ "__inline__", CT___inline__ },
	{ "__asm__", CT___asm__ },
	{ "__volatile__", CT___volatile__ },
	{ "__const__", CT___const__ },
	{ "__signed__", CT___signed__ },
	{ "__typeof__", CT___typeof__ },
	{ "__alignof__", CT___alignof__ },
	{ "__thread", CT___thread },

	//Alternative tokens [lex.digraph].
	CPP_ALTERNATIVE_PUNCTUATORS(CPP_RESERVED_SPELLING)
	{ "and", CT_LogicalAnd },
	{ "and_eq", CT_AssignmentBitwiseAnd },
	{ "bitand", CT_BitwiseAnd },
	{ "bitor", CT_BitwiseOr },
	{ "compl", CT_BitwiseNot },
	{ "not", CT_LogicalNot },
	{ "not_eq", CT_NotEqualTo },
	{ "or", CT_LogicalOr },
	{ "or_eq", CT_AssignmentBitwiseOr },
	{ "xor", CT_BitwiseXor },
	{ "xor_eq", CT_AssignmentBitwiseXor },

	//Microsoft and GNU spellings of the same keywords.
	{ "_cdecl", CT___cdecl },
	{ "_stdcall", CT___stdcall },
	{ "_fastcall", CT___fastcall },
	{ "_inline", CT___inline },
	{ "_asm", CT_asm },
	{ "__asm", CT_asm },
	{ "__volatile", CT_volatile },
	{ "__const", CT_const },
	{ "__signed", CT_signed },
	{ "__typeof", CT___typeof__ },
	{ "__alignof", CT___alignof__ },
	{ "__wchar_t", CT_wchar_t },
	{ "__nullptr", CT_nullptr },

	{ NULL, NUM_CPP_TOKENS }
};
#undef CPP_RESERVED_SPELLING


#define NUM_CPP_RESERVED_SPELLINGS	(sizeof(gasCPPReservedWords) / sizeof(SCPPReservedWord) - 1)


//////////////////////////////////////////////////////////////////////////
//
// pcTokens allocates the array of tokens (NULL for the system allocator):
// one that keeps its memory for the next unit (CCPP65UnitWorkspace).
//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::Init(CMallocator* pcTokens)
{
	size	i;
	int64	lliID;

	mcStack.Init(16 KB);
	mcIdentifiers.Init();
	if (pcTokens)
	{
		mcTokens.Init(pcTokens);
	}
	else
	{
		mcTokens.Init();
	}
	mpcSource = NULL;
	mbError = false;
	mcErrors.Init();
	miSource = -1;

	//IDs are the index of the spelling in gasCPPReservedWords.
	mcReservedWords.Init();
	for (i = 0; i < NUM_CPP_RESERVED_SPELLINGS; i++)
	{
		lliID = mcReservedWords.Get(gasCPPReservedWords[i].szSpelling, NULL);
		if (lliID == -1)
		{
			lliID = mcReservedWords.Add(gasCPPReservedWords[i].szSpelling);
		}
		if ((lliID != (int64)i) || ((i < NUM_CPP_TOKENS) && (gasCPPReservedWords[i].eToken != (ECppTokens)i)))
		{
			gcUserError.Set("C++ reserved words are out of order.");
		}
	}

	//Identifiers with special meaning in some places.  [lex.name] 2
	mlliOverride = GetIdentifierID("override");
	mlliFinal = GetIdentifierID("final");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::Kill(void)
{
	Clear();
	mcTokens.Kill();
	mcStack.Kill();
	mcIdentifiers.Kill();
	mcReservedWords.Kill();
	mcErrors.Kill();
	mpcSource = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::Clear(void)
{
	size			i;
	CCPPToken*		pcToken;

	for (i = 0; i < mcTokens.NumElements(); i++)
	{
		pcToken = *mcTokens.Get(i);
		pcToken->Kill();
	}
	mcTokens.ReInit();
	mcStack.Kill();
	mcStack.Init(16 KB);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Tokenise(CPreprocessedTokens* pcSource)
{
	size					i;
	SPreprocessedToken*		psToken;

	Clear();
	mcErrors.Clear();
	mpcSource = pcSource;
	mbError = false;
	for (i = 0; i < pcSource->NumTokens(); i++)
	{
		psToken = pcSource->Get(i);
		miSource = (int)i;
		Token(psToken, (int)i);
	}
	miSource = -1;
	return !mbError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Token(SPreprocessedToken* psToken, int iSource)
{
	switch (psToken->uiKind)
	{
	case PTK_Identifier:
		return Identifier(psToken, iSource);
	case PTK_Punctuator:
		return Punctuator(psToken, iSource);
	case PTK_Number:
		return Number(psToken, iSource);
	case PTK_Character:
		return Character(psToken, iSource);
	case PTK_String:
		return String(psToken, iSource);
	}

	//A stray character.  System headers are only skimmed so it is dropped there.
	if (psToken->pcFile && psToken->pcFile->IsSystemFile())
	{
		return true;
	}
	return Error(psToken, "Unexpected character.");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Identifier(SPreprocessedToken* psToken, int iSource)
{
	int64				lliID;
	ECppTokens			eToken;
	CCTReservedWord*	pcReservedWord;
	CCTBoolean*			pcBoolean;
	CCTIdentifier*		pcIdentifier;

	lliID = mcReservedWords.Get(psToken->szText, psToken->szText + psToken->uiLength - 1);
	if (lliID != -1)
	{
		eToken = gasCPPReservedWords[lliID].eToken;
		if ((eToken == CT_true) || (eToken == CT_false))
		{
			pcBoolean = CCTBoolean::Construct(mcStack.Add(sizeof(CCTBoolean)));
			pcBoolean->Init(eToken == CT_true);
			AddToken(pcBoolean, iSource);
		}
		else
		{
			pcReservedWord = CCTReservedWord::Construct(mcStack.Add(sizeof(CCTReservedWord)));
			pcReservedWord->Init(eToken);
			AddToken(pcReservedWord, iSource);
		}
		return true;
	}

	lliID = mcIdentifiers.Get(psToken->szText, psToken->szText + psToken->uiLength - 1);
	if (lliID == -1)
	{
		lliID = mcIdentifiers.Add(psToken->szText, psToken->szText + psToken->uiLength - 1);
	}
	pcIdentifier = CCTIdentifier::Construct(mcStack.Add(sizeof(CCTIdentifier)));
	pcIdentifier->Init(lliID);
	AddToken(pcIdentifier, iSource);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Punctuator(SPreprocessedToken* psToken, int iSource)
{
	int64				lliID;
	CCTReservedWord*	pcReservedWord;

	lliID = mcReservedWords.Get(psToken->szText, psToken->szText + psToken->uiLength - 1);
	if (lliID == -1)
	{
		return Error(psToken, "Unknown punctuator.");
	}

	pcReservedWord = CCTReservedWord::Construct(mcStack.Add(sizeof(CCTReservedWord)));
	pcReservedWord->Init(gasCPPReservedWords[lliID].eToken);
	AddToken(pcReservedWord, iSource);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Number(SPreprocessedToken* psToken, int iSource)
{
	SIntegerLiteral		sLiteral;

	switch (ParseIntegerLiteral(psToken->szText, psToken->uiLength, &sLiteral))
	{
	case ILR_Integer:
		return Integer(iSource, &sLiteral);
	case ILR_Floating:
		return Floating(psToken, iSource, sLiteral.szNumber);
	case ILR_InvalidDigit:
		return Error(psToken, "Invalid digit in integer literal.");
	case ILR_TooLarge:
		return Error(psToken, "Integer literal is too large.");
	case ILR_NoDigits:
		return Error(psToken, "Integer literal has no digits.");
	default:
		return Error(psToken, "Invalid integer literal suffix.");
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Integer(int iSource, SIntegerLiteral* psLiteral)
{
	uint64			ulliValue;
	bool			bUnsigned;
	int				iLongs;
	bool			bDecimal;
	CCTInt*			pcInt;
	CCTLongLong*	pcLongLong;

	ulliValue = psLiteral->ulliValue;
	bUnsigned = psLiteral->bUnsigned;
	iLongs = psLiteral->iLongs;

	//[lex.icon] Table 6 with 32 bit int and long and 64 bit long long, as on Windows.  The first type the value fits.
	bDecimal = (psLiteral->iBase == 10);
	if ((iLongs < 2) && !psLiteral->bInt64)
	{
		if (!bUnsigned && (ulliValue <= 0x7FFFFFFFULL))
		{
			pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
			pcInt->Init((unsigned int)ulliValue);
			pcInt->mbSigned = true;
			pcInt->mbLong = (iLongs == 1);
			pcInt->mbDecimal = bDecimal;
			AddToken(pcInt, iSource);
			return true;
		}
		if ((bUnsigned || !bDecimal) && (ulliValue <= 0xFFFFFFFFULL))
		{
			pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
			pcInt->Init((unsigned int)ulliValue);
			pcInt->mbSigned = false;
			pcInt->mbLong = (iLongs == 1);
			pcInt->mbDecimal = bDecimal;
			AddToken(pcInt, iSource);
			return true;
		}
	}

	pcLongLong = CCTLongLong::Construct(mcStack.Add(sizeof(CCTLongLong)));
	pcLongLong->Init(ulliValue);
	pcLongLong->mbSigned = !bUnsigned && ((ulliValue <= 0x7FFFFFFFFFFFFFFFULL) || bDecimal);
	if (ulliValue > 0x7FFFFFFFFFFFFFFFULL)
	{
		//Too large for long long.  Microsoft makes it unsigned long long.
		pcLongLong->mbSigned = false;
	}
	AddToken(pcLongLong, iSource);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Floating(SPreprocessedToken* psToken, int iSource, char* szNumber)
{
	char*			szEnd;
	long double		ldValue;
	CCTFloat*		pcFloat;
	CCTDouble*		pcDouble;
	CCTLongDouble*	pcLongDouble;

	ldValue = strtold(szNumber, &szEnd);
	if ((*szEnd == 'f') || (*szEnd == 'F'))
	{
		if (szEnd[1] != '\0')
		{
			return Error(psToken, "Invalid floating literal suffix.");
		}
		//Read again at the type's precision: rounding the long double again could be a bit out.
		pcFloat = CCTFloat::Construct(mcStack.Add(sizeof(CCTFloat)));
		pcFloat->Init(strtof(szNumber, NULL));
		AddToken(pcFloat, iSource);
	}
	else if ((*szEnd == 'l') || (*szEnd == 'L'))
	{
		if (szEnd[1] != '\0')
		{
			return Error(psToken, "Invalid floating literal suffix.");
		}
		pcLongDouble = CCTLongDouble::Construct(mcStack.Add(sizeof(CCTLongDouble)));
		pcLongDouble->Init(ldValue);
		AddToken(pcLongDouble, iSource);
	}
	else if (*szEnd == '\0')
	{
		pcDouble = CCTDouble::Construct(mcStack.Add(sizeof(CCTDouble)));
		pcDouble->Init(strtod(szNumber, NULL));
		AddToken(pcDouble, iSource);
	}
	else
	{
		return Error(psToken, "Invalid floating literal.");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Character(SPreprocessedToken* psToken, int iSource)
{
	SCharacterLiteral	sLiteral;
	CCTChar*			pcChar;
	CCTShort*			pcShort;
	CCTInt*				pcInt;

	switch (ParseCharacterLiteral(psToken->szText, psToken->uiLength, &sLiteral))
	{
	case CLR_Character:
		break;
	case CLR_Empty:
		return Error(psToken, "Empty character literal.");
	case CLR_InvalidEscape:
		return Error(psToken, "Invalid escape sequence.");
	default:
		return Error(psToken, "Unterminated character literal.");
	}

	if (((sLiteral.eEncoding == CTE_Narrow) || (sLiteral.eEncoding == CTE_UTF8)) && (sLiteral.iCount == 1))
	{
		pcChar = CCTChar::Construct(mcStack.Add(sizeof(CCTChar)));
		pcChar->Init((char)sLiteral.uiValue);
		AddToken(pcChar, iSource);
	}
	else if (((sLiteral.eEncoding == CTE_Wide) || (sLiteral.eEncoding == CTE_UTF16)) && (sLiteral.iCount == 1))
	{
		//wchar_t is 16 bits on Windows.
		pcShort = CCTShort::Construct(mcStack.Add(sizeof(CCTShort)));
		pcShort->Init((unsigned short)sLiteral.uiValue);
		pcShort->mbSigned = false;
		AddToken(pcShort, iSource);
	}
	else
	{
		pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
		pcInt->Init(sLiteral.uiValue);
		pcInt->mbSigned = (sLiteral.eEncoding != CTE_UTF32);
		AddToken(pcInt, iSource);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::String(SPreprocessedToken* psToken, int iSource)
{
	const char*		sz;
	ECTEncoding		eEncoding;
	CCTString*		pcString;
	size			i;

	//The literal keeps its spelling.  Adjacent literals are joined by the parser.  [lex.string] 13
	sz = psToken->szText;
	eEncoding = CTE_Narrow;
	if ((sz[0] == 'u') && (sz[1] == '8'))
	{
		eEncoding = CTE_UTF8;
	}
	else if (sz[0] == 'L')
	{
		eEncoding = CTE_Wide;
	}
	else if (sz[0] == 'u')
	{
		eEncoding = CTE_UTF16;
	}
	else if (sz[0] == 'U')
	{
		eEncoding = CTE_UTF32;
	}

	//The prefix is everything before the opening quote; R in it makes a raw string.
	for (i = 0; sz[i] != '"'; i++)
	{
		if (sz[i] == 'R')
		{
			return Error(psToken, "Raw string literals are not supported.");
		}
	}

	pcString = CCTString::Construct(mcStack.Add(sizeof(CCTString)));
	pcString->Init(psToken->szText, psToken->szText + psToken->uiLength - 1, eEncoding);
	AddToken(pcString, iSource);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::AddToken(CCPPToken* pcToken, int iSource)
{
	pcToken->SetSource(iSource);
	mcTokens.Add(pcToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Error(SPreprocessedToken* psToken, char* szError)
{
	CChars	szDetail;

	//Errors go to GetErrors.  Their tokens are preprocessed token indices.
	szDetail.Init();
	psToken->Append(&szDetail);
	if (psToken->pcFile)
	{
		mpcSource->AddError(&mcErrors, psToken, miSource, szError, szDetail.Text());
	}
	else
	{
		mcErrors.Add(NULL, -1, -1, miSource, szError, szDetail.Text());
	}
	szDetail.Kill();

	mbError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPTokeniser::GetErrors(void)
{
	return &mcErrors;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CCPPTokeniser::NumTokens(void)
{
	return mcTokens.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCPPToken* CCPPTokeniser::Get(size iIndex)
{
	CCPPToken**	ppcToken;

	ppcToken = mcTokens.SafeGet(iIndex);
	if (ppcToken)
	{
		return *ppcToken;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPreprocessedToken* CCPPTokeniser::GetSource(CCPPToken* pcToken)
{
	if (!mpcSource)
	{
		return NULL;
	}
	return mpcSource->Get(pcToken->GetSource());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int64 CCPPTokeniser::GetIdentifierID(char* szName)
{
	int64	lliID;

	lliID = mcIdentifiers.Get(szName, NULL);
	if (lliID == -1)
	{
		lliID = mcIdentifiers.Add(szName);
	}
	return lliID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CCPPTokeniser::GetIdentifier(int64 lliID, int* piLength)
{
	return mcIdentifiers.Get(lliID, piLength);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CCPPTokeniser::GetReservedWord(ECppTokens eToken)
{
	if ((eToken < 0) || (eToken >= NUM_CPP_TOKENS))
	{
		return NULL;
	}
	return (char*)gasCPPReservedWords[eToken].szSpelling;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::Print(CChars* psz)
{
	size				i;
	CCPPToken*			pcToken;
	SPreprocessedToken*	psSource;

	for (i = 0; i < mcTokens.NumElements(); i++)
	{
		pcToken = *mcTokens.Get(i);
		psSource = GetSource(pcToken);
		if (i > 0)
		{
			if (psSource && psSource->IsStartOfLine())
			{
				psz->AppendNewLine();
			}
			else
			{
				psz->Append(' ');
			}
		}
		pcToken->Append(psz, this);
	}
	if (mcTokens.NumElements() > 0)
	{
		psz->AppendNewLine();
	}
}

