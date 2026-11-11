#include "BaseLib/FileUtil.h"
#include "ThreadLib/WorkerThreadJobs.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


#define TEST_CPP65_HEADER_CACHE_THREADS		4


//////////////////////////////////////////////////////////////////////////
//
// Builds the units for eTarget, with the header cache if bHeaderCache,
// compiled by pcJobs (NULL for one at a time).
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65HeaderCacheBuild(CCPP65Builder* pcBuilder, ECPPTarget eTarget, bool bHeaderCache, CJobs* pcJobs, const char** aszUnits, const char* szMain)
{
	CChars		szLibc65;
	CFileUtil	cFileUtil;
	bool		bResult;

	pcBuilder->Init();
	pcBuilder->SetTarget(eTarget);
	pcBuilder->SetHeaderCache(bHeaderCache);
	pcBuilder->SetJobs(pcJobs);
	TestCPP65AddIncludeDirectories(pcBuilder);
	bResult = pcBuilder->AddStartUp();
	if (eTarget == CPPT_W65C816)
	{
		szLibc65.Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65");
		cFileUtil.FullPath(&szLibc65);
		pcBuilder->AddMathTables(szLibc65.Text());
		szLibc65.Kill();
	}
	TestCPP65AddUnits(pcBuilder, aszUnits, eTarget == CPPT_65C64);
	if (szMain)
	{
		pcBuilder->SetRemoveUnused(false);
		pcBuilder->AddSource("Main.cpp", szMain);
	}
	bResult &= pcBuilder->Link();
	AssertString("", pcBuilder->GetErrors());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The two builders built the same program: the same map, errors, warnings
// and bytes.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65HeaderCacheAssertSame(CCPP65Builder* pcWithout, CCPP65Builder* pcWith)
{
	CAsm65Output*	pcWithoutOutput;
	CAsm65Output*	pcWithOutput;
	SAsm65Segment*	psWithout;
	SAsm65Segment*	psWith;
	size			i;

	AssertString(pcWithout->GetErrors(), pcWith->GetErrors());
	AssertString(pcWithout->GetWarnings(), pcWith->GetWarnings());
	AssertString(pcWithout->GetMap(), pcWith->GetMap());
	pcWithoutOutput = pcWithout->GetOutput();
	pcWithOutput = pcWith->GetOutput();
	AssertSize(pcWithoutOutput->NumSegments(), pcWithOutput->NumSegments());
	for (i = 0; (i < pcWithoutOutput->NumSegments()) && (i < pcWithOutput->NumSegments()); i++)
	{
		psWithout = pcWithoutOutput->GetSegment(i);
		psWith = pcWithOutput->GetSegment(i);
		AssertInt(psWithout->uiAddress, psWith->uiAddress);
		AssertSize(psWithout->cBytes.NumElements(), psWith->cBytes.NumElements());
		if (psWithout->cBytes.NumElements() == psWith->cBytes.NumElements())
		{
			AssertTrue(memcmp(psWithout->cBytes.GetData(), psWith->cBytes.GetData(), psWithout->cBytes.NumElements()) == 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The HeaderCacheProject's units include Cache.h (and Inner.h, with
// #pragma once) with different macros: built with the header cache it is
// the same as without, and the units that include them with the same
// macros as one before get them from the cache.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65HeaderCacheProject(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/HeaderCacheProject/Wide.cpp", "Input/CPP65/HeaderCacheProject/Narrow.cpp", "Input/CPP65/HeaderCacheProject/Narrow2.cpp", "Input/CPP65/HeaderCacheProject/InnerFirst.cpp", "Input/CPP65/HeaderCacheProject/Added.cpp", "Input/CPP65/HeaderCacheProject/Main.cpp", NULL };
	CCPP65Builder		cWithout;
	CCPP65Builder		cWith;
	STestCPP65Run		sRun;
	ECPPTarget			eTarget;
	int					i;
	bool				bWith;

	for (i = 0; i < 2; i++)
	{
		eTarget = (i == 0) ? CPPT_65C64 : CPPT_W65C816;
		AssertTrue(TestCPP65HeaderCacheBuild(&cWithout, eTarget, false, NULL, aszUnits, NULL));
		AssertSize(0, cWithout.GetHeaderCache()->NumHits());
		AssertSize(0, cWithout.GetHeaderCache()->NumHeaders());

		bWith = TestCPP65HeaderCacheBuild(&cWith, eTarget, true, NULL, aszUnits, NULL);
		AssertTrue(bWith);
		TestCPP65HeaderCacheAssertSame(&cWithout, &cWith);

		//Narrow2.cpp and Main.cpp get Cache.h as Narrow.cpp kept it; every unit but Wide.cpp and InnerFirst.cpp gets
		//Inner.h as Wide.cpp kept it (Added.cpp's INNER_ADD is its own: it keeps its own Inner.h and Cache.h).
		AssertTrue(cWith.GetHeaderCache()->NumHits() >= 4);
		AssertTrue(cWith.GetHeaderCache()->NumReplayedTokens() > 0);

		if (bWith && (eTarget == CPPT_65C64))
		{
			TestCPP65BeginBuilt(&sRun, &cWith);
			AssertTrue(TestCPP65Run(&sRun, 1000000) < 1000000);
			AssertInt(23, (int32)TestCPP65MPU(&sRun)->GetA());
			TestCPP65End(&sRun);
		}
		cWith.Kill();
		cWithout.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The NamespaceProject's units get Shapes.h from the cache, with what its
// namespaces declare, and build the same program on both targets.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65HeaderCacheNamespaces(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/NamespaceProject/Shapes.cpp", "Input/CPP65/NamespaceProject/Other.cpp", "Input/CPP65/NamespaceProject/Main.cpp", NULL };
	CCPP65Builder		cWithout;
	CCPP65Builder		cWith;
	int					i;

	for (i = 0; i < 2; i++)
	{
		AssertTrue(TestCPP65HeaderCacheBuild(&cWithout, (i == 0) ? CPPT_65C64 : CPPT_W65C816, false, NULL, aszUnits, NULL));
		AssertTrue(TestCPP65HeaderCacheBuild(&cWith, (i == 0) ? CPPT_65C64 : CPPT_W65C816, true, NULL, aszUnits, NULL));
		TestCPP65HeaderCacheAssertSame(&cWithout, &cWith);
		AssertTrue(cWith.GetHeaderCache()->NumHits() >= 2);
		cWith.Kill();
		cWithout.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's index tree units (which include many of the same headers)
// built with the header cache, a unit at a time and by four threads, are
// the same as without, for both targets.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65HeaderCacheBaseLib(void)
{
	static const char*	aszUnits[] = { "BaseLib/IndexTreeFile.cpp", "BaseLib/IndexTreeEvictedList.cpp", "BaseLib/IndexTreeEvicting.cpp", "BaseLib/IndexTreeEvictionStrategy.cpp", NULL };
	CWorkerThreadJobs	cJobs;
	CCPP65Builder		cWithout;
	CCPP65Builder		cWith;
	ECPPTarget			eTarget;
	int					i;
	int					j;

	cJobs.Init(TEST_CPP65_HEADER_CACHE_THREADS);
	for (i = 0; i < 2; i++)
	{
		eTarget = (i == 0) ? CPPT_65C64 : CPPT_W65C816;
		TestCPP65HeaderCacheBuild(&cWithout, eTarget, false, NULL, aszUnits, "int main() { return 0; }\n");
		for (j = 0; j < 2; j++)
		{
			TestCPP65HeaderCacheBuild(&cWith, eTarget, true, (j == 0) ? NULL : &cJobs, aszUnits, "int main() { return 0; }\n");
			if (j == 0)
			{
				AssertTrue(cWith.GetHeaderCache()->NumHits() > 0);
			}
			TestCPP65HeaderCacheAssertSame(&cWithout, &cWith);
			cWith.Kill();
		}
		cWithout.Kill();
	}
	cJobs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65HeaderCache(void)
{
	BeginTests();

	TestCPP65HeaderCacheProject();
	TestCPP65HeaderCacheNamespaces();
	TestCPP65HeaderCacheBaseLib();

	TestStatistics();
}
