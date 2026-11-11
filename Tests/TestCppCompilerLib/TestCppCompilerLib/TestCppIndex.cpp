#include "CppCompilerLib/Preprocessor.h"
#include "CppCompilerLib/CppTokeniser.h"
#include "CppCompilerLib/CppParser.h"
#include "CppCompilerLib/CPPIndex.h"
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
// Check of szSource, whose functions nothing calls.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCppSemanticPassCheck(const char* szSource)
{
	CTranslationUnit	cFile;
	CPreprocessor		cPreprocessor;
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
// The source of an enum whose 78 enumerators are each the one before plus
// one, as urlmon.h's tagBINDSTATUS's are, and of a function with a
// parameter of it (declaring it works out the enum's underlying type).
// szMain is appended after it.
//
//////////////////////////////////////////////////////////////////////////
void TestCppEnumeratorChainSource(CChars* pszDest, const char* szMain)
{
	int	i;

	pszDest->Append("typedef enum tagSTATUS\n{\n\tSTATUS_0 = 1,\n");
	for (i = 1; i < 78; i++)
	{
		pszDest->Append("\tSTATUS_");
		pszDest->Append(i);
		pszDest->Append(" = ( STATUS_");
		pszDest->Append(i - 1);
		pszDest->Append(" + 1 ) ,\n");
	}
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
}


//////////////////////////////////////////////////////////////////////////
//
// A class whose name a macro makes, as BaseLib's ARRAY_BLOCK makes
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
	TestCppSemanticPassQueued();
	TestCppSemanticPassHost();
	TestCppSemanticPassEnumeratorChain();
	TestCppIndexMacroNames();
	TestCppIndexContainers();
	TestCppIndexUsageTypes();
	TestCppIndexOverrides();
	TestCppIndexRemoveFile();
	TestCppIndexRemoveFileHierarchy();

	TestStatistics();
}
