#include <stdio.h>
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Logger.h"
#include "CppParserLib/Preprocessor.h"
#include "CppParserLib/CppTokeniser.h"
#include "CppParserLib/ASCIIIndex.h"
#include "CppParserLib/CTReservedWord.h"
#include "CppParserLib/CTIdentifier.h"
#include "CppParserLib/CTInt.h"
#include "CppParserLib/CTLongLong.h"
#include "CppParserLib/CTShort.h"
#include "CppParserLib/CTChar.h"
#include "CppParserLib/CTFloat.h"
#include "CppParserLib/CTDouble.h"
#include "CppParserLib/CTLongDouble.h"
#include "CppParserLib/CTString.h"
#include "CppParserLib/CTBoolean.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TokeniseText(CTranslationUnit* pcFile, CCPPTokeniser* pcTokeniser, char* szSource)
{
	CPreprocessor	cPreprocessor;
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
// either order), or Microsoft's i8, i16, i32 and i64 after an optional u.
// 0x and 0b need a digit.  [lex.icon]
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserIntegerLiterals(void)
{
	static const char*	aszValid[] = { "1u", "1U", "1l", "1L", "1ul", "1lu", "1ull", "1llu", "1LL", "1uLL", "1LLU", "1i64", "1ui64", "1I32", "1i16", "1i8", "0x1F", "0b101", "0", "017", NULL };
	static const char*	aszBadSuffix[] = { "1uu", "1lul", "1lL", "1Ll", "1lll", "1i64l", "1li64", "1i64u", "1ui", NULL };
	CChars				szLong;
	size				i;

	for (i = 0; aszValid[i]; i++)
	{
		AssertIntegerLiteral(aszValid[i], NULL);
	}
	for (i = 0; aszBadSuffix[i]; i++)
	{
		AssertIntegerLiteral(aszBadSuffix[i], "Invalid integer literal suffix.");
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
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniserErrors(void)
{
	CTranslationUnit	cFile;
	CCPPTokeniser		cTokeniser;

	//Errors are kept in the tokeniser's list.  Their tokens are preprocessed token indices.
	AssertFalse(TokeniseText(&cFile, &cTokeniser, "int x = 12q;"));
	AssertSize(1, cTokeniser.GetErrors()->NumErrors());
	AssertString("Tokens.cpp (1, 9): Invalid integer literal suffix. [12q]\n", cTokeniser.GetErrors()->Text());
	AssertInt(3, cTokeniser.GetErrors()->GetToken(0));
	AssertInt(0, cTokeniser.GetErrors()->GetLine(0));
	AssertInt(8, cTokeniser.GetErrors()->GetColumn(0));
	cTokeniser.Kill();
	cFile.Kill();

	AssertFalse(TokeniseText(&cFile, &cTokeniser, "int x = 0x1FFFFFFFFFFFFFFFF;"));
	AssertString("Tokens.cpp (1, 9): Integer literal is too large. [0x1FFFFFFFFFFFFFFFF]\n", cTokeniser.GetErrors()->Text());
	cTokeniser.Kill();
	cFile.Kill();

	AssertFalse(TokeniseText(&cFile, &cTokeniser, "char c = '';\nint y = 1; int z = 2q;"));
	AssertString("Tokens.cpp (1, 10): Empty character literal. ['']\nTokens.cpp (2, 20): Invalid integer literal suffix. [2q]\n", cTokeniser.GetErrors()->Text());
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
//
//////////////////////////////////////////////////////////////////////////
void TestCppTokeniser(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();

	TestCppTokeniserKeywordsAndPunctuators();
	TestCppTokeniserIdentifiers();
	TestCppTokeniserIntegers();
	TestCppTokeniserFloatingAndCharacters();
	TestCppTokeniserLocations();
	TestCppTokeniserErrors();
	TestCppTokeniserIntegerLiterals();
	TestCppTokeniserIdentifierIndex();

	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

