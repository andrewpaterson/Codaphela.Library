#ifndef __TEST_CPP65_CONTEXT_H__
#define __TEST_CPP65_CONTEXT_H__
#include "MicroprocessorLib/Fast65C64.h"
#include "MicroprocessorLib/FastW65C816.h"
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MemoryDrive.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "Assembler65Lib/Asm65DebugInfo.h"
#include "CppCompilerLib/CPPUnit.h"
#include "CppCompilerLib/CPP65Builder.h"


#define TEST_CPP65_RESULTS		0x100000
#define TEST_CPP65_SECTOR_SIZE	512
#define TEST_CPP65_FIRST_SECTOR	1


//A compiled program running from reset in CAsm65Memory: pseudo registers below 0001:0000, the program from
//0001:0000, results at 0010:0000 and the stack from 001F:FFFC down.  A program linked lazily has its units on
//pcDrive (uiUnits of them).
struct STestCPP65Run
{
	CAsm65Memory*	pcContext;
	CMemoryDrive*	pcDrive;
	CFast65C64		cMPU;
	size			uiInstructions;
	size			uiUnits;
	CAsm65DebugInfo*	pcDebugInfo;	//Its records, its stack walked at every step (TestCPP65RunWalking); NULL without a line table.
};


void		TestCPP65AddIncludeDirectories(CCPP65Builder* pcBuilder);
CCPPCompileOptions*	TestCPP65Options(void);
size		TestCPP65NumWritten(void);
bool		TestCPP65Generate(CChars* pszAssembly, CChars* pszErrors, const char* szSource, bool bProgram = false, CArrayChars* paszExternals = NULL, bool bLineTable = false);
int			TestCPP65Assemble(const char* szSource, bool bProgram = true);
int			TestCPP65AssembleLibraryFile(const char* szFilename);
bool		TestCPP65Begin(STestCPP65Run* psRun, const char* szSource);
bool		TestCPP65BeginAssembly(STestCPP65Run* psRun, const char* szAssembly);
bool		TestCPP65BeginBuilt(STestCPP65Run* psRun, CCPP65Builder* pcBuilder);
size		TestCPP65Run(STestCPP65Run* psRun, size uiMaxInstructions);
size		TestCPP65RunWalking(STestCPP65Run* psRun, size uiMaxInstructions);
CFast65C64*	TestCPP65MPU(STestCPP65Run* psRun);
void		TestCPP65End(STestCPP65Run* psRun);
int32		TestCPP65Main(const char* szSource);
int32		TestCPP65MainFile(const char* szFilename);
void		TestCPP65AddUnits(CCPP65Builder* pcBuilder, const char** aszUnits, bool bAssembly);
void		TestCPP65LibrariesDirectory(CChars* pszDest, const char* szLibrary);
bool		TestCPP65BeginProject(STestCPP65Run* psRun, const char** aszUnits, const char* szStubs, const char* szSource = NULL, CChars* pszMap = NULL, bool bLazy = false);
int32		TestCPP65RunProject(const char** aszUnits, const char* szStubs, size uiMaxInstructions = 20000000, const char* szSource = NULL);
int32		TestCPP65RunProjectLockstep(const char** aszUnits, const char* szStubs, size uiMaxInstructions, size* puiInstructions);
int32		TestCPP65RunProjectLazy(const char** aszUnits, const char* szStubs, size* puiUnits, size* puiLoads, const char* szSource = NULL);
int32		TestCPP65MainFloat(const char* szSource);


//A program whose main measures how its classes and objects are laid out, and what it returns on each target: the
//W65C816 aligns nothing past a byte unless alignas asks it and pads a class's members to 2 bytes at most
//(W65C816AlignmentPlan.txt); the 65C64 lays classes out as MSVC does, and aligns an object to 4 bytes at most.
struct STestCPP65Layout
{
	const char*	szSource;
	int32		iW65C816;
	int32		i65C64;
};


extern const STestCPP65Layout	gasTestCPP65Layouts[];	//Ended by a NULL source.
extern const char* gszTestCPP65StaticConstMembers;
extern const char* gszTestCPP65NestedQualifiers;
extern const char* gszTestCPP65LateMemberDefinitions;
size		TestCPP65CompileLibrary(const char* szLibrary, CChars* pszErrors);
size		TestCPP816RunChecked(CAsm65Memory* pcMemory, CFastW65C816* pcFast, size uiMaxSteps, CAsm65DebugRecords* pcRecords = NULL);
size		TestCPP816RunLockstepChecked(CAsm65Memory* pcMemory, CFastW65C816* pcFast, size uiMaxSteps);


#endif // __TEST_CPP65_CONTEXT_H__

