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
#include "Assembler65Lib/Asm65Memory.h"
#include "Assembler65Lib/Asm65HostFiles.h"
#include "MicroprocessorLib/Fast65C64.h"
#include "MicroprocessorLib/FastW65C816.h"
#include "CppCompilerLib/CPP65Builder.h"
#include "CppCompilerLib/CPP65Generator.h"
#include "CppCompilerLib/IRW65C816Backend.h"
#include "ThreadLib/WorkerThreadJobs.h"
#include "TestLib/Assert.h"
#include "TestProject65.h"


//////////////////////////////////////////////////////////////////////////
//
// A test project (TestBaseLib, TestStandardLib) and the CommonLibraries projects it uses are each added to a
// CCPP65Builder's CProject (as a library whose units are only linked when used) with Libc65, and compiled once with
// the real source.  Each test that the test project's main calls is then linked as its own program, the main's
// file with only that test's call left in its main (a whole test project does not fit below the 65C64's stack), and
// run on a CFast65C64.  The programs read and write a copy of the test project's directory through the host file
// device.  For the W65C816 (geTestProject65Target) the units in 65C64 assembly are left out, the memory is 16MB (24
// bit addresses) and the programs run on a CFastW65C816.
//
//////////////////////////////////////////////////////////////////////////


#define TEST_PROJECT65_COMMON			".." _FS_ ".." _FS_ ".." _FS_ "Libraries"
#define TEST_PROJECT65_PROJECTS			TEST_PROJECT65_COMMON _FS_
#define TEST_PROJECT65_TESTS			".." _FS_ ".." _FS_
#define TEST_PROJECT65_LIBC65			".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65"

//64MB, so the heap is from 0100:0000 (past the devices) to 0400:0000.  The W65C816's 16MB has it from 0020:0000 to
//the devices.
#define TEST_PROJECT65_MEMORY_SIZE		0x4000000
#define TEST_PROJECT816_MEMORY_SIZE		0x1000000

//The program ends below the stack (001F:FFFF down): its heap is above 2MB.
#define TEST_PROJECT65_PROGRAM_LIMIT	0x1C0000

//The W65C816's code is bigger: its program may go to 0080:0000, and Libc65's heap starts after it.
#define TEST_PROJECT816_PROGRAM_LIMIT	0x800000

#define TEST_PROJECT65_MAX_STEPS		4000000000ULL


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


