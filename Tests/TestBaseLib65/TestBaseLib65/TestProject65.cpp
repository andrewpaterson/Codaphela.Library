#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Assembler65Lib/Asm65816Profile.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "Assembler65Lib/Asm65HostFiles.h"
#include "MicroprocessorLib/Fast65C64.h"
#include "MicroprocessorLib/Fast65C816.h"
#include "CppParserLib/CPP65Builder.h"
#include "CppParserLib/CPP65Generator.h"
#include "TestLib/Assert.h"
#include "TestProject65.h"


//////////////////////////////////////////////////////////////////////////
//
// A test project (TestBaseLib, TestStandardLib) and the commonlibraries projects it uses are each added to a
// CCPP65Builder's CProject (as a library whose units are only linked when used) with Libc65, and compiled once with
// the real source.  Each test that the test project's main calls is then linked as its own program, the main's
// file with only that test's call left in its main (a whole test project does not fit below the 65C64's stack), and
// run on a CFast65C64.  The programs read and write a copy of the test project's directory through the host file
// device.  For the W65C816 (geTestProject65Target) the units in 65C64 assembly are left out, the memory is 16MB (24
// bit addresses) and the programs run on a CFast65C816.
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
static CAsm65816Profile*	gpcTestProject65Measure = NULL;

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
static const char* gaszTestProject816Suffixes[] = { "816.asm", "816_16.asm", NULL };


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
			//The W65C816's own assembly for a 65C64 assembly unit (X816.asm for X.asm, and X816_16.asm with 16 bit
			//counts), if it has some.
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
		uiTop = GetCPPTarget(CPPT_W65C816)->uiStackTop;
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
// Runs the linked program.  pszConsole gets what it printed.  Returns true if main returned.
//
//////////////////////////////////////////////////////////////////////////
static bool TestProject65Run(STestProject65* psProject, CCPP65Builder* pcBuilder, CChars* pszConsole)
{
	CAsm65Memory*		pcMemory;
	CAsm65HostFiles		cHostFiles;
	CFast65C64			cMPU;
	CFast65C816			cMPU816;
	CChars				szOutput;
	uint64				uiSteps;
	uint64				uiMaxSteps;
	bool				bReturned;
	bool				bW65C816;

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

	//Run takes a size, which is 32 bits in a 32 bit build.
	if (bW65C816)
	{
		pcMemory->InitFast65816(&cMPU816);
		cMPU816.Reset();
		if (gpcTestProject65Measure)
		{
			//TEST_PROJECT65_PROFILE: each function's direct page accesses, calls and recursion, and its lines' cycles
			//(CAsm65816Profile).
			uiSteps = gpcTestProject65Measure->Run(pcMemory, &cMPU816, pcBuilder->GetMap(), uiMaxSteps, pcBuilder->GetLineTable());
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
			for (uiSteps = 0; (uiSteps < uiMaxSteps) && !cMPU816.IsStopped(); uiSteps += 1000000)
			{
				cMPU816.Run(1000000);
			}
		}
		bReturned = cMPU816.IsStopped() && (cMPU816.GetS() == GetCPPTarget(CPPT_W65C816)->uiStackTop);
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
		if (getenv("TEST_PROJECT65_TRACE"))
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
		for (uiSteps = 0; (uiSteps < uiMaxSteps) && !cMPU.IsStopped() && !getenv("TEST_PROJECT65_TRACE"); uiSteps += 1000000)
		{
			cMPU.Run(1000000);
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
	CAsm65816Profile	cFrameProfile;
	CAsm65816Profile	cMeasure;
	CTextFile		cFile;
	CArrayChars		aszLines;
	CArrayChars		aszTests;
	CChars			szText;
	CChars			szMain;
	CChars			szMainFile;
	CChars			szConsole;
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
	//TEST_PROJECT65_CPP=1: the W65C816 build compiles the C++ of units that have X816.asm, not the assembly.
	CCPP65Generator::SetW65C816Assembly(getenv("TEST_PROJECT65_CPP") == NULL);
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
	TestProject65AddProjects(psProject, &cBuilder);
	bCompiled = cBuilder.Compile();
	AssertString("", cBuilder.GetErrors());
	AssertTrue(bCompiled);
	printf("%s", cBuilder.GetWarnings());

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
			bReturned = TestProject65Run(psProject, &cBuilder, &szConsole);
			iRun = TestProject65ConsoleNumber(&szConsole, "Total Tests Run: ");
			iFailed = TestProject65ConsoleNumber(&szConsole, "Total Failed: ");
			if (!bReturned || (iFailed > 0))
			{
				szActual.Append(bReturned ? " failed:\n" : " did not finish:\n");
				szActual.Append(szConsole);
			}
			iTotalRun += (iRun > 0) ? iRun : 0;
		}
		AssertString(szExpected.Text(), szActual.Text());
		szMissing.Kill();
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

