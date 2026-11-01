#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "CppParserLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


#define TEST_CPP65_SPILL_DIRECTORY		"Output" _FS_ "CPP65Spill"


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
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65Spill(void)
{
	BeginTests();

	TestCPP65SpillSame(CPPT_65C64);
	TestCPP65SpillSame(CPPT_W65C816);

	TestStatistics();
}
