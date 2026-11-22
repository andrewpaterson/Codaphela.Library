#include <stdio.h>
#include <string.h>
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Logger.h"
#include "CppCompilerLib/CPPPreprocessor.h"
#include "CppCompilerLib/CppTokeniser.h"
#include "CppCompilerLib/ASCIIIndex.h"
#include "CppCompilerLib/CTReservedWord.h"
#include "CppCompilerLib/CTIdentifier.h"
#include "CppCompilerLib/CTInt.h"
#include "CppCompilerLib/CTLongLong.h"
#include "CppCompilerLib/CTShort.h"
#include "CppCompilerLib/CTChar.h"
#include "CppCompilerLib/CTFloat.h"
#include "CppCompilerLib/CTDouble.h"
#include "CppCompilerLib/CTLongDouble.h"
#include "CppCompilerLib/CTString.h"
#include "CppCompilerLib/CTUserLiteral.h"
#include "CppCompilerLib/CTBoolean.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TokeniseText(CTranslationUnit* pcFile, CCPPTokeniser* pcTokeniser, char* szSource)
{
	CCPPPreprocessor	cPreprocessor;
	bool			bResult;

	pcFile->Init("Tokens.cpp", NULL, false);
	pcFile->SetContents(szSource);

	cPreprocessor.Init(NULL);
	bResult = cPreprocessor.PreprocessTranslationUnit(pcFile);
	cPreprocessor.Kill();
	if (!bResult)
	{
		return false;
	}

	pcTokeniser->Init();
	return pcTokeniser->Tokenise(pcFile->GetTokens());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertInteger(CCPPToken* pcToken, unsigned int uiValue, bool bSigned, bool bLong)
{
	CCTInt*	pcInt;

	AssertTrue(pcToken->IsNumber());
	pcInt = (CCTInt*)pcToken;
	AssertInt(CTNT_Int, pcInt->meType);
	AssertInt(uiValue, pcInt->mcValue);
	AssertBool(bSigned, pcInt->mbSigned);
	AssertBool(bLong, pcInt->mbLong);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertLongLong(CCPPToken* pcToken, uint64 ulliValue, bool bSigned)
{
	CCTLongLong*	pcLongLong;

	AssertTrue(pcToken->IsNumber());
	pcLongLong = (CCTLongLong*)pcToken;
	AssertInt(CTNT_LongLong, pcLongLong->meType);
	AssertLongLongIntHex(ulliValue, pcLongLong->mcValue);
	AssertBool(bSigned, pcLongLong->mbSigned);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserKeywordsAndPunctuators(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CChars				sz;

	AssertTrue(TokeniseText(&cFile, &cTokeniser, "class X : public Y { int a<:2:>; };\na and b bitor c <<= d->*e %: ::"));
	AssertSize(25, cTokeniser.NumTokens());

	AssertTrue(cTokeniser.Get(0)->Is(CT_class));
	AssertTrue(cTokeniser.Get(1)->IsIdentifier());
	AssertTrue(cTokeniser.Get(2)->Is(CT_Colon));
	AssertTrue(cTokeniser.Get(3)->Is(CT_public));
	AssertTrue(cTokeniser.Get(5)->Is(CT_OpenCurlyBracket));
	AssertTrue(cTokeniser.Get(6)->Is(CT_int));

	//Digraphs and alternative tokens are the tokens they stand for.  [lex.digraph]
	AssertTrue(cTokeniser.Get(8)->Is(CT_OpenSquareBracket));
	AssertTrue(cTokeniser.Get(10)->Is(CT_CloseSquareBracket));
	AssertTrue(cTokeniser.Get(15)->Is(CT_LogicalAnd));
	AssertTrue(cTokeniser.Get(17)->Is(CT_BitwiseOr));
	AssertTrue(cTokeniser.Get(19)->Is(CT_AssignmentLeftShift));
	AssertTrue(cTokeniser.Get(21)->Is(CT_MemberByPointerFunctionPointerIndirection));
	AssertTrue(cTokeniser.Get(23)->Is(CT_Hash));
	AssertTrue(cTokeniser.Get(24)->Is(CT_ScopeResolution));

	//Tokens print as they were spelled.
	sz.Init();
	cTokeniser.Print(&sz);
	AssertString("class X : public Y { int a <: 2 :> ; } ;\na and b bitor c <<= d ->* e %: ::\n", sz.Text());
	sz.Kill();

	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// <:: is < and :: (a template argument that starts with ::), not the
// digraph <: and a :, unless : or > follows it.  [lex.pptoken] 3
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserDigraphBeforeScope(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	static ECppTokens	aeExpected[] = {		//NUM_CPP_TOKENS for an identifier.
		NUM_CPP_TOKENS, CT_LessThan, CT_ScopeResolution, NUM_CPP_TOKENS, CT_GreaterThan,
		NUM_CPP_TOKENS, CT_OpenSquareBracket, CT_ScopeResolution, NUM_CPP_TOKENS,
		CT_OpenSquareBracket, CT_CloseSquareBracket,
		NUM_CPP_TOKENS, CT_OpenSquareBracket, NUM_CPP_TOKENS, CT_CloseSquareBracket,
		CT_LessThan, CT_ScopeResolution };
	size				i;

	//The last line but one's <: is pasted, and the last's <:: ends the source.
	AssertTrue(TokeniseText(&cFile, &cTokeniser, "#define PASTE(a, b) a##b\nX<::Y>\na<:::b\n<::>\nc PASTE(<, :)d:>\n<::"));
	AssertSize(sizeof(aeExpected) / sizeof(ECppTokens), cTokeniser.NumTokens());
	for (i = 0; i < cTokeniser.NumTokens(); i++)
	{
		if (aeExpected[i] == NUM_CPP_TOKENS)
		{
			AssertTrue(cTokeniser.Get(i)->IsIdentifier());
		}
		else
		{
			AssertTrue(cTokeniser.Get(i)->Is(aeExpected[i]));
		}
	}

	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserIdentifiers(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CCTIdentifier*		pcFirst;
	CCTIdentifier*		pcSecond;
	CCTIdentifier*		pcThird;
	int					iLength;

	AssertTrue(TokeniseText(&cFile, &cTokeniser, "Fred _x Fred override"));
	AssertSize(4, cTokeniser.NumTokens());

	//The same name has the same ID.
	pcFirst = (CCTIdentifier*)cTokeniser.Get(0);
	pcSecond = (CCTIdentifier*)cTokeniser.Get(1);
	pcThird = (CCTIdentifier*)cTokeniser.Get(2);
	AssertTrue(pcFirst->IsIdentifier());
	AssertLong(pcFirst->miIdentifier, pcThird->miIdentifier);
	AssertTrue(pcFirst->miIdentifier != pcSecond->miIdentifier);
	AssertString("Fred", cTokeniser.GetIdentifier(pcFirst->miIdentifier, &iLength));
	AssertInt(4, iLength);

	//override is an identifier with a special meaning.  [lex.name] 2
	AssertTrue(cTokeniser.Get(3)->IsIdentifier());
	AssertLong(cTokeniser.mlliOverride, ((CCTIdentifier*)cTokeniser.Get(3))->miIdentifier);

	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szText is one identifier with a character the language doesn't allow in
// one: the one error szError (its message, then the identifier).
//
//////////////////////////////////////////////////////////////////////////
static void AssertBadIdentifier(const char* szText, const char* szError)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CChars				szSource;
	CChars				szExpected;

	szSource.Init("int ");
	szSource.Append(szText);
	szSource.Append(';');
	AssertFalse(TokeniseText(&cFile, &cTokeniser, szSource.Text()));
	szExpected.Init("Tokens.cpp (1, 5): ");
	szExpected.Append(szError);
	szExpected.Append(" [");
	szExpected.Append(szText);
	szExpected.Append("]\n");
	AssertString(szExpected.Text(), cTokeniser.GetErrors()->Text());
	szExpected.Kill();
	cTokeniser.Kill();
	cFile.Kill();
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An identifier may have characters past ASCII, written in the source's
// UTF-8 or as universal character names: it is the same identifier either
// way (its name is UTF-8), through # and ## too.  Only the characters the
// language allows, and not a combining character first.  A byte order
// mark at the start of the source is not part of it.  [lex.name]
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserExtendedIdentifiers(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CCTIdentifier*		pcCafe;
	CCTString*			pcString;
	int					iLength;
	CChars				sz;

	//é is U+00E9 (C3 A9), the combining acute accent U+0301 (CC 81) and € U+20AC (E2 82 AC).
	AssertTrue(TokeniseText(&cFile, &cTokeniser, "caf\xC3\xA9 caf\\u00E9 caf\\U000000E9 ca\\\nf\\u00e9 \xC3\xA9t\xC3\xA9 e\\u0301 _\xE2\x82\xAC" "9 cafe"));
	AssertSize(8, cTokeniser.NumTokens());
	pcCafe = (CCTIdentifier*)cTokeniser.Get(0);
	AssertTrue(pcCafe->IsIdentifier());
	AssertString("caf\xC3\xA9", cTokeniser.GetIdentifier(pcCafe->miIdentifier, &iLength));
	AssertInt(5, iLength);
	AssertInt(pcCafe->miIdentifier, ((CCTIdentifier*)cTokeniser.Get(1))->miIdentifier);
	AssertInt(pcCafe->miIdentifier, ((CCTIdentifier*)cTokeniser.Get(2))->miIdentifier);
	AssertInt(pcCafe->miIdentifier, ((CCTIdentifier*)cTokeniser.Get(3))->miIdentifier);
	AssertString("\xC3\xA9t\xC3\xA9", cTokeniser.GetIdentifier(((CCTIdentifier*)cTokeniser.Get(4))->miIdentifier));
	AssertString("e\xCC\x81", cTokeniser.GetIdentifier(((CCTIdentifier*)cTokeniser.Get(5))->miIdentifier));
	AssertString("_\xE2\x82\xAC" "9", cTokeniser.GetIdentifier(((CCTIdentifier*)cTokeniser.Get(6))->miIdentifier));
	AssertTrue(pcCafe->miIdentifier != ((CCTIdentifier*)cTokeniser.Get(7))->miIdentifier);

	//Tokens print as the characters they are.
	sz.Init();
	cTokeniser.Print(&sz);
	AssertString("caf\xC3\xA9 caf\xC3\xA9 caf\xC3\xA9 caf\xC3\xA9 \xC3\xA9t\xC3\xA9 e\xCC\x81 _\xE2\x82\xAC" "9 cafe\n", sz.Text());
	sz.Kill();
	cTokeniser.Kill();
	cFile.Kill();

	//A macro's name, a stringified argument and a pasted name.
	AssertTrue(TokeniseText(&cFile, &cTokeniser, "#define caf\\u00E9 7\n#define STRING(x) #x\n#define PASTE(a, b) a##b\ncaf\xC3\xA9 STRING(caf\\u00E9) PASTE(\\u00E9, t\xC3\xA9) \xC3\xA9t\\u00E9"));
	AssertSize(4, cTokeniser.NumTokens());
	AssertInteger(cTokeniser.Get(0), 7, true, false);
	pcString = (CCTString*)cTokeniser.Get(1);
	AssertTrue(pcString->IsString());
	AssertInt(7, (int)(pcString->mszEnd - pcString->mszStart) + 1);
	AssertInt(0, memcmp("\"caf\xC3\xA9\"", pcString->mszStart, 7));
	AssertTrue(cTokeniser.Get(2)->IsIdentifier());
	AssertInt(((CCTIdentifier*)cTokeniser.Get(3))->miIdentifier, ((CCTIdentifier*)cTokeniser.Get(2))->miIdentifier);
	cTokeniser.Kill();
	cFile.Kill();

	//A byte order mark.  Columns are still bytes from the start of the line.
	AssertTrue(TokeniseText(&cFile, &cTokeniser, "\xEF\xBB\xBFint x;"));
	AssertSize(3, cTokeniser.NumTokens());
	AssertTrue(cTokeniser.Get(0)->Is(CT_int));
	AssertInt(3, cTokeniser.GetSource(cTokeniser.Get(0))->iColumn);
	cTokeniser.Kill();
	cFile.Kill();

	//A no-break space (U+00A0) and a multiplication sign (U+00D7) are not letters, an identifier can't start with a
	//combining character, and bytes that aren't UTF-8 are no character at all.
	AssertBadIdentifier("\xC2\xA0x", "This character is not allowed in an identifier.");
	AssertBadIdentifier("a\xC3\x97" "b", "This character is not allowed in an identifier.");
	AssertBadIdentifier("\xCC\x81" "e", "An identifier can't start with this character.");
	AssertBadIdentifier("a\xC3", "The source is not UTF-8 here.");
	AssertBadIdentifier("\xFFx", "The source is not UTF-8 here.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserIntegers(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;

	AssertTrue(TokeniseText(&cFile, &cTokeniser, "0 2147483647 2147483648 0x7FFFFFFF 0x80000000 0xFFFFFFFF 0x100000000 10u 10l 10ul 10ll 0xFFFFFFFFFFFFFFFFull 017 0b1010 1'000'000 7i64 0x10'0000"));
	AssertSize(17, cTokeniser.NumTokens());

	//[lex.icon] Table 6 with 32 bit long.
	AssertInteger(cTokeniser.Get(0), 0, true, false);
	AssertInteger(cTokeniser.Get(1), 2147483647, true, false);
	AssertLongLong(cTokeniser.Get(2), 2147483648ULL, true);
	AssertInteger(cTokeniser.Get(3), 0x7FFFFFFF, true, false);
	AssertInteger(cTokeniser.Get(4), 0x80000000, false, false);
	AssertInteger(cTokeniser.Get(5), 0xFFFFFFFF, false, false);
	AssertLongLong(cTokeniser.Get(6), 0x100000000ULL, true);
	AssertInteger(cTokeniser.Get(7), 10, false, false);
	AssertInteger(cTokeniser.Get(8), 10, true, true);
	AssertInteger(cTokeniser.Get(9), 10, false, true);
	AssertLongLong(cTokeniser.Get(10), 10, true);
	AssertLongLong(cTokeniser.Get(11), 0xFFFFFFFFFFFFFFFFULL, false);
	AssertInteger(cTokeniser.Get(12), 15, true, false);
	AssertInteger(cTokeniser.Get(13), 10, true, false);
	AssertInteger(cTokeniser.Get(14), 1000000, true, false);
	AssertLongLong(cTokeniser.Get(15), 7, true);
	AssertInteger(cTokeniser.Get(16), 0x100000, true, false);

	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserFloatingAndCharacters(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CCTString*			pcString;

	AssertTrue(TokeniseText(&cFile, &cTokeniser, "1.5f 2.0 3e2 .25L 0x1p-2 'a' '\\n' '\\x41' '\\101' L'b' 'AB' true false \"x\" L\"wide\" u8\"utf\""));
	AssertSize(16, cTokeniser.NumTokens());

	AssertFloat(1.5f, ((CCTFloat*)cTokeniser.Get(0))->mcValue, 3);
	AssertDouble(2.0, ((CCTDouble*)cTokeniser.Get(1))->mcValue, 3);
	AssertDouble(300.0, ((CCTDouble*)cTokeniser.Get(2))->mcValue, 3);
	AssertLongDouble(0.25L, ((CCTLongDouble*)cTokeniser.Get(3))->mcValue, 3);
	AssertDouble(0.25, ((CCTDouble*)cTokeniser.Get(4))->mcValue, 3);

	AssertTrue(cTokeniser.Get(5)->IsChar());
	AssertChar('a', ((CCTChar*)cTokeniser.Get(5))->mcChar);
	AssertChar('\n', ((CCTChar*)cTokeniser.Get(6))->mcChar);
	AssertChar('A', ((CCTChar*)cTokeniser.Get(7))->mcChar);
	AssertChar('A', ((CCTChar*)cTokeniser.Get(8))->mcChar);

	//wchar_t is 16 bits on Windows.  A multi-character literal is an int.
	AssertInt('b', ((CCTShort*)cTokeniser.Get(9))->mcValue);
	AssertInteger(cTokeniser.Get(10), ('A' << 8) | 'B', true, false);

	AssertTrue(cTokeniser.Get(11)->IsBool());
	AssertTrue(((CCTBoolean*)cTokeniser.Get(11))->mcBool);
	AssertFalse(((CCTBoolean*)cTokeniser.Get(12))->mcBool);

	pcString = (CCTString*)cTokeniser.Get(13);
	AssertTrue(pcString->IsString());
	AssertInt(CTE_Narrow, pcString->meEncoding);
	pcString = (CCTString*)cTokeniser.Get(14);
	AssertInt(CTE_Wide, pcString->meEncoding);
	AssertTrue(pcString->IsWide());
	pcString = (CCTString*)cTokeniser.Get(15);
	AssertInt(CTE_UTF8, pcString->meEncoding);
	AssertFalse(pcString->IsWide());

	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szText is one character literal: uiValue if szError is NULL, else the
// one error szError (its message, then the literal).
//
//////////////////////////////////////////////////////////////////////////
static void AssertCharacterLiteral(const char* szText, uint32 uiValue, const char* szError)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CChars				szSource;
	CChars				szExpected;
	CCPPToken*			pcToken;

	szSource.Init("int x = ");
	szSource.Append(szText);
	szSource.Append(';');
	if (!szError)
	{
		AssertTrue(TokeniseText(&cFile, &cTokeniser, szSource.Text()));
		AssertString("", cTokeniser.GetErrors()->Text());
		pcToken = cTokeniser.Get(3);
		if (pcToken->IsChar())
		{
			AssertIntHex(uiValue, (uint32)(uint8)((CCTChar*)pcToken)->mcChar);
		}
		else if (((CCTNumber*)pcToken)->meType == CTNT_Short)
		{
			AssertIntHex(uiValue, (uint32)(uint16)((CCTShort*)pcToken)->mcValue);
		}
		else
		{
			AssertIntHex(uiValue, (uint32)((CCTInt*)pcToken)->mcValue);
		}
	}
	else
	{
		AssertFalse(TokeniseText(&cFile, &cTokeniser, szSource.Text()));
		szExpected.Init("Tokens.cpp (1, 9): ");
		szExpected.Append(szError);
		szExpected.Append(" [");
		szExpected.Append(szText);
		szExpected.Append("]\n");
		AssertString(szExpected.Text(), cTokeniser.GetErrors()->Text());
		szExpected.Kill();
	}
	cTokeniser.Kill();
	cFile.Kill();
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A character is the same spelled in the source's UTF-8 or as a universal
// character name.  In a narrow literal it is its UTF-8 bytes, each a
// character of the literal; in a u8, L, u or U literal it must be one code
// unit, and u8, u and U literals hold one character.  A numeric escape
// must fit the literal's type.  [lex.ccon]
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserCharacterLiterals(void)
{
	//é is U+00E9 (C3 A9), € is U+20AC (E2 82 AC) and the last is U+1F600 (F0 9F 98 80).  A byte that doesn't start
	//UTF-8 (Latin-1's C0 and E9) is itself.
	AssertCharacterLiteral("u8'a'", 'a', NULL);
	AssertCharacterLiteral("L'\xC3\xA9'", 0xE9, NULL);
	AssertCharacterLiteral("u'\xC3\xA9'", 0xE9, NULL);
	AssertCharacterLiteral("u'\\u00E9'", 0xE9, NULL);
	AssertCharacterLiteral("u'\xE2\x82\xAC'", 0x20AC, NULL);
	AssertCharacterLiteral("U'\xE2\x82\xAC'", 0x20AC, NULL);
	AssertCharacterLiteral("U'\xF0\x9F\x98\x80'", 0x1F600, NULL);
	AssertCharacterLiteral("U'\\U0001F600'", 0x1F600, NULL);
	AssertCharacterLiteral("'\xC3\xA9'", 0xC3A9, NULL);
	AssertCharacterLiteral("'\\u00E9'", 0xC3A9, NULL);
	AssertCharacterLiteral("'\\u0041'", 'A', NULL);
	AssertCharacterLiteral("'\\xE9'", 0xE9, NULL);
	AssertCharacterLiteral("'\xC0'", 0xC0, NULL);
	AssertCharacterLiteral("'\xE9'", 0xE9, NULL);
	AssertCharacterLiteral("L'ab'", ('a' << 8) | 'b', NULL);

	AssertCharacterLiteral("u8'\xC3\xA9'", 0, "The character is more than one code unit of the character literal's encoding.");
	AssertCharacterLiteral("u8'\\u00E9'", 0, "The character is more than one code unit of the character literal's encoding.");
	AssertCharacterLiteral("u'\xF0\x9F\x98\x80'", 0, "The character is more than one code unit of the character literal's encoding.");
	AssertCharacterLiteral("L'\\U0001F600'", 0, "The character is more than one code unit of the character literal's encoding.");
	AssertCharacterLiteral("u8'ab'", 0, "More than one character in a u8, u or U character literal.");
	AssertCharacterLiteral("u'ab'", 0, "More than one character in a u8, u or U character literal.");
	AssertCharacterLiteral("U'ab'", 0, "More than one character in a u8, u or U character literal.");
	AssertCharacterLiteral("'\\x100'", 0, "The escape sequence's value is too large for the character literal's type.");
	AssertCharacterLiteral("u8'\\400'", 0, "The escape sequence's value is too large for the character literal's type.");
	AssertCharacterLiteral("u'\\x10000'", 0, "The escape sequence's value is too large for the character literal's type.");
	AssertCharacterLiteral("'\\q'", 0, "Invalid escape sequence.");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserLocations(void)
{
	CTranslationUnit		cFile;
	CCPPTokeniser			cTokeniser;
	SPreprocessedToken*		psSource;

	AssertTrue(TokeniseText(&cFile, &cTokeniser, "#define N 3\nint\n  x = N;"));
	AssertSize(5, cTokeniser.NumTokens());

	//Each token knows the preprocessed token it came from.
	psSource = cTokeniser.GetSource(cTokeniser.Get(1));
	AssertInt(2, psSource->iLine);
	AssertInt(2, psSource->iColumn);
	AssertInt(-1, psSource->iExpansion);

	psSource = cTokeniser.GetSource(cTokeniser.Get(3));
	AssertInt(0, psSource->iLine);
	AssertInt(10, psSource->iColumn);
	AssertInt(0, psSource->iExpansion);

	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szText is one integer literal: tokenised without errors if szError is
// NULL, else with the one error szError (its message, then the literal).
//
//////////////////////////////////////////////////////////////////////////
static void AssertIntegerLiteral(const char* szText, const char* szError)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CChars				szSource;
	CChars				szExpected;

	szSource.Init("int x = ");
	szSource.Append(szText);
	szSource.Append(';');
	if (!szError)
	{
		AssertTrue(TokeniseText(&cFile, &cTokeniser, szSource.Text()));
		AssertString("", cTokeniser.GetErrors()->Text());
	}
	else
	{
		AssertFalse(TokeniseText(&cFile, &cTokeniser, szSource.Text()));
		szExpected.Init("Tokens.cpp (1, 9): ");
		szExpected.Append(szError);
		szExpected.Append(" [");
		szExpected.Append(szText);
		szExpected.Append("]\n");
		AssertString(szExpected.Text(), cTokeniser.GetErrors()->Text());
		szExpected.Kill();
	}
	cTokeniser.Kill();
	cFile.Kill();
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An integer literal's suffix is u and l or ll (either case, each once, in
// either order), or Microsoft's i8, i16, i32 and i64 after an optional u;
// any other identifier is a user-defined suffix
// (TestCppTokeniserUserLiterals), and anything else an error.  0x and 0b
// need a digit, an exponent needs one, a hexadecimal floating literal
// needs an exponent, and a digit separator is between two digits.
// [lex.icon], [lex.fcon]
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserIntegerLiterals(void)
{
	static const char*	aszValid[] = { "1u", "1U", "1l", "1L", "1ul", "1lu", "1ull", "1llu", "1LL", "1uLL", "1LLU", "1i64", "1ui64", "1I32", "1i16", "1i8", "0x1F", "0b101", "0", "017", NULL };
	static const char*	aszBadSuffix[] = { "1.2.3", "1..", "0x1p3.5", "1.5e3.f", NULL };
	static const char*	aszBadSeparator[] = { "1'_0", "0x'FF", "1.'5", "1'u", NULL };
	static const char*	aszNoExponent[] = { "1e", "1e+", "1.5E-", "0x1.8", "0x1p", "1e_x", NULL };
	CChars				szLong;
	size				i;

	for (i = 0; aszValid[i]; i++)
	{
		AssertIntegerLiteral(aszValid[i], NULL);
	}
	for (i = 0; aszBadSuffix[i]; i++)
	{
		AssertIntegerLiteral(aszBadSuffix[i], "Invalid number literal suffix.");
	}
	for (i = 0; aszBadSeparator[i]; i++)
	{
		AssertIntegerLiteral(aszBadSeparator[i], "A digit separator must be between two digits.");
	}
	for (i = 0; aszNoExponent[i]; i++)
	{
		AssertIntegerLiteral(aszNoExponent[i], "Floating literal has no exponent digits.");
	}
	AssertIntegerLiteral("0x", "Integer literal has no digits.");
	AssertIntegerLiteral("0bu", "Integer literal has no digits.");

	//Longer than the 255 characters that are kept is too large, not cut short.
	szLong.Init();
	szLong.Append('0', 300);
	szLong.Append('1');
	AssertIntegerLiteral(szLong.Text(), "Integer literal is too large.");
	szLong.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The user-defined literal at token uiToken: its suffix, and the literal
// it is without it.
//
//////////////////////////////////////////////////////////////////////////
static CCPPToken* AssertUserLiteral(CCPPTokeniser* pcTokeniser, size uiToken, const char* szSuffix)
{
	CCTUserLiteral*		pcUserLiteral;

	pcUserLiteral = (CCTUserLiteral*)pcTokeniser->Get(uiToken);
	AssertTrue(pcUserLiteral->IsUserLiteral());
	AssertFalse(pcUserLiteral->IsNumber() || pcUserLiteral->IsChar() || pcUserLiteral->IsString());
	AssertString(szSuffix, pcTokeniser->GetIdentifier(pcUserLiteral->miSuffix));
	return pcUserLiteral->mpcLiteral;
}


//////////////////////////////////////////////////////////////////////////
//
// A number, character or string followed by an identifier, with nothing
// between them, is a user-defined literal: the literal as it is without
// the suffix (an integer the widest, a floating literal a long double),
// and the suffix.  Only an identifier that is one of the language's own
// suffixes isn't.  [lex.ext]
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserUserLiterals(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;
	CCPPToken*			pcLiteral;
	CChars				sz;

	AssertTrue(TokeniseText(&cFile, &cTokeniser, "10_km 0x1F_h 1'000uu 1.5_m 1e3f_x 'a'_c u'\\u00E9'_c \"text\"_s u8\"w\\\"\"_w8 12q 1.5f 10u \"s\" _s"));
	AssertSize(14, cTokeniser.NumTokens());

	AssertLongLong(AssertUserLiteral(&cTokeniser, 0, "_km"), 10, false);
	AssertLongLong(AssertUserLiteral(&cTokeniser, 1, "_h"), 0x1F, false);
	AssertLongLong(AssertUserLiteral(&cTokeniser, 2, "uu"), 1000, false);
	AssertLongDouble(1.5L, ((CCTLongDouble*)AssertUserLiteral(&cTokeniser, 3, "_m"))->mcValue, 3);
	AssertLongDouble(1000.0L, ((CCTLongDouble*)AssertUserLiteral(&cTokeniser, 4, "f_x"))->mcValue, 3);
	pcLiteral = AssertUserLiteral(&cTokeniser, 5, "_c");
	AssertTrue(pcLiteral->IsChar());
	AssertChar('a', ((CCTChar*)pcLiteral)->mcChar);
	AssertInt(0xE9, ((CCTShort*)AssertUserLiteral(&cTokeniser, 6, "_c"))->mcValue);
	AssertTrue(cTokeniser.Get(6)->IsUserLiteral() && !cTokeniser.Get(6)->IsUserString());

	//The string is the literal without its suffix, to its closing quote.
	pcLiteral = AssertUserLiteral(&cTokeniser, 7, "_s");
	AssertTrue(pcLiteral->IsString() && cTokeniser.Get(7)->IsUserString());
	AssertInt(CTE_Narrow, ((CCTString*)pcLiteral)->meEncoding);
	AssertInt(6, (int)(((CCTString*)pcLiteral)->mszEnd - ((CCTString*)pcLiteral)->mszStart) + 1);
	pcLiteral = AssertUserLiteral(&cTokeniser, 8, "_w8");
	AssertInt(CTE_UTF8, ((CCTString*)pcLiteral)->meEncoding);
	AssertInt(7, (int)(((CCTString*)pcLiteral)->mszEnd - ((CCTString*)pcLiteral)->mszStart) + 1);
	AssertChar('"', *((CCTString*)pcLiteral)->mszEnd);

	AssertLongLong(AssertUserLiteral(&cTokeniser, 9, "q"), 12, false);

	//The language's suffixes, and an identifier after a space, are not.
	AssertFalse(cTokeniser.Get(10)->IsUserLiteral());
	AssertFloat(1.5f, ((CCTFloat*)cTokeniser.Get(10))->mcValue, 3);
	AssertInteger(cTokeniser.Get(11), 10, false, false);
	AssertTrue(cTokeniser.Get(12)->IsString());
	AssertTrue(cTokeniser.Get(13)->IsIdentifier());

	//Tokens print as they were spelled.
	sz.Init();
	cTokeniser.Print(&sz);
	AssertString("10_km 0x1F_h 1'000uu 1.5_m 1e3f_x 'a'_c u'\\u00E9'_c \"text\"_s u8\"w\\\"\"_w8 12q 1.5f 10u \"s\" _s\n", sz.Text());
	sz.Kill();
	cTokeniser.Kill();
	cFile.Kill();

	//As g++ has it, a macro touching the end of a string is the macro (so "%" PRId64 needs no space), unless its name
	//starts with an underscore.  ## makes a suffix as writing it does.
	AssertTrue(TokeniseText(&cFile, &cTokeniser, "#define PRId64 \"lld\"\n#define _fmt 1\n#define PASTE(a, b) a##b\n\"%\"PRId64 \"%\"_fmt 'a'PRId64 PASTE(\"x\", _s) PASTE(1, _km) PASTE(L, \"wide\") PASTE(u8, 'c')"));
	AssertSize(9, cTokeniser.NumTokens());
	AssertTrue(cTokeniser.Get(0)->IsString());
	AssertTrue(cTokeniser.Get(1)->IsString());
	AssertTrue(AssertUserLiteral(&cTokeniser, 2, "_fmt")->IsString());
	AssertTrue(cTokeniser.Get(3)->IsChar());
	AssertTrue(cTokeniser.Get(4)->IsString());
	AssertTrue(AssertUserLiteral(&cTokeniser, 5, "_s")->IsString());
	AssertLongLong(AssertUserLiteral(&cTokeniser, 6, "_km"), 1, false);
	AssertInt(CTE_Wide, ((CCTString*)cTokeniser.Get(7))->meEncoding);
	AssertTrue(cTokeniser.Get(8)->IsChar());
	cTokeniser.Kill();
	cFile.Kill();

	//A literal that doesn't end is an error, and takes no suffix.
	AssertFalse(TokeniseText(&cFile, &cTokeniser, "const char* s = \"abc;\nchar c = 'd;\n"));
	AssertString("Tokens.cpp (1, 17): Unterminated string literal. [\"abc;]\nTokens.cpp (2, 10): Unterminated character literal. ['d;]\n", cTokeniser.GetErrors()->Text());
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The spelling of the string at token uiToken (a user-defined literal's
// string too).
//
//////////////////////////////////////////////////////////////////////////
static void AssertStringSpelling(CCPPTokeniser* pcTokeniser, size uiToken, const char* szSpelling, ECTEncoding eEncoding, bool bRaw)
{
	CCPPToken*	pcToken;
	CCTString*	pcString;
	CChars		sz;

	pcToken = pcTokeniser->Get(uiToken);
	pcString = (CCTString*)(pcToken->IsUserLiteral() ? ((CCTUserLiteral*)pcToken)->mpcLiteral : pcToken);
	AssertTrue(pcString->IsString());
	sz.Init(pcString->mszStart, 0, (size)(pcString->mszEnd - pcString->mszStart) + 1);
	AssertString(szSpelling, sz.Text());
	sz.Kill();
	AssertInt(eEncoding, pcString->meEncoding);
	AssertBool(bRaw, pcString->IsRaw());
}


//////////////////////////////////////////////////////////////////////////
//
// A raw string, R"delimiter(...)delimiter", is one token whatever is in
// it: new lines, quotes, backslashes (a splice too), comments and
// directives.  Lines are counted through it.  It takes the encoding
// prefixes before its R and a user-defined suffix, and # and ## know it.
// One that doesn't end, or whose delimiter isn't one, is an error on its
// line and no more.  [lex.string], [lex.pptoken]
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserRawStrings(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;

	AssertTrue(TokeniseText(&cFile, &cTokeniser,
		"int a = sizeof(R\"x(one\n"
		"// two /* \\\n"
		"#define THREE 3\n"
		"\"four\" )y\" )x )\"\n"
		")x\"); int b = __LINE__; THREE\n"
		"int c = __LINE__;"));
	AssertSize(19, cTokeniser.NumTokens());
	AssertStringSpelling(&cTokeniser, 5, "R\"x(one\n// two /* \\\n#define THREE 3\n\"four\" )y\" )x )\"\n)x\"", CTE_Narrow, true);
	AssertInt(0, cTokeniser.GetSource(cTokeniser.Get(5))->iLine);
	AssertTrue(cTokeniser.Get(6)->Is(CT_CloseRoundBracket));
	AssertInt(4, cTokeniser.GetSource(cTokeniser.Get(6))->iLine);
	AssertInteger(cTokeniser.Get(11), 5, true, false);
	AssertTrue(cTokeniser.Get(13)->IsIdentifier());
	AssertInteger(cTokeniser.Get(17), 6, true, false);
	cTokeniser.Kill();
	cFile.Kill();

	//Prefixes, a suffix, an empty one, an empty delimiter and the longest one; R alone and R apart from a string are not.
	AssertTrue(TokeniseText(&cFile, &cTokeniser, "u8R\"(a)\" uR\"(b)\" UR\"(c)\" LR\"(d)\" R\"(e)\"_s R\"()\" R\"0123456789abcdef(f)0123456789abcdef\" R R \"g\" \"h\""));
	AssertSize(11, cTokeniser.NumTokens());
	AssertStringSpelling(&cTokeniser, 0, "u8R\"(a)\"", CTE_UTF8, true);
	AssertStringSpelling(&cTokeniser, 1, "uR\"(b)\"", CTE_UTF16, true);
	AssertStringSpelling(&cTokeniser, 2, "UR\"(c)\"", CTE_UTF32, true);
	AssertStringSpelling(&cTokeniser, 3, "LR\"(d)\"", CTE_Wide, true);
	AssertTrue(AssertUserLiteral(&cTokeniser, 4, "_s")->IsString());
	AssertStringSpelling(&cTokeniser, 4, "R\"(e)\"", CTE_Narrow, true);
	AssertStringSpelling(&cTokeniser, 5, "R\"()\"", CTE_Narrow, true);
	AssertStringSpelling(&cTokeniser, 6, "R\"0123456789abcdef(f)0123456789abcdef\"", CTE_Narrow, true);
	AssertTrue(cTokeniser.Get(7)->IsIdentifier());
	AssertTrue(cTokeniser.Get(8)->IsIdentifier());
	AssertStringSpelling(&cTokeniser, 9, "\"g\"", CTE_Narrow, false);
	AssertStringSpelling(&cTokeniser, 10, "\"h\"", CTE_Narrow, false);
	cTokeniser.Kill();
	cFile.Kill();

	//In a macro's body, as an argument, stringified (its quotes and backslashes escaped, its new line \n) and pasted.
	AssertTrue(TokeniseText(&cFile, &cTokeniser,
		"#define TEXT R\"(a\n"
		"b)\"\n"
		"#define STRING(x) #x\n"
		"#define PASTE(a, b) a##b\n"
		"#define SAME(x) x\n"
		"TEXT SAME(R\"(c, d)\") STRING(R\"(e\n"
		"\"f\\)\") PASTE(R, \"(g)\") PASTE(u8R\"(h)\", _s)"));
	AssertSize(5, cTokeniser.NumTokens());
	AssertStringSpelling(&cTokeniser, 0, "R\"(a\nb)\"", CTE_Narrow, true);
	AssertStringSpelling(&cTokeniser, 1, "R\"(c, d)\"", CTE_Narrow, true);
	AssertStringSpelling(&cTokeniser, 2, "\"R\\\"(e\\n\\\"f\\\\)\\\"\"", CTE_Narrow, false);
	AssertStringSpelling(&cTokeniser, 3, "R\"(g)\"", CTE_Narrow, true);
	AssertStringSpelling(&cTokeniser, 4, "u8R\"(h)\"", CTE_UTF8, true);
	AssertTrue(cTokeniser.Get(4)->IsUserLiteral());
	cTokeniser.Kill();
	cFile.Kill();

	//In lines that are skipped: a directive in one is not a directive, and one that doesn't end takes only its line.
	AssertTrue(TokeniseText(&cFile, &cTokeniser, "#if 0\nR\"(\n#endif\n)\" skipped\n#endif\nint x;\n#if 0\nR\"(abc\n#endif\nint y;"));
	AssertSize(6, cTokeniser.NumTokens());
	AssertTrue(cTokeniser.Get(0)->Is(CT_int));
	AssertTrue(cTokeniser.Get(3)->Is(CT_int));
	cTokeniser.Kill();
	cFile.Kill();

	//No end, a delimiter with a space in it, and one of 17 characters.
	AssertFalse(TokeniseText(&cFile, &cTokeniser, "const char* s = R\"x(abc)\";\nint y;"));
	AssertString("Tokens.cpp (1, 17): Raw string literal has no end, or its delimiter is not one. [R\"x(abc)\"]\n", cTokeniser.GetErrors()->Text());
	cTokeniser.Kill();
	cFile.Kill();
	AssertFalse(TokeniseText(&cFile, &cTokeniser, "const char* s = R\"a b(c)a b\";"));
	AssertString("Tokens.cpp (1, 17): Raw string literal has no end, or its delimiter is not one. [R\"a b(c)a b\"]\n", cTokeniser.GetErrors()->Text());
	cTokeniser.Kill();
	cFile.Kill();
	AssertFalse(TokeniseText(&cFile, &cTokeniser, "const char* s = R\"0123456789abcdefg(c)0123456789abcdefg\";"));
	AssertString("Tokens.cpp (1, 17): Raw string literal has no end, or its delimiter is not one. [R\"0123456789abcdefg(c)0123456789abcdefg\"]\n", cTokeniser.GetErrors()->Text());
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserErrors(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;

	//Errors are kept in the tokeniser's list.  Their tokens are preprocessed token indices.
	AssertFalse(TokeniseText(&cFile, &cTokeniser, "int x = 1.q.;"));
	AssertSize(1, cTokeniser.GetErrors()->NumErrors());
	AssertString("Tokens.cpp (1, 9): Invalid number literal suffix. [1.q.]\n", cTokeniser.GetErrors()->Text());
	AssertInt(3, cTokeniser.GetErrors()->GetToken(0));
	AssertInt(0, cTokeniser.GetErrors()->GetLine(0));
	AssertInt(8, cTokeniser.GetErrors()->GetColumn(0));
	cTokeniser.Kill();
	cFile.Kill();

	AssertFalse(TokeniseText(&cFile, &cTokeniser, "int x = 0x1FFFFFFFFFFFFFFFF;"));
	AssertString("Tokens.cpp (1, 9): Integer literal is too large. [0x1FFFFFFFFFFFFFFFF]\n", cTokeniser.GetErrors()->Text());
	cTokeniser.Kill();
	cFile.Kill();

	AssertFalse(TokeniseText(&cFile, &cTokeniser, "char c = '';\nint y = 1; int z = 2..;"));
	AssertString("Tokens.cpp (1, 10): Empty character literal. ['']\nTokens.cpp (2, 20): Invalid number literal suffix. [2..]\n", cTokeniser.GetErrors()->Text());
	cTokeniser.Kill();
	cFile.Kill();

	AssertTrue(TokeniseText(&cFile, &cTokeniser, "int x = 12;"));
	AssertSize(0, cTokeniser.GetErrors()->NumErrors());
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The identifiers' index: IDs in the order names are added, found both ways,
// names added again refused, and still found after the table has grown.
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserIdentifierIndex(void)
{
	CASCIIIndex	cIndex;
	char		sz[32];
	char*		szName;
	int			iLength;
	int			i;
	const char*	szValueAndMore;
	const char*	szCountMe;

	//A name's start and last character are in the same string: two equal literals are only the same string
	//when the compiler pools them (not in Debug).
	szValueAndMore = "ValueAndMore";
	szCountMe = "CountMe";

	cIndex.Init();
	AssertInt(0, cIndex.NumElements());
	AssertLong(-1, cIndex.Get("miCount", NULL));
	AssertNull(cIndex.Get(0));

	AssertLong(0, cIndex.Add("miCount"));
	AssertLong(1, cIndex.Add("Count"));
	AssertLong(2, cIndex.Add(szValueAndMore, szValueAndMore + 4));
	AssertLong(-1, cIndex.Add("Count"));
	AssertInt(3, cIndex.NumElements());

	AssertLong(0, cIndex.Get("miCount", NULL));
	AssertLong(1, cIndex.Get("Count", NULL));
	AssertLong(1, cIndex.Get(szCountMe, szCountMe + 4));
	AssertLong(-1, cIndex.Get("Coun", NULL));
	szName = cIndex.Get(2, &iLength);
	AssertString("Value", szName);
	AssertInt(5, iLength);
	AssertNull(cIndex.Get(3));
	AssertNull(cIndex.Get(-1));

	for (i = 3; i < 2000; i++)
	{
		sprintf(sz, "Name%d", i);
		AssertLong(i, cIndex.Add(sz));
	}
	AssertInt(2000, cIndex.NumElements());
	for (i = 3; i < 2000; i++)
	{
		sprintf(sz, "Name%d", i);
		AssertLong(i, cIndex.Get(sz, NULL));
		AssertString(sz, cIndex.Get(i));
	}
	AssertLong(0, cIndex.Get("miCount", NULL));
	AssertString("Count", cIndex.Get(1));

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What an operator token is, which is asked of the punctuators' table: the
// eleven assignments, the six comparisons, and the operation of each
// compound assignment.  Every other token, a keyword too, is neither and
// is its own operation.
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserOperators(void)
{
	static const ECppTokens	aeAssignments[] = { CT_Assignment, CT_AssignmentAdd, CT_AssignmentSubtract, CT_AssignmentMultiply, CT_AssignmentDivide, CT_AssignmentModulus,
												CT_AssignmentBitwiseAnd, CT_AssignmentBitwiseOr, CT_AssignmentBitwiseXor, CT_AssignmentLeftShift, CT_AssignmentRightShift };
	static const ECppTokens	aeOperations[] = { CT_Assignment, CT_Add, CT_Subtract, CT_Multiply, CT_Divide, CT_Modulus, CT_BitwiseAnd, CT_BitwiseOr, CT_BitwiseXor, CT_LeftShift, CT_RightShift };
	static const ECppTokens	aeComparisons[] = { CT_EqualTo, CT_NotEqualTo, CT_LessThan, CT_LessThanEqualTo, CT_GreaterThan, CT_GreaterThanEqualTo };
	int		iToken;
	size	i;
	size	uiAssignments;
	size	uiComparisons;

	for (i = 0; i < sizeof(aeAssignments) / sizeof(ECppTokens); i++)
	{
		AssertTrue(IsCPPAssignment(aeAssignments[i]));
		AssertFalse(IsCPPComparison(aeAssignments[i]));
		AssertInt(aeOperations[i], CPPCompoundOperation(aeAssignments[i]));
	}
	for (i = 0; i < sizeof(aeComparisons) / sizeof(ECppTokens); i++)
	{
		AssertTrue(IsCPPComparison(aeComparisons[i]));
		AssertFalse(IsCPPAssignment(aeComparisons[i]));
		AssertInt(aeComparisons[i], CPPCompoundOperation(aeComparisons[i]));
	}

	//Nothing else is either: the punctuators, and the keywords after them, which the table doesn't have.
	uiAssignments = 0;
	uiComparisons = 0;
	for (iToken = CT_OpenRoundBracket; iToken < NUM_CPP_TOKENS; iToken++)
	{
		uiAssignments += IsCPPAssignment((ECppTokens)iToken) ? 1 : 0;
		uiComparisons += IsCPPComparison((ECppTokens)iToken) ? 1 : 0;
	}
	AssertSize(11, uiAssignments);
	AssertSize(6, uiComparisons);
	AssertInt(CT_Add, CPPCompoundOperation(CT_Add));
	AssertInt(CT_LogicalAnd, CPPCompoundOperation(CT_LogicalAnd));
	AssertInt(CT___thread, CPPCompoundOperation(CT___thread));
	AssertFalse(IsCPPAssignment(CT___thread));
	AssertFalse(IsCPPComparison(CT_LeftShift));
}


//////////////////////////////////////////////////////////////////////////
//
// The names the compiler knows have numbers from the start, the same ones
// the source's own uses of them get.
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserKnownNames(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;

	AssertTrue(TokeniseText(&cFile, &cTokeniser, (char*)"memcpy __func__ __FUNCTION__ __PRETTY_FUNCTION__ __FUNCSIG__ __FUNCDNAME__ __builtin_memcpy other"));
	AssertLong(cTokeniser.mlliMemcpy, ((CCTIdentifier*)cTokeniser.Get(0))->miIdentifier);
	AssertLong(cTokeniser.mlliFunc, ((CCTIdentifier*)cTokeniser.Get(1))->miIdentifier);
	AssertLong(cTokeniser.mlliFunction, ((CCTIdentifier*)cTokeniser.Get(2))->miIdentifier);
	AssertLong(cTokeniser.mlliPrettyFunction, ((CCTIdentifier*)cTokeniser.Get(3))->miIdentifier);
	AssertLong(cTokeniser.mlliFuncSig, ((CCTIdentifier*)cTokeniser.Get(4))->miIdentifier);
	AssertLong(cTokeniser.mlliFuncDName, ((CCTIdentifier*)cTokeniser.Get(5))->miIdentifier);
	AssertLong(cTokeniser.mlliBuiltinMemcpy, ((CCTIdentifier*)cTokeniser.Get(6))->miIdentifier);
	AssertTrue(((CCTIdentifier*)cTokeniser.Get(7))->miIdentifier != cTokeniser.mlliMemcpy);
	AssertTrue(cTokeniser.mlliFunc != cTokeniser.mlliFunction);
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniser(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();

	TestCppTokeniserKeywordsAndPunctuators();
	TestCppTokeniserDigraphBeforeScope();
	TestCppTokeniserIdentifiers();
	TestCppTokeniserExtendedIdentifiers();
	TestCppTokeniserIntegers();
	TestCppTokeniserFloatingAndCharacters();
	TestCppTokeniserCharacterLiterals();
	TestCppTokeniserLocations();
	TestCppTokeniserErrors();
	TestCppTokeniserIntegerLiterals();
	TestCppTokeniserUserLiterals();
	TestCppTokeniserRawStrings();
	TestCppTokeniserIdentifierIndex();
	TestCppTokeniserOperators();
	TestCppTokeniserKnownNames();

	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

