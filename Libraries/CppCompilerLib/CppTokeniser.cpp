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
#include "CTUserLiteral.h"
#include "CPPNameCharacters.h"
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
		Token(psToken);
	}
	miSource = -1;
	return !mbError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Token(SPreprocessedToken* psToken)
{
	switch (psToken->uiKind)
	{
	case PTK_Identifier:
		return Identifier(psToken);
	case PTK_Punctuator:
		return Punctuator(psToken);
	case PTK_Number:
	case PTK_Character:
	case PTK_String:
		return Literal(psToken);
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
bool CCPPTokeniser::Identifier(SPreprocessedToken* psToken)
{
	int64				lliID;
	ECppTokens			eToken;
	CCTReservedWord*	pcReservedWord;
	CCTBoolean*			pcBoolean;
	CCTIdentifier*		pcIdentifier;
	const char*			szError;

	lliID = mcReservedWords.Get(psToken->szText, psToken->szText + psToken->uiLength - 1);
	if (lliID != -1)
	{
		eToken = gasCPPReservedWords[lliID].eToken;
		if ((eToken == CT_true) || (eToken == CT_false))
		{
			pcBoolean = CCTBoolean::Construct(mcStack.Add(sizeof(CCTBoolean)));
			pcBoolean->Init(eToken == CT_true);
			AddToken(pcBoolean);
		}
		else
		{
			pcReservedWord = CCTReservedWord::Construct(mcStack.Add(sizeof(CCTReservedWord)));
			pcReservedWord->Init(eToken);
			AddToken(pcReservedWord);
		}
		return true;
	}

	szError = InvalidCPPNameCharacter(psToken->szText, psToken->uiLength);
	if (szError)
	{
		return SharedError(psToken, szError);
	}
	pcIdentifier = CCTIdentifier::Construct(mcStack.Add(sizeof(CCTIdentifier)));
	pcIdentifier->Init(GetIdentifierID(psToken->szText, psToken->uiLength));
	AddToken(pcIdentifier);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Punctuator(SPreprocessedToken* psToken)
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
	AddToken(pcReservedWord);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A number, character or string.  One with a user-defined suffix, as
// 10_km and "text"_s, is the literal without it in a CCTUserLiteral.
// [lex.literal], [lex.ext]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Literal(SPreprocessedToken* psToken)
{
	CCPPToken*			pcLiteral;
	CCTUserLiteral*		pcUserLiteral;
	size				uiLength;

	uiLength = psToken->uiLength;
	if (psToken->uiKind == PTK_Number)
	{
		pcLiteral = Number(psToken, &uiLength);
	}
	else
	{
		uiLength = QuotedLiteralLength(psToken->szText, psToken->uiLength);
		if (uiLength == 0)
		{
			return Error(psToken, (psToken->uiKind == PTK_Character) ? "Unterminated character literal." : IsRawString(psToken->szText) ? "Raw string literal has no end, or its delimiter is not one." : "Unterminated string literal.");
		}
		pcLiteral = (psToken->uiKind == PTK_String) ? String(psToken, uiLength) : Character(psToken, uiLength);
	}
	if (!pcLiteral)
	{
		return false;
	}

	if (uiLength < psToken->uiLength)
	{
		pcUserLiteral = CCTUserLiteral::Construct(mcStack.Add(sizeof(CCTUserLiteral)));
		pcUserLiteral->Init(pcLiteral, GetIdentifierID(&psToken->szText[uiLength], psToken->uiLength - uiLength));
		pcLiteral = pcUserLiteral;
	}
	AddToken(pcLiteral);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A number's token, or NULL with an error.  *puiLength is the length of
// the literal without a user-defined suffix.
//
//////////////////////////////////////////////////////////////////////////
CCPPToken* CCPPTokeniser::Number(SPreprocessedToken* psToken, size* puiLength)
{
	SNumberLiteral			sLiteral;
	ENumberLiteralResult	eResult;

	eResult = ParseNumberLiteral(psToken->szText, psToken->uiLength, &sLiteral);
	if (NumberLiteralError(eResult))
	{
		SharedError(psToken, NumberLiteralError(eResult));
		return NULL;
	}
	*puiLength = psToken->uiLength - sLiteral.uiUserSuffix;
	return (eResult == NLR_Integer) ? Integer(&sLiteral) : Floating(psToken, &sLiteral);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCPPToken* CCPPTokeniser::Integer(SNumberLiteral* psLiteral)
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
		if ((!bUnsigned && (ulliValue <= 0x7FFFFFFFULL)) || ((bUnsigned || !bDecimal) && (ulliValue <= 0xFFFFFFFFULL)))
		{
			pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
			pcInt->Init((unsigned int)ulliValue);
			pcInt->mbSigned = !bUnsigned && (ulliValue <= 0x7FFFFFFFULL);
			pcInt->mbLong = (iLongs == 1);
			pcInt->mbDecimal = bDecimal;
			return pcInt;
		}
	}

	//Too large for long long is unsigned long long, as Microsoft has it.
	pcLongLong = CCTLongLong::Construct(mcStack.Add(sizeof(CCTLongLong)));
	pcLongLong->Init(ulliValue);
	pcLongLong->mbSigned = !bUnsigned && (ulliValue <= 0x7FFFFFFFFFFFFFFFULL);
	return pcLongLong;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCPPToken* CCPPTokeniser::Floating(SPreprocessedToken* psToken, SNumberLiteral* psLiteral)
{
	char*			szEnd;
	long double		ldValue;
	CCTFloat*		pcFloat;
	CCTDouble*		pcDouble;
	CCTLongDouble*	pcLongDouble;

	ldValue = strtold(psLiteral->szNumber, &szEnd);
	if (*szEnd != '\0')
	{
		Error(psToken, "Invalid floating literal.");
		return NULL;
	}

	//float and double are read again at their own precision: rounding the long double again could be a bit out.
	switch (psLiteral->eFloating)
	{
	case FL_Float:
		pcFloat = CCTFloat::Construct(mcStack.Add(sizeof(CCTFloat)));
		pcFloat->Init(strtof(psLiteral->szNumber, NULL));
		return pcFloat;
	case FL_LongDouble:
		pcLongDouble = CCTLongDouble::Construct(mcStack.Add(sizeof(CCTLongDouble)));
		pcLongDouble->Init(ldValue);
		return pcLongDouble;
	default:
		pcDouble = CCTDouble::Construct(mcStack.Add(sizeof(CCTDouble)));
		pcDouble->Init(strtod(psLiteral->szNumber, NULL));
		return pcDouble;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The token of the character literal that is the first uiLength
// characters of psToken, or NULL with an error.
//
//////////////////////////////////////////////////////////////////////////
CCPPToken* CCPPTokeniser::Character(SPreprocessedToken* psToken, size uiLength)
{
	SCharacterLiteral	sLiteral;
	const char*			szError;
	CCTChar*			pcChar;
	CCTShort*			pcShort;
	CCTInt*				pcInt;

	szError = CharacterLiteralError(ParseCharacterLiteral(psToken->szText, uiLength, &sLiteral));
	if (szError)
	{
		SharedError(psToken, szError);
		return NULL;
	}

	if (((sLiteral.eEncoding == CTE_Narrow) || (sLiteral.eEncoding == CTE_UTF8)) && (sLiteral.iCount == 1))
	{
		pcChar = CCTChar::Construct(mcStack.Add(sizeof(CCTChar)));
		pcChar->Init((char)sLiteral.uiValue);
		return pcChar;
	}
	if (((sLiteral.eEncoding == CTE_Wide) || (sLiteral.eEncoding == CTE_UTF16)) && (sLiteral.iCount == 1))
	{
		//wchar_t is 16 bits on Windows.
		pcShort = CCTShort::Construct(mcStack.Add(sizeof(CCTShort)));
		pcShort->Init((unsigned short)sLiteral.uiValue);
		pcShort->mbSigned = false;
		pcShort->mbChar16 = (sLiteral.eEncoding == CTE_UTF16);
		return pcShort;
	}
	pcInt = CCTInt::Construct(mcStack.Add(sizeof(CCTInt)));
	pcInt->Init(sLiteral.uiValue);
	pcInt->mbSigned = (sLiteral.eEncoding != CTE_UTF32);
	pcInt->mbChar32 = (sLiteral.eEncoding == CTE_UTF32);
	return pcInt;
}


//////////////////////////////////////////////////////////////////////////
//
// The token of the string literal that is the first uiLength characters
// of psToken.  The literal keeps its spelling: its characters are the
// generator's business (CCPPUnit::DecodeString).  Adjacent literals are
// joined by the parser.  [lex.string] 13
//
//////////////////////////////////////////////////////////////////////////
CCPPToken* CCPPTokeniser::String(SPreprocessedToken* psToken, size uiLength)
{
	const char*		sz;
	CCTString*		pcString;

	sz = psToken->szText;
	pcString = CCTString::Construct(mcStack.Add(sizeof(CCTString)));
	pcString->Init(sz, sz + uiLength - 1, LiteralEncoding(sz));
	return pcString;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPTokeniser::AddToken(CCPPToken* pcToken)
{
	pcToken->SetSource(miSource);
	mcTokens.Add(pcToken);
}


//////////////////////////////////////////////////////////////////////////
//
// An error in the words of LiteralHelper and CPPNameCharacters, which the
// preprocessor's #if uses too, as a sentence.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::SharedError(SPreprocessedToken* psToken, const char* szError)
{
	CChars	sz;

	sz.Init(szError);
	sz.Append('.');
	Error(psToken, sz.Text());
	sz.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPTokeniser::Error(SPreprocessedToken* psToken, const char* szError)
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
size CCPPTokeniser::NumTokens(void) const
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
	return GetIdentifierID(szName, strlen(szName));
}


//////////////////////////////////////////////////////////////////////////
//
// The ID of the name that is uiLength characters at szName, which it is
// given if it is new.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPTokeniser::GetIdentifierID(const char* szName, size uiLength)
{
	int64	lliID;

	lliID = mcIdentifiers.Get((char*)szName, (char*)szName + uiLength - 1);
	if (lliID == -1)
	{
		lliID = mcIdentifiers.Add((char*)szName, (char*)szName + uiLength - 1);
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

