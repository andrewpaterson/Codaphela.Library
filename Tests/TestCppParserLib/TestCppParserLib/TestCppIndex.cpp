#include "CppParserLib/Preprocessor.h"
#include "CppParserLib/CppTokeniser.h"
#include "CppParserLib/CppParser.h"
#include "CppParserLib/CPPIndex.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses and parses szSource as szFileName and adds it to pcIndex.
//
//////////////////////////////////////////////////////////////////////////
static void IndexText(CCPPIndex* pcIndex, char* szFileName, char* szSource)
{
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

	pcIndex->AddUnit(&cTokeniser, &cTree);

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
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndex(void)
{
	BeginTests();

	TestCppIndexUnits();
	TestCppIndexSignatures();

	TestStatistics();
}
