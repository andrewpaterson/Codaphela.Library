#include <string.h>
#include <stdlib.h>
#include "Assembler65Lib/Asm65C64Lockstep.h"
#include "Assembler65Lib/Asm65816FrameCheck.h"
#include "Assembler65Lib/Asm65816Lockstep.h"
#include "TestCPP65Context.h"
#include "Assembler65Lib/Asm65C64Assembler.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "CppParserLib/CPP65Generator.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the include directory szRelative (to this test's directory) as a full path.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65AddIncludeDirectory(CCPP65Builder* pcBuilder, const char* szRelative, bool bIncludeSubDirectories, bool bSystem)
{
	CFileUtil	cFileUtil;
	CChars		szDirectory;

	szDirectory.Init(szRelative);
	cFileUtil.FullPath(&szDirectory);
	pcBuilder->AddIncludeDirectory(szDirectory.Text(), bIncludeSubDirectories, bSystem);
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// <stdint.h> and the rest are the 65C64's (Libc65), "BaseLib/..." are the libraries', and the test's own headers
// are in Input/CPP65.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65AddIncludeDirectories(CCPP65Builder* pcBuilder)
{
	TestCPP65AddIncludeDirectory(pcBuilder, ".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_ "Libc65", false, true);
	TestCPP65AddIncludeDirectory(pcBuilder, ".." _FS_ ".." _FS_ ".." _FS_ "Libraries", true, false);
	TestCPP65AddIncludeDirectory(pcBuilder, "Input" _FS_ "CPP65", true, false);
	TestCPP65AddIncludeDirectory(pcBuilder, ".." _FS_ ".." _FS_ ".." _FS_ "Libraries" _FS_ "BaseLib", false, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses, parses and generates szSource, as a whole program with its start up if bProgram, otherwise as
// an object, appended to pszAssembly, and the errors to pszErrors.  The labels it uses but does not define are
// added to paszExternals.
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP65Generate(CChars* pszAssembly, CChars* pszErrors, const char* szSource, bool bProgram, CArrayChars* paszExternals)
{
	CCPP65Builder	cBuilder;
	bool			bResult;

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	bResult = CCPP65Builder::Generate(pszAssembly, pszErrors, cBuilder.GetIncludeDirectories(), "Generated.cpp", szSource, bProgram, paszExternals);
	cBuilder.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles and assembles szSource as a program, loads it and runs reset.
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP65Begin(STestCPP65Run* psRun, const char* szSource)
{
	CChars	szAssembly;
	CChars	szErrors;
	bool	bResult;

	szAssembly.Init();
	szErrors.Init();
	bResult = TestCPP65Generate(&szAssembly, &szErrors, szSource, true);
	AssertString("", szErrors.Text());
	bResult &= TestCPP65BeginAssembly(psRun, szAssembly.Text());
	szErrors.Kill();
	szAssembly.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Loads a linked program into new memory and runs reset.  A program linked lazily
// gets a drive with its units from TEST_CPP65_FIRST_SECTOR.
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP65BeginBuilt(STestCPP65Run* psRun, CCPP65Builder* pcBuilder)
{
	CArrayUint8*	pcImage;

	psRun->pcContext = NewMalloc<CAsm65Memory>();
	psRun->pcContext->Init();
	psRun->uiInstructions = 0;
	psRun->pcContext->LoadProgram(pcBuilder->GetOutput(), CPP65_PROGRAM_ADDRESS);

	psRun->pcDrive = NULL;
	psRun->uiUnits = pcBuilder->NumUnits();
	if (psRun->uiUnits > 0)
	{
		pcImage = pcBuilder->GetLazyImage();
		psRun->pcDrive = NewMalloc<CMemoryDrive>();
		psRun->pcDrive->Init(TEST_CPP65_FIRST_SECTOR * TEST_CPP65_SECTOR_SIZE + pcImage->NumElements(), TEST_CPP65_SECTOR_SIZE);
		memcpy((uint8*)psRun->pcDrive->GetMemory() + TEST_CPP65_FIRST_SECTOR * TEST_CPP65_SECTOR_SIZE, pcImage->GetData(), pcImage->NumElements());
		psRun->pcContext->SetDrive(psRun->pcDrive);
	}

	psRun->pcContext->InitFast65C64(&psRun->cMPU);
	psRun->cMPU.Reset();
	return true;
}


//Libc65's string routines, which programs call (the optimiser makes loops calls of them).
static const char*	gaszTestCPP65String[] = { "Libc65/string.asm", NULL };


//////////////////////////////////////////////////////////////////////////
//
// Assembles a program (one object with its start up), with Libc65's string.asm if it calls a string routine it
// doesn't define, links it, loads it and runs reset.  Another label it uses but doesn't define is a link error.
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP65BeginAssembly(STestCPP65Run* psRun, const char* szAssembly)
{
	static const char*	aszCalls[] = { "_memcpy", "_memset", "_memmove", "_memcmp", "_strlen", "_memchr", "_strchr", NULL };
	CCPP65Builder		cBuilder;
	bool				bResult;
	bool				bString;
	size				i;
	CChars				szLabel;

	cBuilder.Init();
	cBuilder.SetStubMissing(false);
	bResult = cBuilder.AddAssembly("program", szAssembly);

	//Libc65's string routines when the program calls one it doesn't have (the optimiser makes loops calls of them).
	bString = false;
	for (i = 0; aszCalls[i]; i++)
	{
		szLabel.Init(aszCalls[i]);
		szLabel.Append(':');
		bString |= (strstr(szAssembly, aszCalls[i]) != NULL) && (strstr(szAssembly, szLabel.Text()) == NULL);
		szLabel.Kill();
	}
	if (bString)
	{
		TestCPP65AddUnits(&cBuilder, gaszTestCPP65String, true);
	}
	bResult &= cBuilder.Link();
	AssertString("", cBuilder.GetErrors());
	bResult &= TestCPP65BeginBuilt(psRun, &cBuilder);
	cBuilder.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs until the processor stops.  Returns the number of instructions.
//
//////////////////////////////////////////////////////////////////////////
size TestCPP65Run(STestCPP65Run* psRun, size uiMaxInstructions)
{
	size	i;

	i = psRun->cMPU.Run(uiMaxInstructions);
	psRun->uiInstructions += i;
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFast65C64* TestCPP65MPU(STestCPP65Run* psRun)
{
	return &psRun->cMPU;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65End(STestCPP65Run* psRun)
{
	AssertSize(0, psRun->pcContext->NumBadAccesses());
	psRun->cMPU.Kill();
	SafeKill(psRun->pcContext);
	SafeKill(psRun->pcDrive);
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles szSource (as a program with its start up if bProgram) and assembles it as an object.  Returns the
// number of labels it uses but does not define.
//
//////////////////////////////////////////////////////////////////////////
int TestCPP65Assemble(const char* szSource, bool bProgram)
{
	CChars				szAssembly;
	CChars				szErrors;
	CArrayChars			aszExternals;
	CAsm65C64Assembler		cAssembler;
	CAsm65Object		cObject;
	int					iExternals;

	szAssembly.Init();
	szErrors.Init();
	aszExternals.Init();
	AssertTrue(TestCPP65Generate(&szAssembly, &szErrors, szSource, bProgram, &aszExternals));
	AssertString("", szErrors.Text());

	cObject.Init("unit");
	cAssembler.Init();
	AssertTrue(cAssembler.AssembleObject(szAssembly.Text(), &cObject));
	AssertString("", cAssembler.GetErrors());
	cAssembler.Kill();
	cObject.Kill();

	iExternals = (int)aszExternals.NumElements();
	aszExternals.Kill();
	szErrors.Kill();
	szAssembly.Kill();
	return iExternals;
}


//////////////////////////////////////////////////////////////////////////
//
// TestCPP65Assemble on a file of the libraries, as a translation unit.
//
//////////////////////////////////////////////////////////////////////////
//
// The folder libraries are in for the file or library (such as "BaseLib/ArrayBlock.cpp" or "Libc65"), with a separator
// after it: Libc65 and Assembler65Lib are the HCP65 repository's, the rest are CommonLibraries'.
//
//////////////////////////////////////////////////////////////////////////
static void TestCPP65LibrariesDirectory(CChars* pszDest, const char* szLibrary)
{
	if ((strncmp(szLibrary, "Libc65", 6) == 0) || (strncmp(szLibrary, "Assembler65Lib", 14) == 0))
	{
		pszDest->Init(".." _FS_ ".." _FS_ ".." _FS_ ".." _FS_ "HCP65" _FS_ "Libraries" _FS_);
	}
	else
	{
		pszDest->Init(".." _FS_ ".." _FS_ ".." _FS_ "Libraries" _FS_);
	}
}


//////////////////////////////////////////////////////////////////////////
int TestCPP65AssembleLibraryFile(const char* szFilename)
{
	CTextFile	cFile;
	CChars		szPath;
	int			iExternals;

	TestCPP65LibrariesDirectory(&szPath, szFilename);
	szPath.Append(szFilename);
	cFile.Init();
	AssertTrue(cFile.Read(szPath.Text()));
	cFile.PassifyNewlines();
	iExternals = TestCPP65Assemble(cFile.Text(), false);
	cFile.Kill();
	szPath.Kill();
	return iExternals;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles and runs Input/CPP65/szFilename.  Returns what main returned.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP65MainFile(const char* szFilename)
{
	CTextFile	cFile;
	CChars		szPath;
	int32		iResult;

	szPath.Init("Input" _FS_ "CPP65" _FS_);
	szPath.Append(szFilename);
	cFile.Init();
	AssertTrue(cFile.Read(szPath.Text()));
	cFile.PassifyNewlines();
	iResult = TestCPP65Main(cFile.Text());
	cFile.Kill();
	szPath.Kill();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles and runs szSource.  Returns what main returned.  The program must stop within a million instructions
// with the stack where it started.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP65Main(const char* szSource)
{
	STestCPP65Run	sRun;
	int32			iResult;

	AssertTrue(TestCPP65Begin(&sRun, szSource));
	AssertTrue(TestCPP65Run(&sRun, 1000000) < 1000000);
	AssertIntHex(CPP65_STACK_TOP, TestCPP65MPU(&sRun)->GetS());
	iResult = (int32)TestCPP65MPU(&sRun)->GetA();
	TestCPP65End(&sRun);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles every .cpp file of a library under commonlibraries (szLibrary, such as "BaseLib") as a link library,
// with a main that uses none of it, and links.  pszErrors gets the compiler's and linker's errors.  Returns how
// many files were compiled.
//
//////////////////////////////////////////////////////////////////////////
size TestCPP65CompileLibrary(const char* szLibrary, CChars* pszErrors)
{
	CCPP65Builder	cBuilder;
	CFileUtil		cFileUtil;
	CArrayChars		aszFiles;
	CChars			szDirectory;
	CChars*			pszFile;
	size			i;
	size			iSlash;

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	cBuilder.SetStubMissing(true);
	cBuilder.AddStartUp();
	cBuilder.AddSource("Main.cpp", "int main() { return 0; }\n");

	TestCPP65LibrariesDirectory(&szDirectory, szLibrary);
	szDirectory.Append(szLibrary);
	cFileUtil.FullPath(&szDirectory);
	aszFiles.Init();
	cFileUtil.FindFilesWithExtension(szDirectory.Text(), "cpp", &aszFiles);
	aszFiles.QuickSort();
	for (i = 0; i < aszFiles.NumElements(); i++)
	{
		pszFile = aszFiles.Get(i);
		iSlash = pszFile->FindFromEnd(FILE_SEPARATOR[0]);
		cBuilder.AddUnit(szLibrary, szDirectory.Text(), pszFile->Text(iSlash + 1), true);
	}

	cBuilder.Link();
	pszErrors->Append(cBuilder.GetErrors());
	i = aszFiles.NumElements();
	aszFiles.Kill();
	szDirectory.Kill();
	cBuilder.Kill();
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the units (paths under commonlibraries, or under this test's directory when they start with "Input"), each
// to be compiled into an object.  The units in a directory share a CLibrary (their include settings).  Units in
// 65C64 assembly (.asm) are left out unless bAssembly, and units in W65C816 assembly (.asm with 816 in the name:
// string816.asm, string816_16.asm) when it is.
//
//////////////////////////////////////////////////////////////////////////
void TestCPP65AddUnits(CCPP65Builder* pcBuilder, const char** aszUnits, bool bAssembly)
{
	CFileUtil	cFileUtil;
	CChars		szDirectory;
	CChars		szLibrary;
	CChars		szFile;
	size		iSlash;
	size		i;

	for (i = 0; aszUnits[i]; i++)
	{
		szLibrary.Init(aszUnits[i]);
		iSlash = szLibrary.FindFromEnd('/');
		szFile.Init(szLibrary.Text(iSlash + 1));
		szLibrary.SetLength(iSlash);
		if ((szFile.EndsWithIgnoreCase(".asm") && szFile.Contains("816")) ? !bAssembly : (bAssembly || !szFile.EndsWithIgnoreCase(".asm")))
		{
			if (szLibrary.StartsWith("Input"))
			{
				szDirectory.Init(szLibrary);
			}
			else
			{
				TestCPP65LibrariesDirectory(&szDirectory, szLibrary.Text());
				szDirectory.Append(szLibrary);
			}
			szDirectory.Replace('/', FILE_SEPARATOR[0]);
			cFileUtil.FullPath(&szDirectory);
			pcBuilder->AddUnit(szLibrary.Text(), szDirectory.Text(), szFile.Text());
			szDirectory.Kill();
		}
		szFile.Kill();
		szLibrary.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles the units (see TestCPP65AddUnits) and szSource if it is given, each into an object, links them after
// the start up and loads the program.  The labels in szStubs (space separated,
// sorted) must be exactly the ones used but defined by no unit; the linker makes each a stp so a call to one
// stops the program.  pszMap, if given, gets the linker's map.
//
//////////////////////////////////////////////////////////////////////////
bool TestCPP65BeginProject(STestCPP65Run* psRun, const char** aszUnits, const char* szStubs, const char* szSource, CChars* pszMap, bool bLazy)
{
	CCPP65Builder		cBuilder;
	CChars				szMissing;
	size				i;
	bool				bResult;

	cBuilder.Init();
	TestCPP65AddIncludeDirectories(&cBuilder);
	if (bLazy)
	{
		cBuilder.SetLazy(TEST_CPP65_SECTOR_SIZE, TEST_CPP65_FIRST_SECTOR);
	}

	bResult = cBuilder.AddStartUp();
	if (szSource)
	{
		bResult &= cBuilder.AddSource("source", szSource);
	}
	TestCPP65AddUnits(&cBuilder, aszUnits, true);

	//Libc65's string routines (the optimiser makes loops calls of them).
	for (i = 0; aszUnits && aszUnits[i] && !strstr(aszUnits[i], "string.asm"); i++)
	{
	}
	if (!aszUnits || !aszUnits[i])
	{
		TestCPP65AddUnits(&cBuilder, gaszTestCPP65String, true);
	}

	//A program that doesn't build isn't loaded.
	bResult &= cBuilder.Link();
	AssertString("", cBuilder.GetErrors());
	if (bResult)
	{
		TestCPP65BeginBuilt(psRun, &cBuilder);
	}

	//What is used but written by no unit.
	szMissing.Init();
	cBuilder.GetMissingNames(&szMissing);
	AssertString(szStubs, szMissing.Text());
	if (pszMap)
	{
		pszMap->Append(cBuilder.GetMap());
	}

	szMissing.Kill();
	cBuilder.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Builds a project (see TestCPP65BeginProject) and runs it.  Returns A when main returns.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP65RunProject(const char** aszUnits, const char* szStubs, size uiMaxInstructions, const char* szSource)
{
	STestCPP65Run	sRun;
	int32			iResult;

	iResult = 0;
	if (TestCPP65BeginProject(&sRun, aszUnits, szStubs, szSource))
	{
		AssertTrue(TestCPP65Run(&sRun, uiMaxInstructions) < uiMaxInstructions);
		AssertIntHex(CPP65_STACK_TOP, TestCPP65MPU(&sRun)->GetS());
		iResult = (int32)TestCPP65MPU(&sRun)->GetA();
		TestCPP65End(&sRun);
	}
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// TestCPP65RunProject on CFast65C64 and C65C64 side by side, each with its own copy of memory.  The registers
// must agree after every instruction and memory at the end.  *puiInstructions is how many instructions ran.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP65RunProjectLockstep(const char** aszUnits, const char* szStubs, size uiMaxInstructions, size* puiInstructions)
{
	STestCPP65Run	sRun;
	CAsm65C64Lockstep	cLockstep;
	int32			iResult;

	*puiInstructions = 0;
	if (!TestCPP65BeginProject(&sRun, aszUnits, szStubs))
	{
		return 0;
	}

	cLockstep.Init(sRun.pcContext, &sRun.cMPU);
	*puiInstructions = cLockstep.Run(uiMaxInstructions, 0);
	AssertString("", cLockstep.GetDifferences());
	cLockstep.Kill();

	AssertTrue(sRun.cMPU.IsStopped());
	AssertIntHex(CPP65_STACK_TOP, sRun.cMPU.GetS());
	iResult = (int32)sRun.cMPU.GetA();
	TestCPP65End(&sRun);
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// TestCPP65RunProject with the functions loaded when they are first called.  Returns A
// when main returns; *puiUnits is how many units there are and *puiLoads how many were
// loaded.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP65RunProjectLazy(const char** aszUnits, const char* szStubs, size* puiUnits, size* puiLoads, const char* szSource)
{
	STestCPP65Run	sRun;
	int32			iResult;

	iResult = 0;
	*puiUnits = 0;
	*puiLoads = 0;
	if (TestCPP65BeginProject(&sRun, aszUnits, szStubs, szSource, NULL, true))
	{
		AssertTrue(TestCPP65Run(&sRun, 20000000) < 20000000);
		AssertIntHex(CPP65_STACK_TOP, TestCPP65MPU(&sRun)->GetS());
		iResult = (int32)TestCPP65MPU(&sRun)->GetA();
		*puiUnits = sRun.uiUnits;
		*puiLoads = sRun.pcContext->NumDriveReads();
		TestCPP65End(&sRun);
	}
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles and runs szSource with Libc65's soft float runtime.  Returns what main returned.
//
//////////////////////////////////////////////////////////////////////////
int32 TestCPP65MainFloat(const char* szSource)
{
	static const char*	aszUnits[] = { "Libc65/softfloat.cpp", "Libc65/softfloat.asm", NULL };

	return TestCPP65RunProject(aszUnits, "", 20000000, szSource);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a W65C816 program (pcFast reset on pcMemory) up to uiMaxSteps
// instructions, as CFast65C816::Run, checking that no function writes its
// caller's frame through the direct page (CAsm65816FrameCheck).  Returns
// the steps run.
//
//////////////////////////////////////////////////////////////////////////
size TestCPP816RunChecked(CAsm65Memory* pcMemory, CFast65C816* pcFast, size uiMaxSteps)
{
	CAsm65816FrameCheck	cCheck;
	size				uiSteps;

	cCheck.Init(pcMemory, pcFast);
	uiSteps = cCheck.Run(uiMaxSteps);
	AssertString("", cCheck.GetWrites());
	cCheck.Kill();
	return uiSteps;
}


//////////////////////////////////////////////////////////////////////////
//
// TestCPP816RunChecked on the W65C816 simulator and CFast65C816 in
// lockstep (CAsm65816Lockstep): no step may differ either.
//
//////////////////////////////////////////////////////////////////////////
size TestCPP816RunLockstepChecked(CAsm65Memory* pcMemory, CFast65C816* pcFast, size uiMaxSteps)
{
	CAsm65816Lockstep	cLockstep;
	CAsm65816FrameCheck	cCheck;
	size				uiSteps;

	cLockstep.Init(pcMemory, pcFast);
	cCheck.Init(pcMemory, pcFast);
	cLockstep.SetFrameCheck(&cCheck);
	uiSteps = cLockstep.Run(uiMaxSteps, 64);
	AssertString("", cLockstep.GetDifferences());
	AssertString("", cCheck.GetWrites());
	cCheck.Kill();
	cLockstep.Kill();
	return uiSteps;
}
