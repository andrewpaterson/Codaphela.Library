#include "BaseLib/FileUtil.h"
#include "CppCompilerLib/HeaderFileMap.h"
#include "CppCompilerLib/HeaderFiles.h"
#include "CppCompilerLib/CPPPreprocessor.h"
#include "CppCompilerLib/CppTokeniser.h"
#include "CppCompilerLib/CppParser.h"
#include "CppCompilerLib/CPPIndex.h"
#include "CppCompilerLib/CPPSyntaxes.h"
#include "CppCompilerLib/CPP65Type.h"
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
	CCPPPreprocessor		cPreprocessor;
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
// A constructor defined outside its class, CTank::CTank(int), is that
// constructor's definition (its name finds the class's own name in the
// class, which isn't what is declared): each overload's, by its
// parameters.  The class named before the :: is a use of the class.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexConstructorDefinitions(void)
{
	CCPPIndex			cIndex;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;

	cIndex.Init();
	IndexText(&cIndex, "A.cpp", "\
class CTank\n\
{\n\
public:\n\
	int	miSize;\n\
	CTank(void);\n\
	CTank(int iSize);\n\
	void Init(void);\n\
};\n\
CTank::CTank(void) : miSize(0)\n\
{\n\
}\n\
CTank::CTank(int iCapacity) : miSize(iCapacity)\n\
{\n\
}\n\
void CTank::Init(void)\n\
{\n\
}\n");

	pcSymbol = cIndex.FindSymbol("CTank::CTank()");
	AssertNotNull(pcSymbol);
	AssertInt(CPPSK_Function, pcSymbol->meKind);
	AssertSize(2, pcSymbol->NumLocations());
	AssertNotNull(pcSymbol->GetDeclaration());
	psLocation = pcSymbol->GetDefinition();
	AssertNotNull(psLocation);
	AssertInt(8, psLocation->iLine);
	AssertInt(7, psLocation->iColumn);

	pcSymbol = cIndex.FindSymbol("CTank::CTank(int)");
	AssertNotNull(pcSymbol);
	AssertSize(2, pcSymbol->NumLocations());
	psLocation = pcSymbol->GetDefinition();
	AssertNotNull(psLocation);
	AssertInt(11, psLocation->iLine);
	AssertInt(7, psLocation->iColumn);

	//The class: its definition, and the three names before a ::.
	pcSymbol = cIndex.FindSymbol("CTank");
	AssertNotNull(pcSymbol);
	AssertSize(4, pcSymbol->NumLocations());

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
// Check of szSource, whose functions nothing calls.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCppSemanticPassCheck(const char* szSource)
{
	CTranslationUnit	cFile;
	CCPPPreprocessor		cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPPSemanticPass	cPass;
	bool				bResult;

	cFile.Init("Queued.cpp", NULL, false);
	cFile.SetContents((char*)szSource);
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
	bResult = cPass.Check(&cTree);
	cPass.Kill();
	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Checking one function queues those it calls, a template's instance that
// isn't there yet among them, after the first drain of the queue: they are
// checked, and what is wrong in a plain function among them is found.
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassQueued(void)
{
	AssertTrue(TestCppSemanticPassCheck(
		"template<class T> T Twice(T t) { return t + t; }\n"
		"inline int Second(int x) { return Twice(x); }\n"
		"inline int First(int x) { return Second(x); }\n"));
	AssertFalse(TestCppSemanticPassCheck(
		"inline int Third(int x) { return x + giNotDeclared; }\n"
		"inline int Second(int x) { return Third(x); }\n"
		"inline int First(int x) { return Second(x); }\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's declaration is checked as it is compiled (CCPPUnit::
// CheckDeclared): what the compiler makes isn't an error here (an
// aggregate's default member initialiser that names an earlier member, an
// array of characters from a string in braces or as a member's value, a
// bool initialised directly with nullptr, a reference to an object made
// from a value, string literals for an array of char*), and what it
// refuses is (an object nothing can make, a reference with nothing to
// refer to).
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassDeclarations(void)
{
	AssertTrue(TestCppSemanticPassCheck("struct W { int a = 1; int b = a + 1; int c; };\nint F(void) { W w = { 10 }; return w.b; }\n"));
	AssertTrue(TestCppSemanticPassCheck("struct N { char sz[4]; int i; };\nint F(void) { char c[] = { \"xyz\" }; N n = { \"ab\", 3 }; return c[0] + n.i; }\n"));
	AssertTrue(TestCppSemanticPassCheck("int F(void) { bool b{nullptr}; bool c(nullptr); return b || c; }\n"));
	AssertTrue(TestCppSemanticPassCheck("struct S { int v; S(int a) : v(a) {} };\nint F(void) { const S& r = 4; return r.v; }\n"));
	AssertTrue(TestCppSemanticPassCheck("int F(void) { char* asz[] = { \"one\", \"two\" }; return asz[1][0]; }\n"));
	AssertFalse(TestCppSemanticPassCheck("struct S { int v; S(int a) : v(a) {} };\nint F(void) { S s; return s.v; }\n"));
	AssertFalse(TestCppSemanticPassCheck("struct S { int v; S(int a) : v(a) {} };\nint F(void) { S s(1, 2); return s.v; }\n"));
	AssertFalse(TestCppSemanticPassCheck("int F(void) { int& r; return 0; }\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A lambda's body is checked where the lambda is, whether or not anything
// calls it: what is wrong in it is found, a lambda in it too.
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassLambdaBodies(void)
{
	AssertTrue(TestCppSemanticPassCheck(
		"int Take(int (*pf)(int)) { return pf(2); }\n"
		"int First(int x) { auto l = [x](int y) { int z = x + y; return z; }; return Take([](int v) { return v + 1; }) + l(1); }\n"));
	AssertFalse(TestCppSemanticPassCheck(
		"int First(int x) { auto l = [x](int y) { return x + y + giNotDeclared; }; return 0; }\n"));
	AssertFalse(TestCppSemanticPassCheck(
		"int Take(int (*pf)(int)) { return pf(2); }\n"
		"int First(int x) { return Take([](int v) { return v + giNotDeclared; }); }\n"));
	AssertFalse(TestCppSemanticPassCheck(
		"int First(int x) { auto l = [x]() { auto m = [](int y) { return y + giNotDeclared; }; return 1; }; return 0; }\n"));
}


//////////////////////////////////////////////////////////////////////////
//
// A kept check of szSource with szEnum taken as an enum class
// (CCPPSemanticPass::SetRecordedEnum; NULL for none): whether it checked
// without an error, and the kinds (CPPEC_, a digit each, in order of
// kind) of the casts recorded.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCppSemanticPassRecord(const char* szSource, const char* szEnum, CChars* pszKinds)
{
	CTranslationUnit	cFile;
	CCPPPreprocessor	cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPPSemanticPass	cPass;
	size				i;
	uint8				uiKind;
	bool				bResult;

	cFile.Init("Recorded.cpp", NULL, false);
	cFile.SetContents((char*)szSource);
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
	cPass.SetRecordedEnum(szEnum);
	bResult = cPass.Check(&cTree, CPPT_Host, true);
	pszKinds->Clear();
	for (uiKind = CPPEC_TO_OTHER; uiKind <= CPPEC_INCREMENT; uiKind++)
	{
		for (i = 0; i < cPass.NumEnumCasts(); i++)
		{
			if (cPass.GetEnumCast(i)->uiKind == uiKind)
			{
				pszKinds->Append((char)('0' + uiKind));
			}
		}
	}
	cPass.Kill();
	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An enum checked as if it were an enum class, for the refactoring that
// makes it one: each place its value would be refused is recorded, not
// reported, and the check goes on, so what follows from a cast is found
// too.  Another enum that is an enum class is still refused.
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassRecordedEnum(void)
{
	CChars	szKinds;

	szKinds.Init();

	//Nothing is asked about: an enum's value is an integer.
	AssertTrue(TestCppSemanticPassRecord("enum E { A, B };\nint F(E e) { int i = e; return i + (e + 1); }\n", NULL, &szKinds));
	AssertString("", szKinds.Text());

	//Given to an int, and added to.
	AssertTrue(TestCppSemanticPassRecord("enum E { A, B };\nint F(E e) { int i = e; return i + (e + 1); }\n", "E", &szKinds));
	AssertString("02", szKinds.Text());

	//A condition and a comparison with an integer; a cast, its own values and what a function of its type returns need nothing.
	AssertTrue(TestCppSemanticPassRecord("enum E { A, B };\nE G(int i) { E e = (E)i; if (e) { e = B; } return (e == 1) ? A : e; }\n", "E", &szKinds));
	AssertString("34", szKinds.Text());

	//Returned from a function of another type, and an integer returned as it.
	AssertTrue(TestCppSemanticPassRecord("enum E { A };\nint F(E e) { return e; }\nE G(int i) { return i; }\n", "E", &szKinds));
	AssertString("01", szKinds.Text());

	//Flags: each operator, and what the or gives is an int given to the enum only through its cast.
	AssertTrue(TestCppSemanticPassRecord("enum E { A = 1, B = 2 };\nint F(E e) { e = (E)(e | B); return e & A; }\n", "E", &szKinds));
	AssertString("22", szKinds.Text());

	//An argument for an int parameter still finds the function.
	AssertTrue(TestCppSemanticPassRecord("enum E { A };\nint G(int i) { return i; }\nint F(E e) { return G(e); }\n", "E", &szKinds));
	AssertString("0", szKinds.Text());

	//++ has no cast.
	AssertTrue(TestCppSemanticPassRecord("enum E { A, B };\nvoid F(E e) { e++; }\n", "E", &szKinds));
	AssertString("5", szKinds.Text());

	//An enum in a class, by its name with the class's.
	AssertTrue(TestCppSemanticPassRecord("class C { public: enum E { A }; };\nint F(C::E e) { return e + 1; }\n", "C::E", &szKinds));
	AssertString("2", szKinds.Text());

	//Another enum's value isn't recorded, and one that is an enum class is refused as it always is.
	AssertTrue(TestCppSemanticPassRecord("enum E { A };\nenum F { B };\nint G(F f) { return f + 1; }\n", "E", &szKinds));
	AssertString("", szKinds.Text());
	AssertFalse(TestCppSemanticPassRecord("enum class S { A };\nenum E { B };\nint F(S s, E e) { return (int)(s + e); }\n", "E", &szKinds));

	//What is wrong in an arm of ?: is said (it was typed quietly, and the error lost).
	AssertFalse(TestCppSemanticPassRecord("enum class S { A };\nint F(bool b) { return b ? (int)(S::A + 1) : 0; }\n", NULL, &szKinds));
	AssertTrue(TestCppSemanticPassRecord("enum class S { A };\nint F(bool b) { return b ? (int)S::A + 1 : 0; }\n", NULL, &szKinds));
	AssertTrue(TestCppSemanticPassRecord("enum E { A };\nint F(bool b) { return b ? A + 1 : 0; }\n", "E", &szKinds));
	AssertString("2", szKinds.Text());

	szKinds.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A type a declaration names that nothing declares is an error of a
// check, whether or not what is declared is used: one declared further
// down, and one never declared, in a member function's parameter, a
// function's parameter and what it returns, a field, a variable, a base
// class and a member held by value.  A class declared ahead of where it
// is named isn't one, nor is a name in a template that only an instance
// has.
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassUndeclaredTypes(void)
{
	//Declared further down.
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: int Use(CLater* pc); };\nclass CLater { public: int mi; };\n"));
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: int Use(CLater* pc); };\nclass CLater { public: int mi; };\nint CVat::Use(CLater* pc) { return pc->mi; }\n"));
	AssertFalse(TestCppSemanticPassCheck("int Use(CLater* pc);\nclass CLater { public: int mi; };\n"));
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: CLater* mpc; };\nclass CLater { public: int mi; };\n"));
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: int Use(void) { CLater c; return c.mi; } };\nclass CLater { public: int mi; };\n"));
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: CLater mc; };\nclass CLater { public: int mi; };\n"));

	//Never declared.
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: CNever* mpc; };\n"));
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: int Use(CNever* pc); };\n"));
	AssertFalse(TestCppSemanticPassCheck("int Use(CNever* pc);\n"));
	AssertFalse(TestCppSemanticPassCheck("CNever* Make(void);\n"));
	AssertFalse(TestCppSemanticPassCheck("class CVat { public: CNever* Make(void); };\n"));
	AssertFalse(TestCppSemanticPassCheck("CNever* gpc;\n"));
	AssertFalse(TestCppSemanticPassCheck("int Use(void) { CNever* pc; return 0; }\n"));
	AssertFalse(TestCppSemanticPassCheck("class CVat : public CNever { public: int mi; };\n"));

	//Declared first: by its definition, and ahead of it.
	AssertTrue(TestCppSemanticPassCheck("class CLater { public: int mi; };\nclass CVat { public: int Use(CLater* pc); CLater* Make(void); CLater mc; };\n"));
	AssertTrue(TestCppSemanticPassCheck("class CLater;\nclass CVat { public: int Use(CLater* pc); CLater* mpc; };\nCLater* Make(void);\nclass CLater { public: int mi; };\n"));
	AssertTrue(TestCppSemanticPassCheck("typedef int Unit;\nstruct SOrder { Unit iUnit; };\nUnit Count(Unit iFirst);\n"));

	//In a template a name may be an instance's: it isn't looked at.
	AssertTrue(TestCppSemanticPassCheck("template<class M> class CHolder { public: M* mp; typename M::SMark msMark; int Use(M* p); };\n"));
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
	CCPPPreprocessor		cPreprocessor;
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
// iCount enumerators named szPrefix0, szPrefix1 ..., the first 1 and each
// after it the one before plus one, as urlmon.h's tagBINDSTATUS's are.
//
//////////////////////////////////////////////////////////////////////////
void TestCppChainedEnumerators(CChars* pszDest, const char* szPrefix, int iCount)
{
	int	i;

	pszDest->Append("\t");
	pszDest->Append(szPrefix);
	pszDest->Append("0 = 1,\n");
	for (i = 1; i < iCount; i++)
	{
		pszDest->Append("\t");
		pszDest->Append(szPrefix);
		pszDest->Append(i);
		pszDest->Append(" = ( ");
		pszDest->Append(szPrefix);
		pszDest->Append(i - 1);
		pszDest->Append(" + 1 ) ,\n");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The source of an enum whose 78 enumerators are each the one before plus
// one, as urlmon.h's tagBINDSTATUS's are, and of a function with a
// parameter of it (declaring it works out the enum's underlying type).
// szMain is appended after it.
//
//////////////////////////////////////////////////////////////////////////
void TestCppEnumeratorChainSource(CChars* pszDest, const char* szMain)
{
	pszDest->Append("typedef enum tagSTATUS\n{\n");
	TestCppChainedEnumerators(pszDest, "STATUS_", 78);
	pszDest->Append("} STATUS;\n");
	pszDest->Append("enum EMixed { MIXED_A = 5, MIXED_B, MIXED_C = MIXED_B + 10, MIXED_D };\n");
	pszDest->Append("int Status(STATUS eStatus) { return eStatus; }\n");
	pszDest->Append(szMain);
}


//////////////////////////////////////////////////////////////////////////
//
// The enum of chained enumerators is checked in a moment: an enumerator's
// value doesn't work out the values of those before it again, which
// doubled the work with each (TestCPP65IREnumeratorChain has the values).
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassEnumeratorChain(void)
{
	CChars	szSource;

	szSource.Init();
	TestCppEnumeratorChainSource(&szSource, "inline int Last(void) { return Status(STATUS_77); }\n");
	AssertTrue(TestCppSemanticPassCheck(szSource.Text()));
	szSource.Kill();

	//Each value given is folded once, and the chain one fold deep, however long it is.
	szSource.Init();
	szSource.Append("enum ELong\n{\n");
	TestCppChainedEnumerators(&szSource, "LONG_", 1000);
	szSource.Append("};\ninline int Last(void) { return LONG_999; }\n");
	AssertTrue(TestCppSemanticPassCheck(szSource.Text()));
	szSource.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A class whose name a macro makes, as BaseLib's ARRAY_BLOCK made
// CArrayBlock and, included again, CArrayLargeBlock, is defined where the
// macro was used (flagged CPPIF_MACRO), so Go to Class and Go to
// Definition find it.  Names a macro body only uses aren't references.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexMacroNames(void)
{
	CCPPIndex			cIndex;
	CChars				sz;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;

	cIndex.Init();
	IndexText(&cIndex, "M.cpp", "\
#define ARRAY_BLOCK CArrayBlock\n\
#define MAKE_COUNT(n) int n(void)\n\
class ARRAY_BLOCK\n\
{\n\
public:\n\
	MAKE_COUNT(Count);\n\
};\n\
#undef ARRAY_BLOCK\n\
#define ARRAY_BLOCK CArrayLargeBlock\n\
class ARRAY_BLOCK\n\
{\n\
};\n\
CArrayBlock gcBlock;\n");

	sz.Init();
	cIndex.Print(&sz);
	AssertString("\
CArrayBlock\n\
  M.cpp:3:7 definition macro\n\
  M.cpp:13:1 reference\n\
CArrayBlock::Count()\n\
  M.cpp:6:13 declaration\n\
CArrayLargeBlock\n\
  M.cpp:10:7 definition macro\n\
gcBlock\n\
  M.cpp:13:13 declaration\n\
", sz.Text());
	sz.Kill();

	pcSymbol = cIndex.FindSymbol("CArrayBlock");
	AssertNotNull(pcSymbol);
	psLocation = pcSymbol ? pcSymbol->GetDefinition() : NULL;
	AssertNotNull(psLocation);
	if (psLocation)
	{
		AssertInt(2, psLocation->iLine);
		AssertInt(6, psLocation->iColumn);
		AssertInt(11, psLocation->iLength);
		AssertInt(CPPIF_MACRO, psLocation->uiFlags);
	}
	pcSymbol = cIndex.FindSymbol("CArrayLargeBlock");
	AssertNotNull(pcSymbol);
	psLocation = pcSymbol ? pcSymbol->GetDefinition() : NULL;
	AssertNotNull(psLocation);
	if (psLocation)
	{
		AssertInt(9, psLocation->iLine);
		AssertInt(6, psLocation->iColumn);
		AssertInt(CPPIF_MACRO, psLocation->uiFlags);
	}

	//Go to Definition on the macro's name finds the class it made there.
	AssertTrue(cIndex.FindSymbolAt(cIndex.GetFileName(0), 2, 8) == cIndex.FindSymbol("CArrayBlock"));

	cIndex.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
// A function or variable declared inside extern "C" (braced or not, in a
// namespace too) is flagged CPPIF_C_LINKAGE where it is declared: its
// label is only its name.  Not extern "C++", a class's members, or the
// definition outside it.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexCLinkage(void)
{
	CCPPIndex			cIndex;
	CChars				sz;

	cIndex.Init();
	IndexText(&cIndex, "L.cpp", "\
namespace NOs\n\
{\n\
	extern \"C\"\n\
	{\n\
		int Read(char* sz);\n\
		extern int giCount;\n\
		struct SFile\n\
		{\n\
			void Close(void);\n\
		};\n\
	}\n\
	extern \"C\" void Write(int i);\n\
	extern \"C++\" void Seek(int i);\n\
	void Tell(void);\n\
}\n\
int NOs::Read(char* sz) { return NOs::giCount; }\n");

	sz.Init();
	cIndex.Print(&sz);
	AssertString("\
NOs\n\
  L.cpp:1:11 declaration\n\
  L.cpp:16:5 reference\n\
  L.cpp:16:34 reference\n\
NOs::Read(char*)\n\
  L.cpp:5:7 declaration extern-C\n\
  L.cpp:16:10 definition\n\
@L.cpp:5:12:sz\n\
  L.cpp:5:18 declaration\n\
NOs::giCount\n\
  L.cpp:6:14 declaration extern-C\n\
  L.cpp:16:39 reference\n\
NOs::SFile\n\
  L.cpp:7:10 definition\n\
NOs::SFile::Close()\n\
  L.cpp:9:9 declaration\n\
NOs::Write(int)\n\
  L.cpp:12:18 declaration extern-C\n\
@L.cpp:12:24:i\n\
  L.cpp:12:28 declaration\n\
NOs::Seek(int)\n\
  L.cpp:13:20 declaration\n\
@L.cpp:13:25:i\n\
  L.cpp:13:29 declaration\n\
NOs::Tell()\n\
  L.cpp:14:7 declaration\n\
@L.cpp:16:15:sz\n\
  L.cpp:16:21 declaration\n\
", sz.Text());
	sz.Kill();

	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each place records the class or function definition it is in, where
// Find Usages groups it: a member declared in its class, used in a member
// function and in a free function.
//
//////////////////////////////////////////////////////////////////////////
static char* GetCppIndexContainer(CCPPIndex* pcIndex, SCPPIndexLocation* psLocation)
{
	if (psLocation->iContainer < 0)
	{
		return (char*)"";
	}
	return pcIndex->GetSymbol((size)psLocation->iContainer)->mszKey.Text();
}


void TestCppIndexContainers(void)
{
	CCPPIndex			cIndex;
	CCPPIndexSymbol*	pcSymbol;

	cIndex.Init();
	IndexText(&cIndex, "C.cpp", "\
class CFoo\n\
{\n\
public:\n\
	int		miCount;\n\
	void	Init(void);\n\
};\n\
void CFoo::Init(void)\n\
{\n\
	miCount = 0;\n\
}\n\
int Use(CFoo* pc)\n\
{\n\
	return pc->miCount;\n\
}\n\
int giTotal;\n", true);

	pcSymbol = cIndex.FindSymbol("CFoo::miCount");
	AssertNotNull(pcSymbol);
	if (pcSymbol)
	{
		AssertSize(3, pcSymbol->NumLocations());
		AssertString("CFoo", GetCppIndexContainer(&cIndex, pcSymbol->GetLocation(0)));
		AssertString("CFoo::Init()", GetCppIndexContainer(&cIndex, pcSymbol->GetLocation(1)));
		AssertString("Use(CFoo*)", GetCppIndexContainer(&cIndex, pcSymbol->GetLocation(2)));
	}
	pcSymbol = cIndex.FindSymbol("giTotal");
	AssertNotNull(pcSymbol);
	if (pcSymbol)
	{
		AssertString("", GetCppIndexContainer(&cIndex, pcSymbol->GetLocation(0)));
	}
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each place says how it uses its symbol, as Find Usages groups them: the
// declaration a type is named in (a field's, a parameter's, a local's, a
// global's, a function's, a typedef's, a base class), a type in an
// expression, and a variable read, written or a function called.
//
//////////////////////////////////////////////////////////////////////////
static CCPPIndexSymbol* FindCppIndexSymbolStarting(CCPPIndex* pcIndex, const char* szStart)
{
	size	i;

	for (i = 0; i < pcIndex->NumSymbols(); i++)
	{
		if (pcIndex->GetSymbol(i)->mszKey.StartsWith(szStart))
		{
			return pcIndex->GetSymbol(i);
		}
	}
	return NULL;
}


//The usage type of the uiNth place (by column) on iLine of a symbol, -1 if it has none there.
static int GetCppIndexUsage(CCPPIndexSymbol* pcSymbol, int iLine, size uiNth)
{
	size				i;
	size				uiFound;
	SCPPIndexLocation*	psLocation;
	SCPPIndexLocation*	psBest;
	int					iAfter;

	if (pcSymbol == NULL)
	{
		return -1;
	}
	iAfter = -1;
	psBest = NULL;
	for (uiFound = 0; uiFound <= uiNth; uiFound++)
	{
		psBest = NULL;
		for (i = 0; i < pcSymbol->NumLocations(); i++)
		{
			psLocation = pcSymbol->GetLocation(i);
			if ((psLocation->iLine == iLine) && (psLocation->iColumn > iAfter) && ((psBest == NULL) || (psLocation->iColumn < psBest->iColumn)))
			{
				psBest = psLocation;
			}
		}
		if (psBest == NULL)
		{
			return -1;
		}
		iAfter = psBest->iColumn;
	}
	return psBest->uiUsage;
}


//////////////////////////////////////////////////////////////////////////
//
// The semantic pass keeps what a build would warn of apart from what it
// can't compile: an override not declared override is a warning, a name
// not declared an error.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexSemanticPassWarnings(void)
{
	CTranslationUnit	cFile;
	CCPPPreprocessor	cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPPSemanticPass	cPass;

	cFile.Init("Dir/Warned.cpp", NULL, false);
	cFile.SetContents("\
class CBase\n\
{\n\
public:\n\
	virtual int	Get(void) { return 1; }\n\
};\n\
class CDerived : public CBase\n\
{\n\
public:\n\
	int	Get(void) { return 2; }\n\
};\n\
int Missing(void) { return giMissing; }\n");
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
	AssertFalse(cPass.Check(&cTree));
	AssertSize(1, cPass.GetWarnings()->NumErrors());
	AssertTrue(strstr(cPass.GetWarnings()->Text(), "Warned.cpp (9, 6): ") != NULL);
	AssertTrue(strstr(cPass.GetWarnings()->Text(), "isn't declared override") != NULL);
	AssertSize(1, cPass.GetErrors()->NumErrors());
	AssertTrue(strstr(cPass.GetErrors()->Text(), "Warned.cpp (11, ") != NULL);
	AssertTrue(strstr(cPass.GetErrors()->Text(), "isn't declared override") == NULL);
	cPass.Kill();
	cTree.Kill();
	cTokeniser.Kill();

	//Preprocessed with errors naming files by their full paths, the check's do too.
	cPreprocessor.Init(NULL);
	cPreprocessor.SetFullPathErrors(true);
	AssertTrue(cPreprocessor.PreprocessTranslationUnit(&cFile));
	cPreprocessor.Kill();
	cTokeniser.Init();
	AssertTrue(cTokeniser.Tokenise(cFile.GetTokens()));
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	cParser.Parse();
	cParser.Kill();
	cPass.Init();
	AssertFalse(cPass.Check(&cTree));
	AssertTrue(strstr(cPass.GetWarnings()->Text(), "Dir/Warned.cpp (9, 6): ") != NULL);
	AssertTrue(strstr(cPass.GetErrors()->Text(), "Dir/Warned.cpp (11, ") != NULL);
	cPass.Kill();
	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexUsageTypes(void)
{
	CCPPIndex			cIndex;
	CCPPIndexSymbol*	pcFoo;
	CCPPIndexSymbol*	pcCount;

	cIndex.Init();
	IndexText(&cIndex, "U.cpp", "\
class CBase { };\n\
class CFoo : public CBase\n\
{\n\
public:\n\
	CFoo*	mpcNext;\n\
	int		miCount;\n\
	CFoo(int i);\n\
	CFoo*	Get(CFoo* pc);\n\
};\n\
typedef CFoo* PFoo;\n\
CFoo	gcFoo(1);\n\
CFoo::CFoo(int i) : miCount(i)\n\
{\n\
}\n\
CFoo* CFoo::Get(CFoo* pc)\n\
{\n\
	CFoo*	pcLocal;\n\
	pcLocal = (CFoo*)pc;\n\
	miCount++;\n\
	miCount = miCount + 1;\n\
	Get(pcLocal);\n\
	return pcLocal;\n\
}\n", true);

	AssertInt(CPPUT_BaseClass, GetCppIndexUsage(cIndex.FindSymbol("CBase"), 1, 0));

	pcFoo = cIndex.FindSymbol("CFoo");
	AssertNotNull(pcFoo);
	AssertInt(CPPUT_Field, GetCppIndexUsage(pcFoo, 4, 0));
	AssertInt(CPPUT_Function, GetCppIndexUsage(pcFoo, 7, 0));
	AssertInt(CPPUT_Parameter, GetCppIndexUsage(pcFoo, 7, 1));
	AssertInt(CPPUT_Alias, GetCppIndexUsage(pcFoo, 9, 0));
	AssertInt(CPPUT_Global, GetCppIndexUsage(pcFoo, 10, 0));
	AssertInt(CPPUT_Function, GetCppIndexUsage(pcFoo, 14, 0));
	AssertInt(CPPUT_Function, GetCppIndexUsage(pcFoo, 14, 1));
	AssertInt(CPPUT_Parameter, GetCppIndexUsage(pcFoo, 14, 2));
	AssertInt(CPPUT_Local, GetCppIndexUsage(pcFoo, 16, 0));
	AssertInt(CPPUT_TypeInExpression, GetCppIndexUsage(pcFoo, 17, 0));

	pcCount = cIndex.FindSymbol("CFoo::miCount");
	AssertNotNull(pcCount);
	AssertInt(CPPUT_Write, GetCppIndexUsage(pcCount, 11, 0));
	AssertInt(CPPUT_Write, GetCppIndexUsage(pcCount, 18, 0));
	AssertInt(CPPUT_Write, GetCppIndexUsage(pcCount, 19, 0));
	AssertInt(CPPUT_Read, GetCppIndexUsage(pcCount, 19, 1));

	AssertInt(CPPUT_Call, GetCppIndexUsage(FindCppIndexSymbolStarting(&cIndex, "CFoo::Get("), 20, 0));
	cIndex.Kill();

	//A class in a template's arguments, however deep and whatever it is in: a variable's type, a base class, a typedef,
	//a function template's call, sizeof; and as a type parameter's default.  The class beside them is used as itself.
	cIndex.Init();
	IndexText(&cIndex, "T.cpp", "\
class CFoo { public: int miX; };\n\
template<class M> class CHolder { public: M* mp; };\n\
template<class M> M* New(void) { return 0; }\n\
template<class M = CFoo> class CDefault { };\n\
CHolder<CFoo> gcHolder;\n\
CHolder<CHolder<CFoo> > gcNested;\n\
class CDerived : public CHolder<CFoo> { };\n\
typedef CHolder<CFoo> HFoo;\n\
int F(void) { CFoo* p = New<CFoo>(); CHolder<CFoo*> c; return (int)sizeof(CHolder<CFoo>); }\n", true);

	pcFoo = cIndex.FindSymbol("CFoo");
	AssertNotNull(pcFoo);
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 3, 0));
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 4, 0));
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 5, 0));
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 6, 0));
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 7, 0));
	AssertInt(CPPUT_Local, GetCppIndexUsage(pcFoo, 8, 0));
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 8, 1));
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 8, 2));
	AssertInt(CPPUT_TemplateArgument, GetCppIndexUsage(pcFoo, 8, 3));
	AssertInt(CPPUT_Global, GetCppIndexUsage(cIndex.FindSymbol("CHolder"), 4, 0));
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The overrides of a virtual member function are the functions with its
// name and parameters up and down its class's hierarchy, siblings too,
// whichever unit (and whichever class of them) said virtual or override.
// A function no class made virtual has none, and neither has one in an
// unrelated class.
//
//////////////////////////////////////////////////////////////////////////
static size CountCppIndexOverrides(CCPPIndex* pcIndex, const char* szKey)
{
	CArrayInt			aiOverrides;
	CCPPIndexSymbol*	pcSymbol;
	size				uiCount;

	pcSymbol = pcIndex->FindSymbol(szKey);
	AssertNotNull(pcSymbol);
	if (pcSymbol == NULL)
	{
		return 0;
	}
	aiOverrides.Init();
	uiCount = pcIndex->FindOverrides(pcSymbol, &aiOverrides) ? aiOverrides.NumElements() : 0;
	aiOverrides.Kill();
	return uiCount;
}


