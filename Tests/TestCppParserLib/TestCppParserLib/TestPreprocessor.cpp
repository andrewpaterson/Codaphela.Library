#include "BaseLib/PointerRemapper.h"
#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/FileUtil.h"
#include "CppParserLib/DirectiveMap.h"
#include "CppParserLib/Preprocessor.h"
#include "CppParserLib/HeaderFileMap.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorSimple(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("char sz;", &szDest);

	AssertString("char sz;\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#define XACT 3\n\
int x = XACT + 2;", &szDest);

	AssertString("int x = 3 + 2;\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorSimpleReplacement(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#define D5( X ) X \n\
#define D9(P,Q)  D5(P)  D5(Q) \n\
D9(Hello, World)", &szDest);

	AssertString("Hello World\n", szDest.Text());
	szDest.Kill();
}


#define D8(P,Q) P " " Q

D8(char sz[] =, ;)

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorStringReplacement(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#define D8(P,Q) P \" \" Q\n\
D8(char sz[] =,;)\
", &szDest);

	//Actually I don't know what this is supposed to look like.  Just make the test pass for now.
	AssertString("char sz[] = \" \" ;\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorReplacement(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#define D3(X) #X\n\
#define D7(P,Q) D3(P) D3(Q)\n\
#define D4() " "\n\
#define D6(P,Q) D3(P)##D4()##D3(Q)\n\
#define D8(P,Q) D3(P) " " D3(Q)\n\
#define D5(X) X\n\
#define D9(P,Q) D5(P) D4() D5(Q)\n\
#define D2(P,Q) D9(#P, #Q)\n\
#define D1(P,Q) #P#Q\n\
\n\
void DoStuff()\n\
{\n\
	//You can only legally have hashes in the replacement text.  Bizzare things happen otherwise.\n\
	//You can only have a single hash preceding a replacement argument.\n\
	//Tokens after the hash are not expanded before hashing.\n\
	char sz6[]=D9(\"Hello\", \"World\");\n\
	char sz1[] = D7(Hello, World);\n\
	char sz3[] = D8(Hello, World);\n\
	char sz5[] = D1(Hello, World);\n\
	char sz4[] = D2(Hello, World);\n\
	char sz2[] = D6(Hello, World);\n\
}\n\
", &szDest);

	//D4 and the " " in D8 are empty here: the quotes end and restart the C++ string literal this test is written in.
	//D6 pastes ")" with "D4", which is not a valid token.  That is undefined; the tokens are kept apart, as MSVC does.
	AssertString("\
void DoStuff()\n\
{\n\
char sz6[]=\"Hello\" \"World\";\n\
char sz1[] = \"Hello\" \"World\";\n\
char sz3[] = \"Hello\" \"World\";\n\
char sz5[] = \"Hello\"\"World\";\n\
char sz4[] = \"Hello\" \"World\";\n\
char sz2[] = \"Hello\"\"World\";\n\
}\n\
", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
int x = ++y / z;\n\
", &szDest);

	AssertString("int x = ++y / z;\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#define SOME_FOO_1(ARG1, ARG2)          0xDE + ARG1 - ARG2 +ARG1 //  \'\'\"\' for US\n\
#define SOME_FOO_2        0xE2  //  \"<>\" or \"\\|\" on RT 102-key kbd.\n\
#define SOME_FOO_3         0x00000080L /* Don't do \"&\" character translation */\n\
int dufi = SOME_FOO_1(1, 2), dasf = SOME_FOO_2, ewqr = SOME_FOO_3;\n\
", &szDest);

	AssertString("int dufi = 0xDE + 1 - 2 +1, dasf = 0xE2, ewqr = 0x00000080L;\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
char gsz[] = (\"//@  Don't Collapse me please     ! /*     \");\n\
char g_Char = \'\\n\';\n\
char g_Char2 = \'\"\';\n\
", &szDest);

	//Literals keep their spelling.
	AssertString("char gsz[] = (\"//@  Don't Collapse me please     ! /*     \");\n\
char g_Char = \'\\n\';\n\
char g_Char2 = \'\"\';\n\
", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
# define __nothrow __declspec(nothrow)\n\
__nothrow;\n\
", &szDest);

	AssertString("__declspec(nothrow);\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorExactDefine(void)
{
		CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#define floa	int giJoe;\n\
float	gFriend;\n\
", &szDest);

	AssertString("float gFriend;\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorConditionals(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 1\n\
Passed\n\
#else\n\
Failed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 0\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 1\n\
#define Fred Quintin\n\
#else\n\
#define Larry Bird\n\
#endif\n\
#define Dick Moby\n\
Fred\n\
Larry\n\
Dick\n\
", &szDest);
	AssertString("Quintin\nLarry\nMoby\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorOperatorPrecedence(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 1 || 0 && 0\n\
Passed\n\
#else\n\
Failed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 0 && 1 || 0\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 0 && 0 | 1\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 1 | 0 ^ 1\n\
Passed\n\
#else\n\
Failed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 1 ^ 0 & 0\n\
Passed\n\
#else\n\
Failed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 0 & 0 == 0\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 0 == 0 < 0\n\
Passed\n\
#else\n\
Failed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 1 < 1 << 1\n\
Passed\n\
#else\n\
Failed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 0 << 0 + 1\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if 1 + 0 * 0\n\
Passed\n\
#else\n\
Failed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if ~1 * 0\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("\
#if -(1 - 1)\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
CPreprocessor::Preprocess("\
#if +(0 + 0)\n\
Failed\n\
#else\n\
Passed\n\
#endif\n\
", &szDest);
	AssertString("Passed\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorBlockSkipping(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CListLibraries		cLibraries;
	CConfig				cConfig;
	CChars				szName;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
					   Start\n\
					   #if 1\n\
					   #define FOO\n\
					   #undef FOO\n\
					   #endif\n\
					   Passed\n\
					   #define FOO\n\
					   End\n\
					   ");

	cLibraries.Init();
	cConfig.Init("DEBUG");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("Start\nPassed\nEnd\n", szDest.Text());

	cPreprocessor.Kill();
	szDest.Kill();
	cLibraries.Kill();
	cConfig.Kill();
	cFile.Kill();

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
					  Start\n\
					  #if 0\n\
					  #define FOO\n\
					  #undef FOO\n\
					  #endif\n\
					  Passed\n\
					  #define FOO\n\
					  End\n\
					  Test\n\
					  ");
	
	cLibraries.Init();
	cConfig.Init("DEBUG");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("Start\nPassed\nEnd\nTest\n", szDest.Text());

	cPreprocessor.Kill();
	szDest.Kill();
	cLibraries.Kill();
	cConfig.Kill();
	cFile.Kill();

	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorParentheses(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#if (defined _M_ARM || defined _M_HYBRID_X86_ARM64) && !defined _M_CEE_PURE\n\
Unexpected\n\
#define _VA_ALIGN       4\n\
#define _SLOTSIZEOF(t)  ((sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1))\n\
#define _APALIGN(t,ap)  (((va_list)0 - (ap)) & (__alignof(t) - 1))\n\
#elif (defined _M_ARM64 || defined _M_ARM64EC) && !defined _M_CEE_PURE\n\
Unexpected\n\
#define _VA_ALIGN       8\n\
#define _SLOTSIZEOF(t)  ((sizeof(t) + _VA_ALIGN - 1) & ~(_VA_ALIGN - 1))\n\
#define _APALIGN(t,ap)  (((va_list)0 - (ap)) & (__alignof(t) - 1))\n\
#else\n\
Expected\n\
#define _SLOTSIZEOF(t)  (sizeof(t))\n\
#define _APALIGN(t,ap)  (__alignof(t))\n\
#endif\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("Expected\n", szDest.Text());

	cPreprocessor.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorSimpleVariadic(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#define _CRT_UNPARENTHESIZE_(FIRST, ARGS ...) ARGS\n\
Expected\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("Expected\n", szDest.Text());

	cPreprocessor.Kill();
	szName.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorComplexVariadic(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#define CHECK1(x, a...) if (!(x)) { printf(a); }\n\
CHECK1(0, \"here % s % s % s\", \"are\", \"some\", \"varargs(1)\\n\");\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("if (!(0)) { printf(\"here % s % s % s\", \"are\", \"some\", \"varargs(1)\\n\"); };\n", szDest.Text());

	cPreprocessor.Kill();
	szName.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorRedefinedVariadic(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#define _CRT_UNPARENTHESIZE_(...) __VA_ARGS__\n\
Expected1\n\
#define _CRT_UNPARENTHESIZE(...)  _CRT_UNPARENTHESIZE_ __VA_ARGS__\n\
Expected2\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("Expected1\nExpected2\n", szDest.Text());

	cPreprocessor.Kill();
	szName.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHasInclude(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;
	CHeaderFileMap		cHeaderFiles;
	CHeaderFiles		cHeaderNames;
	CFileUtil			cFileUtil;
	CChars				szDirectory;

	szName.Init("None.cpp");

	szDirectory.Init();
	cFileUtil.CurrentDirectory(&szDirectory);
	cFileUtil.AppendToPath(&szDirectory, "Input");


	cHeaderFiles.Init();
	cHeaderNames.Init(szDirectory.Text(), &cHeaderFiles, false, true);

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#if defined __has_include\n\
#  if __has_include (<Header.h>)\n\
IncludeWasHadded\n\
#  else\n\
Nope\n\
#  endif\n\
#endif\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.AddIncludeDirectory(&cHeaderNames);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("IncludeWasHadded\n", szDest.Text());

	cPreprocessor.Kill();
	cHeaderFiles.Kill();
	cHeaderNames.Kill();

	szName.Kill();
	szDirectory.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorDefineEvaluateToZero(void)
{
	CChars				szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#if NOT_DEFINED > 1\n\
Should not get here\n\
#else\n\
Yuppers, here is good\n\
#endif\n\
", &szDest);

	AssertString("Yuppers, here is good\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorExistingEmpty(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#if EMPTY > 1997L\n\
Should not get here\n\
#else\n\
Evaluate Good!\n\
#endif\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.AddDefine("EMPTY");
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("Evaluate Good!\n", szDest.Text());

	cPreprocessor.Kill();
	szName.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorRedefinedEmpty(void)
{
	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;
	CDefine*			pcDefine;
	CPPText*			pcText;
	CChars				szText;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#if defined(EMPTY)\n\
#define REEMPTY EMPTY\n\
#endif\n\
#if REEMPTY > 1996\n\
#define _HAS_X 1\n\
#define _HAS_Y 1\n\
#elif REEMPTY > 1\n\
#define _HAS_X 1\n\
#define _HAS_Y 0\n\
#else\n\
#define _HAS_X 0\n\
#define _HAS_Y 0\n\
#endif\n\
#undef REEMPTY\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.AddDefine("EMPTY");
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("", szDest.Text());

	pcDefine = cPreprocessor.GetDefine("_HAS_X", true);
	pcText = (CPPText*)pcDefine->GetReplacement()->Get(0);
	szText.Init();
	AssertString("0", pcText->Print(&szText));
	szText.Kill();

	pcDefine = cPreprocessor.GetDefine("_HAS_Y", true);
	pcText = (CPPText*)pcDefine->GetReplacement()->Get(0);
	szText.Init();
	AssertString("0", pcText->Print(&szText));
	szText.Kill();

	cPreprocessor.Kill();
	szName.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();


}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorSimpleEmpty(void)
{


	CChars				szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#if EMPTY(1)\n\
Nope, not good.\n\
#else\n\
Good, good.\n\
#endif\
", &szDest);

	AssertString("Good, good.\n", szDest.Text());
	szDest.Kill();


}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorBrackettedEmpty(void)
{

	CChars				szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#if FUNC(X_TOKE | Y_TOKE)\n\
Nope, not good.\n\
#else\n\
Good, good.\n\
#endif\
", &szDest);

	AssertString("Good, good.\n", szDest.Text());
	szDest.Kill();


}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorDefineBrackettedOmmitted(void)
{

	CChars				szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#define F(X) \n\
F Surfe();\n\
", &szDest);

	//A function-like macro name that is not followed by ( is not replaced.  [cpp.replace] 10
	AssertString("F Surfe();\n", szDest.Text());
	szDest.Kill();


}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHashedArgumentExpansion()
{

	CChars				szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#define D3(X) #X\n\
#define F1(X) X##__##X\n\
char	gszF1[] = D3(F1(H));\
", &szDest);

	//The argument of # is not macro replaced.  [cpp.subst] 1
	AssertString("char gszF1[] = \"F1(H)\";\n", szDest.Text());
	szDest.Kill();


}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHasCPPNameSpaceAttribute()
{


	CChars				szDest;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CConfig				cConfig;
	CChars				szName;
	CDefine* pcDefine;

	szName.Init("None.cpp");

	cFile.Init(szName.Text(), NULL, false);
	cFile.SetContents("\
#ifndef __has_cpp_attribute\n\
#define _KNOWN_SEMANTICS_1\n\
#elif defined(CUDA)\n\
#define _KNOWN_SEMANTICS_2\n\
#elif __has_cpp_attribute(name_space::known_semantics)\n\
#define _KNOWN_SEMANTICS_3 [[msvc::known_semantics]]\n\
#else\n\
#define _KNOWN_SEMANTICS_4\n\
#endif\n\
");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	cPreprocessor.PreprocessTranslationUnit(&cFile);
	szDest.Init();
	cFile.Print(&szDest);

	AssertString("", szDest.Text());

	//__has_cpp_attribute counts as defined, as it does for MSVC and g++ (and C++20 [cpp.cond]), but knows no attributes.
	pcDefine = cPreprocessor.GetDefine("_KNOWN_SEMANTICS_1", true);
	AssertNull(pcDefine);
	pcDefine = cPreprocessor.GetDefine("_KNOWN_SEMANTICS_2", true);
	AssertNull(pcDefine);
	pcDefine = cPreprocessor.GetDefine("_KNOWN_SEMANTICS_3", true);
	AssertNull(pcDefine);
	pcDefine = cPreprocessor.GetDefine("_KNOWN_SEMANTICS_4", true);
	AssertNotNull(pcDefine);

	cPreprocessor.Kill();
	szName.Kill();
	szDest.Kill();
	cConfig.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorNegativeExpression()
{
	CChars				szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#if (1 == -1)\n\
You broke maths\n\
#else\n\
Maths is still okay\n\
#endif\n\
", &szDest);

	AssertString("Maths is still okay\n", szDest.Text());
	szDest.Kill();


}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorStandardExample3(void)
{
	CChars	szDest;

	//C++14 [cpp.scope] / C11 6.10.3.5 EXAMPLE 3: rescanning and nested replacement.
	szDest.Init();
	CPreprocessor::Preprocess(
		"#define x 3\n"
		"#define f(a) f(x * (a))\n"
		"#undef x\n"
		"#define x 2\n"
		"#define g f\n"
		"#define z z[0]\n"
		"#define h g(~\n"
		"#define m(a) a(w)\n"
		"#define w 0,1\n"
		"#define t(a) a\n"
		"#define p() int\n"
		"#define q(x) x\n"
		"#define r(x,y) x ## y\n"
		"#define str(x) # x\n"
		"f(y+1) + f(f(z)) % t(t(g)(0) + t)(1);\n"
		"g(x+(3,4)-w) | h 5) & m\n"
		"(f)^m(m);\n"
		"p() i[q()] = { q(1), r(2,3), r(4,), r(,5), r(,) };\n"
		"char c[2][6] = { str(hello), str() };\n", &szDest);

	AssertString(
		"f(2 * (y+1)) + f(2 * (f(2 * (z[0])))) % f(2 * (0)) + t(1);\n"
		"f(2 * (2+(3,4)-0,1)) | f(2 * (~ 5)) & f(2 * (0,1))^m(0,1);\n"
		"int i[] = { 1, 23, 4, 5, };\n"
		"char c[2][6] = { \"hello\", \"\" };\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorStandardExample4(void)
{
	CChars	szDest;

	//C11 6.10.3.5 EXAMPLE 4: # and ##.
	szDest.Init();
	CPreprocessor::Preprocess(
		"#define str(s) # s\n"
		"#define xstr(s) str(s)\n"
		"#define debug(s, t) printf(\"x\" # s \"= %d, x\" # t \"= %s\", \\\n"
		" x ## s, x ## t)\n"
		"#define INCFILE(n) vers ## n\n"
		"#define glue(a, b) a ## b\n"
		"#define xglue(a, b) glue(a, b)\n"
		"#define HIGHLOW \"hello\"\n"
		"#define LOW LOW \", world\"\n"
		"debug(1, 2);\n"
		"fputs(str(strncmp(\"abc\\0d\", \"abc\", '\\4') // this goes away\n"
		" == 0) str(: @\\n), s);\n"
		"xstr(INCFILE(2).h)\n"
		"glue(HIGH, LOW);\n"
		"xglue(HIGH, LOW)\n", &szDest);

	AssertString(
		"printf(\"x\" \"1\" \"= %d, x\" \"2\" \"= %s\", x1, x2);\n"
		"fputs(\"strncmp(\\\"abc\\\\0d\\\", \\\"abc\\\", '\\\\4') == 0\" \": @\\n\", s);\n"
		"\"vers2.h\"\n"
		"\"hello\";\n"
		"\"hello\" \", world\"\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorStandardExample5(void)
{
	CChars	szDest;

	//C11 6.10.3.5 EXAMPLE 5: placemarkers.
	szDest.Init();
	CPreprocessor::Preprocess(
		"#define t(x,y,z) x ## y ## z\n"
		"int j[] = { t(1,2,3), t(,4,5), t(6,,7), t(8,9,),\n"
		" t(10,,), t(,11,), t(,,12), t(,,) };\n", &szDest);

	AssertString(
		"int j[] = { 123, 45, 67, 89,\n"
		"10, 11, 12, };\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorStandardExample7(void)
{
	CChars	szDest;

	//C11 6.10.3.5 EXAMPLE 7: variadic macros, and the GNU , ## __VA_ARGS__.
	szDest.Init();
	CPreprocessor::Preprocess(
		"#define debug(...) fprintf(stderr, __VA_ARGS__)\n"
		"#define showlist(...) puts(#__VA_ARGS__)\n"
		"#define report(test, ...) ((test)?puts(#test):\\\n"
		" printf(__VA_ARGS__))\n"
		"debug(\"Flag\");\n"
		"debug(\"X = %d\\n\", x);\n"
		"showlist(The first, second, and third items.);\n"
		"report(x>y, \"x is %d but y is %d\", x, y);\n"
		"#define F(...) f(0 __VA_ARGS__)\n"
		"#define G(X, ...) f(0, X __VA_ARGS__)\n"
		"#define SDEF(sname, ...) S sname = { __VA_ARGS__ }\n"
		"#define EMP\n"
		"F(a,b,c) F() F(EMP) G(a,b,c) G(a,) G(a)\n"
		"SDEF(foo); SDEF(bar, 1, 2);\n"
		"#define GNU(fmt, ...) printf(fmt, ## __VA_ARGS__)\n"
		"GNU(\"a\") GNU(\"b\", 1, 2)\n", &szDest);

	AssertString(
		"fprintf(stderr, \"Flag\");\n"
		"fprintf(stderr, \"X = %d\\n\", x);\n"
		"puts(\"The first, second, and third items.\");\n"
		"((x>y)?puts(\"x>y\"): printf(\"x is %d but y is %d\", x, y));\n"
		"f(0 a,b,c) f(0) f(0) f(0, a b,c) f(0, a) f(0, a)\n"
		"S foo = { }; S bar = { 1, 2 };\n"
		"printf(\"a\") printf(\"b\", 1, 2)\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorConditionalArithmetic(void)
{
	CChars	szDest;

	//#if is evaluated in intmax_t and uintmax_t.  [cpp.cond]
	szDest.Init();
	CPreprocessor::Preprocess(
		"#define V 0x700\n"
		"#if (V >= 0x1400) || (V >= 0xE00 && V < 0x1000)\n"
		"bad1\n"
		"#endif\n"
		"#if 0xE00 == 3584 && 0x700 == 1792 && 1 < 0x1000\n"
		"good1\n"
		"#endif\n"
		"#if 201402L >= 201103L && 0xFFFFFFFFUL > 5u && 1'000 == 1000\n"
		"good2\n"
		"#endif\n"
		"#if -1 < 0 && (1 << 4) == 16 && 'A' == 65 && 7 / 2 == 3 && true && !false\n"
		"good3\n"
		"#endif\n"
		"#if -1 > 0u\n"
		"good4\n"
		"#endif\n"
		"#if 1 ? 2 : 0\n"
		"good5\n"
		"#endif\n"
		"#if 0 && (1 / 0)\n"
		"bad2\n"
		"#endif\n", &szDest);

	AssertString("good1\ngood2\ngood3\ngood4\ngood5\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Universal character names are escape sequences in #if character
// literals too.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorConditionalUniversalCharacterNames(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess(
		"#if '\\u0041' == 65 && '\\U00000042' == 66 && u'\\u00E9' == 0xE9\n"
		"good\n"
		"#endif\n", &szDest);

	AssertString("good\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// #if takes the same integer suffixes as the C++ tokeniser, Microsoft's
// i64 and friends included.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorConditionalIntegerSuffixes(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess(
		"#if 7i64 == 7 && 0x10ui64 == 16 && 3i32 == 3 && 2i8 == 2 && 5LLU == 5\n"
		"good\n"
		"#endif\n", &szDest);

	AssertString("good\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorSkippedConditionals(void)
{
	CChars	szDest;

	//Conditionals in skipped groups are not evaluated, and unknown directives in them are ignored.  [cpp.cond] 6
	szDest.Init();
	CPreprocessor::Preprocess(
		"#ifdef UNDEFINED\n"
		"#if NOT_A_MACRO (1) +\n"
		"#warning not here\n"
		"#unknown_directive\n"
		"#elif 1 / 0\n"
		"bad\n"
		"#endif\n"
		"#else\n"
		"good\n"
		"#endif\n"
		"#if 1\n"
		"first\n"
		"#elif 1 / 0\n"
		"bad\n"
		"#else\n"
		"bad\n"
		"#endif\n", &szDest);

	AssertString("good\nfirst\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorNumbersAndPunctuators(void)
{
	CChars	szDest;

	//pp-numbers include letters, '.', exponent signs and digit separators.  [lex.ppnumber]
	szDest.Init();
	CPreprocessor::Preprocess(
		"#define CAT(a, b) a ## b\n"
		"int a = 0x10'0000 + 201402L + .5f + 1.5e+3 + 0x1p-3 + 0b1010;\n"
		"a<<=b->*c; x = y ... z; CAT(<, <=) CAT(-, >) CAT(x, 1) CAT(1, e) CAT(L, \"s\")\n"
		"char c = L'x'; const char* s = u8\"y\";\n", &szDest);

	AssertString(
		"int a = 0x10'0000 + 201402L + .5f + 1.5e+3 + 0x1p-3 + 0b1010;\n"
		"a<<=b->*c; x = y ... z; <<= -> x1 1e L\"s\"\n"
		"char c = L'x'; const char* s = u8\"y\";\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorMultiLineArguments(void)
{
	CChars	szDest;

	//Macro arguments can cover lines, and directives in them are processed (as g++ and MSVC do).
	szDest.Init();
	CPreprocessor::Preprocess(
		"#define F(a, b) [a|b]\n"
		"#define G F\n"
		"x F(1,\n"
		"    2) y\n"
		"G\n"
		"(3,\n"
		"#define FOUR 4\n"
		"FOUR) FOUR\n"
		"F", &szDest);

	AssertString(
		"x [1|2] y\n"
		"[3|4] 4\n"
		"F\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorLocations(void)
{
	CTranslationUnit		cFile;
	CPreprocessor			cPreprocessor;
	CConfig					cConfig;
	CPreprocessedTokens*	pcTokens;
	SPreprocessedToken*		psToken;
	SMacroExpansion*		psExpansion;
	SInactiveRegion*		psRegion;
	CArrayPPComments*		pacComments;
	CChars					szDest;

	cFile.Init("Locations.cpp", NULL, false);
	cFile.SetContents(
		"#define TWO 2  // two\n"
		"int a = TWO;\n"
		"#if 0\n"
		"skipped\n"
		"#else\n"
		"/* c */ int b;\n"
		"#endif\n"
		"#define ADD(x, y) x + y\n"
		"int c = ADD(a,\n"
		"            TWO);\n");

	cConfig.Init("");
	cPreprocessor.Init(&cConfig);
	AssertTrue(cPreprocessor.PreprocessTranslationUnit(&cFile));

	szDest.Init();
	cFile.Print(&szDest);
	AssertString("int a = 2;\nint b;\nint c = a + 2;\n", szDest.Text());
	szDest.Kill();

	pcTokens = cFile.GetTokens();
	AssertSize(15, pcTokens->NumTokens());

	//Tokens from the source are where they were written.
	psToken = pcTokens->Get(1);
	AssertTrue(psToken->Is("a"));
	AssertPointer(&cFile, psToken->pcFile);
	AssertInt(1, psToken->iLine);
	AssertInt(4, psToken->iColumn);
	AssertInt(-1, psToken->iExpansion);
	AssertInt(PTF_LEADING_SPACE, psToken->uiFlags);

	//Tokens from a macro body are where the #define wrote them, and refer to the expansion that made them.
	psToken = pcTokens->Get(3);
	AssertTrue(psToken->Is("2"));
	AssertInt(0, psToken->iLine);
	AssertInt(12, psToken->iColumn);
	AssertInt(0, psToken->iExpansion);
	psExpansion = pcTokens->GetExpansion(0);
	AssertInt(3, psExpansion->uiNameLength);
	AssertInt(1, psExpansion->iLine);
	AssertInt(8, psExpansion->iColumn);
	AssertInt(-1, psExpansion->iParent);

	psToken = pcTokens->Get(5);
	AssertTrue(psToken->Is("int"));
	AssertInt(PTF_START_OF_LINE, psToken->uiFlags);
	AssertInt(5, psToken->iLine);
	AssertInt(8, psToken->iColumn);

	//Arguments keep their own locations.
	psToken = pcTokens->Get(11);
	AssertTrue(psToken->Is("a"));
	AssertInt(8, psToken->iLine);
	AssertInt(12, psToken->iColumn);
	AssertInt(-1, psToken->iExpansion);
	psToken = pcTokens->Get(12);
	AssertTrue(psToken->Is("+"));
	AssertInt(7, psToken->iLine);
	AssertInt(20, psToken->iColumn);
	AssertInt(1, psToken->iExpansion);
	psToken = pcTokens->Get(13);
	AssertTrue(psToken->Is("2"));
	AssertInt(2, psToken->iExpansion);
	psExpansion = pcTokens->GetExpansion(2);
	AssertInt(9, psExpansion->iLine);
	AssertInt(12, psExpansion->iColumn);

	//Only the lines skipped by #if 0.
	AssertSize(1, pcTokens->NumInactiveRegions());
	psRegion = pcTokens->GetInactiveRegion(0);
	AssertInt(3, psRegion->iStartLine);
	AssertInt(3, psRegion->iEndLine);

	pacComments = cFile.GetComments();
	AssertSize(2, pacComments->NumElements());
	AssertInt(15, pacComments->Get(0)->iStart);
	AssertInt(6, pacComments->Get(0)->iLength);
	AssertInt(55, pacComments->Get(1)->iStart);
	AssertInt(7, pacComments->Get(1)->iLength);

	cPreprocessor.Kill();
	cConfig.Kill();
	cFile.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// The preprocessor's errors for szSource, one a line.
//
//////////////////////////////////////////////////////////////////////////
static bool PreprocessErrors(char* szSource, CChars* pszErrors, CChars* pszWarnings = NULL)
{
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	bool				bResult;

	cFile.Init("Errors.cpp", NULL, false);
	cFile.SetContents(szSource);
	cPreprocessor.Init(NULL);
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	pszErrors->Append(cPreprocessor.GetErrors()->Text());
	if (pszWarnings)
	{
		pszWarnings->Append(cPreprocessor.GetWarnings()->Text());
	}
	cPreprocessor.Kill();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorErrors(void)
{
	CChars	szErrors;
	CChars	szWarnings;

	szErrors.Init();
	AssertTrue(PreprocessErrors("int x;\n", &szErrors));
	AssertString("", szErrors.Text());
	szErrors.Kill();

	szErrors.Init();
	PreprocessErrors("int x;\n#frobnicate 3\n", &szErrors);
	AssertString("Errors.cpp (2, 2): Unknown directive #frobnicate 3\n", szErrors.Text());
	szErrors.Kill();

	szErrors.Init();
	PreprocessErrors("#endif\n", &szErrors);
	AssertString("Errors.cpp (1, 7): #elif, #else or #endif without #if.\n", szErrors.Text());
	szErrors.Kill();

	szErrors.Init();
	PreprocessErrors("#include \"Missing.h\"\n", &szErrors);
	AssertString("Errors.cpp (1, 10): Could not include file \"Missing.h\"\n", szErrors.Text());
	szErrors.Kill();

	szErrors.Init();
	PreprocessErrors("#if 1\nint x;\n", &szErrors);
	AssertString("Errors.cpp: Unterminated conditional directive.\n", szErrors.Text());
	szErrors.Kill();

	//#error is an error in the list; #warning is a warning, and preprocessing goes on.
	szErrors.Init();
	AssertFalse(PreprocessErrors("int x;\n#error Not this way\n", &szErrors));
	AssertString("Errors.cpp (2, 7): #error Not this way\n", szErrors.Text());
	szErrors.Kill();
	szErrors.Init();
	szWarnings.Init();
	AssertTrue(PreprocessErrors("int x;\n#warning Look out\nint y;\n", &szErrors, &szWarnings));
	AssertString("", szErrors.Text());
	AssertString("Errors.cpp (2, 9): #warning Look out\n", szWarnings.Text());
	szWarnings.Kill();
	szErrors.Kill();
	szErrors.Init();
	AssertTrue(PreprocessErrors("#if 0\n#error Skipped\n#warning Skipped\n#endif\n", &szErrors));
	AssertString("", szErrors.Text());
	szErrors.Kill();

	//A conditional has one #else, after every #elif.
	szErrors.Init();
	PreprocessErrors("#if 0\n#else\n#else\n#endif\n", &szErrors);
	AssertString("Errors.cpp (3, 6): #elif or #else after #else.\n", szErrors.Text());
	szErrors.Kill();
	szErrors.Init();
	PreprocessErrors("#if 0\n#else\n#elif 1\n#endif\n", &szErrors);
	AssertString("Errors.cpp (3, 6): #elif or #else after #else.\n", szErrors.Text());
	szErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A directive removed from the map is gone; the others are still there.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorDirectiveMap(void)
{
	CDirectiveMap	cMap;

	cMap.Init();
	cMap.AddDirective("define", PPD_define);
	cMap.AddDirective("undef", PPD_undef);
	AssertNotNull(cMap.GetDirective("define", true));
	cMap.RemoveDirective("define");
	AssertNull(cMap.GetDirective("define", true));
	AssertNotNull(cMap.GetDirective("undef", true));
	cMap.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A backslash and a new line join two lines anywhere, even in the middle
// of an identifier or a string.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorLineContinuation(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("int ab\\\ncd = 1;", &szDest);
	AssertString("int abcd = 1;\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	CPreprocessor::Preprocess("char* sz = \"x\\\ny\";", &szDest);
	AssertString("char* sz = \"xy\";\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// szSource preprocessed as szFileName with pcConfig (which can be NULL),
// searching the include directories pcFirst then pcSecond (either can be
// NULL).  The include log goes in pszLog if it is not NULL.
//
//////////////////////////////////////////////////////////////////////////
static bool PreprocessFile(char* szFileName, char* szSource, CConfig* pcConfig, CHeaderFiles* pcFirst, CHeaderFiles* pcSecond, CChars* pszDest, CChars* pszLog)
{
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	bool				bResult;

	cFile.Init(szFileName, NULL, false);
	cFile.SetContents(szSource);
	cPreprocessor.Init(pcConfig);
	cPreprocessor.LogIncludes(pszLog);
	if (pcFirst)
	{
		cPreprocessor.AddIncludeDirectory(pcFirst);
	}
	if (pcSecond)
	{
		cPreprocessor.AddIncludeDirectory(pcSecond);
	}
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	cFile.Print(pszDest);
	cPreprocessor.Kill();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// __LINE__ is the line of the outermost macro name written in the source,
// __FILE__ the translation unit's name and __COUNTER__ counts up from 0.
// __cplusplus is C++14.  [cpp.predefined]
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorPredefinedMacros(void)
{
	CChars	szDest;
	char*	sz;

	szDest.Init();
	AssertTrue(PreprocessFile("Predefined.cpp", "\
int a = __LINE__;\n\
\n\
#define L __LINE__\n\
#define M(x) x + L\n\
int b = L;\n\
int c = M(\n\
\n\
1);\n\
int d = __LINE__;\n", NULL, NULL, NULL, &szDest, NULL));
	AssertString("\
int a = 1;\n\
int b = 5;\n\
int c = 1 + 6;\n\
int d = 9;\n", szDest.Text());
	szDest.Kill();

	szDest.Init();
	AssertTrue(PreprocessFile("Predefined.cpp", "\
char* f = __FILE__;\n\
int c0 = __COUNTER__;\n\
int c1 = __COUNTER__;\n\
long v = __cplusplus;\n\
int h = __STDC_HOSTED__;\n", NULL, NULL, NULL, &szDest, NULL));
	AssertString("\
char* f = \"Predefined.cpp\";\n\
int c0 = 0;\n\
int c1 = 1;\n\
long v = 201402L;\n\
int h = 1;\n", szDest.Text());
	szDest.Kill();

	//__DATE__ is "Mmm dd yyyy" and __TIME__ is "hh:mm:ss".
	szDest.Init();
	AssertTrue(PreprocessFile("Predefined.cpp", "__DATE__ __TIME__", NULL, NULL, NULL, &szDest, NULL));
	sz = szDest.Text();
	AssertInt(25, (int)szDest.Length());
	AssertChar('"', sz[0]);
	AssertTrue((strncmp(&sz[1], "Jan", 3) == 0) || (strncmp(&sz[1], "Feb", 3) == 0) || (strncmp(&sz[1], "Mar", 3) == 0) || (strncmp(&sz[1], "Apr", 3) == 0) ||
			   (strncmp(&sz[1], "May", 3) == 0) || (strncmp(&sz[1], "Jun", 3) == 0) || (strncmp(&sz[1], "Jul", 3) == 0) || (strncmp(&sz[1], "Aug", 3) == 0) ||
			   (strncmp(&sz[1], "Sep", 3) == 0) || (strncmp(&sz[1], "Oct", 3) == 0) || (strncmp(&sz[1], "Nov", 3) == 0) || (strncmp(&sz[1], "Dec", 3) == 0));
	AssertChar(' ', sz[4]);
	AssertTrue((sz[5] == ' ') || ((sz[5] >= '1') && (sz[5] <= '3')));
	AssertTrue((sz[6] >= '0') && (sz[6] <= '9'));
	AssertChar(' ', sz[7]);
	AssertTrue((sz[8] >= '1') && (sz[8] <= '9'));
	AssertTrue((sz[9] >= '0') && (sz[9] <= '9'));
	AssertTrue((sz[10] >= '0') && (sz[10] <= '9'));
	AssertTrue((sz[11] >= '0') && (sz[11] <= '9'));
	AssertChar('"', sz[12]);
	AssertChar(' ', sz[13]);
	AssertChar('"', sz[14]);
	AssertTrue((sz[15] >= '0') && (sz[15] <= '2'));
	AssertTrue((sz[16] >= '0') && (sz[16] <= '9'));
	AssertChar(':', sz[17]);
	AssertTrue((sz[18] >= '0') && (sz[18] <= '5'));
	AssertTrue((sz[19] >= '0') && (sz[19] <= '9'));
	AssertChar(':', sz[20]);
	AssertTrue((sz[21] >= '0') && (sz[21] <= '6'));
	AssertTrue((sz[22] >= '0') && (sz[22] <= '9'));
	AssertChar('"', sz[23]);
	AssertChar('\n', sz[24]);
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// __has_builtin, __has_feature, __has_extension and __has_attribute are
// defined, but know nothing: they are always 0.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorHasOperators(void)
{
	CChars	szDest;

	szDest.Init();
	CPreprocessor::Preprocess("\
#if __has_builtin(__builtin_expect)\n\
builtin\n\
#endif\n\
#if __has_feature(cxx_rvalue_references)\n\
feature\n\
#endif\n\
#if __has_extension(cxx_rvalue_references)\n\
extension\n\
#endif\n\
#if __has_attribute(noreturn)\n\
attribute\n\
#endif\n\
#if defined(__has_builtin) && defined(__has_feature) && defined __has_extension && defined(__has_attribute)\n\
defined\n\
#endif\n\
#if !__has_builtin(x) && !__has_feature(x) && !__has_attribute(x)\n\
none\n\
#endif\n", &szDest);
	AssertString("defined\nnone\n", szDest.Text());
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A configuration define is NAME or NAME=VALUE.  It replaces a predefined
// macro of the same name.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorConfigDefines(void)
{
	CConfig	cConfig;
	CChars	szDest;

	cConfig.Init("Release");
	cConfig.mbHostDefines = false;
	cConfig.AddDefines("WIDTH=16;FLAG;__cplusplus=201703L;EMPTY=");

	szDest.Init();
	AssertTrue(PreprocessFile("Config.cpp", "\
int w = WIDTH;\n\
#ifdef FLAG\n\
int f;\n\
#endif\n\
long v = __cplusplus;\n\
int e = EMPTY 1;\n", &cConfig, NULL, NULL, &szDest, NULL));
	AssertString("\
int w = 16;\n\
int f;\n\
long v = 201703L;\n\
int e = 1;\n", szDest.Text());
	szDest.Kill();

	cConfig.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A configuration define without a value is 1, as -D NAME and /D NAME
// are.  NAME= is empty.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorConfigDefineWithoutValue(void)
{
	CConfig	cConfig;
	CChars	szDest;

	cConfig.Init("Release");
	cConfig.mbHostDefines = false;
	cConfig.AddDefines("FLAG;EMPTY=");

	szDest.Init();
	AssertTrue(PreprocessFile("Config.cpp", "\
int f = FLAG;\n\
#if FLAG\n\
int g;\n\
#endif\n\
#if FLAG == 1\n\
int h;\n\
#endif\n\
int e = EMPTY 2;\n", &cConfig, NULL, NULL, &szDest, NULL));
	AssertString("\
int f = 1;\n\
int g;\n\
int h;\n\
int e = 2;\n", szDest.Text());
	szDest.Kill();

	cConfig.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Headers made in memory, in two include directories: Input/SFT (which has
// no headers on disk) and then Input (which has Header.h).
//
//////////////////////////////////////////////////////////////////////////
static void InitMemoryHeaders(CHeaderFileMap* pcHeaderFiles, CHeaderFiles* pcFirst, CHeaderFiles* pcSecond)
{
	CFileUtil	cFileUtil;
	CChars		szFirst;
	CChars		szSecond;

	szSecond.Init();
	cFileUtil.CurrentDirectory(&szSecond);
	cFileUtil.AppendToPath(&szSecond, "Input");
	szFirst.Init(szSecond);
	cFileUtil.AppendToPath(&szFirst, "SFT");

	pcHeaderFiles->Init();
	pcFirst->Init(szFirst.Text(), pcHeaderFiles, false, false);
	pcSecond->Init(szSecond.Text(), pcHeaderFiles, false, true);

	pcFirst->AddFile("Once.h")->SetContents("#pragma once\nint giOnce;\n");
	pcFirst->AddFile("Twice.h")->SetContents("int giTwice;\n");
	pcFirst->AddFile("Next.h")->SetContents("\
int giFirst;\n\
#if __has_include_next(<Next.h>)\n\
#include_next <Next.h>\n\
#endif\n\
#if __has_include_next(<Header.h>)\n\
int giHeaderIsNext;\n\
#endif\n\
#if __has_include_next(<Once.h>)\n\
int giOnceIsNext;\n\
#endif\n");
	pcSecond->AddFile("Next.h")->SetContents("\
int giSecond;\n\
#if __has_include_next(<Next.h>)\n\
int giThird;\n\
#endif\n");

	szFirst.Kill();
	szSecond.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void KillMemoryHeaders(CHeaderFileMap* pcHeaderFiles, CHeaderFiles* pcFirst, CHeaderFiles* pcSecond)
{
	pcFirst->Kill();
	pcSecond->Kill();
	pcHeaderFiles->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A header with #pragma once is only read the first time it is included;
// a header without it every time.  Both are logged every time.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorPragmaOnce(void)
{
	CHeaderFileMap	cHeaderFiles;
	CHeaderFiles	cFirst;
	CHeaderFiles	cSecond;
	CChars			szDest;
	CChars			szLog;

	InitMemoryHeaders(&cHeaderFiles, &cFirst, &cSecond);

	szDest.Init();
	szLog.Init();
	AssertTrue(PreprocessFile("Once.cpp", "\
#include \"Once.h\"\n\
#include \"Once.h\"\n\
#include <Twice.h>\n\
#include <Twice.h>\n\
int x;\n", NULL, &cFirst, &cSecond, &szDest, &szLog));
	AssertString("\
int giOnce;\n\
int giTwice;\n\
int giTwice;\n\
int x;\n", szDest.Text());
	AssertString("\
     Once.cpp (Included)\n\
         Once.h (Included)\n\
         Once.h (Included)\n\
         Twice.h (Included)\n\
         Twice.h (Included)\n", szLog.Text());
	szLog.Kill();
	szDest.Kill();

	KillMemoryHeaders(&cHeaderFiles, &cFirst, &cSecond);
}


//////////////////////////////////////////////////////////////////////////
//
// #include_next and __has_include_next search the include directories
// after the one the current header was found in.  In the source file
// they search them all, like #include.
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessorIncludeNext(void)
{
	CHeaderFileMap	cHeaderFiles;
	CHeaderFiles	cFirst;
	CHeaderFiles	cSecond;
	CChars			szDest;
	CChars			szLog;

	InitMemoryHeaders(&cHeaderFiles, &cFirst, &cSecond);

	szDest.Init();
	szLog.Init();
	AssertTrue(PreprocessFile("Next.cpp", "#include <Next.h>\nint x;\n", NULL, &cFirst, &cSecond, &szDest, &szLog));
	AssertString("\
int giFirst;\n\
int giSecond;\n\
int giHeaderIsNext;\n\
int x;\n", szDest.Text());
	AssertString("\
     Next.cpp (Included)\n\
         Next.h (Included)\n\
             Next.h (Included)\n", szLog.Text());
	szLog.Kill();
	szDest.Kill();

	szDest.Init();
	AssertTrue(PreprocessFile("Next.cpp", "#include_next <Next.h>\n#if __has_include_next(<Once.h>)\nint y;\n#endif\n", NULL, &cFirst, &cSecond, &szDest, NULL));
	AssertString("\
int giFirst;\n\
int giSecond;\n\
int giHeaderIsNext;\n\
int y;\n", szDest.Text());
	szDest.Kill();

	KillMemoryHeaders(&cHeaderFiles, &cFirst, &cSecond);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestPreprocessor(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();

	TestPreprocessorSimple();
	TestPreprocessorSimpleReplacement();
	TestPreprocessorStringReplacement();
	TestPreprocessorReplacement();
	TestPreprocessorExactDefine();
	TestPreprocessorConditionals();
	TestPreprocessorOperatorPrecedence();
	TestPreprocessorBlockSkipping();
	TestPreprocessorHasInclude();
	TestPreprocessorSimpleVariadic();
	TestPreprocessorComplexVariadic();
	TestPreprocessorRedefinedVariadic();
	TestPreprocessorDefineEvaluateToZero();
	TestPreprocessorSimpleEmpty();
	TestPreprocessorExistingEmpty();
	TestPreprocessorRedefinedEmpty();
	TestPreprocessorBrackettedEmpty();
	TestPreprocessorHashedArgumentExpansion();
	TestPreprocessorDefineBrackettedOmmitted();
	TestPreprocessorHasCPPNameSpaceAttribute();
	TestPreprocessorNegativeExpression();
	TestPreprocessorStandardExample3();
	TestPreprocessorStandardExample4();
	TestPreprocessorStandardExample5();
	TestPreprocessorStandardExample7();
	TestPreprocessorConditionalArithmetic();
	TestPreprocessorConditionalUniversalCharacterNames();
	TestPreprocessorConditionalIntegerSuffixes();
	TestPreprocessorSkippedConditionals();
	TestPreprocessorNumbersAndPunctuators();
	TestPreprocessorMultiLineArguments();
	TestPreprocessorLocations();
	TestPreprocessorErrors();
	TestPreprocessorLineContinuation();
	TestPreprocessorDirectiveMap();
	TestPreprocessorParentheses();
	TestPreprocessorPredefinedMacros();
	TestPreprocessorHasOperators();
	TestPreprocessorConfigDefines();
	TestPreprocessorConfigDefineWithoutValue();
	TestPreprocessorPragmaOnce();
	TestPreprocessorIncludeNext();

	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