static const char* gaszTestProject65Libc65[] = { "string.cpp", "string.asm", "stdlib.cpp", "ctype.cpp", "time.cpp", "math.cpp", "softfloat.cpp", "softfloat.asm", "new.cpp", "signal.cpp", "stdio.cpp", "host.cpp", NULL };
static const char* gaszTestProject816Suffixes[] = { "W65C816.asm", "W65C816_16.asm", NULL };


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
	for (i = 0; (geTestProject65Target == CPPT_W65C816) && psProject->aszExcluded816 && psProject->aszExcluded816[i]; i++)
	{
		if (StringCompare(psProject->aszExcluded816[i], szTest) == 0)
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
// The directory the programs read and write, Output/<name>.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65OutputDirectory(STestProject65* psProject, CChars* pszDest)
{
	pszDest->Init("Output" _FS_);
	pszDest->Append(psProject->szTestProject);
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of the test project's directory (without its Output) for the programs to read and write.
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65CopyDirectory(STestProject65* psProject)
{
	CFileUtil	cFileUtil;
	CArrayChars	aszEntries;
	CChars		szName;
	CChars		szDest;
	CChars		szTestDirectory;
	CChars		szOutput;
	size		i;

	TestProject65TestDirectory(psProject, &szTestDirectory);
	TestProject65OutputDirectory(psProject, &szOutput);
	cFileUtil.RemoveDir(szOutput.Text());
	cFileUtil.MakeDir("Output");
	AssertTrue(cFileUtil.MakeDir(szOutput.Text()));

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
			for (j = 0; gaszTestProject816Suffixes[j]; j++)
			{
				szUnit.Init();
				szUnit.Append(gaszTestProject65Libc65[i], (size)(strrchr(gaszTestProject65Libc65[i], '.') - gaszTestProject65Libc65[i]));
				szUnit.Append(gaszTestProject816Suffixes[j]);
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
	pcBuilder->AddMathTables(szPath.Text());
	szPath.Kill();
	pcBuilder->SetLimit((geTestProject65Target == CPPT_W65C816) ? TEST_PROJECT816_PROGRAM_LIMIT : TEST_PROJECT65_PROGRAM_LIMIT);
	szTestDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends uiAddress and the symbol at or before it (from the linker's map).
//
//////////////////////////////////////////////////////////////////////////
static void TestProject65AppendSymbol(CCPP65Builder* pcBuilder, uint32 uiAddress, CChars* pszDest)
{
	const char*	szMap;
	const char*	szLine;
	const char*	szBest;
	uint32		uiBest;
	uint32		uiValue;
	int			iLength;

	szMap = strstr(pcBuilder->GetMap(), "Symbols\n");
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
static void TestProject65AppendStop(CCPP65Builder* pcBuilder, uint32 uiAddress, uint32 uiS, uint64 uiSteps, CChars* pszDest, CAsm65Memory* pcMemory = NULL)
{
	uint32	uiTop;
	uint32	uiReturn;
	int		iCalls;

	pszDest->Append("Stopped at ");
	TestProject65AppendSymbol(pcBuilder, uiAddress, pszDest);
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
				TestProject65AppendSymbol(pcBuilder, uiReturn - 3, pszDest);
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
// stack, each at its function (pcBuilder's map).  Returns the steps run, to
// the million.
//
//////////////////////////////////////////////////////////////////////////
static uint64 TestProject65RunW65C816(CCPP65Builder* pcBuilder, CAsm65Memory* pcMemory, CFastW65C816* pcMPU, uint64 uiMaxSteps, CChars* pszFaults)
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
			TestProject65AppendSymbol(pcBuilder, cCheck.GetWrite(ui)->uiAddress, pszFaults);
			CAsmW65C816FrameCheck::AppendWrite(cCheck.GetWrite(ui), pszFaults);
		}
		cCheck.Kill();
	}
	return uiSteps;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs the linked program.  pszConsole gets what it printed, and pszFaults
// a W65C816 program's stores to a caller's stack (TestProject65RunW65C816;
// not when it is timed, TEST_PROJECT65_RUN_TIME).  Returns true if main
// returned.
//
//////////////////////////////////////////////////////////////////////////
static bool TestProject65Run(STestProject65* psProject, CCPP65Builder* pcBuilder, CChars* pszConsole, CChars* pszFaults)
{
	CAsm65Memory*		pcMemory;
	CAsm65HostFiles		cHostFiles;
	CFast65C64			cMPU;
	CFastW65C816			cMPU816;
	CAsm65Profiled65C64		cProfiled65C64;
	CAsm65ProfiledW65C816	cProfiled816;
	CChars				szOutput;
	uint64				uiSteps;
	uint64				uiMaxSteps;
	bool				bReturned;
	bool				bW65C816;
	clock_t				iStart;

	//TEST_PROJECT65_STEPS (millions of instructions) gives a slow test longer, for timing it.
	uiMaxSteps = getenv("TEST_PROJECT65_STEPS") ? (uint64)strtoul(getenv("TEST_PROJECT65_STEPS"), NULL, 10) * 1000000 : TEST_PROJECT65_MAX_STEPS;
	bW65C816 = geTestProject65Target == CPPT_W65C816;
	pcMemory = NewMalloc<CAsm65Memory>();
	if (bW65C816)
	{
		pcMemory->Init(TEST_PROJECT816_MEMORY_SIZE);
		pcMemory->Load(pcBuilder->GetOutput());
	}
	else
	{
		pcMemory->Init(TEST_PROJECT65_MEMORY_SIZE);
		pcMemory->LoadProgram(pcBuilder->GetOutput(), CPP65_PROGRAM_ADDRESS);
	}
	pcMemory->SetConsole(pszConsole);
	cHostFiles.Init();
	TestProject65OutputDirectory(psProject, &szOutput);
	cHostFiles.SetDirectory(szOutput.Text());
	szOutput.Kill();
	pcMemory->SetHostFiles(&cHostFiles);

	//TEST_PROJECT65_RUN_TIME prints how long each program ran on the host (not its compiling or linking).
	iStart = clock();

	if (bW65C816)
	{
		pcMemory->InitFast65816(&cMPU816);
		cMPU816.Reset();
		if (gpcTestProject65Measure)
		{
			//TEST_PROJECT65_PROFILE: each function's direct page accesses, calls and recursion, and its lines' cycles
			//(CAsm65Profile).
			cProfiled816.Init(pcMemory, &cMPU816);
			uiSteps = gpcTestProject65Measure->Run(&cProfiled816, pcBuilder->GetMap(), uiMaxSteps, pcBuilder->GetLineTable());
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
			for (uiSteps = 0; (uiSteps < uiMaxSteps) && !cMPU816.IsStopped() && (cMPU816.GetProgramAddress() >= 0xFF00); uiSteps++)
			{
				auiTrace[uiTrace % 64] = cMPU816.GetProgramAddress();
				auiTraceS[uiTrace % 64] = cMPU816.GetS();
				uiTrace++;
				cMPU816.Step();
			}
			for (ui = (uiTrace > 64) ? uiTrace - 64 : 0; ui < uiTrace; ui++)
			{
				pszConsole->Append("  ");
				TestProject65AppendSymbol(pcBuilder, auiTrace[ui % 64], pszConsole);
				pszConsole->Append(" S 0x");
				pszConsole->AppendHexHiLo(&auiTraceS[ui % 64], sizeof(uint32));
				pszConsole->AppendNewLine();
			}
		}
		else
		{
			uiSteps = TestProject65RunW65C816(pcBuilder, pcMemory, &cMPU816, uiMaxSteps, getenv("TEST_PROJECT65_RUN_TIME") ? NULL : pszFaults);
		}
		bReturned = cMPU816.IsStopped() && (cMPU816.GetS() == GetCPPTarget(CPPT_W65C816)->GetStackTop());
		if (getenv("TEST_PROJECT65_CYCLES"))
		{
			printf("CYCLES %llu\n", (unsigned long long)cMPU816.NumCycles());
			fflush(stdout);
		}
		if (!bReturned)
		{
			TestProject65AppendStop(pcBuilder, cMPU816.GetProgramAddress(), cMPU816.GetS(), uiSteps / 1000000, pszConsole, pcMemory);
		}
		cMPU816.Kill();
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
				TestProject65AppendSymbol(pcBuilder, auiTrace[ui % 64], pszConsole);
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
			TestProject65AppendStop(pcBuilder, cMPU.GetProgramCounter(), cMPU.GetS(), uiSteps / 1000000, pszConsole);
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
	CChars			szConsole;
	CChars			szFaults;
	CChars			szMissing;
	CChars			szExpected;
	CChars			szActual;
	size			i;
	int				iRun;
	int				iFailed;
	int				iTotalRun;
	size			uiPrograms;
	bool			bReturned;
	bool			bLinked;
	bool			bCompiled;

	TestProject65CopyDirectory(psProject);

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
	//TEST_PROJECT65_PREFER_SIZE=1: the compiler prefers size to speed (CCPPCompileOptions::mePrefer).
	if (getenv("TEST_PROJECT65_PREFER_SIZE"))
	{
		cBuilder.GetOptions()->mePrefer = CPPP_Size;
	}
	if (getenv("TEST_PROJECT65_LISTINGS"))
	{
		//TEST_PROJECT65_LISTINGS=directory: each unit's assembly with its C++ source lines as comments (Unit.lst).
		cBuilder.SetListingDirectory(getenv("TEST_PROJECT65_LISTINGS"));
	}
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

	iTotalRun = 0;
	uiPrograms = 0;
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

		//The test's name, and how it went if it did not pass.
		szExpected.Init(aszTests.GetText(i));
		szActual.Init(aszTests.GetText(i));
		szConsole.Init();
		szFaults.Init();
		if (!bLinked)
		{
			szActual.Append(" did not link: ");
			szActual.Append(cBuilder.GetErrors());
		}
		else if (!szMissing.Empty())
		{
			szActual.Append(" is missing: ");
			szActual.Append(szMissing);
		}
		else
		{
			uiPrograms++;
			if (getenv("TEST_PROJECT65_CYCLES"))
			{
				//With each program's cycles (TestProject65Run), its size: the bytes of its linked image.
				printf("SIZE %llu %s\n", (unsigned long long)cBuilder.GetOutput()->NumBytes(), aszTests.GetText(i));
			}
			bReturned = TestProject65Run(psProject, &cBuilder, &szConsole, &szFaults);
			iRun = TestProject65ConsoleNumber(&szConsole, "Total Tests Run: ");
			iFailed = TestProject65ConsoleNumber(&szConsole, "Total Failed: ");
			if (!bReturned || (iFailed > 0) || !szFaults.Empty())
			{
				szActual.Append(bReturned ? " failed:\n" : " did not finish:\n");
				szActual.Append(szFaults);
				szActual.Append(szConsole);
			}
			iTotalRun += (iRun > 0) ? iRun : 0;
		}
		AssertString(szExpected.Text(), szActual.Text());
		szMissing.Kill();
		szFaults.Kill();
		szConsole.Kill();
		szActual.Kill();
		szExpected.Kill();
	}
	printf("%s: %d programs, %d asserts\n", psProject->szTestProject, (int)uiPrograms, iTotalRun);
	if (!getenv("TEST_PROJECT65_ONLY"))
	{
		if (geTestProject65Target == CPPT_W65C816)
		{
			AssertTrue(uiPrograms >= psProject->uiMinPrograms816);
			AssertTrue(iTotalRun >= psProject->iMinTestsRun816);
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
// The 65C64 unless the test's command line is "816".
//
//////////////////////////////////////////////////////////////////////////
void TestProject65ChooseTarget(int argc, char** argv)
{
	geTestProject65Target = ((argc > 1) && (strcmp(argv[1], "816") == 0)) ? CPPT_W65C816 : CPPT_65C64;
}