void TestCppIndexOverrides(void)
{
	CCPPIndex	cIndex;

	cIndex.Init();
	IndexText(&cIndex, "Base.cpp", "\
class CBase\n\
{\n\
public:\n\
	virtual void	Run(int i);\n\
	void			Run(char c);\n\
	void			Plain(void);\n\
};\n\
class CMiddle : public CBase\n\
{\n\
public:\n\
	void	Run(int i) override;\n\
	void	Plain(void);\n\
};\n\
class CSibling : public CBase\n\
{\n\
public:\n\
	void	Run(int i);\n\
};\n\
class COther\n\
{\n\
public:\n\
	virtual void	Run(int i);\n\
};\n");
	IndexText(&cIndex, "Leaf.cpp", "\
class CBase\n\
{\n\
public:\n\
	virtual void	Run(int i);\n\
	void			Run(char c);\n\
	void			Plain(void);\n\
};\n\
class CMiddle : public CBase\n\
{\n\
public:\n\
	void	Run(int i) override;\n\
	void	Plain(void);\n\
};\n\
class CLeaf : public CMiddle\n\
{\n\
public:\n\
	void	Run(int i);\n\
};\n\
void CLeaf::Run(int i)\n\
{\n\
}\n");

	AssertSize(4, CountCppIndexOverrides(&cIndex, "CLeaf::Run(int)"));
	AssertSize(4, CountCppIndexOverrides(&cIndex, "CBase::Run(int)"));
	AssertSize(4, CountCppIndexOverrides(&cIndex, "CSibling::Run(int)"));
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CBase::Run(char)"));
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CMiddle::Plain()"));
	AssertSize(0, CountCppIndexOverrides(&cIndex, "COther::Run(int)"));
	cIndex.Kill();

	//override and final say a function overrides a virtual one; with none virtual they override nothing (an error),
	//so nothing in the family overrides anything.  With the base's virtual the same family does.
	cIndex.Init();
	IndexText(&cIndex, "Claimed.cpp", "\
class CBase\n\
{\n\
public:\n\
	bool	IsThing(void) const;\n\
};\n\
class CDerived : public CBase\n\
{\n\
public:\n\
	bool	IsThing(void) const override;\n\
};\n\
class CFinal : public CBase\n\
{\n\
public:\n\
	bool	IsThing(void) const final;\n\
};\n");
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CBase::IsThing() const"));
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CDerived::IsThing() const"));
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CFinal::IsThing() const"));
	cIndex.Kill();

	cIndex.Init();
	IndexText(&cIndex, "Virtual.cpp", "\
class CBase\n\
{\n\
public:\n\
	virtual bool	IsThing(void) const;\n\
};\n\
class CDerived : public CBase\n\
{\n\
public:\n\
	bool	IsThing(void) const override;\n\
};\n\
class CFinal : public CBase\n\
{\n\
public:\n\
	bool	IsThing(void) const final;\n\
};\n");
	AssertSize(3, CountCppIndexOverrides(&cIndex, "CBase::IsThing() const"));
	AssertSize(3, CountCppIndexOverrides(&cIndex, "CDerived::IsThing() const"));
	AssertSize(3, CountCppIndexOverrides(&cIndex, "CFinal::IsThing() const"));
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A file that changed is forgotten and indexed again: its old places go,
// the other files' stay, and its new ones are found.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexRemoveFile(void)
{
	CCPPIndex			cIndex;
	CCPPIndexSymbol*	pcSymbol;

	cIndex.Init();
	IndexText(&cIndex, "A.cpp", "\
int giCount;\n\
int Get(void)\n\
{\n\
	return giCount;\n\
}\n");
	IndexText(&cIndex, "B.cpp", "\
extern int giCount;\n\
int Twice(void)\n\
{\n\
	return giCount + giCount;\n\
}\n");
	pcSymbol = cIndex.FindSymbol("giCount");
	AssertNotNull(pcSymbol);
	if (pcSymbol == NULL)
	{
		cIndex.Kill();
		return;
	}
	AssertSize(5, pcSymbol->NumLocations());
	AssertNotNull(cIndex.FindSymbolAt("A.cpp", 3, 8));

	cIndex.RemoveFile("A.cpp");
	AssertSize(3, pcSymbol->NumLocations());
	AssertNull(cIndex.FindSymbolAt("A.cpp", 3, 8));
	AssertNotNull(cIndex.FindSymbolAt("B.cpp", 3, 8));
	AssertSize(0, cIndex.FindSymbol("Get()")->NumLocations());

	IndexText(&cIndex, "A.cpp", "\
int giCount;\n\
\n\
int Get(void)\n\
{\n\
	return giCount;\n\
}\n");
	AssertSize(5, pcSymbol->NumLocations());
	AssertNull(cIndex.FindSymbolAt("A.cpp", 3, 8));
	AssertNotNull(cIndex.FindSymbolAt("A.cpp", 4, 8));
	AssertSize(1, cIndex.FindSymbol("Get()")->NumLocations());
	cIndex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A file forgotten (RemoveFile) takes its classes' base classes and its
// virtual declarations with it: indexed again without them, nothing
// overrides.  Another file's are kept.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexRemoveFileHierarchy(void)
{
	CCPPIndex	cIndex;

	cIndex.Init();
	IndexText(&cIndex, "Base.cpp", "\
class CBase\n\
{\n\
public:\n\
	virtual void	Run(int i);\n\
};\n");
	IndexText(&cIndex, "Derived.cpp", "\
class CBase\n\
{\n\
public:\n\
	virtual void	Run(int i);\n\
};\n\
class CDerived : public CBase\n\
{\n\
public:\n\
	void	Run(int i) override;\n\
};\n");
	AssertSize(2, CountCppIndexOverrides(&cIndex, "CBase::Run(int)"));

	//Derived.cpp gone: CBase's other declaration, in Base.cpp, is still virtual but nothing derives from it.
	cIndex.RemoveFile("Derived.cpp");
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CBase::Run(int)"));

	//Again, but CDerived no longer derives from CBase.
	IndexText(&cIndex, "Derived.cpp", "\
class CBase\n\
{\n\
public:\n\
	virtual void	Run(int i);\n\
};\n\
class CDerived\n\
{\n\
public:\n\
	void	Run(int i);\n\
};\n");
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CBase::Run(int)"));

	//Derived again, but nothing says virtual any more.
	cIndex.RemoveFile("Derived.cpp");
	cIndex.RemoveFile("Base.cpp");
	IndexText(&cIndex, "Base.cpp", "\
class CBase\n\
{\n\
public:\n\
	void	Run(int i);\n\
};\n");
	IndexText(&cIndex, "Derived.cpp", "\
class CBase\n\
{\n\
public:\n\
	void	Run(int i);\n\
};\n\
class CDerived : public CBase\n\
{\n\
public:\n\
	void	Run(int i);\n\
};\n");
	AssertSize(0, CountCppIndexOverrides(&cIndex, "CBase::Run(int)"));
	cIndex.Kill();
}


//Writes what a kept check remembers of every expression in a tree, in the order they are written: its source, its
//type as C++ and, for a name, how what it names is changed there.
class CTestKeptCheckPrinter : public CCPPSyntaxChildVisitor
{
public:
	CCPPSemanticPass*	mpcPass;
	CChars*				mpszDest;
	bool				mbDeclare;	//A name's type is written as a declaration of x, and nothing else is written.

	void	Visit(CCPPSyntax* pcChild) override
	{
		SCPP65Type*	psType;
		uint8		uiChanges;

		psType = mpcPass->GetType(pcChild);
		if (psType && mbDeclare)
		{
			if (pcChild->GetKind() == CPPS_IdExpression)
			{
				pcChild->PrettyPrint(mpszDest, 0);
				mpszDest->Append(" : ");
				if (!mpcPass->AppendDeclaration(psType, "x", mpszDest))
				{
					mpszDest->Append("?");
				}
				mpszDest->AppendNewLine();
			}
		}
		else if (psType)
		{
			pcChild->PrettyPrint(mpszDest, 0);
			mpszDest->Append(" : ");
			if (!mpcPass->AppendType(psType, mpszDest))
			{
				mpszDest->Append("?");
			}
			if (pcChild->GetKind() == CPPS_IdExpression)
			{
				uiChanges = mpcPass->FindChanges(pcChild);
				mpszDest->Append((uiChanges & CPPPC_ASSIGNED) ? " assigned" : "");
				mpszDest->Append((uiChanges & CPPPC_REFERENCE) ? " reference" : "");
				mpszDest->Append((uiChanges & CPPPC_ADDRESS) ? " address" : "");
				mpszDest->Append((uiChanges & CPPPC_MEMBER_CALL) ? " member call" : "");
			}
			mpszDest->AppendNewLine();
		}
		pcChild->VisitFields(this);
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Checks szSource with a kept check and prints what it remembers of the
// expressions (CTestKeptCheckPrinter).
//
//////////////////////////////////////////////////////////////////////////
static void TestCppKeptCheckPrint(const char* szSource, CChars* pszDest, bool bDeclare = false)
{
	CTranslationUnit		cFile;
	CCPPPreprocessor		cPreprocessor;
	CCPPTokeniser			cTokeniser;
	CCPPSyntaxTree			cTree;
	CCPPParser				cParser;
	CCPPSemanticPass		cPass;
	CTestKeptCheckPrinter	cPrinter;

	cFile.Init("Kept.cpp", NULL, false);
	cFile.SetContents((char*)szSource);
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
	AssertFalse(cPass.IsKept());
	cPass.Check(&cTree, CPPT_Host, true);
	AssertString("", cPass.GetErrors()->Text());
	AssertTrue(cPass.IsKept());
	cPrinter.mpcPass = &cPass;
	cPrinter.mpszDest = pszDest;
	cPrinter.mbDeclare = bDeclare;
	cTree.GetRoot()->VisitFields(&cPrinter);
	cPass.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A kept check (CCPPSemanticPass::Check's bKeep) remembers the type of
// every expression, which is written as C++ (a class in its namespace, a
// template's instance with its arguments, an enum by its name), and how
// what a name names is changed where it is used: assigned, bound to a
// reference that isn't const, its address taken, or a member function
// that isn't const called on it - through its members and elements too.
// A check that isn't kept remembers none of it.
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassKept(void)
{
	CChars				sz;
	CCPPSemanticPass	cPass;

	sz.Init();
	TestCppKeptCheckPrint("\
namespace NS\n\
{\n\
	class CBar\n\
	{\n\
	public:\n\
		int		miValue;\n\
		void	Set(int i);\n\
		int		Get(void) const;\n\
	};\n\
	enum EKind { K_A, K_B };\n\
}\n\
template<class T, int N> class CList\n\
{\n\
public:\n\
	T*		mpFirst;\n\
	T*		First(void);\n\
	int		Count(void) const;\n\
};\n\
typedef unsigned int uint32;\n\
void Fill(int& i, const NS::CBar& cIn, NS::CBar& cOut);\n\
void Take(int* pi);\n\
int (*gpfCallback)(char);\n\
int Use(NS::CBar* pcBar, CList<NS::CBar, 4>& cList, const char* sz, NS::EKind eKind)\n\
{\n\
	int			i;\n\
	int			j;\n\
	uint32		ui;\n\
	NS::CBar	cBar;\n\
	NS::CBar	cOther;\n\
	char		ac[4];\n\
	float		f;\n\
\n\
	i = pcBar->Get();\n\
	ui = i + 1u;\n\
	f = i * 2.0f;\n\
	Fill(j, cBar, cOther);\n\
	Take(&i);\n\
	cBar.Set(i);\n\
	cBar.Get();\n\
	cOther.miValue = 2;\n\
	ac[1] = sz[0];\n\
	(j)++;\n\
	pcBar->Set(ui);\n\
	return cList.First()->miValue + cList.Count() + gpfCallback(ac[0]) + (eKind == NS::K_B) + (int)f;\n\
}\n", &sz);
	AssertString("\
i = pcBar->Get() : int\n\
i : int assigned\n\
pcBar->Get() : int\n\
pcBar : NS::CBar*\n\
ui = i + 1u : unsigned int\n\
ui : unsigned int assigned\n\
i + 1u : unsigned int\n\
i : int\n\
1u : unsigned int\n\
f = i * 2.0f : float\n\
f : float assigned\n\
i * 2.0f : float\n\
i : int\n\
2.0f : float\n\
Fill(j, cBar, cOther) : void\n\
j : int reference\n\
cBar : NS::CBar\n\
cOther : NS::CBar reference\n\
Take(&i) : void\n\
&i : int*\n\
i : int address\n\
cBar.Set(i) : void\n\
cBar : NS::CBar member call\n\
i : int\n\
cBar.Get() : int\n\
cBar : NS::CBar\n\
cOther.miValue = 2 : int\n\
cOther.miValue : int\n\
cOther : NS::CBar assigned\n\
2 : int\n\
ac[1] = sz[0] : char\n\
ac[1] : char\n\
ac : char* assigned\n\
1 : int\n\
sz[0] : const char\n\
sz : const char*\n\
0 : int\n\
(j)++ : int\n\
(j) : int\n\
j : int assigned\n\
pcBar->Set(ui) : void\n\
pcBar : NS::CBar*\n\
ui : unsigned int\n\
cList.First()->miValue + cList.Count() + gpfCallback(ac[0]) + (eKind == NS::K_B) + (int)f : int\n\
cList.First()->miValue + cList.Count() + gpfCallback(ac[0]) + (eKind == NS::K_B) : int\n\
cList.First()->miValue + cList.Count() + gpfCallback(ac[0]) : int\n\
cList.First()->miValue + cList.Count() : int\n\
cList.First()->miValue : int\n\
cList.First() : NS::CBar*\n\
cList : CList<NS::CBar, 4> member call\n\
cList.Count() : int\n\
cList : CList<NS::CBar, 4>\n\
gpfCallback(ac[0]) : int\n\
gpfCallback : int (*)(char)\n\
ac[0] : char\n\
ac : char*\n\
0 : int\n\
(eKind == NS::K_B) : bool\n\
eKind == NS::K_B : bool\n\
eKind : NS::EKind\n\
NS::K_B : NS::EKind\n\
(int)f : int\n\
f : float\n\
", sz.Text());
	sz.Kill();

	//Declarations: pointers against the type, a pointer to a function or an array in parentheses with the name, an
	//array's sizes after it.  What is in a namespace without a name is written without one; a class without a name
	//can't be written.
	sz.Init();
	TestCppKeptCheckPrint("\
namespace\n\
{\n\
	class CHidden\n\
	{\n\
	public:\n\
		int		mi;\n\
	};\n\
}\n\
typedef struct\n\
{\n\
	int		a;\n\
} SUnnamed;\n\
int Add(int a, int b);\n\
char gac[3][5];\n\
void Use(SUnnamed* psUnnamed)\n\
{\n\
	int					(*pf)(int, int);\n\
	void				(*pfNone)(void);\n\
	CHidden				c;\n\
	const char* const*	ppsz;\n\
	char				(*pac)[5];\n\
	volatile long long	lli;\n\
	unsigned char*		puc;\n\
\n\
	pf = Add;\n\
	pfNone = 0;\n\
	c.mi = 1;\n\
	psUnnamed->a = 2;\n\
	pac = gac;\n\
	ppsz = 0;\n\
	lli = 0;\n\
	puc = 0;\n\
}\n", &sz, true);
	AssertString("\
pf : int (*x)(int, int)\n\
pfNone : void (*x)(void)\n\
c : CHidden x\n\
psUnnamed : ?\n\
pac : char (*x)[5]\n\
gac : char (*x)[5]\n\
ppsz : const char* const* x\n\
lli : volatile long long x\n\
puc : unsigned char* x\n\
", sz.Text());
	sz.Kill();

	//Without a kept check nothing can be written.
	cPass.Init();
	sz.Init();
	AssertFalse(cPass.AppendType(NULL, &sz));
	AssertString("", sz.Text());
	sz.Kill();
	cPass.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A kept check types a template's body as it is written, the template's
// parameters standing for themselves (CCPPUnit::CheckTemplatesAsThemselves):
// M* and CHolder<M, N>*, whatever it is instantiated with, and when
// nothing instantiates it.  What only an argument would have (pv->Init(),
// pv->miCount) has no type; what is around it and in it still has.  Code
// that isn't in a template is typed as ever, an instance by its arguments.
//
//////////////////////////////////////////////////////////////////////////
void TestCppSemanticPassTemplateAsItself(void)
{
	CChars	sz;

	sz.Init();
	TestCppKeptCheckPrint("\
class CFoo\n\
{\n\
public:\n\
	int		miCount;\n\
	void	Init(void);\n\
};\n\
template<class M, int N>\n\
class CHolder\n\
{\n\
public:\n\
	M*		mp;\n\
	M		mItem;\n\
	int		miUsed;\n\
	M		maItems[N];\n\
\n\
	M*				Get(int i) { return &maItems[i]; }\n\
	void			Start(void);\n\
	int				Count(void) const { return miUsed; }\n\
	CHolder<M, N>*	Self(void) { return this; }\n\
};\n\
template<class M, int N>\n\
void CHolder<M, N>::Start(void)\n\
{\n\
	M*	pv;\n\
	int	i;\n\
\n\
	pv = Get(0);\n\
	mp = pv;\n\
	i = Count() + N;\n\
	mItem = *pv;\n\
	pv->Init();\n\
	miUsed = pv->miCount + i;\n\
	Self()->Start();\n\
}\n\
template<class T>\n\
T* Step(T* pt, int iCount)\n\
{\n\
	return pt + iCount;\n\
}\n\
template<class T>\n\
class CNeverUsed\n\
{\n\
public:\n\
	T		mValue;\n\
\n\
	T*		Address(void) { return &mValue; }\n\
};\n\
int Use(void)\n\
{\n\
	CHolder<CFoo, 4>	c;\n\
\n\
	c.Start();\n\
	return c.Count();\n\
}\n", &sz);
	AssertString("\
&maItems[i] : M*\n\
maItems[i] : M\n\
maItems : M* address\n\
i : int\n\
miUsed : const int\n\
this : CHolder<M, N>*\n\
&maItems[i] : M*\n\
maItems[i] : M\n\
maItems : M* address\n\
i : int\n\
miUsed : const int\n\
this : CHolder<M, N>*\n\
pv = Get(0) : M*\n\
pv : M* assigned\n\
Get(0) : M*\n\
0 : int\n\
mp = pv : M*\n\
mp : M* assigned\n\
pv : M*\n\
i = Count() + N : int\n\
i : int assigned\n\
Count() + N : int\n\
Count() : int\n\
N : int\n\
mItem = *pv : M\n\
mItem : M assigned\n\
*pv : M\n\
pv : M*\n\
pv : M*\n\
miUsed : int assigned\n\
pv : M*\n\
i : int\n\
Self()->Start() : void\n\
Self() : CHolder<M, N>*\n\
pt + iCount : T*\n\
pt : T*\n\
iCount : int\n\
&mValue : T*\n\
mValue : T address\n\
&mValue : T*\n\
mValue : T address\n\
4 : int\n\
4 : int\n\
c.Start() : void\n\
c : CHolder<CFoo, 4> member call\n\
c.Count() : int\n\
c : CHolder<CFoo, 4>\n\
", sz.Text());
	sz.Kill();
}


//The files the units indexed so far have indexed, as the IDE's index keeps them (CIDECodeModel::IsIndexedByAnother):
//a unit gathered with these skips their names, but for what its semantic pass resolved.
class CTestIndexedFiles : public CCPPIndexedFiles
{
public:
	CArrayChars	maszFiles;

	bool	IsIndexed(CSourceFile* pcFile) override
	{
		return maszFiles.Contains(pcFile->FullName());
	}
};


//////////////////////////////////////////////////////////////////////////
//
// Indexes szSource as the unit szFileName, which can include pcHeaders'
// files, as the IDE indexes a unit: checked, and gathered without the
// names of the files the units before it indexed.
//
//////////////////////////////////////////////////////////////////////////
static void IndexUnitWithHeaders(CCPPIndex* pcIndex, CTestIndexedFiles* pcIndexed, CHeaderFiles* pcHeaders, const char* szFileName, const char* szSource)
{
	CTranslationUnit	cFile;
	CCPPPreprocessor	cPreprocessor;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPPSemanticPass	cPass;
	CCPPIndexUnit		cGathered;
	size				i;

	cFile.Init((char*)szFileName, NULL, false);
	cFile.SetContents((char*)szSource);
	cPreprocessor.Init(NULL);
	cPreprocessor.AddIncludeDirectory(pcHeaders);
	AssertTrue(cPreprocessor.PreprocessTranslationUnit(&cFile));
	cPreprocessor.Kill();
	cTokeniser.Init();
	AssertTrue(cTokeniser.Tokenise(cFile.GetTokens()));
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	cParser.Parse();
	cParser.Kill();

	cPass.Init();
	cPass.Check(&cTree);
	AssertString("", cPass.GetErrors()->Text());
	cGathered.Init();
	CCPPIndex::Gather(&cTokeniser, &cTree, &cGathered, &cPass, pcIndexed);
	pcIndex->Merge(&cGathered);
	for (i = 0; i < cGathered.mcFiles.NumElements(); i++)
	{
		if (!pcIndexed->maszFiles.Contains(cGathered.mcFiles.GetKey(i)))
		{
			pcIndexed->maszFiles.Add(cGathered.mcFiles.GetKey(i));
		}
	}
	cGathered.Kill();
	cPass.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The places szKey is named in the file whose name ends szFile, as
// "line:column" each followed by a space, 1 based.
//
//////////////////////////////////////////////////////////////////////////
static void PrintIndexPlacesIn(CCPPIndex* pcIndex, const char* szKey, const char* szFile, CChars* pszDest)
{
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;
	size				i;
	CChars				szName;

	pcSymbol = pcIndex->FindSymbol(szKey);
	if (!pcSymbol)
	{
		pszDest->Append("no symbol");
		return;
	}
	for (i = 0; i < pcSymbol->NumLocations(); i++)
	{
		psLocation = pcSymbol->GetLocation(i);
		szName.Init(pcIndex->GetFileName(psLocation->uiFile));
		if (szName.EndsWith(szFile))
		{
			pszDest->Append(psLocation->iLine + 1)->Append(':')->Append(psLocation->iColumn + 1)->Append(' ');
		}
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The keys of szKey's family (CCPPIndex::FindFamily), each followed by a
// space, then the place it shares with the first of them.
//
//////////////////////////////////////////////////////////////////////////
static void PrintIndexFamily(CCPPIndex* pcIndex, const char* szKey, CChars* pszDest)
{
	CCPPIndexSymbol*	pcSymbol;
	CArrayInt			aiFamily;
	SCPPIndexLocation	sWhere;
	size				i;

	pcSymbol = pcIndex->FindSymbol(szKey);
	if (!pcSymbol)
	{
		pszDest->Append("no symbol");
		return;
	}
	aiFamily.Init();
	if (pcIndex->FindFamily(pcSymbol, &aiFamily, &sWhere) > 0)
	{
		for (i = 0; i < aiFamily.NumElements(); i++)
		{
			pszDest->Append(pcIndex->GetSymbol((size)aiFamily.GetValue(i))->mszKey)->Append(' ');
		}
		pszDest->Append("at ")->Append(sWhere.iLine + 1)->Append(':')->Append(sWhere.iColumn + 1);
	}
	aiFamily.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What a name in a template's body names depends on the template's
// arguments: mp->Init() in CHolder<M> is CFoo::Init in CHolder<CFoo> and
// CBar::Init in CHolder<CBar>.  It is a reference of each it is in any
// instance, in whichever unit the instance is, so Find Usages and Rename
// of CFoo::Init or of CBar::Init have it:
//   - when the unit that indexed the template's header has no instance of
//     it (A.cpp), and other units (B.cpp, C.cpp) have one each;
//   - when one unit (D.cpp) has two instances of a template.
// Those a template names by one name are each other's family (FindFamily):
// the template is written once for them all.  A class with the same
// member that no instance of the template uses (CLone) is in no family,
// nor is a member of the template itself.
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexTemplateInstances(void)
{
	CCPPIndex			cIndex;
	CTestIndexedFiles	cIndexed;
	CHeaderFileMap		cHeaderFiles;
	CHeaderFiles		cHeaders;
	CFileUtil			cFileUtil;
	CChars				szDirectory;
	CChars				sz;

	szDirectory.Init();
	cFileUtil.CurrentDirectory(&szDirectory);
	cFileUtil.AppendToPath(&szDirectory, "Input");
	cFileUtil.AppendToPath(&szDirectory, "SFT");
	cHeaderFiles.Init();
	cHeaders.Init(szDirectory.Text(), &cHeaderFiles, false, false);
	szDirectory.Kill();
	cHeaders.AddFile("Holder.h")->SetContents("\
template<class M> class CHolder\n\
{\n\
public:\n\
	M*		mp;\n\
\n\
	void	Start(void) { mp->Init(); }\n\
};\n\
\n\
\n\
template<class M> class CKeeper\n\
{\n\
public:\n\
	M*		mp;\n\
\n\
	void	Stop(void) { mp->Kill(); }\n\
};\n\
\n\
\n\
class CFoo\n\
{\n\
public:\n\
	void	Init(void);\n\
	void	Kill(void);\n\
};\n\
\n\
\n\
class CBar\n\
{\n\
public:\n\
	void	Init(void);\n\
	void	Kill(void);\n\
};\n\
\n\
\n\
class CLone\n\
{\n\
public:\n\
	void	Init(void);\n\
};\n");

	cIndex.Init();
	cIndexed.maszFiles.Init();
	IndexUnitWithHeaders(&cIndex, &cIndexed, &cHeaders, "A.cpp", "\
#include \"Holder.h\"\n\
void UseA(void)\n\
{\n\
}\n");
	IndexUnitWithHeaders(&cIndex, &cIndexed, &cHeaders, "B.cpp", "\
#include \"Holder.h\"\n\
void UseB(void)\n\
{\n\
	CHolder<CFoo>	c;\n\
\n\
	c.Start();\n\
}\n");
	IndexUnitWithHeaders(&cIndex, &cIndexed, &cHeaders, "C.cpp", "\
#include \"Holder.h\"\n\
void UseC(void)\n\
{\n\
	CHolder<CBar>	c;\n\
\n\
	c.Start();\n\
}\n");
	IndexUnitWithHeaders(&cIndex, &cIndexed, &cHeaders, "D.cpp", "\
#include \"Holder.h\"\n\
void UseD(void)\n\
{\n\
	CKeeper<CFoo>	cFoo;\n\
	CKeeper<CBar>	cBar;\n\
	CLone			cLone;\n\
\n\
	cFoo.Stop();\n\
	cBar.Stop();\n\
	cLone.Init();\n\
}\n");

	//The header's own names are there once, from the unit that indexed it.
	sz.Init();
	PrintIndexPlacesIn(&cIndex, "CHolder", "Holder.h", &sz);
	AssertString("1:25 ", sz.Text());
	sz.Kill();

	//An instance in each of two units that didn't index the header: mp->Init() (6:25) is a reference of both.
	sz.Init();
	PrintIndexPlacesIn(&cIndex, "CFoo::Init()", "Holder.h", &sz);
	AssertString("22:7 6:25 ", sz.Text());
	sz.Kill();
	sz.Init();
	PrintIndexPlacesIn(&cIndex, "CBar::Init()", "Holder.h", &sz);
	AssertString("30:7 6:25 ", sz.Text());
	sz.Kill();

	//Two instances in one unit: mp->Kill() (15:24) is a reference of both.
	sz.Init();
	PrintIndexPlacesIn(&cIndex, "CFoo::Kill()", "Holder.h", &sz);
	AssertString("23:7 15:24 ", sz.Text());
	sz.Kill();
	sz.Init();
	PrintIndexPlacesIn(&cIndex, "CBar::Kill()", "Holder.h", &sz);
	AssertString("31:7 15:24 ", sz.Text());
	sz.Kill();

	//The families: CFoo's and CBar's Init by CHolder's mp->Init(), their Kill by CKeeper's mp->Kill().
	sz.Init();
	PrintIndexFamily(&cIndex, "CFoo::Init()", &sz);
	AssertString("CBar::Init() at 6:25", sz.Text());
	sz.Kill();
	sz.Init();
	PrintIndexFamily(&cIndex, "CBar::Init()", &sz);
	AssertString("CFoo::Init() at 6:25", sz.Text());
	sz.Kill();
	sz.Init();
	PrintIndexFamily(&cIndex, "CBar::Kill()", &sz);
	AssertString("CFoo::Kill() at 15:24", sz.Text());
	sz.Kill();
	sz.Init();
	PrintIndexFamily(&cIndex, "CLone::Init()", &sz);
	AssertString("", sz.Text());
	sz.Kill();
	sz.Init();
	PrintIndexFamily(&cIndex, "CKeeper::Stop()", &sz);
	AssertString("", sz.Text());
	sz.Kill();

	cIndexed.maszFiles.Kill();
	cIndex.Kill();
	cHeaders.Kill();
	cHeaderFiles.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An explicit specialisation of a variable template declares the
// template's name: a declaration of the template's symbol, which Rename and
// Find Usages want (a variable's definition is a declaration too).
//
//////////////////////////////////////////////////////////////////////////
void TestCppIndexVariableTemplateSpecialisation(void)
{
	CCPPIndex			cIndex;
	CCPPIndexSymbol*	pcSymbol;
	SCPPIndexLocation*	psLocation;
	CChars				szPlaces;
	size				i;

	cIndex.Init();
	IndexText(&cIndex, "A.cpp", "\
template<class M> M gmPi = M(3.14159265);\n\
template<> int gmPi<int> = 3;\n\
int Pi(void) { return gmPi<int>; }\n", true);

	pcSymbol = cIndex.FindSymbol("gmPi");
	AssertNotNull(pcSymbol);
	szPlaces.Init();
	for (i = 0; i < pcSymbol->NumLocations(); i++)
	{
		psLocation = pcSymbol->GetLocation(i);
		szPlaces.Append(psLocation->iLine);
		szPlaces.Append(':');
		szPlaces.Append(psLocation->iColumn);
		szPlaces.Append((psLocation->uiRole == CPPIR_Definition) ? " definition" : ((psLocation->uiRole == CPPIR_Declaration) ? " declaration" : " reference"));
		szPlaces.Append("\n");
	}
	AssertString("0:20 declaration\n1:15 declaration\n2:22 reference\n", szPlaces.Text());
	szPlaces.Kill();

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
	TestCppIndexConstructorDefinitions();
	TestCppIndexSemanticPass();
	TestCppSemanticPassQueued();
	TestCppSemanticPassLambdaBodies();
	TestCppSemanticPassDeclarations();
	TestCppSemanticPassUndeclaredTypes();
	TestCppSemanticPassRecordedEnum();
	TestCppSemanticPassHost();
	TestCppSemanticPassEnumeratorChain();
	TestCppSemanticPassKept();
	TestCppSemanticPassTemplateAsItself();
	TestCppIndexTemplateInstances();
	TestCppIndexMacroNames();
	TestCppIndexCLinkage();
	TestCppIndexContainers();
	TestCppIndexUsageTypes();
	TestCppIndexSemanticPassWarnings();
	TestCppIndexOverrides();
	TestCppIndexRemoveFile();
	TestCppIndexRemoveFileHierarchy();
	TestCppIndexVariableTemplateSpecialisation();

	TestStatistics();
}
