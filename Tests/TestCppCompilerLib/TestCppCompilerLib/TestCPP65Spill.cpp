#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "CppCompilerLib/CPP65Assembly.h"
#include "CppCompilerLib/Preprocessor.h"
#include "CppCompilerLib/CppParser.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


#define TEST_CPP65_SPILL_DIRECTORY		"Output" _FS_ "CPP65Spill"


//A spill store in memory that says how many keys it holds.
class CTestCountedSpillStore : public CMemorySpillStore
{
public:
	size	NumKeys(void) { return mcMap.NumElements(); }
};


//////////////////////////////////////////////////////////////////////////
//
// Builds the units (all code kept) for eTarget with the functions' code
// waiting in memory, or in an index tree with a small cache (so pieces are
// written out and read back) when bSpill.  Listings go to szListings.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65SpillBuild(CCPP65Builder* pcBuilder, ECPPTarget eTarget, bool bSpill, const char* szListings)
{
	static const char*	aszUnits[] = { "BaseLib/Chars.cpp", "BaseLib/TimSort.cpp", "BaseLib/MD5.cpp", "BaseLib/RedBlackTree.cpp", "Libc65/string.cpp", NULL };
	CChars				szLibc65;
	CFileUtil			cFileUtil;

	pcBuilder->Init();
	pcBuilder->SetTarget(eTarget);
	TestCPP65AddIncludeDirectories(pcBuilder);
	if (bSpill)
	{
		AssertTrue(pcBuilder->SetSpillDirectory(TEST_CPP65_SPILL_DIRECTORY, 4 KB));
	}
	pcBuilder->SetListingDirectory(szListings);
	pcBuilder->SetRemoveUnused(false);
	pcBuilder->AddStartUp();
	if (eTarget == CPPT_W65C816)
	{
		szLibc65.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
		cFileUtil.FullPath(&szLibc65);
		pcBuilder->SetMathTables(false);  //The runtime without the maths tables: they don't fit with all the code kept.
		pcBuilder->AddRuntime(szLibc65.Text());
		szLibc65.Kill();
	}
	pcBuilder->AddSource("Main.cpp", "int main() { return 0; }\n");
	TestCPP65AddUnits(pcBuilder, aszUnits, false);
	return pcBuilder->Link();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void AssertSameListing(const char* szName)
{
	CTextFile	cMemory;
	CTextFile	cSpilled;
	CChars		szMemory;
	CChars		szSpilled;

	szMemory.Init("Output" _FS_ "CPP65SpillMemory" _FS_);
	szMemory.Append(szName);
	szSpilled.Init("Output" _FS_ "CPP65SpillSpilled" _FS_);
	szSpilled.Append(szName);
	cMemory.Init();
	cSpilled.Init();
	AssertTrue(cMemory.Read(szMemory.Text()));
	AssertTrue(cSpilled.Read(szSpilled.Text()));
	AssertTrue(cMemory.Length() > 1000);
	AssertString(cMemory.Text(), cSpilled.Text());
	cSpilled.Kill();
	cMemory.Kill();
	szSpilled.Kill();
	szMemory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The functions' code waiting in an index tree (written out and read back
// in pieces) gives the same objects, program and listings as in memory.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65SpillSame(ECPPTarget eTarget)
{
	CCPP65Builder	cMemory;
	CCPP65Builder	cSpilled;
	CAsm65Output*	pcMemory;
	CAsm65Output*	pcSpilled;
	SAsm65Segment*	psMemory;
	SAsm65Segment*	psSpilled;
	CFileUtil		cFileUtil;
	size			i;
	bool			bMemory;
	bool			bSpilled;

	cFileUtil.RemoveDir("Output" _FS_ "CPP65SpillMemory");
	cFileUtil.RemoveDir("Output" _FS_ "CPP65SpillSpilled");
	bMemory = TestCPP65SpillBuild(&cMemory, eTarget, false, "Output" _FS_ "CPP65SpillMemory");
	AssertString("", cMemory.GetErrors());
	AssertTrue(bMemory);
	bSpilled = TestCPP65SpillBuild(&cSpilled, eTarget, true, "Output" _FS_ "CPP65SpillSpilled");
	AssertString("", cSpilled.GetErrors());
	AssertTrue(bSpilled);
	AssertTrue(cFileUtil.IsDirectory(TEST_CPP65_SPILL_DIRECTORY));

	AssertString(cMemory.GetMap(), cSpilled.GetMap());
	pcMemory = cMemory.GetOutput();
	pcSpilled = cSpilled.GetOutput();
	AssertSize(pcMemory->NumSegments(), pcSpilled->NumSegments());
	for (i = 0; i < pcMemory->NumSegments(); i++)
	{
		psMemory = pcMemory->GetSegment(i);
		psSpilled = pcSpilled->GetSegment(i);
		AssertInt(psMemory->uiAddress, psSpilled->uiAddress);
		AssertSize(psMemory->cBytes.NumElements(), psSpilled->cBytes.NumElements());
		AssertTrue(memcmp(psMemory->cBytes.GetData(), psSpilled->cBytes.GetData(), psMemory->cBytes.NumElements()) == 0);
	}
	AssertSameListing("Chars.lst");
	AssertSameListing("TimSort.lst");
	AssertSameListing("RedBlackTree.lst");

	cSpilled.Kill();
	AssertFalse(cFileUtil.IsDirectory(TEST_CPP65_SPILL_DIRECTORY));
	cMemory.Kill();
	cFileUtil.RemoveDir("Output" _FS_ "CPP65SpillMemory");
	cFileUtil.RemoveDir("Output" _FS_ "CPP65SpillSpilled");
}


//////////////////////////////////////////////////////////////////////////
//
// Units sharing a store each have their own functions' code: the same
// function index of two units is two keys.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65SpillUnits(void)
{
	CTestCountedSpillStore	cStore;
	uint16					uiFirst;
	uint16					uiSecond;
	CChars					sz;

	cStore.Init();
	uiFirst = 0;
	uiSecond = 0;
	AssertTrue(CCPP65Assembly::PutCode(&cStore, 1, 0, "\t\tnop\n", 6, &uiFirst));
	AssertTrue(CCPP65Assembly::PutCode(&cStore, 2, 0, "\t\trtl\n", 6, &uiSecond));
	AssertSize(2, cStore.NumKeys());
	sz.Init();
	AssertTrue(CCPP65Assembly::GetCode(&cStore, 1, 0, uiFirst, &sz));
	AssertTrue(CCPP65Assembly::GetCode(&cStore, 2, 0, uiSecond, &sz));
	AssertString("\t\tnop\n\t\trtl\n", sz.Text());
	sz.Kill();
	CCPP65Assembly::RemoveCode(&cStore, 1, 0, 0, uiFirst);
	CCPP65Assembly::RemoveCode(&cStore, 2, 0, 0, uiSecond);
	AssertSize(0, cStore.NumKeys());
	cStore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A generator killed before its unit's assembly has its functions' code (a
// unit generated again, CCPP65Builder::RestartGenerating) takes that code
// out of the store; generated whole, the assembly takes it out.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65SpillRestart(bool bWhole)
{
	CTranslationUnit		cFile;
	CCPPPreprocessor			cPreprocessor;
	CCPPTokeniser			cTokeniser;
	CCPPSyntaxTree			cTree;
	CCPPParser				cParser;
	CCPP65Generator			cGenerator;
	CTestCountedSpillStore	cStore;
	CChars					szAssembly;

	cFile.Init("Restart.cpp", NULL, false);
	cFile.SetContents("int Add(int a, int b) { return a + b; }\nint Twice(int a) { return Add(a, a); }\n");
	cPreprocessor.Init(NULL);
	AssertTrue(cPreprocessor.PreprocessTranslationUnit(&cFile));
	cPreprocessor.Kill();
	cTokeniser.Init();
	AssertTrue(cTokeniser.Tokenise(cFile.GetTokens()));
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	AssertTrue(cParser.Parse());
	cParser.Kill();

	cStore.Init();
	cGenerator.Init(&cTree, CPPT_65C64);
	cGenerator.SetSpillStore(&cStore, 3);
	if (bWhole)
	{
		szAssembly.Init();
		AssertTrue(cGenerator.Generate(&szAssembly));
		AssertTrue(szAssembly.Contains("_Add__Fii:"));
		szAssembly.Kill();
	}
	else
	{
		AssertTrue(cGenerator.GenerateBeforeTurn());
		AssertTrue(cStore.NumKeys() > 0);
	}
	cGenerator.Kill();
	AssertSize(0, cStore.NumKeys());
	cStore.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65Spill(void)
{
	BeginTests();

	TestCPP65SpillUnits();
	TestCPP65SpillRestart(false);
	TestCPP65SpillRestart(true);
	TestCPP65SpillSame(CPPT_65C64);
	TestCPP65SpillSame(CPPT_W65C816);

	TestStatistics();
}
