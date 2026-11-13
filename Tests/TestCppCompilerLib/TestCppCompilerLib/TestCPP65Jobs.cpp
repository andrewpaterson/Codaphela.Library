#include "BaseLib/FileUtil.h"
#include "ThreadLib/WorkerThreadJobs.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "TestLib/Assert.h"
#include "TestCPP65Context.h"


#define TEST_CPP65_JOBS_THREADS		4


//////////////////////////////////////////////////////////////////////////
//
// Starts a build of the units for eTarget, its units compiled by pcJobs
// (NULL for one at a time).
//
//////////////////////////////////////////////////////////////////////////
static bool TestCPP65JobsBegin(CCPP65Builder* pcBuilder, ECPPTarget eTarget, CJobs* pcJobs, const char** aszUnits)
{
	CChars		szLibc65;
	CFileUtil	cFileUtil;
	bool		bResult;

	pcBuilder->Init();
	pcBuilder->SetTarget(eTarget);
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
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The two builders built the same program: the same map, errors, warnings
// and bytes.  (How many header declarations were left unparsed isn't the
// same: units parsed at the same time find fewer in the header index.)
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65JobsAssertSame(CCPP65Builder* pcSerial, CCPP65Builder* pcParallel)
{
	CAsm65Output*	pcSerialOutput;
	CAsm65Output*	pcParallelOutput;
	SAsm65Segment*	psSerial;
	SAsm65Segment*	psParallel;
	size			i;

	AssertString(pcSerial->GetErrors(), pcParallel->GetErrors());
	AssertString(pcSerial->GetWarnings(), pcParallel->GetWarnings());
	AssertString(pcSerial->GetMap(), pcParallel->GetMap());
	AssertSize(pcSerial->NumCompiledAgain(), pcParallel->NumCompiledAgain());
	pcSerialOutput = pcSerial->GetOutput();
	pcParallelOutput = pcParallel->GetOutput();
	AssertSize(pcSerialOutput->NumSegments(), pcParallelOutput->NumSegments());
	for (i = 0; (i < pcSerialOutput->NumSegments()) && (i < pcParallelOutput->NumSegments()); i++)
	{
		psSerial = pcSerialOutput->GetSegment(i);
		psParallel = pcParallelOutput->GetSegment(i);
		AssertInt(psSerial->uiAddress, psParallel->uiAddress);
		AssertSize(psSerial->cBytes.NumElements(), psParallel->cBytes.NumElements());
		if (psSerial->cBytes.NumElements() == psParallel->cBytes.NumElements())
		{
			AssertTrue(memcmp(psSerial->cBytes.GetData(), psParallel->cBytes.GetData(), psSerial->cBytes.NumElements()) == 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The JobsProject compiled a unit at a time and by four threads gives the
// same program.  On the W65C816 First.cpp is compiled again (Twice, in
// Last.cpp, was exported after it), and so is Middle.cpp: the Outer it
// inlines (exported in First.cpp's first turn) calls Twice too.  Main.cpp,
// after Last.cpp, finds all of them.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65JobsProject(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/JobsProject/First.cpp", "Input/CPP65/JobsProject/Middle.cpp", "Input/CPP65/JobsProject/Last.cpp", "Input/CPP65/JobsProject/Main.cpp", NULL };
	CWorkerThreadJobs	cJobs;
	CCPP65Builder		cSerial;
	CCPP65Builder		cParallel;
	CCPP65Builder		cForced;
	STestCPP65Run		sRun;
	ECPPTarget			eTarget;
	int					i;
	bool				bSerial;
	bool				bParallel;

	cJobs.Init(TEST_CPP65_JOBS_THREADS);
	AssertInt(TEST_CPP65_JOBS_THREADS, cJobs.NumThreads());
	for (i = 0; i < 2; i++)
	{
		eTarget = (i == 0) ? CPPT_65C64 : CPPT_W65C816;
		bSerial = TestCPP65JobsBegin(&cSerial, eTarget, NULL, aszUnits);
		bSerial &= cSerial.Link();
		AssertString("", cSerial.GetErrors());
		AssertTrue(bSerial);

		bParallel = TestCPP65JobsBegin(&cParallel, eTarget, &cJobs, aszUnits);
		bParallel &= cParallel.Link();
		AssertString("", cParallel.GetErrors());
		AssertTrue(bParallel);

		TestCPP65JobsAssertSame(&cSerial, &cParallel);
		AssertSize((eTarget == CPPT_W65C816) ? 2 : 0, cParallel.NumCompiledAgain());

		//Every W65C816 unit generated again in its turn, as if it had missed a function exported while it inlined.
		TestCPP65JobsBegin(&cForced, eTarget, &cJobs, aszUnits);
		cForced.SetForceImportsMissed(true);
		AssertTrue(cForced.Link());
		AssertString("", cForced.GetErrors());
		TestCPP65JobsAssertSame(&cSerial, &cForced);
		if (eTarget == CPPT_W65C816)
		{
			//Each of the four units' turns, and the two compiled again.
			AssertSize(4 + 2, cForced.NumImportsMissed());
		}
		else
		{
			AssertSize(0, cForced.NumImportsMissed());
		}
		cForced.Kill();

		if (bParallel && (eTarget == CPPT_65C64))
		{
			TestCPP65BeginBuilt(&sRun, &cParallel);
			AssertTrue(TestCPP65Run(&sRun, 1000000) < 1000000);
			AssertInt((5 * 2 + 1) * 3, (int32)TestCPP65MPU(&sRun)->GetA());
			TestCPP65End(&sRun);
		}
		cParallel.Kill();
		cSerial.Kill();
	}
	cJobs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// BaseLib's index tree units (their headers' declarations shared by the
// units through the header index) compiled a unit at a time and by four
// threads are the same, for both targets.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65JobsBaseLib(void)
{
	static const char*	aszUnits[] = { "BaseLib/IndexTreeFile.cpp", "BaseLib/IndexTreeEvictedList.cpp", "BaseLib/IndexTreeEvicting.cpp", "BaseLib/IndexTreeEvictionStrategy.cpp", NULL };
	CWorkerThreadJobs	cJobs;
	CCPP65Builder		cSerial;
	CCPP65Builder		cParallel;
	CCPP65Builder*		pcBuilder;
	ECPPTarget			eTarget;
	int					i;
	int					j;

	cJobs.Init(TEST_CPP65_JOBS_THREADS);
	for (i = 0; i < 2; i++)
	{
		eTarget = (i == 0) ? CPPT_65C64 : CPPT_W65C816;
		for (j = 0; j < 2; j++)
		{
			pcBuilder = (j == 0) ? &cSerial : &cParallel;
			TestCPP65JobsBegin(pcBuilder, eTarget, (j == 0) ? NULL : &cJobs, aszUnits);
			pcBuilder->SetRemoveUnused(false);
			pcBuilder->AddSource("Main.cpp", "int main() { return 0; }\n");
			pcBuilder->Link();
			AssertString("", pcBuilder->GetErrors());
		}
		AssertTrue(cSerial.NumLazySkipped() > 0);
		TestCPP65JobsAssertSame(&cSerial, &cParallel);
		cParallel.Kill();
		cSerial.Kill();
	}
	cJobs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// With unit reports each unit is reported once as it finishes, by name,
// and on the W65C816 the two units compiled again are reported again.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65JobsUnitReports(void)
{
	static const char*		aszUnits[] = { "Input/CPP65/JobsProject/First.cpp", "Input/CPP65/JobsProject/Middle.cpp", "Input/CPP65/JobsProject/Last.cpp", "Input/CPP65/JobsProject/Main.cpp", NULL };
	CWorkerThreadJobs		cJobs;
	CCPP65Builder			cBuilder;
	CArrayCPP65UnitReports	asReports;
	SCPP65UnitReport*		psReport;
	ECPPTarget				eTarget;
	int						i;
	size					j;
	size					uiFirst;
	size					uiAgain;
	CArrayChars				aszNames;

	cJobs.Init(TEST_CPP65_JOBS_THREADS);
	for (i = 0; i < 2; i++)
	{
		eTarget = (i == 0) ? CPPT_65C64 : CPPT_W65C816;
		TestCPP65JobsBegin(&cBuilder, eTarget, &cJobs, aszUnits);
		cBuilder.SetUnitReports(true);
		AssertTrue(cBuilder.Link());
		AssertString("", cBuilder.GetErrors());

		asReports.Init();
		cBuilder.TakeUnitReports(&asReports);
		aszNames.Init();
		uiFirst = 0;
		uiAgain = 0;
		for (j = 0; j < asReports.NumElements(); j++)
		{
			psReport = asReports.Get(j);
			AssertString("", psReport->szErrors.Text());
			if (psReport->bAgain)
			{
				uiAgain++;
			}
			else
			{
				uiFirst++;
				aszNames.Add(psReport->szName.Text());
			}
		}
		AssertSize(4, uiFirst);
		AssertSize((eTarget == CPPT_W65C816) ? 2 : 0, uiAgain);
		AssertTrue(aszNames.Contains("First.cpp"));
		AssertTrue(aszNames.Contains("Middle.cpp"));
		AssertTrue(aszNames.Contains("Last.cpp"));
		AssertTrue(aszNames.Contains("Main.cpp"));
		aszNames.Kill();
		CCPP65Builder::KillUnitReports(&asReports);

		//Taken: none are left.
		asReports.Init();
		cBuilder.TakeUnitReports(&asReports);
		AssertSize(0, asReports.NumElements());
		CCPP65Builder::KillUnitReports(&asReports);
		cBuilder.Kill();
	}
	cJobs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A project's sources given as full paths, and as a path out of the
// project's directory and back, are the same units as the names relative
// to it (CCPP65Builder::AddProjectSources): hcp65build's plain command
// line passes them as they were typed.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65JobsFullPathSources(void)
{
	static const char*	aszUnits[] = { "Input/CPP65/JobsProject/First.cpp", "Input/CPP65/JobsProject/Middle.cpp", "Input/CPP65/JobsProject/Last.cpp", "Input/CPP65/JobsProject/Main.cpp", NULL };
	CCPP65Builder		cRelative;
	CCPP65Builder		cFull;
	CArrayChars			aszSources;
	CFileUtil			cFileUtil;
	CChars				szDirectory;
	CChars				szPath;
	bool				bResult;

	bResult = TestCPP65JobsBegin(&cRelative, CPPT_65C64, NULL, aszUnits);
	bResult &= cRelative.Link();
	AssertString("", cRelative.GetErrors());
	AssertTrue(bResult);

	szDirectory.Init("Input" _FS_ "CPP65" _FS_ "JobsProject");
	cFileUtil.FullPath(&szDirectory);
	aszSources.Init();
	aszSources.Add("First.cpp");
	szPath.Init(szDirectory);
	cFileUtil.AppendToPath(&szPath, "Middle.cpp");
	aszSources.Add(szPath.Text());
	szPath.Kill();
	aszSources.Add(".." _FS_ "JobsProject" _FS_ "Last.cpp");
	szPath.Init(szDirectory);
	cFileUtil.AppendToPath(&szPath, "Main.cpp");
	aszSources.Add(szPath.Text());
	szPath.Kill();

	cFull.Init();
	cFull.SetTarget(CPPT_65C64);
	TestCPP65AddIncludeDirectories(&cFull);
	bResult = cFull.AddStartUp();
	cFull.AddProjectSources("JobsProject", szDirectory.Text(), &aszSources, false);
	AssertSize(4, cFull.NumUnitsToCompile());
	bResult &= cFull.Link();
	AssertString("", cFull.GetErrors());
	AssertTrue(bResult);
	TestCPP65JobsAssertSame(&cRelative, &cFull);

	cFull.Kill();
	aszSources.Kill();
	szDirectory.Kill();
	cRelative.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65Jobs(void)
{
	BeginTests();

	TestCPP65JobsProject();
	TestCPP65JobsBaseLib();
	TestCPP65JobsUnitReports();
	TestCPP65JobsFullPathSources();

	TestStatistics();
}
