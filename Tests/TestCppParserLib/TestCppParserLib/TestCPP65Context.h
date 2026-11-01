#ifndef __TEST_CPP65_CONTEXT_H__
#define __TEST_CPP65_CONTEXT_H__
#include "MicroprocessorLib/Fast65C64.h"
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MemoryDrive.h"
#include "Assembler65Lib/Asm65Memory.h"
#include "CppParserLib/CPP65Generator.h"
#include "CppParserLib/CPP65Builder.h"


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
};


void		TestCPP65AddIncludeDirectories(CCPP65Builder* pcBuilder);
bool		TestCPP65Generate(CChars* pszAssembly, CChars* pszErrors, const char* szSource, bool bProgram = false, CArrayChars* paszExternals = NULL);
int			TestCPP65Assemble(const char* szSource, bool bProgram = true);
int			TestCPP65AssembleLibraryFile(const char* szFilename);
bool		TestCPP65Begin(STestCPP65Run* psRun, const char* szSource);
bool		TestCPP65BeginAssembly(STestCPP65Run* psRun, const char* szAssembly);
bool		TestCPP65BeginBuilt(STestCPP65Run* psRun, CCPP65Builder* pcBuilder);
size		TestCPP65Run(STestCPP65Run* psRun, size uiMaxInstructions);
CFast65C64*	TestCPP65MPU(STestCPP65Run* psRun);
void		TestCPP65End(STestCPP65Run* psRun);
int32		TestCPP65Main(const char* szSource);
int32		TestCPP65MainFile(const char* szFilename);
void		TestCPP65AddUnits(CCPP65Builder* pcBuilder, const char** aszUnits, bool bAssembly);
bool		TestCPP65BeginProject(STestCPP65Run* psRun, const char** aszUnits, const char* szStubs, const char* szSource = NULL, CChars* pszMap = NULL, bool bLazy = false);
int32		TestCPP65RunProject(const char** aszUnits, const char* szStubs, size uiMaxInstructions = 20000000, const char* szSource = NULL);
int32		TestCPP65RunProjectLockstep(const char** aszUnits, const char* szStubs, size uiMaxInstructions, size* puiInstructions);
int32		TestCPP65RunProjectLazy(const char** aszUnits, const char* szStubs, size* puiUnits, size* puiLoads, const char* szSource = NULL);
int32		TestCPP65MainFloat(const char* szSource);
size		TestCPP65CompileLibrary(const char* szLibrary, CChars* pszErrors);


#endif // __TEST_CPP65_CONTEXT_H__

