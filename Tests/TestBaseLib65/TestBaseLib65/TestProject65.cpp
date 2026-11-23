#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "Assembler65Lib/AsmW65C816FrameCheck.h"
#include "Assembler65Lib/Asm65Profile.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/ConditionVariable.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "Assembler65Lib/Asm65HostFiles.h"
#include "MicroprocessorLib/Fast65C64.h"
#include "MicroprocessorLib/FastW65C816.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "CppCompilerLib/CPPUnit.h"
#include "CppCompilerLib/IRW65C816Backend.h"
#include "ThreadLib/WorkerThreadJobs.h"
#include "TestLib/Assert.h"
#include "TestTarget65.h"
#include "TestProject65.h"


//////////////////////////////////////////////////////////////////////////
//
// A test project (TestBaseLib, TestStandardLib) and the CommonLibraries projects it uses are each added to a
// CCPP65Builder's CProject (as a library whose units are only linked when used) with Libc65, and compiled once with
// the real source.  Each test that the test project's main calls is then linked as its own program, the main's
// file with only that test's call left in its main (a whole test project does not fit below the 65C64's stack), and
// run on a CFast65C64, several side by side on worker threads (TEST_PROJECT65_RUNS) while the next is linked, their
// results asserted in order once all have run.  The programs read and write a copy of the test project's directory
// through the host file device, each worker its own copy.  For the W65C816 (geTestProject65Target) the units in 65C64 assembly are left out, the memory is 16MB (24
// bit addresses) and the programs run on a CFastW65C816.
//
//////////////////////////////////////////////////////////////////////////


#define TEST_PROJECT65_COMMON			".." _FS_ ".." _FS_ ".." _FS_ "Libraries"
#define TEST_PROJECT65_PROJECTS			TEST_PROJECT65_COMMON _FS_
#define TEST_PROJECT65_TESTS			".." _FS_ ".." _FS_
#define TEST_PROJECT65_LIBC65			".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65"

//64MB, so the heap is from 0100:0000 (past the devices) to 0400:0000.  The W65C816's 16MB has it from 0020:0000 to
//the devices.
#define TEST_PROJECT65_MEMORY_SIZE			0x4000000
#define TEST_PROJECT_W65C816_MEMORY_SIZE	0x1000000

//The program ends below the stack (001F:FFFF down): its heap is above 2MB.
#define TEST_PROJECT65_PROGRAM_LIMIT	0x1C0000

//The W65C816's code is bigger: its program may go to 0080:0000, and Libc65's heap starts after it.
#define TEST_PROJECT_W65C816_PROGRAM_LIMIT	0x800000

#define TEST_PROJECT65_MAX_STEPS		4000000000ULL

//How many programs run side by side, each on its own thread with its own emulator and copy of the test project's
//directory (TEST_PROJECT65_RUNS=n, 1 for one after another).  The profile, TEST_PROJECT65_CYCLES and
//TEST_PROJECT65_TRACE run them one after another.
#define TEST_PROJECT65_RUNS				4
#define TEST_PROJECT65_MAX_RUNS			16
#define TEST_PROJECT65_NO_WORKER		((size)-1)


ECPPTarget	geTestProject65Target = CPPT_65C64;
static CAsm65Profile*	gpcTestProject65Measure = NULL;

#define TEST_PROJECT65_REPORT_LINES		60		//The most lines and functions a profile report lists.


//////////////////////////////////////////////////////////////////////////
//
// The file TEST_PROJECT65_PROFILE (or TEST_PROJECT65_MEASURE_FRAMES) names
// for the profile, or NULL.
//
//////////////////////////////////////////////////////////////////////////
static const char* TestProject65ProfileFile(void)
{
	return getenv("TEST_PROJECT65_PROFILE") ? getenv("TEST_PROJECT65_PROFILE") : getenv("TEST_PROJECT65_MEASURE_FRAMES");
}


