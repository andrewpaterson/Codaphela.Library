#include "BaseLib/TypeNames.h"
#include "BaseLib/TypeConverter.h"
#include "CppParserLib/Preprocessor.h"
#include "CppParserLib/CppTokeniser.h"
#include "CppParserLib/CppParser.h"
#include "CppParserLib/CPPHighlighter.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
//Preprocesses, parses and highlights szSource, and prints the highlights.
static char* HighlightText(char* szSource, CChars* psz)
{
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPPHighlighter		cHighlighter;

	cFile.Init("Highlighted.cpp", NULL, false);
	cFile.SetContents(szSource);
	cPreprocessor.Init(NULL);
	AssertTrue(cPreprocessor.PreprocessTranslationUnit(&cFile));
	cPreprocessor.Kill();
	cTokeniser.Init();
	AssertTrue(cTokeniser.Tokenise(cFile.GetTokens()));
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	cParser.Parse();
	cParser.Kill();

	cHighlighter.Init();
	cHighlighter.Highlight(&cFile, cFile.GetTokens(), &cTokeniser, &cTree);
	cHighlighter.Print(psz);
	cHighlighter.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppHighlighterSource(void)
{
	CChars	sz;

	sz.Init();
	AssertString("\
1:1 14 directive\n\
2:1 6 comment\n\
3:1 11 comment\n\
4:1 5 directive\n\
5:1 9 inactive\n\
6:1 6 directive\n\
7:1 5 keyword\n\
7:7 6 class declaration\n\
8:1 1 operator\n\
9:1 6 keyword\n\
9:7 1 operator\n\
10:2 3 keyword\n\
10:7 1 field declaration\n\
10:8 1 operator\n\
11:2 4 keyword\n\
11:7 4 function declaration\n\
11:11 1 operator\n\
11:12 3 keyword\n\
11:16 2 parameter declaration\n\
11:18 1 operator\n\
11:20 1 operator\n\
11:22 1 field\n\
11:24 2 operator\n\
11:27 2 parameter\n\
11:29 1 operator\n\
11:31 1 operator\n\
12:1 1 operator\n\
12:2 1 operator\n\
13:1 4 keyword\n\
13:6 7 enum declaration\n\
13:14 1 operator\n\
13:16 3 enumerator declaration\n\
13:20 1 operator\n\
13:21 1 operator\n\
14:1 3 keyword\n\
14:5 3 function declaration\n\
14:8 1 operator\n\
14:9 6 class\n\
14:15 1 operator\n\
14:17 1 parameter declaration\n\
14:18 1 operator\n\
14:20 3 keyword\n\
14:24 1 parameter declaration\n\
14:25 1 operator\n\
15:1 1 operator\n\
16:2 3 keyword\n\
16:6 1 variable declaration\n\
16:7 1 operator\n\
16:8 4 macro\n\
16:12 1 operator\n\
16:14 1 operator\n\
16:16 1 operator\n\
16:18 3 character\n\
16:22 1 operator\n\
16:23 1 operator\n\
17:2 2 keyword\n\
17:5 1 operator\n\
17:6 1 parameter\n\
17:7 1 operator\n\
17:9 4 keyword\n\
17:14 4 label\n\
17:18 1 operator\n\
18:2 1 parameter\n\
18:3 2 operator\n\
18:5 4 member\n\
18:9 1 operator\n\
18:10 1 parameter\n\
18:12 1 operator\n\
18:14 4 number\n\
18:18 1 operator\n\
18:19 1 operator\n\
18:22 8 comment\n\
19:1 4 label\n\
19:5 1 operator\n\
20:2 6 keyword\n\
20:9 1 variable\n\
20:10 1 operator\n\
20:11 1 number\n\
20:12 1 operator\n\
20:14 1 operator\n\
20:16 1 parameter\n\
20:17 2 operator\n\
20:19 1 member\n\
20:21 1 operator\n\
20:23 3 enumerator\n\
20:27 1 operator\n\
20:29 1 unresolved\n\
20:30 1 operator\n\
21:1 1 operator\n\
22:1 5 keyword\n\
22:7 4 keyword\n\
22:11 1 operator\n\
22:13 3 variable declaration\n\
22:17 1 operator\n\
22:19 6 string\n\
22:25 1 operator\n", HighlightText("\
#define SIZE 4\n\
/* Two\n\
   lines */\n\
#if 0\n\
int gone;\n\
#endif\n\
class CPoint\n\
{\n\
public:\n\
	int  x;\n\
	void Move(int dx) { x += dx; }\n\
};\n\
enum EColour { RED };\n\
int Sum(CPoint* p, int n)\n\
{\n\
	int a[SIZE] = { 'c' };\n\
	if (n) goto done;\n\
	p->Move(n + 0x10);  // Moved\n\
done:\n\
	return a[0] + p->x + RED + q;\n\
}\n\
const char* gsz = \"Text\";\n", &sz));
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppHighlighterSplices(void)
{
	CChars	sz;

	sz.Init();
	AssertString("\
1:1 28 directive\n\
2:1 3 keyword\n\
2:5 1 function declaration\n\
2:6 1 operator\n\
2:7 3 keyword\n\
2:11 1 parameter declaration\n\
2:12 1 operator\n\
2:14 1 operator\n\
2:16 6 keyword\n\
2:23 5 macro\n\
2:29 1 parameter\n\
2:31 1 operator\n\
2:33 1 operator\n\
3:1 5 keyword\n\
3:7 4 keyword\n\
3:11 1 operator\n\
3:13 3 variable declaration\n\
3:17 1 operator\n\
3:19 3 string\n\
4:1 2 string\n\
4:3 1 operator\n", HighlightText("\
#define TWICE(x) ((x) + (x))\n\
int f(int v) { return TWICE(v); }\n\
const char* gsz = \"a\\\nb\";\n", &sz));
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Namespaces, typedefs, type parameters, built-in types with identifier
// names and identifiers the parser does not look up (attribute names).
// override and final are keywords where they are specifiers, and names
// elsewhere.
//
//////////////////////////////////////////////////////////////////////////
void TestCppHighlighterNames(void)
{
	CChars	sz;

	sz.Init();
	AssertString("\
1:1 9 keyword\n\
1:11 2 namespace declaration\n\
1:14 1 operator\n\
1:16 7 keyword\n\
1:24 3 keyword\n\
1:28 3 typedef declaration\n\
1:31 1 operator\n\
1:33 1 operator\n\
2:1 8 keyword\n\
2:9 1 operator\n\
2:10 5 keyword\n\
2:16 1 type parameter declaration\n\
2:17 1 operator\n\
2:19 1 type parameter\n\
2:21 3 function declaration\n\
2:24 1 operator\n\
2:25 1 type parameter\n\
2:27 1 parameter declaration\n\
2:28 1 operator\n\
2:30 1 operator\n\
2:32 6 keyword\n\
2:39 1 parameter\n\
2:40 1 operator\n\
2:42 1 operator\n\
3:1 8 builtin type\n\
3:10 6 variable declaration\n\
3:16 1 operator\n\
4:1 2 namespace\n\
4:3 2 operator\n\
4:5 3 typedef\n\
4:9 7 variable declaration\n\
4:16 1 operator\n\
5:1 6 keyword\n\
5:8 5 class declaration\n\
5:14 1 operator\n\
5:16 7 keyword\n\
5:24 4 keyword\n\
5:29 3 function declaration\n\
5:32 1 operator\n\
5:33 4 keyword\n\
5:37 1 operator\n\
5:38 1 operator\n\
5:40 1 operator\n\
5:41 1 operator\n\
6:1 6 keyword\n\
6:8 5 class declaration\n\
6:14 5 keyword\n\
6:20 1 operator\n\
6:22 5 class\n\
6:28 1 operator\n\
6:30 4 keyword\n\
6:35 3 function declaration\n\
6:38 1 operator\n\
6:39 4 keyword\n\
6:43 1 operator\n\
6:45 8 keyword\n\
6:54 5 keyword\n\
6:59 1 operator\n\
6:61 1 operator\n\
6:62 1 operator\n\
7:1 1 operator\n\
7:2 1 operator\n\
7:3 9 identifier\n\
7:12 1 operator\n\
7:13 1 operator\n\
7:15 3 keyword\n\
7:19 3 function declaration\n\
7:22 1 operator\n\
7:23 4 keyword\n\
7:27 1 operator\n\
7:28 1 operator\n\
8:1 3 keyword\n\
8:5 8 variable declaration\n\
8:14 1 operator\n\
8:16 1 number\n\
8:17 1 operator\n", HighlightText("\
namespace NS { typedef int INT; }\n\
template<class M> M Max(M a) { return a; }\n\
__int128 giWide;\n\
NS::INT giValue;\n\
struct CBase { virtual void Run(void); };\n\
struct CLeaf final : CBase { void Run(void) override final; };\n\
[[nodiscard]] int Get(void);\n\
int override = 1;\n", &sz));
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppHighlighter(void)
{
	BeginTests();

	TypesInit();
	TypeConverterInit();
	NumberInit();

	TestCppHighlighterSource();
	TestCppHighlighterSplices();
	TestCppHighlighterNames();

	NumberKill();
	TypeConverterKill();
	TypesKill();

	TestStatistics();
}

