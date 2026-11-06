#include "CppParserLib/Preprocessor.h"
#include "CppParserLib/CppTokeniser.h"
#include "CppParserLib/CppParser.h"
#include "CppParserLib/CPPIndex.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses and parses szSource as szFileName and adds it to pcIndex,
// with what the semantic pass resolves when bSemantic.
//
//////////////////////////////////////////////////////////////////////////
static void IndexText(CCPPIndex* pcIndex, char* szFileName, char* szSource, bool bSemantic = false)
{
	CCPPSemanticPass	cPass;
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;

	cFile.Init(szFileName, NULL, false);
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

	cPass.Init();
	if (bSemantic)
	{
		cPass.Check(&cTree);
	}
	pcIndex->AddUnit(&cTokeniser, &cTree, &cPass);
	cPass.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


#define TEST_INDEX_CLASS "\
class CFoo\n\
{\n\
public:\n\
	int		miCount;\n\
	void	Init(void);\n\
	void	Init(int iCount, const char* sz = 0);\n\
	int		Get(void) const;\n\
};\n"


//////////////////////////////////////////////////////////////////////////
//
// Two units that both declare a class are the same symbols.  Overloads
// are kept apart by their parameter types, a definition is found from
// its declarator whatever the parser looked its name up as, and a call
// is the overload its number of arguments fits.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexUnits(void)
{
	CCPPIndex			cIndex;
	CChars				sz;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;

	cIndex.Init();
	IndexText(&cIndex, "A.cpp", TEST_INDEX_CLASS "\
void CFoo::Init(void)\n\
{\n\
	miCount = 0;\n\
}\n\
int CFoo::Get(void) const\n\
{\n\
	return miCount;\n\
}\n");
	IndexText(&cIndex, "B.cpp", TEST_INDEX_CLASS "\
void CFoo::Init(int iCount, const char* sz)\n\
{\n\
	Init();\n\
	miCount = iCount;\n\
}\n\
int Use(CFoo* pc)\n\
{\n\
	pc->Init(1);\n\
	return pc->Get();\n\
}\n");

	sz.Init();
	cIndex.Print(&sz);
	AssertString("\
CFoo\n\
  A.cpp:1:7 definition\n\
  A.cpp:9:6 reference\n\
  A.cpp:13:5 reference\n\
  B.cpp:1:7 definition\n\
  B.cpp:9:6 reference\n\
  B.cpp:14:9 reference\n\
CFoo::miCount\n\
  A.cpp:4:7 declaration\n\
  A.cpp:11:2 reference\n\
  A.cpp:15:9 reference\n\
  B.cpp:4:7 declaration\n\
  B.cpp:12:2 reference\n\
CFoo::Init()\n\
  A.cpp:5:7 declaration\n\
  A.cpp:9:12 definition\n\
  B.cpp:5:7 declaration\n\
  B.cpp:11:2 reference\n\
CFoo::Init(int, char const*)\n\
  A.cpp:6:7 declaration\n\
  B.cpp:6:7 declaration\n\
  B.cpp:9:12 definition\n\
@A.cpp:6:12:iCount\n\
  A.cpp:6:16 declaration\n\
@A.cpp:6:24:sz\n\
  A.cpp:6:36 declaration\n\
CFoo::Get() const\n\
  A.cpp:7:7 declaration\n\
  A.cpp:13:11 definition\n\
  B.cpp:7:7 declaration\n\
@B.cpp:6:12:iCount\n\
  B.cpp:6:16 declaration\n\
@B.cpp:6:24:sz\n\
  B.cpp:6:36 declaration\n\
@B.cpp:9:17:iCount\n\
  B.cpp:9:21 declaration\n\
  B.cpp:12:12 reference\n\
@B.cpp:9:29:sz\n\
  B.cpp:9:41 declaration\n\
Use(CFoo*)\n\
  B.cpp:14:5 definition\n\
@B.cpp:14:9:pc\n\
  B.cpp:14:15 declaration\n\
  B.cpp:16:2 reference\n\
  B.cpp:17:9 reference\n\
", sz.Text());
	sz.Kill();

	pcSymbol = cIndex.FindSymbol("CFoo::Init(int, char const*)");
	AssertNotNull(pcSymbol);
	psLocation = pcSymbol ? pcSymbol->GetDefinition() : NULL;
	AssertNotNull(psLocation);
	AssertTrue(cIndex.FindSymbolAt(cIndex.GetFileName(0), 4, 7) == cIndex.FindSymbol("CFoo::Init()"));
	AssertNull(cIndex.FindSymbolAt(cIndex.GetFileName(0), 4, 10));
	AssertNull(cIndex.FindSymbolAt(cIndex.GetFileName(0), 3, 1));

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Declarations of the same function spelled differently are one
// symbol: const before or after the type, top level const, (void) and
// (), parameter names and defaults.  Calls an argument count doesn't
// pick an overload by are references to each overload they could be.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexSignatures(void)
{
	CCPPIndex			cIndex;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;
	size				i;
	int					iAmbiguous;

	cIndex.Init();
	IndexText(&cIndex, "A.cpp", "\
class CBar;\n\
void Set(const CBar& c, const int i, char* const p);\n\
void Reset(void);\n\
int Print(const char* sz, ...);\n\
void Pick(int a);\n\
void Pick(char c);\n\
void Call(void)\n\
{\n\
	Reset();\n\
	Pick(1);\n\
	Print(\"x\", 1, 2);\n\
}\n");
	IndexText(&cIndex, "B.cpp", "\
class CBar;\n\
void Set(CBar const& cOther, int iOther, char* pOther = 0)\n\
{\n\
}\n\
void Reset()\n\
{\n\
}\n");

	pcSymbol = cIndex.FindSymbol("Set(CBar const&, int, char*)");
	AssertNotNull(pcSymbol);
	AssertSize(2, pcSymbol->NumLocations());
	AssertNotNull(pcSymbol->GetDeclaration());
	AssertNotNull(pcSymbol->GetDefinition());

	pcSymbol = cIndex.FindSymbol("Reset()");
	AssertNotNull(pcSymbol);
	AssertSize(3, pcSymbol->NumLocations());

	pcSymbol = cIndex.FindSymbol("Print(char const*, ...)");
	AssertNotNull(pcSymbol);
	AssertSize(2, pcSymbol->NumLocations());
	psLocation = pcSymbol->GetLocation(1);
	AssertInt(CPPIR_Reference, psLocation->uiRole);
	AssertInt(0, psLocation->uiFlags);

	//Pick(1) has one argument, as both Picks do.
	iAmbiguous = 0;
	for (i = 0; i < cIndex.NumSymbols(); i++)
	{
		pcSymbol = cIndex.GetSymbol(i);
		if (pcSymbol->mszKey.StartsWith("Pick(") && (pcSymbol->NumLocations() == 2) && (pcSymbol->GetLocation(1)->uiFlags & CPPIF_AMBIGUOUS))
		{
			iAmbiguous++;
		}
	}
	AssertInt(2, iAmbiguous);
	AssertNotNull(cIndex.FindSymbol("Pick(int)"));
	AssertNotNull(cIndex.FindSymbol("Pick(char)"));

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With the semantic pass, members named through objects (pc->Init(1),
// c.miCount) are references, and a call is the one overload the
// argument types pick.  A statement it can't type doesn't stop the
// others, and the names in it are still resolved where they can be.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexSemanticPass(void)
{
	CCPPIndex			cIndex;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;

	cIndex.Init();
	IndexText(&cIndex, "B.cpp", TEST_INDEX_CLASS "\
void Pick(int a);\n\
void Pick(char c);\n\
int Use(CFoo* pc, CFoo& c)\n\
{\n\
	char	x;\n\
\n\
	pc->Init(1);\n\
	c.Init();\n\
	Pick(x);\n\
	Pick(c.miCount);\n\
	Unknown(pc->Get());\n\
	return pc->Get() + c.miCount;\n\
}\n", true);

	//pc->Init(1) is Init(int, char const*); c.Init() is Init().
	pcSymbol = cIndex.FindSymbol("CFoo::Init(int, char const*)");
	AssertNotNull(pcSymbol);
	AssertSize(2, pcSymbol->NumLocations());
	psLocation = pcSymbol->GetLocation(1);
	AssertInt(CPPIR_Reference, psLocation->uiRole);
	AssertInt(14, psLocation->iLine);
	AssertInt(5, psLocation->iColumn);
	AssertInt(0, psLocation->uiFlags);

	pcSymbol = cIndex.FindSymbol("CFoo::Init()");
	AssertNotNull(pcSymbol);
	AssertSize(2, pcSymbol->NumLocations());
	AssertInt(15, pcSymbol->GetLocation(1)->iLine);

	//Pick(x) is Pick(char), Pick(c.miCount) Pick(int): neither ambiguous.
	pcSymbol = cIndex.FindSymbol("Pick(char)");
	AssertNotNull(pcSymbol);
	AssertSize(2, pcSymbol->NumLocations());
	AssertInt(16, pcSymbol->GetLocation(1)->iLine);
	AssertInt(0, pcSymbol->GetLocation(1)->uiFlags);
	pcSymbol = cIndex.FindSymbol("Pick(int)");
	AssertNotNull(pcSymbol);
	AssertSize(2, pcSymbol->NumLocations());
	AssertInt(17, pcSymbol->GetLocation(1)->iLine);
	AssertInt(0, pcSymbol->GetLocation(1)->uiFlags);

	//pc->Get() inside Unknown(...) is still found, as is the one after it.
	pcSymbol = cIndex.FindSymbol("CFoo::Get() const");
	AssertNotNull(pcSymbol);
	AssertSize(3, pcSymbol->NumLocations());
	AssertInt(18, pcSymbol->GetLocation(1)->iLine);
	AssertInt(19, pcSymbol->GetLocation(2)->iLine);

	//c.miCount twice.
	pcSymbol = cIndex.FindSymbol("CFoo::miCount");
	AssertNotNull(pcSymbol);
	AssertSize(3, pcSymbol->NumLocations());

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The host's C++ checks without errors: calling conventions in
// declarators (void* __cdecl f()), its compiler's builtins and intrinsics
// (__builtin_va_arg(ap, T) is a T), and its 8 byte pointers and size_t.
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassHost(void)
{
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPPSemanticPass	cPass;

	cFile.Init("Host.cpp", NULL, false);
	cFile.SetContents("\
typedef __builtin_va_list va_list;\n\
void* __cdecl Copy(void* pvDest, const void* pvSource, unsigned long long uiSize);\n\
int __stdcall Sum(int iCount, ...)\n\
{\n\
	va_list	ap;\n\
	int		iTotal;\n\
\n\
	iTotal = 0;\n\
	__builtin_va_start(ap, iCount);\n\
	iTotal += __builtin_va_arg(ap, int);\n\
	__builtin_va_end(ap);\n\
	return iTotal;\n\
}\n\
void Use(char* sz, char* szOther)\n\
{\n\
	static_assert(sizeof(sz) == 8, \"8 byte pointers\");\n\
	unsigned long long	uiSize;\n\
\n\
	uiSize = sizeof(sz);\n\
	Copy(sz, szOther, uiSize);\n\
	__debugbreak();\n\
	Sum(2, 1, 2);\n\
}\n");
	cPreprocessor.Init(NULL);
	AssertTrue(cPreprocessor.PreprocessTranslationUnit(&cFile));
	cPreprocessor.Kill();
	cTokeniser.Init();
	AssertTrue(cTokeniser.Tokenise(cFile.GetTokens()));
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	cParser.Parse();
	cParser.Kill();

	cPass.Init();
	AssertTrue(cPass.Check(&cTree));
	AssertString("", cPass.GetErrors()->Text());
	AssertTrue(cPass.NumResolved() >= 2);
	cPass.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndex(void)
{
	BeginTests();

	TestCppIndexUnits();
	TestCppIndexSignatures();
	TestCppIndexSemanticPass();
	TestCppSemanticPassHost();

	TestStatistics();
}
