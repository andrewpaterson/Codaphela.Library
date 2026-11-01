#include "CppParserLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


//////////////////////////////////////////////////////////////////////////
//
// The LazyHeadersProject (First.cpp compiled first indexes the headers),
// with header declarations parsed when first used or all parsed.
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65LazyHeadersBuild(CCPP65Builder* pcBuilder, bool bLazy)
{
	static const char*	aszUnits[] = { "Input/CPP65/LazyHeadersProject/First.cpp", "Input/CPP65/LazyHeadersProject/Thing.cpp", "Input/CPP65/LazyHeadersProject/Main.cpp", "Libc65/string.asm", NULL };
	bool				bResult;

	pcBuilder->Init();
	TestCPP65AddIncludeDirectories(pcBuilder);
	pcBuilder->SetLazyHeaders(bLazy);
	bResult = pcBuilder->AddStartUp();
	TestCPP65AddUnits(pcBuilder, aszUnits, true);
	bResult &= pcBuilder->Link();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Header declarations the units after the first leave unparsed until used
// give the same program.  Two cases were wrong before they were fixed: a
// class's members defined outside it after a forward declaration and a
// friend declared it (both stay parsed), and enumerators named before their
// enum (they are names of the enum's declaration in the index).
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65LazyHeadersSame(void)
{
	CCPP65Builder	cEager;
	CCPP65Builder	cLazy;
	CAsm65Output*	pcEager;
	CAsm65Output*	pcLazy;
	SAsm65Segment*	psEager;
	SAsm65Segment*	psLazy;
	STestCPP65Run	sRun;
	size			i;
	bool			bEager;
	bool			bLazy;

	bEager = TestCPP65LazyHeadersBuild(&cEager, false);
	AssertString("", cEager.GetErrors());
	AssertTrue(bEager);
	AssertSize(0, cEager.NumLazySkipped());

	bLazy = TestCPP65LazyHeadersBuild(&cLazy, true);
	AssertString("", cLazy.GetErrors());
	AssertTrue(bLazy);
	AssertTrue(cLazy.NumLazySkipped() > 0);
	AssertTrue(cLazy.NumLazyLoaded() > 0);
	AssertTrue(cLazy.NumLazyLoaded() < cLazy.NumLazySkipped());

	AssertString(cEager.GetMap(), cLazy.GetMap());
	pcEager = cEager.GetOutput();
	pcLazy = cLazy.GetOutput();
	AssertSize(pcEager->NumSegments(), pcLazy->NumSegments());
	for (i = 0; i < pcEager->NumSegments(); i++)
	{
		psEager = pcEager->GetSegment(i);
		psLazy = pcLazy->GetSegment(i);
		AssertInt(psEager->uiAddress, psLazy->uiAddress);
		AssertSize(psEager->cBytes.NumElements(), psLazy->cBytes.NumElements());
		AssertTrue(memcmp(psEager->cBytes.GetData(), psLazy->cBytes.GetData(), psEager->cBytes.NumElements()) == 0);
	}

	if (bLazy)
	{
		TestCPP65BeginBuilt(&sRun, &cLazy);
		AssertTrue(TestCPP65Run(&sRun, 1000000) < 1000000);
		AssertInt(10 + 200 + 3 * 1000 + 7 * 10000, (int32)TestCPP65MPU(&sRun)->GetA());
		TestCPP65End(&sRun);
	}

	cLazy.Kill();
	cEager.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's index tree units, where IndexTreeFile.h's class has a friend
// (the class IndexTreeEvicting.h defines) that other headers also declare
// forward: compiled with lazy header declarations they are the same.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65LazyHeadersFriends(void)
{
	static const char*	aszUnits[] = { "BaseLib/IndexTreeFile.cpp", "BaseLib/IndexTreeEvictedList.cpp", "BaseLib/IndexTreeEvicting.cpp", "BaseLib/IndexTreeEvictionStrategy.cpp", NULL };
	CCPP65Builder		cEager;
	CCPP65Builder		cLazy;
	int					i;
	CCPP65Builder*		pcBuilder;

	for (i = 0; i < 2; i++)
	{
		pcBuilder = (i == 0) ? &cEager : &cLazy;
		pcBuilder->Init();
		TestCPP65AddIncludeDirectories(pcBuilder);
		pcBuilder->SetLazyHeaders(i == 1);
		pcBuilder->SetRemoveUnused(false);
		pcBuilder->AddStartUp();
		pcBuilder->AddSource("Main.cpp", "int main() { return 0; }\n");
		TestCPP65AddUnits(pcBuilder, aszUnits, true);
		pcBuilder->Link();
		AssertString("", pcBuilder->GetErrors());
	}
	AssertTrue(cLazy.NumLazySkipped() > 0);
	AssertString(cEager.GetMap(), cLazy.GetMap());
	cLazy.Kill();
	cEager.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The LazyMembersProject: members defined outside their classes (parsed after
// the class), a header included twice with other macros (two declarations
// start at one place) and a class with a stringified token.  Eager and lazy give the same
// program, and Other.cpp, which names nothing, leaves all of it unparsed.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65LazyHeadersMembers(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/LazyMembersProject/First.cpp", "Input/CPP65/LazyMembersProject/Other.cpp", "Input/CPP65/LazyMembersProject/Main.cpp", "Libc65/string.asm", NULL };
	CCPP65Builder		cEager;
	CCPP65Builder		cLazy;
	CCPP65Builder*		pcBuilder;
	CAsm65Output*		pcEager;
	CAsm65Output*		pcLazy;
	SAsm65Segment*		psEager;
	SAsm65Segment*		psLazy;
	STestCPP65Run		sRun;
	size				i;
	int					j;
	bool				abLinked[2];

	for (j = 0; j < 2; j++)
	{
		pcBuilder = (j == 0) ? &cEager : &cLazy;
		pcBuilder->Init();
		TestCPP65AddIncludeDirectories(pcBuilder);
		pcBuilder->SetLazyHeaders(j == 1);
		pcBuilder->AddStartUp();
		TestCPP65AddUnits(pcBuilder, aszUnits, true);
		abLinked[j] = pcBuilder->Link();
		AssertString("", pcBuilder->GetErrors());
		AssertTrue(abLinked[j]);
	}
	//Each unit after First.cpp skips Box.h's three classes and six members and Scale.h's three
	//functions; Main.cpp parses CBox, CCounter, their members and the functions.
	AssertSize(0, cEager.NumLazySkipped());
	AssertSize(2 * 12, cLazy.NumLazySkipped());
	AssertSize(2 + 5 + 3, cLazy.NumLazyLoaded());

	AssertString(cEager.GetMap(), cLazy.GetMap());
	pcEager = cEager.GetOutput();
	pcLazy = cLazy.GetOutput();
	AssertSize(pcEager->NumSegments(), pcLazy->NumSegments());
	for (i = 0; i < pcEager->NumSegments(); i++)
	{
		psEager = pcEager->GetSegment(i);
		psLazy = pcLazy->GetSegment(i);
		AssertInt(psEager->uiAddress, psLazy->uiAddress);
		AssertSize(psEager->cBytes.NumElements(), psLazy->cBytes.NumElements());
		AssertTrue(memcmp(psEager->cBytes.GetData(), psLazy->cBytes.GetData(), psEager->cBytes.NumElements()) == 0);
	}

	if (abLinked[1])
	{
		TestCPP65BeginBuilt(&sRun, &cLazy);
		AssertTrue(TestCPP65Run(&sRun, 1000000) < 1000000);
		AssertInt(1 + 20 + 300 + 2 * 1000 + 10 * 10000 + 12 * 100000, (int32)TestCPP65MPU(&sRun)->GetA());
		TestCPP65End(&sRun);
	}

	cLazy.Kill();
	cEager.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65LazyHeaders(void)
{
	BeginTests();

	TestCPP65LazyHeadersSame();
	TestCPP65LazyHeadersFriends();
	TestCPP65LazyHeadersMembers();

	TestStatistics();
}