static const char* gaszTestProject65Libc65[] = { "string.cpp", "string.asm", "stdlib.cpp", "ctype.cpp", "time.cpp", "math.cpp", "softfloat.cpp", "softfloat.asm", "new.cpp", "exception.cpp", "signal.cpp", "stdio.cpp", "host.cpp", NULL };
static const char* gaszTestProjectW65C816Suffixes[] = { "W65C816.asm", "W65C816_16.asm", NULL };
static const char* gaszTestProjectW65C816Libc65[] = { "stdlibW65C816.asm", NULL };		//W65C816 assembly no 65C64 unit has: the heap's kernal call.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool TestProject65IsExcluded(STestProject65* psProject, const char* szTest)
{
	size	i;

	for (i = 0; psProject->aszExcluded[i]; i++)
	{
		if (StringCompare(psProject->aszExcluded[i], szTest) == 0)
		{
			return true;
		}
	}
	for (i = 0; (geTestProject65Target == CPPT_W65C816) && psProject->aszExcludedW65C816 && psProject->aszExcludedW65C816[i]; i++)
	{
		if (StringCompare(psProject->aszExcludedW65C816[i], szTest) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szTest is run: always, unless TEST_PROJECT65_ONLY lists the tests
// to run (comma separated, for debugging them; excluded ones too).
//
//////////////////////////////////////////////////////////////////////////
static bool TestProject65IsOnly(const char* szTest)
{
	const char*	szOnly;
	const char*	szFound;
	size		uiLength;

	szOnly = getenv("TEST_PROJECT65_ONLY");
	if (!szOnly)
	{
		return true;
	}
	uiLength = strlen(szTest);
	for (szFound = strstr(szOnly, szTest); szFound; szFound = strstr(szFound + 1, szTest))
	{
		if (((szFound == szOnly) || (szFound[-1] == ',')) && ((szFound[uiLength] == '\0') || (szFound[uiLength] == ',')))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szLine (in main) is a call to a test, Test...();.  pszName gets the test's name.
//
//////////////////////////////////////////////////////////////////////////
static bool TestProject65TestCall(const char* szLine, CChars* pszName)
{
	pszName->Init(szLine);
	pszName->StripWhitespace();
	if (pszName->StartsWith("Test") && pszName->EndsWith("();"))
	{
		pszName->RemoveFromEnd(3);
		return true;
	}
	pszName->Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether szLine starts main.
//
//////////////////////////////////////////////////////////////////////////
static bool TestProject65MainStart(const char* szLine)
{
	CChars	szStripped;
	bool	bMain;

	szStripped.Init(szLine);
	szStripped.StripWhitespace();
	bMain = szStripped.StartsWith("int __cdecl main");
	szStripped.Kill();
	return bMain;
}


//////////////////////////////////////////////////////////////////////////
//
// Each test the main calls, in order (once each).
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65ListTests(CArrayChars* paszLines, CArrayChars* paszTests)
{
	CChars	szName;
	size	i;
	bool	bMain;

	bMain = false;
	for (i = 0; i < paszLines->NumElements(); i++)
	{
		bMain |= TestProject65MainStart(paszLines->GetText(i));
		if (bMain && TestProject65TestCall(paszLines->GetText(i), &szName))
		{
			if (!paszTests->Contains(szName.Text()))
			{
				paszTests->Add(szName.Text());
			}
			szName.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The main's file with only szTest's (first) call left in main.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65SingleTestMain(CArrayChars* paszLines, const char* szTest, CChars* pszDest)
{
	CChars	szName;
	char*	szLine;
	size	i;
	bool	bMain;
	bool	bDone;

	bMain = false;
	bDone = false;
	for (i = 0; i < paszLines->NumElements(); i++)
	{
		szLine = paszLines->GetText(i);
		bMain |= TestProject65MainStart(szLine);

		if (bMain && TestProject65TestCall(szLine, &szName))
		{
			if (!bDone && szName.Equals(szTest))
			{
				pszDest->Append(szLine)->AppendNewLine();
				bDone = true;
			}
			szName.Kill();
		}
		else
		{
			pszDest->Append(szLine)->AppendNewLine();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The number after szKey on the console, or -1.
//
//////////////////////////////////////////////////////////////////////////
static int TestProject65ConsoleNumber(CChars* pszConsole, const char* szKey)
{
	const char*	sz;

	sz = strstr(pszConsole->Text(), szKey);
	return sz ? atoi(sz + strlen(szKey)) : -1;
}


//////////////////////////////////////////////////////////////////////////
//
// The test project's directory, ../../<name>/<name>.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65TestDirectory(STestProject65* psProject, CChars* pszDest)
{
	pszDest->Init(TEST_PROJECT65_TESTS);
	pszDest->Append(psProject->szTestProject);
	pszDest->Append(_FS_);
	pszDest->Append(psProject->szTestProject);
}


//////////////////////////////////////////////////////////////////////////
//
// The directory the programs read and write, Output/<target>/<name>: the
// 65C64's and the W65C816's runs of a test can go at once.  Programs run
// side by side (TEST_PROJECT65_RUNS) have one each: Output/<target>/<name>.n
// for worker n, when uiWorker isn't TEST_PROJECT65_NO_WORKER.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65OutputDirectory(STestProject65* psProject, CChars* pszDest, size uiWorker)
{
	pszDest->Init("Output" _FS_);
	pszDest->Append(TestTarget65Name(geTestProject65Target));
	pszDest->Append(_FS_);
	pszDest->Append(psProject->szTestProject);
	if (uiWorker != TEST_PROJECT65_NO_WORKER)
	{
		pszDest->Append(".");
		pszDest->Append((int)uiWorker);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the test project's directory (without its Output) for the programs to read and write (uiWorker's own,
// see TestProject65OutputDirectory).
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65CopyDirectory(STestProject65* psProject, size uiWorker)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszEntries;
	CChars		szName;
	CChars		szDest;
	CChars		szTestDirectory;
	CChars		szOutput;
	size		i;

	TestProject65TestDirectory(psProject, &szTestDirectory);
	TestProject65OutputDirectory(psProject, &szOutput, uiWorker);
	cFileUtil.RemoveDir(szOutput.Text());
	AssertTrue(cFileUtil.TouchDir(szOutput.Text()));

	aszEntries.Init();
	cFileUtil.FindAllDirectories(szTestDirectory.Text(), &aszEntries, true);
	for (i = 0; i < aszEntries.NumElements(); i++)
	{
		szName.Init(aszEntries.GetText(i));
		cFileUtil.RemovePath(&szName);
		if (!szName.Equals("Output"))
		{
			szDest.Init(szOutput);
			cFileUtil.AppendToPath(&szDest, szName.Text());
			AssertTrue(cFileUtil.CopyDir(aszEntries.GetText(i), szDest.Text()));
			szDest.Kill();
		}
		szName.Kill();
	}
	aszEntries.Kill();

	//The tests' own Output (empty: what an earlier run left there is not copied), which some tests make directories in.
	szDest.Init(szOutput);
	cFileUtil.AppendToPath(&szDest, "Output");
	AssertTrue(cFileUtil.MakeDir(szDest.Text()));
	szDest.Kill();

	aszEntries.Init();
	cFileUtil.FindAllFiles(szTestDirectory.Text(), &aszEntries, false, true);
	for (i = 0; i < aszEntries.NumElements(); i++)
	{
		szName.Init(aszEntries.GetText(i));
		cFileUtil.RemovePath(&szName);
		szDest.Init(szOutput);
		cFileUtil.AppendToPath(&szDest, szName.Text());
		AssertTrue(cFileUtil.Copy(aszEntries.GetText(i), szDest.Text()));
		szDest.Kill();
		szName.Kill();
	}
	aszEntries.Kill();
	szOutput.Kill();
	szTestDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The projects as libraries of the builder's CProject, and Libc65.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65AddProjects(STestProject65* psProject, CCPP65Builder* pcBuilder)
{
	CFileUtil	cFileUtil;
	CChars		szPath;
	CChars		szTestDirectory;
	CChars		szMain;
	CChars		szUnit;
	CChars		szUnitPath;
	size		i;
	size		j;

	TestProject65TestDirectory(psProject, &szTestDirectory);
	szPath.Init(TEST_PROJECT65_LIBC65);
	cFileUtil.FullPath(&szPath);
	pcBuilder->AddIncludeDirectory(szPath.Text(), false, true);
	szPath.Kill();
	szPath.Init(TEST_PROJECT65_COMMON);
	cFileUtil.FullPath(&szPath);
	pcBuilder->AddIncludeDirectory(szPath.Text(), true, false);
	szPath.Kill();
	for (i = 0; psProject->aszLibraries[i]; i++)
	{
		szPath.Init(TEST_PROJECT65_PROJECTS);
		szPath.Append(psProject->aszLibraries[i]);
		cFileUtil.FullPath(&szPath);
		pcBuilder->AddIncludeDirectory(szPath.Text(), false, false);
		szPath.Kill();
	}
	szPath.Init(szTestDirectory);
	cFileUtil.FullPath(&szPath);
	pcBuilder->AddIncludeDirectory(szPath.Text(), false, false);
	szPath.Kill();

	AssertTrue(pcBuilder->AddStartUp());

	//The main's file is replaced for each test.
	szPath.Init(szTestDirectory);
	szPath.Append(_FS_);
	szPath.Append(psProject->szTestProject);
	szPath.Append(".vcxproj");
	szMain.Init(psProject->szTestProject);
	szMain.Append(".cpp");
	AssertTrue(pcBuilder->AddVisualStudioProject(psProject->szTestProject, szPath.Text(), true, szMain.Text()));
	szMain.Kill();
	szPath.Kill();
	for (i = 0; psProject->aszLibraries[i]; i++)
	{
		szPath.Init(TEST_PROJECT65_PROJECTS);
		szPath.Append(psProject->aszLibraries[i]);
		szPath.Append(_FS_);
		szPath.Append(psProject->aszLibraries[i]);
		szPath.Append(".vcxproj");
		AssertTrue(pcBuilder->AddVisualStudioProject(psProject->aszLibraries[i], szPath.Text(), true));
		szPath.Kill();
	}

	szPath.Init(TEST_PROJECT65_LIBC65);
	cFileUtil.FullPath(&szPath);
	for (i = 0; gaszTestProject65Libc65[i]; i++)
	{
		if ((geTestProject65Target == CPPT_65C64) || (strcmp(strrchr(gaszTestProject65Libc65[i], '.'), ".asm") != 0))
		{
			pcBuilder->AddUnit("Libc65", szPath.Text(), gaszTestProject65Libc65[i], true);
		}
		else
		{
			//The W65C816's own assembly for a 65C64 assembly unit (XW65C816.asm for X.asm, and XW65C816_16.asm with 16 bit
			//counts), if it has some.  softfloat.cpp and math.cpp compile softfloatW65C816.asm's functions themselves
			//when the C++ is used instead of the assembly (TEST_PROJECT65_CPP).
			if (!pcBuilder->GetOptions()->mbAssemblyUnits && (strcmp(gaszTestProject65Libc65[i], "softfloat.asm") == 0))
			{
				continue;
			}
			for (j = 0; gaszTestProjectW65C816Suffixes[j]; j++)
			{
				szUnit.Init();
				szUnit.Append(gaszTestProject65Libc65[i], (size)(strrchr(gaszTestProject65Libc65[i], '.') - gaszTestProject65Libc65[i]));
				szUnit.Append(gaszTestProjectW65C816Suffixes[j]);
				szUnitPath.Init(szPath);
				szUnitPath.Append(_FS_);
				szUnitPath.Append(szUnit);
				if (cFileUtil.Exists(szUnitPath.Text()))
				{
					pcBuilder->AddUnit("Libc65", szPath.Text(), szUnit.Text(), true);
				}
				szUnitPath.Kill();
				szUnit.Kill();
			}
		}
	}
	for (i = 0; (geTestProject65Target == CPPT_W65C816) && gaszTestProjectW65C816Libc65[i]; i++)
	{
		pcBuilder->AddUnit("Libc65", szPath.Text(), gaszTestProjectW65C816Libc65[i], true);
	}
	pcBuilder->AddMathTables(szPath.Text());
	szPath.Kill();
	pcBuilder->SetLimit((geTestProject65Target == CPPT_W65C816) ? TEST_PROJECT_W65C816_PROGRAM_LIMIT : TEST_PROJECT65_PROGRAM_LIMIT);
	szTestDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends uiAddress and the symbol at or before it (from szLinkerMap, the
// linker's map).
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65AppendSymbol(const char* szLinkerMap, uint32 uiAddress, CChars* pszDest)
{
	const char*	szMap;
	const char*	szLine;
	const char*	szBest;
	uint32		uiBest;
	uint32		uiValue;
	int			iLength;

	szMap = strstr(szLinkerMap, "Symbols\n");
	szBest = NULL;
	uiBest = 0;
	for (szLine = szMap; szLine; szLine = strchr(szLine + 1, '\n'))
	{
		//  0x00012345 G Name (object), or L for a local (but not a block's label, L and digits).
		if ((szLine[1] == ' ') && (szLine[3] == '0') && (szLine[4] == 'x') && ((szLine[14] == 'G') || ((szLine[14] == 'L') && !((szLine[16] == 'L') && (szLine[17] >= '0') && (szLine[17] <= '9')))))
		{
			uiValue = (uint32)strtoul(&szLine[3], NULL, 16);
			if ((uiValue <= uiAddress) && (uiValue >= uiBest))
			{
				uiBest = uiValue;
				szBest = &szLine[16];
			}
		}
	}
	pszDest->Append("0x");
	pszDest->AppendHexHiLo(&uiAddress, sizeof(uint32));
	if (szBest)
	{
		iLength = (int)(strchr(szBest, ' ') - szBest);
		pszDest->Append(" (");
		pszDest->Append(szBest, iLength);
		pszDest->Append(" + ");
		pszDest->Append(uiAddress - uiBest);
		pszDest->Append(")");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Appends where a program that didn't return stopped: the address, the symbol at or before it (from the linker's
// map), S and how many instructions ran.  On the W65C816 (pcMemory given) the return addresses on the stack (three
// bytes after a jsl) are its calls, innermost first.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65AppendStop(const char* szMap, uint32 uiAddress, uint32 uiS, uint64 uiSteps, CChars* pszDest, CAsm65Memory* pcMemory = NULL)
{
	uint32	uiTop;
	uint32	uiReturn;
	int		iCalls;

	pszDest->Append("Stopped at ");
	TestProject65AppendSymbol(szMap, uiAddress, pszDest);
	pszDest->Append(", S 0x");
	pszDest->AppendHexHiLo(&uiS, sizeof(uint32));
	pszDest->Append(", after ");
	pszDest->Append((int64)uiSteps);
	pszDest->Append(" million instructions\n");

	if (pcMemory)
	{
		uiTop = GetCPPTarget(CPPT_W65C816)->GetStackTop();
		iCalls = 0;
		for (; (uiS + 3 <= uiTop) && (iCalls < 12); uiS++)
		{
			uiReturn = pcMemory->GetByte(uiS + 1) | ((uint32)pcMemory->GetByte(uiS + 2) << 8) | ((uint32)pcMemory->GetByte(uiS + 3) << 16);
			if ((uiReturn >= 0x10003) && (uiReturn < 0x800000) && (pcMemory->GetByte(uiReturn - 3) == 0x22))
			{
				pszDest->Append("  called from ");
				TestProject65AppendSymbol(szMap, uiReturn - 3, pszDest);
				pszDest->AppendNewLine();
				iCalls++;
				uiS += 2;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a W65C816 program (pcMPU reset on pcMemory) up to uiMaxSteps
// instructions, a million at a time (Run takes a size, which is 32 bits in
// a 32 bit build).  With pszFaults, every function's direct page stores are
// checked (CAsmW65C816FrameCheck) and pszFaults gets those to a caller's
// stack, each at its function (szMap, the linker's map).  Returns the steps
// run, to the million.
//
//////////////////////////////////////////////////////////////////////////
static uint64 TestProject65RunW65C816(const char* szMap, CAsm65Memory* pcMemory, CFastW65C816* pcMPU, uint64 uiMaxSteps, CChars* pszFaults)
{
	CAsmW65C816FrameCheck	cCheck;
	uint64				uiSteps;
	size				ui;

	if (pszFaults)
	{
		cCheck.Init(pcMemory, pcMPU);
	}
	for (uiSteps = 0; (uiSteps < uiMaxSteps) && !pcMPU->IsStopped(); uiSteps += 1000000)
	{
		if (pszFaults)
		{
			cCheck.Run(1000000);
		}
		else
		{
			pcMPU->Run(1000000);
		}
	}
	if (pszFaults)
	{
		for (ui = 0; ui < cCheck.NumWrites(); ui++)
		{
			TestProject65AppendSymbol(szMap, cCheck.GetWrite(ui)->uiAddress, pszFaults);
			CAsmW65C816FrameCheck::AppendWrite(cCheck.GetWrite(ui), pszFaults);
		}
		cCheck.Kill();
	}
	return uiSteps;
}


//////////////////////////////////////////////////////////////////////////
//
// A memory with the builder's linked program in it, to run (a copy: the
// builder links the next program while this one runs).
//
//////////////////////////////////////////////////////////////////////////
static CAsm65Memory* TestProject65Load(CCPP65Builder* pcBuilder)
{
	CAsm65Memory*	pcMemory;

	pcMemory = NewMalloc<CAsm65Memory>();
	if (geTestProject65Target == CPPT_W65C816)
	{
		pcMemory->Init(TEST_PROJECT_W65C816_MEMORY_SIZE);
		pcMemory->Load(pcBuilder->GetOutput());
	}
	else
	{
		pcMemory->Init(TEST_PROJECT65_MEMORY_SIZE);
		pcMemory->LoadProgram(pcBuilder->GetOutput(), CPP65_PROGRAM_ADDRESS);
	}
	return pcMemory;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the program loaded in pcMemory (TestProject65Load), its host files
// in szDirectory, and kills the memory.  pszConsole gets what it printed,
// and pszFaults a W65C816 program's stores to a caller's stack
// (TestProject65RunW65C816; not when it is timed, TEST_PROJECT65_RUN_TIME).
// szMap is its link's map, to name where it stopped; pcBuilder its builder,
// for the profile (whose programs run one at a time).  Returns true if main
// returned.
//
//////////////////////////////////////////////////////////////////////////
static bool TestProject65Run(CCPP65Builder* pcBuilder, CAsm65Memory* pcMemory, const char* szMap, const char* szDirectory, CChars* pszConsole, CChars* pszFaults)
{
	CAsm65HostFiles		cHostFiles;
	CFast65C64			cMPU;
	CFastW65C816			cMPUW65C816;
	CAsm65Profiled65C64		cProfiled65C64;
	CAsm65ProfiledW65C816	cProfiledW65C816;
	uint64				uiSteps;
	uint64				uiMaxSteps;
	bool				bReturned;
	bool				bW65C816;
	clock_t				iStart;

	//TEST_PROJECT65_STEPS (millions of instructions) gives a slow test longer, for timing it.
	uiMaxSteps = getenv("TEST_PROJECT65_STEPS") ? (uint64)strtoul(getenv("TEST_PROJECT65_STEPS"), NULL, 10) * 1000000 : TEST_PROJECT65_MAX_STEPS;
	bW65C816 = geTestProject65Target == CPPT_W65C816;
	pcMemory->SetConsole(pszConsole);
	cHostFiles.Init();
	cHostFiles.SetDirectory(szDirectory);
	pcMemory->SetHostFiles(&cHostFiles);

	//TEST_PROJECT65_RUN_TIME prints how long each program ran on the host (not its compiling or linking).
	iStart = clock();

	if (bW65C816)
	{
		pcMemory->InitFast65816(&cMPUW65C816);
		cMPUW65C816.Reset();
		if (gpcTestProject65Measure)
		{
			//TEST_PROJECT65_PROFILE: each function's direct page accesses, calls and recursion, and its lines' cycles
			//(CAsm65Profile).
			cProfiledW65C816.Init(pcMemory, &cMPUW65C816);
			uiSteps = gpcTestProject65Measure->Run(&cProfiledW65C816, pcBuilder->GetMap(), uiMaxSteps, pcBuilder->GetLineTable());
		}
		else if (getenv("TEST_PROJECT65_TRACE"))
		{
			//TEST_PROJECT65_TRACE: one instruction at a time, stopping when the program counter runs into bank 0 (below
			//the stubs at 0xFF00); the last addresses it was at are printed.
			uint32	auiTrace[64];
			uint32	auiTraceS[64];
			size	uiTrace;
			size	ui;

			uiTrace = 0;
			for (uiSteps = 0; (uiSteps < uiMaxSteps) && !cMPUW65C816.IsStopped() && (cMPUW65C816.GetProgramAddress() >= 0xFF00); uiSteps++)
			{
				auiTrace[uiTrace % 64] = cMPUW65C816.GetProgramAddress();
				auiTraceS[uiTrace % 64] = cMPUW65C816.GetS();
				uiTrace++;
				cMPUW65C816.Step();
			}
			for (ui = (uiTrace > 64) ? uiTrace - 64 : 0; ui < uiTrace; ui++)
			{
				pszConsole->Append("  ");
				TestProject65AppendSymbol(szMap, auiTrace[ui % 64], pszConsole);
				pszConsole->Append(" S 0x");
				pszConsole->AppendHexHiLo(&auiTraceS[ui % 64], sizeof(uint32));
				pszConsole->AppendNewLine();
			}
		}
		else
		{
			uiSteps = TestProject65RunW65C816(szMap, pcMemory, &cMPUW65C816, uiMaxSteps, getenv("TEST_PROJECT65_RUN_TIME") ? NULL : pszFaults);
		}
		bReturned = cMPUW65C816.IsStopped() && (cMPUW65C816.GetS() == GetCPPTarget(CPPT_W65C816)->GetStackTop());
		if (getenv("TEST_PROJECT65_CYCLES"))
		{
			printf("CYCLES %llu\n", (unsigned long long)cMPUW65C816.NumCycles());
			fflush(stdout);
		}
		if (!bReturned)
		{
			TestProject65AppendStop(szMap, cMPUW65C816.GetProgramAddress(), cMPUW65C816.GetS(), uiSteps / 1000000, pszConsole, pcMemory);
		}
		cMPUW65C816.Kill();
	}
	else
	{
		pcMemory->InitFast65C64(&cMPU);
		cMPU.Reset();
		if (gpcTestProject65Measure)
		{
			//TEST_PROJECT65_PROFILE: each function's calls and recursion, and its lines' cycles (CAsm65Profile).
			cProfiled65C64.Init(pcMemory, &cMPU);
			uiSteps = gpcTestProject65Measure->Run(&cProfiled65C64, pcBuilder->GetMap(), uiMaxSteps, pcBuilder->GetLineTable());
		}
		else if (getenv("TEST_PROJECT65_TRACE"))
		{
			//TEST_PROJECT65_TRACE: one instruction at a time, stopping when the program counter leaves the program; the
			//last addresses it was at are printed.
			uint32	auiTrace[64];
			uint32	auiTraceS[64];
			size	uiTrace;
			size	ui;

			uiTrace = 0;
			for (uiSteps = 0; (uiSteps < uiMaxSteps) && !cMPU.IsStopped() && (cMPU.GetProgramCounter() >= CPP65_PROGRAM_ADDRESS); uiSteps++)
			{
				auiTrace[uiTrace % 64] = cMPU.GetProgramCounter();
				auiTraceS[uiTrace % 64] = cMPU.GetS();
				uiTrace++;
				cMPU.Run(1);
			}
			for (ui = (uiTrace > 64) ? uiTrace - 64 : 0; ui < uiTrace; ui++)
			{
				pszConsole->Append("  ");
				TestProject65AppendSymbol(szMap, auiTrace[ui % 64], pszConsole);
				pszConsole->Append(" S 0x");
				pszConsole->AppendHexHiLo(&auiTraceS[ui % 64], sizeof(uint32));
				pszConsole->AppendNewLine();
			}
			uiSteps *= 1000000;
		}
		else
		{
			for (uiSteps = 0; (uiSteps < uiMaxSteps) && !cMPU.IsStopped(); uiSteps += 1000000)
			{
				cMPU.Run(1000000);
			}
		}
		bReturned = cMPU.IsStopped() && (cMPU.GetS() == CPP65_STACK_TOP);
		if (getenv("TEST_PROJECT65_CYCLES"))
		{
			printf("CYCLES %llu\n", (unsigned long long)cMPU.NumCycles());
			fflush(stdout);
		}
		if (!bReturned)
		{
			TestProject65AppendStop(szMap, cMPU.GetProgramCounter(), cMPU.GetS(), uiSteps / 1000000, pszConsole);
		}
		cMPU.Kill();
	}

	if (getenv("TEST_PROJECT65_RUN_TIME"))
	{
		printf("RUN_TIME %.2f\n", (double)(clock() - iStart) / CLOCKS_PER_SEC);
		fflush(stdout);
	}

	cHostFiles.Kill();
	SafeKill(pcMemory);
	return bReturned;
}


//The workers that run programs side by side, each with its own directory (TestProject65OutputDirectory): which are
//running one, under the mutex, the condition woken as one finishes.
struct STestProject65Workers
{
	CMutex				cMutex;
	CConditionVariable	cFinished;
	size				uiWorkers;
	bool				abBusy[TEST_PROJECT65_MAX_RUNS];
	CChars				aszDirectories[TEST_PROJECT65_MAX_RUNS];
};


//A test's program: linked (or why not) and run, its results kept until all the programs have run, then asserted in
//their order, so Log.txt and the counts are as if they had run one after another.
struct STestProject65Program
{
	CChars					szExpected;		//The test's name.
	CChars					szActual;		//The name, and why it didn't link or what was missing.
	bool					bRun;			//It linked with nothing missing, and runs.
	CAsm65Memory*			pcMemory;		//The program loaded, until it has run.
	CChars					szMap;			//Its link's map.
	CChars					szConsole;
	CChars					szFaults;
	bool					bReturned;
	size					uiWorker;
	STestProject65Workers*	psWorkers;
};


//////////////////////////////////////////////////////////////////////////
//
// A worker's job: runs the program in its worker's directory, and frees
// the worker.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65RunProgram(void* pvData)
{
	STestProject65Program*	psProgram;
	STestProject65Workers*	psWorkers;

	psProgram = (STestProject65Program*)pvData;
	psWorkers = psProgram->psWorkers;
	psProgram->bReturned = TestProject65Run(NULL, psProgram->pcMemory, psProgram->szMap.Text(), psWorkers->aszDirectories[psProgram->uiWorker].Text(), &psProgram->szConsole, &psProgram->szFaults);
	psProgram->pcMemory = NULL;
	psWorkers->cMutex.Lock();
	psWorkers->abBusy[psProgram->uiWorker] = false;
	psWorkers->cFinished.WakeAll();
	psWorkers->cMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// A worker that isn't running a program, waiting for one to finish if
// they all are.
//
//////////////////////////////////////////////////////////////////////////
static size TestProject65FreeWorker(STestProject65Workers* psWorkers)
{
	size	ui;

	psWorkers->cMutex.Lock();
	for (;;)
	{
		for (ui = 0; ui < psWorkers->uiWorkers; ui++)
		{
			if (!psWorkers->abBusy[ui])
			{
				psWorkers->abBusy[ui] = true;
				psWorkers->cMutex.Unlock();
				return ui;
			}
		}
		psWorkers->cFinished.Wait(&psWorkers->cMutex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every test the test project's main calls (but the excluded ones) runs to the end without a failure.
//
//////////////////////////////////////////////////////////////////////////
void TestProject65RunTests(STestProject65* psProject)
{
	CCPP65Builder	cBuilder;
	CWorkerThreadJobs	cJobs;
	CAsm65Profile	cFrameProfile;
	CAsm65Profile	cMeasure;
	CTextFile		cFile;
	CArrayChars		aszLines;
	CArrayChars		aszTests;
	CChars			szText;
	CChars			szMain;
	CChars			szMainFile;
	CChars			szMissing;
	STestProject65Workers	sWorkers;
	STestProject65Program*	asPrograms;
	STestProject65Program*	psProgram;
	size			uiNumPrograms;
	size			i;
	size			uiWorker;
	int				iRun;
	int				iFailed;
	int				iTotalRun;
	size			uiPrograms;
	bool			bLinked;
	bool			bCompiled;

	//The programs run side by side on TEST_PROJECT65_RUNS workers (default TEST_PROJECT65_RUNS), each its own copy of
	//the test project's directory; one after another, in the one copy, for the profile, TEST_PROJECT65_CYCLES and
	//TEST_PROJECT65_TRACE (which print as they go, or share the profile).
	sWorkers.uiWorkers = getenv("TEST_PROJECT65_RUNS") ? (size)atoi(getenv("TEST_PROJECT65_RUNS")) : TEST_PROJECT65_RUNS;
	if (TestProject65ProfileFile() || getenv("TEST_PROJECT65_CYCLES") || getenv("TEST_PROJECT65_TRACE") || (sWorkers.uiWorkers < 1))
	{
		sWorkers.uiWorkers = 1;
	}
	if (sWorkers.uiWorkers > TEST_PROJECT65_MAX_RUNS)
	{
		sWorkers.uiWorkers = TEST_PROJECT65_MAX_RUNS;
	}
	sWorkers.cMutex.Init();
	sWorkers.cFinished.Init();
	for (uiWorker = 0; uiWorker < sWorkers.uiWorkers; uiWorker++)
	{
		sWorkers.abBusy[uiWorker] = false;
		TestProject65CopyDirectory(psProject, (sWorkers.uiWorkers == 1) ? TEST_PROJECT65_NO_WORKER : uiWorker);
		TestProject65OutputDirectory(psProject, &sWorkers.aszDirectories[uiWorker], (sWorkers.uiWorkers == 1) ? TEST_PROJECT65_NO_WORKER : uiWorker);
	}
	if (sWorkers.uiWorkers > 1)
	{
		//The 65C64s share their decoded opcodes: all are decoded before they run side by side.
		CFast65C64::DecodeAll();
	}

	cBuilder.Init();
	cBuilder.SetTarget(geTestProject65Target);
	//TEST_PROJECT65_SERIAL=1 compiles the units one at a time (it builds the same); TEST_PROJECT65_THREADS=n compiles
	//them on n threads (the hardware's by default).
	if (getenv("TEST_PROJECT65_THREADS"))
	{
		cJobs.Init(atoi(getenv("TEST_PROJECT65_THREADS")));
	}
	else
	{
		cJobs.Init();
	}
	if (getenv("TEST_PROJECT65_SERIAL") == NULL)
	{
		cBuilder.SetJobs(&cJobs);
	}
	//TEST_PROJECT65_FORCE_MISSES=1: every unit is generated a second time in its turn, as if it had missed an
	//imported function inlining before it (it builds the same).
	cBuilder.SetForceImportsMissed(getenv("TEST_PROJECT65_FORCE_MISSES") != NULL);
	//TEST_PROJECT65_NO_HEADER_CACHE=1: every unit preprocesses every header it includes (it builds the same).
	cBuilder.SetHeaderCache(getenv("TEST_PROJECT65_NO_HEADER_CACHE") == NULL);
	//TEST_PROJECT65_CPP=1: the W65C816 build compiles the C++ of units that have XW65C816.asm, not the assembly.
	cBuilder.GetOptions()->SetAssemblyUnits(getenv("TEST_PROJECT65_CPP") == NULL);
	//TEST_PROJECT65_STACK_ARGUMENTS=1: every W65C816 call pushes its arguments (no register arguments), to compare.
	cBuilder.GetOptions()->mbRegisterArguments = getenv("TEST_PROJECT65_STACK_ARGUMENTS") == NULL;
	//TEST_PROJECT65_FRAME_V=1: the 65C64's V is only ever the frame's register (CCPPCompileOptions::mbHomeV off), to compare.
	cBuilder.GetOptions()->mbHomeV = getenv("TEST_PROJECT65_FRAME_V") == NULL;
	//TEST_PROJECT65_ZERO_W=1: the 65C64's W is always 0 (CCPPCompileOptions::mbHomeW off), to compare.
	cBuilder.GetOptions()->mbHomeW = getenv("TEST_PROJECT65_ZERO_W") == NULL;
	//TEST_PROJECT65_WARN_NARROWING=1: an implicit conversion to a narrower integer is warned about
	//(CCPPCompileOptions::mbWarnNarrowing), to find the counts a 16 bit size_t would cut (Size16Plan.txt).
	cBuilder.GetOptions()->mbWarnNarrowing = getenv("TEST_PROJECT65_WARN_NARROWING") != NULL;
	//TEST_PROJECT65_PREFER_SIZE=1: the compiler prefers size to speed (CCPPCompileOptions::mePrefer).
	if (getenv("TEST_PROJECT65_PREFER_SIZE"))
	{
		cBuilder.GetOptions()->mePrefer = CPPP_Size;
	}
	//TEST_PROJECT65_DEBUG=1: the programs are built for the debugger (CCPPCompileOptions::mbDebuggable, DebuggerPlan.txt A2),
	//and =unoptimised without the optimiser too: they must run as they do built for speed.
	if (getenv("TEST_PROJECT65_DEBUG"))
	{
		cBuilder.GetOptions()->mbDebuggable = true;
		cBuilder.GetOptions()->mbOptimise = strcmp(getenv("TEST_PROJECT65_DEBUG"), "unoptimised") != 0;
	}
	//TEST_PROJECT65_EXCEPTIONS=1: the programs are built with exceptions (CCPPCompileOptions::mbExceptions,
	//ExceptionsPlan.txt), which none of them uses: they must run as they do without.
	if (getenv("TEST_PROJECT65_EXCEPTIONS"))
	{
		cBuilder.GetOptions()->mbExceptions = true;
	}
	if (getenv("TEST_PROJECT65_LISTINGS"))
	{
		//TEST_PROJECT65_LISTINGS=directory: each unit's assembly with its C++ source lines as comments (Unit.lst).
		cBuilder.SetListingDirectory(getenv("TEST_PROJECT65_LISTINGS"));
	}
	//TEST_PROJECT65_LAYOUTS=file: each class the library and its tests lay out, a line each (CCPP65Builder::SetLayoutsFile).
	cBuilder.SetLayoutsFile(getenv("TEST_PROJECT65_LAYOUTS"));
	//TEST_PROJECT65_FRAME_PROFILE builds with a profile (the W65C816's aligned frames measured, not estimated);
	//TEST_PROJECT65_PROFILE (or TEST_PROJECT65_MEASURE_FRAMES) builds with a line table, profiles the programs and
	//writes their profile to the file it names, and TEST_PROJECT65_PROFILE_REPORT writes where their cycles went.
	//TEST_PROJECT65_CYCLES prints each program's cycles.
	cFrameProfile.Init();
	if (getenv("TEST_PROJECT65_FRAME_PROFILE") && cFrameProfile.Read(getenv("TEST_PROJECT65_FRAME_PROFILE")))
	{
		cBuilder.SetFrameProfile(&cFrameProfile);
	}
	cMeasure.Init();
	gpcTestProject65Measure = TestProject65ProfileFile() ? &cMeasure : NULL;
	cBuilder.SetLineTable(gpcTestProject65Measure != NULL);
	//TEST_PROJECT65_COMPILE_STATS: the compile is timed by phase, and its counts and times printed.
	cBuilder.SetTimes(getenv("TEST_PROJECT65_COMPILE_STATS") != NULL);
	TestProject65AddProjects(psProject, &cBuilder);
	bCompiled = cBuilder.Compile();
	//The errors are printed (and flushed) before the assert, which breaks before stdout is written.
	if (!bCompiled || (cBuilder.GetErrors()[0] != '\0'))
	{
		fprintf(stderr, "COMPILE FAILED (compiled %d):\n%s\nEND COMPILE ERRORS\n", (int)bCompiled, cBuilder.GetErrors());
		fflush(stderr);
	}
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bCompiled);
	printf("%s", cBuilder.GetWarnings());
	if (getenv("TEST_PROJECT65_COMPILE_STATS"))
	{
		//TEST_PROJECT65_COMPILE_STATS: the units generated again for a missed import, and compiled again.
		printf("%d imports stale, %d missed, %d compiled again; headers: %d kept, %d replayed (%d tokens); lazy declarations: %d skipped, %d loaded (%d read)\n", (int)cBuilder.NumImportsStale(), (int)cBuilder.NumImportsMissed(), (int)cBuilder.NumCompiledAgain(),
			(int)cBuilder.GetHeaderCache()->NumHeaders(), (int)cBuilder.GetHeaderCache()->NumHits(), (int)cBuilder.GetHeaderCache()->NumReplayedTokens(), (int)cBuilder.NumLazySkipped(), (int)cBuilder.NumLazyLoaded(), (int)cBuilder.NumLazyRead());
		szText.Init();
		cBuilder.GetStatistics()->Print(&szText);
		printf("%s", szText.Text());
		szText.Kill();
		fflush(stdout);
	}

	TestProject65TestDirectory(psProject, &szMainFile);
	szMainFile.Append(_FS_);
	szMainFile.Append(psProject->szTestProject);
	szMainFile.Append(".cpp");
	cFile.Init();
	AssertTrue(cFile.Read(szMainFile.Text()));
	cFile.PassifyNewlines();
	aszLines.Init();
	szText.Init(cFile.Text());
	szText.SplitLines(&aszLines);
	szText.Kill();
	cFile.Kill();
	aszTests.Init();
	TestProject65ListTests(&aszLines, &aszTests);
	AssertTrue(aszTests.NumElements() >= psProject->uiMinPrograms);

	//LinkWith's name is the main's file's name.
	szMainFile.Kill();
	szMainFile.Init(psProject->szTestProject);
	szMainFile.Append(".cpp");

	//Each program is linked here, one after another (the builder's objects are reused), loaded into a memory of its
	//own and run by the next free worker while the next is linked.
	asPrograms = (STestProject65Program*)malloc(sizeof(STestProject65Program) * (aszTests.NumElements() + 1));
	uiNumPrograms = 0;
	for (i = 0; i < aszTests.NumElements(); i++)
	{
		if (!getenv("TEST_PROJECT65_ONLY") && TestProject65IsExcluded(psProject, aszTests.GetText(i)))
		{
			continue;
		}
		if (!TestProject65IsOnly(aszTests.GetText(i)))
		{
			continue;
		}

		szMain.Init();
		TestProject65SingleTestMain(&aszLines, aszTests.GetText(i), &szMain);
		bLinked = cBuilder.LinkWith(szMainFile.Text(), szMain.Text());
		szMain.Kill();

		//A label no object defines is linked as address 0: calling it runs off into bank 0.
		szMissing.Init();
		if (bLinked)
		{
			cBuilder.GetMissingNames(&szMissing);
		}

		//The test's name, and how it went if it did not link.
		psProgram = &asPrograms[uiNumPrograms];
		uiNumPrograms++;
		psProgram->szExpected.Init(aszTests.GetText(i));
		psProgram->szActual.Init(aszTests.GetText(i));
		psProgram->szConsole.Init();
		psProgram->szFaults.Init();
		psProgram->szMap.Init();
		psProgram->pcMemory = NULL;
		psProgram->bReturned = false;
		psProgram->psWorkers = &sWorkers;
		psProgram->bRun = false;
		if (!bLinked)
		{
			psProgram->szActual.Append(" did not link: ");
			psProgram->szActual.Append(cBuilder.GetErrors());
		}
		else if (!szMissing.Empty())
		{
			psProgram->szActual.Append(" is missing: ");
			psProgram->szActual.Append(szMissing);
		}
		else
		{
			psProgram->bRun = true;
			psProgram->pcMemory = TestProject65Load(&cBuilder);
			psProgram->szMap.Append(cBuilder.GetMap());
			if (sWorkers.uiWorkers == 1)
			{
				if (getenv("TEST_PROJECT65_CYCLES"))
				{
					//With each program's cycles (TestProject65Run), its size: the bytes of its linked image.
					printf("SIZE %llu %s\n", (unsigned long long)cBuilder.GetOutput()->NumBytes(), aszTests.GetText(i));
				}
				psProgram->bReturned = TestProject65Run(&cBuilder, psProgram->pcMemory, psProgram->szMap.Text(), sWorkers.aszDirectories[0].Text(), &psProgram->szConsole, &psProgram->szFaults);
				psProgram->pcMemory = NULL;
			}
			else
			{
				psProgram->uiWorker = TestProject65FreeWorker(&sWorkers);
				cJobs.Run(TestProject65RunProgram, psProgram);
			}
		}
		szMissing.Kill();
	}
	cJobs.Wait();

	//The results, in the programs' order.
	iTotalRun = 0;
	uiPrograms = 0;
	for (i = 0; i < uiNumPrograms; i++)
	{
		psProgram = &asPrograms[i];
		if (psProgram->bRun)
		{
			uiPrograms++;
			iRun = TestProject65ConsoleNumber(&psProgram->szConsole, "Total Tests Run: ");
			iFailed = TestProject65ConsoleNumber(&psProgram->szConsole, "Total Failed: ");
			if (!psProgram->bReturned || (iFailed > 0) || !psProgram->szFaults.Empty())
			{
				psProgram->szActual.Append(psProgram->bReturned ? " failed:\n" : " did not finish:\n");
				psProgram->szActual.Append(psProgram->szFaults);
				psProgram->szActual.Append(psProgram->szConsole);
			}
			iTotalRun += (iRun > 0) ? iRun : 0;
			if (getenv("TEST_PROJECT65_COUNTS"))
			{
				//TEST_PROJECT65_COUNTS: each program's asserts, to compare runs.
				printf("COUNT %d %s\n", iRun, psProgram->szExpected.Text());
			}
		}
		AssertString(psProgram->szExpected.Text(), psProgram->szActual.Text());
		psProgram->szMap.Kill();
		psProgram->szFaults.Kill();
		psProgram->szConsole.Kill();
		psProgram->szActual.Kill();
		psProgram->szExpected.Kill();
	}
	free(asPrograms);
	for (uiWorker = 0; uiWorker < sWorkers.uiWorkers; uiWorker++)
	{
		sWorkers.aszDirectories[uiWorker].Kill();
	}
	sWorkers.cFinished.Kill();
	sWorkers.cMutex.Kill();
	printf("%s: %d programs, %d asserts\n", psProject->szTestProject, (int)uiPrograms, iTotalRun);
	if (!getenv("TEST_PROJECT65_ONLY"))
	{
		if (geTestProject65Target == CPPT_W65C816)
		{
			AssertTrue(uiPrograms >= psProject->uiMinProgramsW65C816);
			AssertTrue(iTotalRun >= psProject->iMinTestsRunW65C816);
		}
		else
		{
			AssertTrue(uiPrograms >= psProject->uiMinPrograms);
			AssertTrue(iTotalRun >= psProject->iMinTestsRun);
		}
	}

	if (gpcTestProject65Measure)
	{
		AssertTrue(cMeasure.Write(TestProject65ProfileFile()));
		if (getenv("TEST_PROJECT65_PROFILE_REPORT"))
		{
			AssertTrue(cMeasure.WriteReport(getenv("TEST_PROJECT65_PROFILE_REPORT"), TEST_PROJECT65_REPORT_LINES));
		}
		gpcTestProject65Measure = NULL;
	}
	cMeasure.Kill();
	cFrameProfile.Kill();
	szMainFile.Kill();
	aszTests.Kill();
	aszLines.Kill();
	cBuilder.Kill();
	cJobs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The target the test's command line names (TestTarget65Choose).  False,
// having said what it takes, for an argument it doesn't know.
//
//////////////////////////////////////////////////////////////////////////
bool TestProject65ChooseTarget(int argc, char** argv)
{
	return TestTarget65Choose(argc, argv, &geTestProject65Target);
}

