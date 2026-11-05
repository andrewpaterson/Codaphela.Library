/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <time.h>
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ConstructorCall.h"
#include "VisualStudioProject.h"
#include "Assembler65Lib/Asm65C64Assembler.h"
#include "Assembler65Lib/Asm65816Assembler.h"
#include "CPP65IRFrontEnd.h"
#include "Preprocessor.h"
#include "HeaderFileMap.h"
#include "HeaderFiles.h"
#include "CppTokeniser.h"
#include "CppParser.h"
#include "CPP65Generator.h"
#include "CPP65Builder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::Init(void)
{
	macIncludeDirectories.Init();
	macLinkLibraries.Init();
	mcProject.Init(false);
	meTarget = CPPT_65C64;
	muiUnits = 0;
	muiLimit = CPP65_HEAP_ADDRESS;
	mbCompiled = false;
	mbStubMissing = true;
	mbRemoveUnused = true;
	mbLazy = false;
	muiLazySectorSize = 0;
	muiLazyFirstSector = 0;
	mcObjects.Init("objects");
	macObjects.Init();
	mcLinkWithObjects.Init("link with");
	mcLinker.Init();
	mcLinker.SetStubMissing(true);
	mcLinker.SetRemoveUnused(true);
	mszErrors.Init();
	mszWarnings.Init();
	mcImports.Init(IR65_INLINE_LOOP_INSTRUCTIONS);
	mcImports.SetResume(ResumeUnit, this);
	mpcUnitJobs = NULL;
	mpcFrameProfile = NULL;
	mszListings.Init();
	maszOutsideProjects.Init();
	maszOutsideLibraries.Init();
	mbLineTable = false;
	mlliCompileTime = 0;
	mcMemorySpillStore.Init();
	mpcSpillStore = &mcMemorySpillStore;
	mbLazyHeaders = true;
	mcHeaderIndex.Init();
	mcHeaderCache.Init();
	mcProject.SetHeaderCache(&mcHeaderCache);
	muiLazySkipped = 0;
	muiLazyLoaded = 0;
	muiLazyRead = 0;
	mpcJobs = NULL;
	mcCompilingMutex.Init();
	mcCompiled.Init();
	muiCompiling = 0;
	muiUnitsFinished = 0;
	muiCompiledAgain = 0;
	mapcWorkspaces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::Kill(void)
{
	size	i;

	if (mpcSpillStore == &mcIndexTreeSpillStore)
	{
		mcIndexTreeSpillStore.Kill();
	}
	KillWorkspaces();
	mapcWorkspaces.Kill();
	mcCompiled.Kill();
	mcCompilingMutex.Kill();
	mcMemorySpillStore.Kill();
	mcHeaderCache.Kill();
	mcHeaderIndex.Kill();
	maszOutsideLibraries.Kill();
	maszOutsideProjects.Kill();
	mszListings.Kill();
	mcImports.Kill();
	mszWarnings.Kill();
	mszErrors.Kill();
	mcLinker.Kill();
	mcLinkWithObjects.Kill();
	macObjects.Kill();
	mcObjects.Kill();
	for (i = 0; i < macLinkLibraries.NumElements(); i++)
	{
		macLinkLibraries.Get(i)->cObjects.Kill();
	}
	macLinkLibraries.Kill();
	mcProject.Kill();
	for (i = 0; i < macIncludeDirectories.NumElements(); i++)
	{
		macIncludeDirectories.Get(i)->szDirectory.Kill();
	}
	macIncludeDirectories.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A directory searched for #include files: by sources, and by the project's units.  A system directory is
// searched for <file>, the others for "file".
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddIncludeDirectory(const char* szDirectory, bool bIncludeSubDirectories, bool bSystem)
{
	SCPP65IncludeDirectory*	psInclude;

	psInclude = macIncludeDirectories.Add();
	psInclude->szDirectory.Init(szDirectory);
	psInclude->bIncludeSubDirectories = bIncludeSubDirectories;
	psInclude->bSystem = bSystem;
	mcProject.AddIncludeDirectory((char*)szDirectory, bIncludeSubDirectories, bSystem);
}


//////////////////////////////////////////////////////////////////////////
//
// A translation unit of the project, szFile in szDirectory.  The units in a directory share the library szLibrary
// (their include settings).  They are compiled by Link.  bLinkAsLibrary, given with the library's first unit,
// links its units as a library rather than as objects.  A unit whose name ends in .asm is 65C64 assembly (such as
// Libc65/softfloat.asm), assembled now.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddUnit(const char* szLibrary, const char* szDirectory, const char* szFile, bool bLinkAsLibrary)
{
	CLibrary*			pcLibrary;
	SCPP65LinkLibrary*	psLinkLibrary;
	CAsm65Library*		pcLinkLibrary;
	CTextFile			cFile;
	CFileUtil			cFileUtil;
	CChars				szPath;
	CChars				szName;
	CChars				szSourceFile;

	pcLibrary = mcProject.GetLibrary((char*)szLibrary);
	if (!pcLibrary)
	{
		pcLibrary = mcProject.AddLibrary((char*)szLibrary, (char*)szDirectory, false);
		CCPP65Generator::AddDefines(pcLibrary->AddConfiguration((char*)GetCPPTarget(meTarget)->szName), meTarget);
		if (bLinkAsLibrary)
		{
			psLinkLibrary = macLinkLibraries.Add();
			psLinkLibrary->pcLibrary = pcLibrary;
			psLinkLibrary->cObjects.Init(szLibrary);
		}
	}

	szName.Init(szFile);
	if (szName.EndsWithIgnoreCase(".asm"))
	{
		szPath.Init(szDirectory);
		cFileUtil.AppendToPath(&szPath, szFile);
		cFile.Init();
		if (cFile.Read(szPath.Text()))
		{
			cFile.PassifyNewlines();
			pcLinkLibrary = GetLinkLibrary(pcLibrary);
			//With a line table its lines that write bytes are source lines, as compiled code's .lines are.
			szSourceFile.Init(szPath);
			cFileUtil.RemoveLastFromPath(&szPath);
			if (pcLinkLibrary)
			{
				Assemble(pcLinkLibrary->AddObject(szFile), cFile.Text(), szPath.Text(), mbLineTable ? szSourceFile.Text() : NULL);
			}
			else
			{
				AddObject(szFile, cFile.Text(), false, szPath.Text(), mbLineTable ? szSourceFile.Text() : NULL);
			}
			szSourceFile.Kill();
		}
		else
		{
			mszErrors.Append("Could not read ")->Append(szPath.Text())->Append(".\n");
		}
		cFile.Kill();
		szPath.Kill();
	}
	else
	{
		pcLibrary->AddTranslationUnit((char*)szFile);
		muiUnits++;
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The W65C816's maths tables (Libc65/MathTbls, Garth Wilson's): MathMult for
// the runtime's multiply and MathInvert for its divide.  Each is linked only
// when it is used.  The other targets need none.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddMathTables(const char* szLibc65Directory)
{
	CChars		szDirectory;
	CFileUtil	cFileUtil;

	if (meTarget != CPPT_W65C816)
	{
		return;
	}
	szDirectory.Init(szLibc65Directory);
	cFileUtil.AppendToPath(&szDirectory, "MathTbls");
	AddUnit("MathTables", szDirectory.Text(), "MathMult.asm", true);
	AddUnit("MathTables", szDirectory.Text(), "MathInvert.asm");
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Libc65's runtime for the W65C816 (its own assembly for string and soft
// float), each unit linked only when it is used.  A program that calls
// the C library or the compiler's runtime links with these.  The other
// targets have none yet.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddLibc65(const char* szLibc65Directory)
{
	static const char* aszUnits[] = { "string.cpp", "stdlib.cpp", "ctype.cpp", "time.cpp", "math.cpp", "softfloat.cpp", "new.cpp", "signal.cpp", "stdio.cpp", "host.cpp", "string816.asm", "string816_16.asm", "softfloat816.asm", NULL };
	size	i;

	if (meTarget != CPPT_W65C816)
	{
		return;
	}
	for (i = 0; aszUnits[i]; i++)
	{
		AddUnit("Libc65", szLibc65Directory, aszUnits[i], true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsExcluded(CArrayChars* paszExcluded, const char* szFile)
{
	size	i;

	for (i = 0; i < paszExcluded->NumElements(); i++)
	{
		if (StringInsensitiveCompare(paszExcluded->GetText(i), szFile) == 0)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Every C++ source of a Visual Studio project as a unit of the library szLibrary (AddUnit), except those named in
// szExcluded (space separated).  A source outside the project's directory (..\Other\File.cpp) is a unit of a library
// of its own directory, szLibrary.Other, linked as szLibrary is and given szLibrary's defines (AddDefine): a library's
// units are found, and their listings named, from the library's directory.  Returns false if the project can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddVisualStudioProject(const char* szLibrary, const char* szProjectFile, bool bLinkAsLibrary, const char* szExcluded)
{
	CVisualStudioProject	cProject;
	CArrayChars				aszExcluded;
	CChars					szExclude;
	CChars					szDirectory;
	CChars					szFile;
	CChars					szPath;
	CChars					szUnitLibrary;
	CFileUtil				cFileUtil;
	size					i;
	bool					bResult;

	aszExcluded.Init();
	if (szExcluded)
	{
		szExclude.Init(szExcluded);
		szExclude.Split(&aszExcluded, ' ');
		szExclude.Kill();
	}

	cProject.Init();
	bResult = cProject.Read(szProjectFile);
	if (!bResult)
	{
		mszErrors.Append("Could not read ")->Append(szProjectFile)->Append(".\n");
	}
	for (i = 0; i < cProject.NumSources(); i++)
	{
		if (IsExcluded(&aszExcluded, cProject.GetSource(i)))
		{
			continue;
		}
		szDirectory.Init(cProject.GetDirectory());
		szFile.Init(cProject.GetSource(i));
		szUnitLibrary.Init(szLibrary);
		if (strstr(szFile.Text(), "..") != NULL)
		{
			szPath.Init(szDirectory);
			cFileUtil.AppendToPath(&szPath, szFile.Text());
			cFileUtil.CollapsePath(&szPath);
			szFile.Kill();
			szDirectory.Kill();
			szFile.Init();
			szDirectory.Init();
			cFileUtil.SplitPath(szPath.Text(), &szFile, &szDirectory);
			szPath.Kill();

			szPath.Init(szDirectory);
			cFileUtil.RemovePath(&szPath);
			szUnitLibrary.Append('.');
			szUnitLibrary.Append(szPath);
			szPath.Kill();
			if (!mcProject.GetLibrary(szUnitLibrary.Text()))
			{
				maszOutsideProjects.Add(szLibrary);
				maszOutsideLibraries.Add(szUnitLibrary.Text());
			}
		}
		AddUnit(szUnitLibrary.Text(), szDirectory.Text(), szFile.Text(), bLinkAsLibrary);
		if (meTarget == CPPT_W65C816)
		{
			AddW65C816Assembly(szUnitLibrary.Text(), szDirectory.Text(), szFile.Text(), bLinkAsLibrary);
		}
		szUnitLibrary.Kill();
		szFile.Kill();
		szDirectory.Kill();
	}
	cProject.Kill();
	aszExcluded.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// szDefine ("NAME" or "NAME=value") is defined for the library
// szLibrary's units, as well as the target's own defines; a project's
// defines, say.  False if no unit of szLibrary has been added.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddDefine(const char* szLibrary, const char* szDefine)
{
	CLibrary*	pcLibrary;
	CConfig*	pcConfig;
	size		i;

	pcLibrary = mcProject.GetLibrary((char*)szLibrary);
	pcConfig = pcLibrary ? pcLibrary->GetConfig((char*)GetCPPTarget(meTarget)->szName) : NULL;
	if (!pcConfig)
	{
		return false;
	}
	pcConfig->AddDefine((char*)szDefine);

	//And to the libraries of its project's sources in other directories (AddVisualStudioProject).
	for (i = 0; i < maszOutsideProjects.NumElements(); i++)
	{
		if (StringCompare(maszOutsideProjects.GetText(i), szLibrary) == 0)
		{
			pcLibrary = mcProject.GetLibrary(maszOutsideLibraries.GetText(i));
			pcConfig = pcLibrary ? pcLibrary->GetConfig((char*)GetCPPTarget(meTarget)->szName) : NULL;
			if (pcConfig)
			{
				pcConfig->AddDefine((char*)szDefine);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A project's unit X.cpp may have W65C816 assembly next to it, X816.asm,
// with the functions X.cpp leaves out on the W65C816 (#ifndef
// __W65C816_ASSEMBLY__).  It is added with it, unless the C++ is used
// instead (CCPP65Generator::SetW65C816Assembly(false)).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddW65C816Assembly(const char* szLibrary, const char* szDirectory, const char* szSource, bool bLinkAsLibrary)
{
	CFileUtil	cFileUtil;
	CChars		szUnit;
	CChars		szPath;
	const char*	szExtension;

	if (!CCPP65Generator::IsW65C816Assembly())
	{
		return;
	}
	szExtension = strrchr(szSource, '.');
	if ((szExtension == NULL) || (StringInsensitiveCompare(szExtension, ".cpp") != 0))
	{
		return;
	}
	szUnit.Init();
	szUnit.Append(szSource, (size)(szExtension - szSource));
	szUnit.Append("816.asm");
	szPath.Init(szDirectory);
	cFileUtil.AppendToPath(&szPath, szUnit.Text());
	if (cFileUtil.Exists(szPath.Text()))
	{
		AddUnit(szLibrary, szDirectory, szUnit.Text(), bLinkAsLibrary);
	}
	szPath.Kill();
	szUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a label no object defines is stubbed (the default) or is a link error.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetStubMissing(bool bStubMissing)
{
	mbStubMissing = bStubMissing;
	mcLinker.SetStubMissing(bStubMissing);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what nothing uses is left out of the program (the default).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetRemoveUnused(bool bRemoveUnused)
{
	mbRemoveUnused = bRemoveUnused;
	mcLinker.SetRemoveUnused(bRemoveUnused);
}


//////////////////////////////////////////////////////////////////////////
//
// Functions are loaded when they are first called, from a drive with uiSectorSize byte
// sectors from uiFirstSector, into the memory after the core up to the heap.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLazy(uint32 uiSectorSize, uint32 uiFirstSector)
{
	mbLazy = true;
	muiLazySectorSize = uiSectorSize;
	muiLazyFirstSector = uiFirstSector;
	mcLinker.SetLazy(CPP65_HEAP_ADDRESS, uiSectorSize, uiFirstSector);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnits(void)
{
	return mcLinker.NumUnits();
}


//////////////////////////////////////////////////////////////////////////
//
// The C++ translation units added (AddUnit), which Compile compiles.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnitsToCompile(void)
{
	return muiUnits;
}


//////////////////////////////////////////////////////////////////////////
//
// The processor to build for.  Set it before adding anything.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetTarget(ECPPTarget eTarget)
{
	meTarget = eTarget;
}


//////////////////////////////////////////////////////////////////////////
//
// A profile of the program's functions (CAsm65816Profile, from a run): on the
// W65C816 those it measured worth it get page aligned frames (without one
// none do), and its hottest calls are inlined first, bigger callees for
// hot ones (CIRInliner::SetProfile).  Set it before compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetFrameProfile(CAsm65816Profile* pcProfile)
{
	mpcFrameProfile = pcProfile;
}


//////////////////////////////////////////////////////////////////////////
//
// Each unit compiled is also written to szDirectory as a listing: its
// assembly with comments giving the C++ source line each piece of code is
// for (UnitName.lst).  The code is the same as without.  NULL for none.
// Set it before compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetListingDirectory(const char* szDirectory)
{
	mszListings.Kill();
	mszListings.Init(szDirectory ? szDirectory : "");
}


//////////////////////////////////////////////////////////////////////////
//
// A header declaration (a class, enum, typedef, template or function) a
// unit parsed is not parsed by the units after until they use one of its
// names.  The code is the same.  On by default; false parses every header
// declaration in every unit.  Set it before compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLazyHeaders(bool bLazyHeaders)
{
	mbLazyHeaders = bLazyHeaders;
}


//////////////////////////////////////////////////////////////////////////
//
// The project's units are compiled by pcJobs: several at once by ThreadLib's
// CWorkerThreadJobs (the builder doesn't own them).  What is built is the
// same as without (though NumLazySkipped and NumLazyLoaded depend on how
// the units overlap).  NULL compiles them one at a time, as do jobs of one
// thread and a spill directory (SetSpillDirectory).  Set it before
// compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetJobs(CJobs* pcJobs)
{
	mpcJobs = pcJobs;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumLazySkipped(void)
{
	return muiLazySkipped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumLazyLoaded(void)
{
	return muiLazyLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the lazy declarations loaded were read as parsed before (in
// an earlier unit), not parsed again.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumLazyRead(void)
{
	return muiLazyRead;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the W65C816's units were compiled again because a function
// they call was exported after they were compiled (CompileUnits).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumCompiledAgain(void)
{
	return muiCompiledAgain;
}


//////////////////////////////////////////////////////////////////////////
//
// A header a unit preprocessed is kept, and the units after that include
// it with the same macros get its output without preprocessing it
// (CPreprocessor::SetHeaderCache).  The code is the same.  On by default;
// false preprocesses every header in every unit.  Set it before compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetHeaderCache(bool bHeaderCache)
{
	mcProject.SetHeaderCache(bHeaderCache ? &mcHeaderCache : NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The headers kept, and how often units found them (CPPHeaderCache).
//
//////////////////////////////////////////////////////////////////////////
CPPHeaderCache* CCPP65Builder::GetHeaderCache(void)
{
	return &mcHeaderCache;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a W65C816 unit, inlining before its turn, missed a
// function another unit exported meanwhile, and was generated again in its
// turn (CIRImports::Missed).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumImportsMissed(void)
{
	return mcImports.NumMissed();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a W65C816 unit found before its turn that it had missed
// a function another unit exported while it inlined, and was generated
// again then (CIRImports::Arrive).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumImportsStale(void)
{
	return mcImports.NumStale();
}


//////////////////////////////////////////////////////////////////////////
//
// For testing: every W65C816 unit is generated twice, as if it missed an
// imported function the first time.  The code is the same.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetForceImportsMissed(bool bForce)
{
	mcImports.SetForceMisses(bForce);
}


//////////////////////////////////////////////////////////////////////////
//
// Each unit's functions' code waits to be assembled in an evicting index
// tree in szDirectory (made, and removed by Kill), with at most uiCacheSize
// bytes of it in memory, instead of all in memory.  The code is the same.
// NULL for memory.  Set it before compiling.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::SetSpillDirectory(const char* szDirectory, lsize uiCacheSize)
{
	if (mpcSpillStore == &mcIndexTreeSpillStore)
	{
		mcIndexTreeSpillStore.Kill();
	}
	mpcSpillStore = &mcMemorySpillStore;
	if (StrEmpty(szDirectory))
	{
		return true;
	}
	if (!mcIndexTreeSpillStore.Init(szDirectory, uiCacheSize))
	{
		mszErrors.Append("Can't make the spill directory ");
		mszErrors.Append(szDirectory);
		mszErrors.AppendNewLine();
		return false;
	}
	mpcSpillStore = &mcIndexTreeSpillStore;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The units are compiled with their C++ source lines and functions'
// fingerprints (now as their compile time), so after Link the linker's
// line table (GetLineTable) gives the source line of an address: a profile
// by line.  Assembly units (.asm) give their lines that write bytes.  The
// code is the same as without.  Set it before adding units.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLineTable(bool bLineTable)
{
	mbLineTable = bLineTable;
	mlliCompileTime = (int64)time(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The linked program's source lines and fingerprints (SetLineTable).
//
//////////////////////////////////////////////////////////////////////////
CAsm65LineTable* CCPP65Builder::GetLineTable(void)
{
	return mcLinker.GetLineTable();
}


//////////////////////////////////////////////////////////////////////////
//
// A global's address after Link (see CAsm65Linker::GetSymbol).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::GetSymbol(const char* szName, int64* plliValue)
{
	return mcLinker.GetSymbol(szName, plliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Where the program must end (CPP65_HEAP_ADDRESS unless set).  A program in a memory of more than 2MB has its heap
// above 0020:0000 (Libc65's malloc), so it can end nearer the stack.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLimit(uint32 uiLimit)
{
	muiLimit = uiLimit;
}


//////////////////////////////////////////////////////////////////////////
//
// The lazy units' sectors, to write to the drive from the first sector given to
// SetLazy.
//
//////////////////////////////////////////////////////////////////////////
CArrayUint8* CCPP65Builder::GetLazyImage(void)
{
	return mcLinker.GetLazyImage();
}


//////////////////////////////////////////////////////////////////////////
//
// The start up that calls main.  It must be the first object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddStartUp(void)
{
	CChars	szAssembly;
	bool	bResult;

	szAssembly.Init();
	CCPP65Generator::AppendStartUp(&szAssembly, meTarget);
	bResult = AddObject("start up", szAssembly.Text(), true);
	szAssembly.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles szSource as a translation unit named szName and adds it as an object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddSource(const char* szName, const char* szSource)
{
	CChars	szAssembly;
	bool	bResult;

	szAssembly.Init();
	bResult = GenerateSource(&szAssembly, szName, szSource);
	bResult &= AddObject(szName, szAssembly.Text());
	szAssembly.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles szSource (AddSource's and LinkWith's) with the builder's
// settings: its line table, and its listing (UnitName.lst) when it keeps
// listings.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::GenerateSource(CChars* pszAssembly, const char* szName, const char* szSource)
{
	bool	bResult;

	bResult = Generate(pszAssembly, &mszErrors, &macIncludeDirectories, szName, szSource, false, NULL, meTarget, &mszWarnings, mpcFrameProfile, !mszListings.Empty(), mbLineTable);
	if (!mszListings.Empty())
	{
		bResult &= WriteListing(szName, pszAssembly);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddAssembly(const char* szName, const char* szAssembly)
{
	return AddObject(szName, szAssembly);
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles the project's units and links every object.  Returns false if anything had errors (see GetErrors).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Link(void)
{
	bool	bResult;

	bResult = Compile();
	bResult &= LinkObjects();
	return bResult && mszErrors.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles the project's units (once).  Returns false if they had errors (see GetErrors).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Compile(void)
{
	bool	bResult;

	if (mbCompiled)
	{
		return mszErrors.Empty();
	}
	mbCompiled = true;
	bResult = CompileUnits();
	return bResult && mszErrors.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles szSource as the translation unit szName and links it with the compiled objects, instead of the source
// given to the last LinkWith.  The project is compiled first if it hasn't been.  The errors (GetErrors) are this
// link's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::LinkWith(const char* szName, const char* szSource)
{
	CChars	szAssembly;
	bool	bResult;

	bResult = Compile();
	if (!bResult)
	{
		return false;
	}

	ResetLinker();
	mcLinkWithObjects.Kill();
	mcLinkWithObjects.Init("link with");
	szAssembly.Init();
	bResult = GenerateSource(&szAssembly, szName, szSource);
	bResult &= Assemble(mcLinkWithObjects.AddObject(szName), szAssembly.Text());
	szAssembly.Kill();
	mcLinker.AddObject(mcLinkWithObjects.GetObject(0));

	bResult &= LinkObjects();
	return bResult && mszErrors.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// A new linker with the builder's settings and objects, for LinkWith.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::ResetLinker(void)
{
	SCPP65Object*	psObject;
	size			i;

	mszErrors.Kill();
	mszErrors.Init();
	mcLinker.Kill();
	mcLinker.Init();
	mcLinker.SetStubMissing(mbStubMissing);
	mcLinker.SetRemoveUnused(mbRemoveUnused);
	if (mbLazy)
	{
		mcLinker.SetLazy(CPP65_HEAP_ADDRESS, muiLazySectorSize, muiLazyFirstSector);
	}
	for (i = 0; i < macObjects.NumElements(); i++)
	{
		psObject = macObjects.Get(i);
		if (psObject->bResident)
		{
			mcLinker.AddResidentObject(psObject->pcObject);
		}
		else
		{
			mcLinker.AddObject(psObject->pcObject);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::LinkObjects(void)
{
	bool	bResult;
	size	i;

	//The libraries are only added now, when their array won't move them any more.
	for (i = 0; i < macLinkLibraries.NumElements(); i++)
	{
		mcLinker.AddLibrary(&macLinkLibraries.Get(i)->cObjects);
	}
	mcLinker.SetAddress(ASK65_Text, CPP65_PROGRAM_ADDRESS);
	mcLinker.SetLimit(muiLimit);
	mcLinker.SetBankSize(GetCPPTarget(meTarget)->uiBankSize);
	if (meTarget == CPPT_W65C816)
	{
		mcLinker.SetStubOpcode(0xDB);  //stp
	}
	bResult = mcLinker.Link();
	mszErrors.Append(mcLinker.GetErrors());
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Output* CCPP65Builder::GetOutput(void)
{
	return mcLinker.GetOutput();
}


//////////////////////////////////////////////////////////////////////////
//
// Every stage's errors: preprocessing, tokenising, parsing, generating, assembling and linking.
//
//////////////////////////////////////////////////////////////////////////
char* CCPP65Builder::GetErrors(void)
{
	return mszErrors.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPP65Builder::GetWarnings(void)
{
	return mszWarnings.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPP65Builder::GetMap(void)
{
	return mcLinker.GetMap();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumMissing(void)
{
	return mcLinker.NumMissing();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPP65Builder::GetMissing(size uiIndex)
{
	return mcLinker.GetMissing(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The objects linked (not those in link libraries): the units' (once
// compiled) and the assembly's, in the order they were added.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnitObjects(void)
{
	return macObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CCPP65Builder::GetUnitObject(size uiIndex)
{
	return macObjects.Get(uiIndex)->pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The labels no object defines, sorted and space separated.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::GetMissingNames(CChars* pszDest)
{
	CArrayChars	aszMissing;
	size		i;

	aszMissing.Init();
	for (i = 0; i < mcLinker.NumMissing(); i++)
	{
		aszMissing.Add(mcLinker.GetMissing(i));
	}
	aszMissing.QuickSort(true);
	for (i = 0; i < aszMissing.NumElements(); i++)
	{
		if (i > 0)
		{
			pszDest->Append(' ');
		}
		pszDest->Append(aszMissing.GetText(i));
	}
	aszMissing.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayCPP65IncludeDirectories* CCPP65Builder::GetIncludeDirectories(void)
{
	return &macIncludeDirectories;
}


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses, parses and generates szSource with the include directories.  As a program (with its start up) if
// bProgram, otherwise as an object.  The assembly is appended to pszAssembly and every stage's errors to
// pszErrors.  The labels it uses but does not define are added to paszExternals if it isn't NULL.  On the W65C816,
// pcFrameProfile's measured functions get aligned frames (NULL for none).  bListing gives the code comments with the
// C++ source lines it is for (CCPP65Generator::SetListing), bLineTable .line and .fingerprint directives
// (CCPP65Generator::SetLineTable).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Generate(CChars* pszAssembly, CChars* pszErrors, CArrayCPP65IncludeDirectories* pacIncludeDirectories, const char* szName, const char* szSource, bool bProgram, CArrayChars* paszExternals, ECPPTarget eTarget, CChars* pszWarnings, CAsm65816Profile* pcFrameProfile, bool bListing, bool bLineTable)
{
	CTranslationUnit				cFile;
	CPreprocessor					cPreprocessor;
	CCPPTokeniser					cTokeniser;
	CCPPSyntaxTree					cTree;
	CCPPParser						cParser;
	CCPP65Generator					cGenerator;
	CHeaderFileMap					cHeaderFileMap;
	CArrayTemplate<CHeaderFiles>	acHeaderFiles;
	SCPP65IncludeDirectory*			psInclude;
	CConfig							cConfig;
	size							uiIncludes;
	size							i;
	bool							bResult;

	cHeaderFileMap.Init();
	uiIncludes = pacIncludeDirectories->NumElements();
	//All added before any is used: the preprocessor keeps their addresses.
	acHeaderFiles.Init();
	for (i = 0; i < uiIncludes; i++)
	{
		acHeaderFiles.Add();
	}
	for (i = 0; i < uiIncludes; i++)
	{
		psInclude = pacIncludeDirectories->Get(i);
		acHeaderFiles.Get(i)->Init(psInclude->szDirectory.Text(), &cHeaderFileMap, psInclude->bIncludeSubDirectories, psInclude->bSystem);
	}

	cConfig.Init((char*)GetCPPTarget(eTarget)->szName);
	CCPP65Generator::AddDefines(&cConfig, eTarget);
	cFile.Init((char*)szName, NULL, false);
	cFile.SetContents((char*)szSource);
	cPreprocessor.Init(&cConfig);
	for (i = 0; i < uiIncludes; i++)
	{
		cPreprocessor.AddIncludeDirectory(acHeaderFiles.Get(i));
	}
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	pszErrors->Append(cPreprocessor.GetErrors()->Text());
	if (pszWarnings)
	{
		pszWarnings->Append(cPreprocessor.GetWarnings()->Text());
	}
	cPreprocessor.Kill();

	cTokeniser.Init();
	bResult &= cTokeniser.Tokenise(cFile.GetTokens());
	pszErrors->Append(cTokeniser.GetErrors()->Text());

	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	bResult &= cParser.Parse();
	pszErrors->Append(cTree.GetErrors()->Text());
	cParser.Kill();

	cGenerator.Init(&cTree, eTarget);
	cGenerator.SetFrameProfile(pcFrameProfile);
	cGenerator.SetListing(bListing);
	cGenerator.SetLineTable(bLineTable, (int64)time(NULL));
	bResult &= bProgram ? cGenerator.GenerateProgram(pszAssembly) : cGenerator.Generate(pszAssembly);
	if (paszExternals)
	{
		cGenerator.GetExternals(paszExternals);
	}
	pszErrors->Append(cGenerator.GetErrors()->Text());
	if (pszWarnings)
	{
		pszWarnings->Append(cGenerator.GetWarnings()->Text());
	}
	cGenerator.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	cFile.Kill();
	cConfig.Kill();
	for (i = 0; i < uiIncludes; i++)
	{
		acHeaderFiles.Get(i)->Kill();
	}
	acHeaderFiles.Kill();
	cHeaderFileMap.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Assembles one object and adds it to the link.  .inchex file names are
// relative to szDirectory.  szSourceFile, the assembly's own file, makes
// its lines that write bytes source lines (CAsm65BaseAssembler::SetSourceFile).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddObject(const char* szName, const char* szAssembly, bool bResident, const char* szDirectory, const char* szSourceFile)
{
	CAsm65TextSource	cSource;
	bool				bResult;

	cSource.Init(szAssembly);
	bResult = AddObject(szName, &cSource, bResident, szDirectory, szSourceFile);
	cSource.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddObject(const char* szName, CAsm65Source* pcSource, bool bResident, const char* szDirectory, const char* szSourceFile)
{
	CAsm65Object*	pcObject;
	SCPP65Object*	psObject;
	bool			bResult;

	pcObject = mcObjects.AddObject(szName);
	bResult = Assemble(pcObject, pcSource, szDirectory, &mszErrors, szSourceFile);
	psObject = macObjects.Add();
	psObject->pcObject = pcObject;
	psObject->bResident = bResident;
	if (bResident)
	{
		mcLinker.AddResidentObject(pcObject);
	}
	else
	{
		mcLinker.AddObject(pcObject);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// szDirectory is the directory .inchex file names are relative to (an
// assembly unit's own).  The errors go to pszErrors.  szSourceFile, the
// assembly's own file, makes its lines that write bytes source lines.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Assemble(CAsm65Object* pcObject, const char* szAssembly, const char* szDirectory, const char* szSourceFile)
{
	CAsm65TextSource	cSource;
	bool				bResult;

	cSource.Init(szAssembly);
	bResult = Assemble(pcObject, &cSource, szDirectory, &mszErrors, szSourceFile);
	cSource.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Assemble(CAsm65Object* pcObject, CAsm65Source* pcSource, const char* szDirectory, CChars* pszErrors, const char* szSourceFile)
{
	CAsm65C64Assembler		cAssembler;
	CAsm65816Assembler	cAssembler816;
	bool				bResult;

	if (meTarget == CPPT_W65C816)
	{
		cAssembler816.Init();
		if (szDirectory)
		{
			cAssembler816.SetDirectory(szDirectory);
		}
		if (szSourceFile)
		{
			cAssembler816.SetSourceFile(szSourceFile);
		}
		bResult = cAssembler816.AssembleObject(pcSource, pcObject);
		pszErrors->Append(cAssembler816.GetErrors());
		cAssembler816.Kill();
		return bResult;
	}
	cAssembler.Init();
	if (szDirectory)
	{
		cAssembler.SetDirectory(szDirectory);
	}
	if (szSourceFile)
	{
		cAssembler.SetSourceFile(szSourceFile);
	}
	bResult = cAssembler.AssembleObject(pcSource, pcObject);
	pszErrors->Append(cAssembler.GetErrors());
	cAssembler.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The library a project library's units are linked as, or NULL if they are linked as objects.
//
//////////////////////////////////////////////////////////////////////////
CAsm65Library* CCPP65Builder::GetLinkLibrary(CLibrary* pcLibrary)
{
	size	i;

	for (i = 0; i < macLinkLibraries.NumElements(); i++)
	{
		if (macLinkLibraries.Get(i)->pcLibrary == pcLibrary)
		{
			return &macLinkLibraries.Get(i)->cObjects;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a unit's assembly (with its source comments) to the listings
// directory as the unit's name with .lst for its extension.  A unit with
// no code (all of it left out by #if) has no listing.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::WriteListing(const char* szUnit, CChars* pszAssembly)
{
	CTextFile	cFile;
	CChars		szPath;
	bool		bResult;

	if (pszAssembly->Empty())
	{
		return true;
	}
	szPath.Init();
	ListingName(szUnit, &szPath);
	cFile.Init(pszAssembly->Text());
	bResult = cFile.Write(szPath.Text());
	cFile.Kill();
	if (!bResult)
	{
		mszErrors.Append("Can't write the listing ");
		mszErrors.Append(szPath);
		mszErrors.AppendNewLine();
	}
	szPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// As WriteListing(CChars*), a line at a time, with its error in pszErrors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::WriteListing(const char* szUnit, CCPP65Assembly* pcAssembly, CChars* pszErrors)
{
	CChars	szPath;
	bool	bResult;

	if (pcAssembly->IsEmpty())
	{
		return true;
	}
	szPath.Init();
	ListingName(szUnit, &szPath);
	bResult = pcAssembly->Write(szPath.Text());
	if (!bResult)
	{
		pszErrors->Append("Can't write the listing ");
		pszErrors->Append(szPath);
		pszErrors->AppendNewLine();
	}
	szPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The listings directory (made if it isn't there) and the unit's name with
// .lst for its extension.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::ListingName(const char* szUnit, CChars* pszPath)
{
	CFileUtil	cFileUtil;
	CChars		szName;

	szName.Init(szUnit);
	cFileUtil.RemoveExtension(&szName);
	szName.Append(".lst");
	pszPath->Append(mszListings);
	cFileUtil.AppendToPath(pszPath, szName.Text());
	szName.Kill();
	return cFileUtil.TouchDir(mszListings.Text());
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles the project's units, each into an object (of its library when it
// is linked as one).  Each unit is preprocessed just before it is compiled,
// and its tokens and source freed after, so only the units being compiled
// are kept (headers stay tokenised for the units after).
//
// The units are preprocessed and compiled by the jobs (SetJobs), several at
// once.  What they share is used as if they were compiled one after
// another: a header is tokenised by the first to include it (CPreprocessor::
// TokeniseFile); their objects are added in order, here; the W65C816's
// small functions are found and exported in each unit's turn (mcImports);
// the header declarations they parse can't change what any of them is
// compiled to (mcHeaderIndex); and their errors and warnings are added in
// order at the end.  A W65C816 unit that called one of those functions
// before it was exported is compiled again after them all, in another turn.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::CompileUnits(void)
{
	CTranslationUnit*			pcFile;
	SProcessIter				sIter;
	CArrayCPP65CompiledUnits	apsUnits;
	SCPP65CompiledUnit*			psUnit;
	CSerialJobs					cSerialJobs;
	CJobs*						pcJobs;
	size						uiMaxCompiling;
	size						uiTurn;
	size						ui;
	bool						bResult;
	CFileUtil					cFileUtil;

	if (muiUnits == 0)
	{
		return true;
	}

	mcProject.CreatePost();
	if (!mszListings.Empty())
	{
		cFileUtil.TouchDir(mszListings.Text());
	}
	pcJobs = UnitJobs(&cSerialJobs);
	mpcUnitJobs = pcJobs;
	uiMaxCompiling = (size)pcJobs->NumThreads() * 2;
	mcImports.ResetTurns();
	bResult = true;
	uiTurn = 0;
	apsUnits.Init();
	for (pcFile = mcProject.Start(&sIter, (char*)GetCPPTarget(meTarget)->szName); pcFile; pcFile = mcProject.Iterate(&sIter))
	{
		psUnit = NewMalloc<SCPP65CompiledUnit>();
		StartUnit(psUnit, pcFile, uiTurn, mpcSpillStore != &mcIndexTreeSpillStore);
		uiTurn++;
		apsUnits.Add(psUnit);
		AddUnitObject(psUnit);
		RunUnit(pcJobs, CompileUnitJob, psUnit, uiMaxCompiling);
	}
	pcJobs->Wait();
	for (ui = 0; ui < apsUnits.NumElements(); ui++)
	{
		psUnit = *apsUnits.Get(ui);
		FinishUnit(psUnit, true);
		bResult &= psUnit->bResult;
	}

	//The units that call functions exported after they were compiled, compiled again in order.  A unit decides in its
	//turn if it calls only functions not yet exported (a unit before it may export one).
	if (bResult && (meTarget == CPPT_W65C816))
	{
		for (ui = 0; ui < apsUnits.NumElements(); ui++)
		{
			psUnit = *apsUnits.Get(ui);
			if (psUnit->aszMissing.NumElements() == 0)
			{
				psUnit->eAgain = CPP65CA_No;
			}
			else
			{
				psUnit->eAgain = CalleeExported(psUnit) ? CPP65CA_Yes : CPP65CA_IfExported;
			}
		}
		for (ui = 0; ui < apsUnits.NumElements(); ui++)
		{
			psUnit = *apsUnits.Get(ui);
			if (psUnit->eAgain != CPP65CA_No)
			{
				psUnit->uiTurn = uiTurn;
				uiTurn++;
				psUnit->bResult = true;
				RunUnit(pcJobs, CompileUnitAgainJob, psUnit, uiMaxCompiling);
			}
		}
		pcJobs->Wait();
		for (ui = 0; ui < apsUnits.NumElements(); ui++)
		{
			psUnit = *apsUnits.Get(ui);
			if (psUnit->eAgain == CPP65CA_Yes)
			{
				muiCompiledAgain++;
				FinishUnit(psUnit, false);
				bResult &= psUnit->bResult;
			}
		}
	}

	for (ui = 0; ui < apsUnits.NumElements(); ui++)
	{
		psUnit = *apsUnits.Get(ui);
		psUnit->aszMissing.Kill();
		psUnit->szWarnings.Kill();
		psUnit->szErrors.Kill();
		free(psUnit);
	}
	KillWorkspaces();
	mpcUnitJobs = NULL;

	if (apsUnits.NumElements() != muiUnits)
	{
		mszErrors.Append("Only ")->Append((int)apsUnits.NumElements())->Append(" of the ")->Append((int)muiUnits)->Append(" translation units were compiled.\n");
		bResult = false;
	}
	apsUnits.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What compiles the units: mpcJobs, or pcSerialJobs to compile them one at a
// time (without SetJobs, with an index tree to spill to, or while the IR
// front end is being debugged).
//
//////////////////////////////////////////////////////////////////////////
CJobs* CCPP65Builder::UnitJobs(CSerialJobs* pcSerialJobs)
{
	if (!mpcJobs || (mpcJobs->NumThreads() <= 1))
	{
		return pcSerialJobs;
	}
	if ((mpcSpillStore == &mcIndexTreeSpillStore) || CCPP65IRFrontEnd::sbPrint || (CCPP65IRFrontEnd::suiLimit != SIZE_MAX))
	{
		return pcSerialJobs;
	}

	//Built before the units, which only look them up.
	CIR816Backend::IndexMnemonics();
	CAsm65816Assembler::IndexMnemonics();
	return mpcJobs;
}


//////////////////////////////////////////////////////////////////////////
//
// psUnit compiles pcFile in its turn uiTurn, with its own spill store or
// the builder's index tree.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::StartUnit(SCPP65CompiledUnit* psUnit, CTranslationUnit* pcFile, size uiTurn, bool bOwnSpillStore)
{
	psUnit->pcBuilder = this;
	psUnit->pcFile = pcFile;
	psUnit->pcObject = NULL;
	psUnit->aszMissing.Init();
	psUnit->eAgain = CPP65CA_No;
	psUnit->uiTurn = uiTurn;
	psUnit->bResult = true;
	psUnit->szErrors.Init();
	psUnit->szWarnings.Init();
	psUnit->pcSpillStore = bOwnSpillStore ? &psUnit->cSpillStore : mpcSpillStore;
	psUnit->uiLazySkipped = 0;
	psUnit->uiLazyLoaded = 0;
	psUnit->uiLazyRead = 0;
	psUnit->psCompile = NULL;
	psUnit->eStage = CPP65US_Generating;
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's (empty) object, added to its link library or to the objects
// linked, after the units before it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddUnitObject(SCPP65CompiledUnit* psUnit)
{
	CAsm65Library*	pcLinkLibrary;
	SCPP65Object*	psObject;

	pcLinkLibrary = GetLinkLibrary(psUnit->pcFile->GetLibrary());
	if (pcLinkLibrary)
	{
		psUnit->pcObject = pcLinkLibrary->AddObject(psUnit->pcFile->ShortName());
	}
	else
	{
		psUnit->pcObject = mcObjects.AddObject(psUnit->pcFile->ShortName());
		psObject = macObjects.Add();
		psObject->pcObject = psUnit->pcObject;
		psObject->bResident = false;
		mcLinker.AddObject(psUnit->pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Gives psUnit to the jobs to compile (fJob), once fewer than
// uiMaxCompiling units are compiling (each keeps its file's tokens).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::RunUnit(CJobs* pcJobs, JobFunction fJob, SCPP65CompiledUnit* psUnit, size uiMaxCompiling)
{
	mcCompilingMutex.Lock();
	while (muiCompiling >= uiMaxCompiling)
	{
		mcCompiled.Wait(&mcCompilingMutex);
	}
	muiCompiling++;
	mcCompilingMutex.Unlock();

	pcJobs->Run(fJob, psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function psUnit calls, that wasn't found to inline, has been
// exported since.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::CalleeExported(SCPP65CompiledUnit* psUnit)
{
	size	ui;

	for (ui = 0; ui < psUnit->aszMissing.NumElements(); ui++)
	{
		if (mcImports.Find(psUnit->aszMissing.GetText(ui)) != NULL)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds what compiling the unit gave to the builder's: its errors, and its
// warnings if bWarnings (the first time it is compiled).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::FinishUnit(SCPP65CompiledUnit* psUnit, bool bWarnings)
{
	mszErrors.Append(psUnit->szErrors);
	if (bWarnings)
	{
		mszWarnings.Append(psUnit->szWarnings);
	}
	psUnit->szErrors.ReInit();
	psUnit->szWarnings.ReInit();
	muiLazySkipped += psUnit->uiLazySkipped;
	muiLazyLoaded += psUnit->uiLazyLoaded;
	muiLazyRead += psUnit->uiLazyRead;
	psUnit->uiLazySkipped = 0;
	psUnit->uiLazyLoaded = 0;
	psUnit->uiLazyRead = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A job: compiles the unit (CompileUnit).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::CompileUnitJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcBuilder->CompileUnit(psUnit, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A job: compiles the unit again (CompileUnitAgain).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::CompileUnitAgainJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcBuilder->CompileUnitAgain(psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles psUnit into its object again, if it is to be (eAgain).  If it
// calls only functions that weren't exported after the first pass, it
// decides in its turn, when the units before it have exported theirs
// (DecideAgain; till then it waits without a thread, CIRImports::Arrive).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::CompileUnitAgain(SCPP65CompiledUnit* psUnit)
{
	if (psUnit->eAgain == CPP65CA_IfExported)
	{
		psUnit->eStage = CPP65US_Deciding;
		if (mcImports.Arrive(psUnit->uiTurn, psUnit, NULL) == IRA_Turn)
		{
			DecideAgain(psUnit);
		}
		return;
	}
	CompileUnit(psUnit, false);
}


//////////////////////////////////////////////////////////////////////////
//
// In psUnit's turn: it is compiled again (in that turn, which it keeps) if
// a unit before it has exported a function it calls, otherwise its object
// is as it was (eAgain is No).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::DecideAgain(SCPP65CompiledUnit* psUnit)
{
	if (!CalleeExported(psUnit))
	{
		mcImports.EndTurn(psUnit->uiTurn);
		psUnit->eAgain = CPP65CA_No;
		UnitFinished();
		return;
	}
	psUnit->eAgain = CPP65CA_Yes;
	CompileUnit(psUnit, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A workspace for compiling a unit (CCPP65UnitWorkspace): one another
// unit has finished with, or a new one.  Units compiled at the same time
// have their own.
//
//////////////////////////////////////////////////////////////////////////
CCPP65UnitWorkspace* CCPP65Builder::TakeWorkspace(void)
{
	CCPP65UnitWorkspace*	pcWorkspace;

	mcCompilingMutex.Lock();
	pcWorkspace = mapcWorkspaces.IsNotEmpty() ? mapcWorkspaces.Pop() : NULL;
	mcCompilingMutex.Unlock();

	if (!pcWorkspace)
	{
		pcWorkspace = NewMalloc<CCPP65UnitWorkspace>();
		pcWorkspace->Init();
	}
	pcWorkspace->Begin();
	return pcWorkspace;
}


//////////////////////////////////////////////////////////////////////////
//
// The unit compiled with pcWorkspace has finished with it.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::GiveBackWorkspace(CCPP65UnitWorkspace* pcWorkspace)
{
	mcCompilingMutex.Lock();
	mapcWorkspaces.Push(pcWorkspace);
	mcCompilingMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// The workspaces' memory is freed (no unit is compiling).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::KillWorkspaces(void)
{
	size					i;
	CCPP65UnitWorkspace*	pcWorkspace;

	for (i = 0; i < mapcWorkspaces.NumElements(); i++)
	{
		pcWorkspace = *mapcWorkspaces.Get(i);
		pcWorkspace->Kill();
		free(pcWorkspace);
	}
	mapcWorkspaces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A unit given to the jobs has finished (muiCompiling).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::UnitFinished(void)
{
	mcCompilingMutex.Lock();
	muiCompiling--;
	muiUnitsFinished++;
	mcCompiled.WakeAll();
	mcCompilingMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit has finished compiling, for showing progress from
// another thread while Compile runs.  A W65C816 unit compiled again counts
// twice, so it can pass NumUnitsToCompile.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnitsFinished(void)
{
	size	uiFinished;

	mcCompilingMutex.Lock();
	uiFinished = muiUnitsFinished;
	mcCompilingMutex.Unlock();
	return uiFinished;
}


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses and compiles psUnit's file into its object, then frees the
// file's tokens and source.  Its errors and warnings go to the unit's (the
// preprocessor's only if bReport; compiled again, its object is made
// again), the functions it calls that weren't found for inlining to its
// aszMissing.  The unit is counted as compiling (muiCompiling) until it has
// finished.  On the W65C816 it may wait for its turn with the imports
// without holding the thread (StartGenerating): the rest is done later.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::CompileUnit(SCPP65CompiledUnit* psUnit, bool bReport)
{
	SCPP65UnitCompile*	psCompile;
	char*				szConfiguration;
	bool				bResult;

	if (!bReport)
	{
		psUnit->aszMissing.ReInit();
		psUnit->pcObject->Kill();
		psUnit->pcObject->Init(psUnit->pcFile->ShortName());
	}

	psCompile = NewMalloc<SCPP65UnitCompile>();
	psUnit->psCompile = psCompile;
	psCompile->uiAttempts = 0;
	psCompile->pcWorkspace = TakeWorkspace();
	psUnit->pcFile->GetTokens()->SetMallocators(psCompile->pcWorkspace->GetPreprocessedTokensMallocators());

	szConfiguration = (char*)GetCPPTarget(meTarget)->szName;
	if (bReport)
	{
		bResult = mcProject.ProcessUnit(psUnit->pcFile, szConfiguration, &psUnit->szErrors, &psUnit->szWarnings);
	}
	else
	{
		//Preprocessed again: its errors and warnings were reported the first time.
		mcProject.ProcessUnit(psUnit->pcFile, szConfiguration);
		bResult = true;
	}

	if (psUnit->pcSpillStore == &psUnit->cSpillStore)
	{
		psUnit->cSpillStore.Init();
	}

	psCompile->cTokeniser.Init(psCompile->pcWorkspace->GetCPPTokensMallocator());
	bResult &= psCompile->cTokeniser.Tokenise(psUnit->pcFile->GetTokens());
	psUnit->szErrors.Append(psCompile->cTokeniser.GetErrors()->Text());
	psCompile->cTree.Init(&psCompile->cTokeniser, psCompile->pcWorkspace->GetSyntaxTreeMallocators());
	psCompile->cParser.Init(&psCompile->cTokeniser, &psCompile->cTree);
	if (mbLazyHeaders)
	{
		psCompile->cParser.SetHeaderIndex(&mcHeaderIndex);
	}
	bResult &= psCompile->cParser.Parse();
	psUnit->szErrors.Append(psCompile->cTree.GetErrors()->Text());
	psCompile->bResult = bResult;

	StartGenerating(psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// A generator for psUnit's syntax tree, into its assembly (which it
// initialises).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::InitGenerator(SCPP65CompiledUnit* psUnit)
{
	SCPP65UnitCompile*	psCompile;

	psCompile = psUnit->psCompile;
	psCompile->cAssembly.Init(psUnit->pcSpillStore);
	psCompile->cGenerator.Init(&psCompile->cTree, meTarget);
	psCompile->cGenerator.SetSpillStore(psUnit->pcSpillStore);
	if (meTarget == CPPT_W65C816)
	{
		psCompile->cGenerator.SetImports(&mcImports, psUnit->uiTurn);
		psCompile->cGenerator.SetFrameProfile(mpcFrameProfile);
	}
	psCompile->cGenerator.SetListing(!mszListings.Empty());
	psCompile->cGenerator.SetLineTable(mbLineTable, mlliCompileTime);
}


//////////////////////////////////////////////////////////////////////////
//
// Generates psUnit up to its turn with the imports (CCPP65Generator::
// GenerateBeforeTurn), then arrives at its turn: in it, generation goes on
// (ContinueGenerating).  Before it the unit waits without a thread, and is
// resumed in its turn, or as soon as a function it looked for and didn't
// find is exported (it is generated again, up to CPP65_STALE_ATTEMPTS
// times).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::StartGenerating(SCPP65CompiledUnit* psUnit)
{
	SCPP65UnitCompile*	psCompile;
	EIRArrival			eArrival;

	psCompile = psUnit->psCompile;
	InitGenerator(psUnit);
	psCompile->bGenerated = psCompile->cGenerator.GenerateBeforeTurn();
	if (meTarget != CPPT_W65C816)
	{
		ContinueGenerating(psUnit);
		return;
	}

	psUnit->eStage = CPP65US_Generating;
	eArrival = mcImports.Arrive(psUnit->uiTurn, psUnit, (psCompile->uiAttempts < CPP65_STALE_ATTEMPTS) ? psCompile->cGenerator.GetNotImported() : NULL);
	if (eArrival == IRA_Turn)
	{
		ContinueGenerating(psUnit);
	}
	else if (eArrival == IRA_Stale)
	{
		RestartGenerating(psUnit);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// psUnit missed a function exported while it was generated: it is
// generated again (its code in the spill store is replaced, by index).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::RestartGenerating(SCPP65CompiledUnit* psUnit)
{
	SCPP65UnitCompile*	psCompile;

	psCompile = psUnit->psCompile;
	psCompile->cGenerator.Kill();
	psCompile->cAssembly.Kill();
	psCompile->uiAttempts++;
	StartGenerating(psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// In psUnit's turn (or with no turns): the rest of its generation
// (CCPP65Generator::GenerateAfterTurn), then FinishCompile.  If it missed
// a function exported since it inlined it is generated again, in its turn,
// which it keeps (then it finds everything it would have).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::ContinueGenerating(SCPP65CompiledUnit* psUnit)
{
	SCPP65UnitCompile*	psCompile;
	bool				bGenerated;

	psCompile = psUnit->psCompile;
	bGenerated = psCompile->cGenerator.GenerateAfterTurn(&psCompile->cAssembly);
	if (psCompile->cGenerator.MissedImports())
	{
		psCompile->cGenerator.Kill();
		psCompile->cAssembly.Kill();
		InitGenerator(psUnit);
		psCompile->bGenerated = psCompile->cGenerator.GenerateBeforeTurn();
		ContinueGenerating(psUnit);
		return;
	}
	FinishCompile(psUnit, bGenerated && psCompile->bGenerated);
}


//////////////////////////////////////////////////////////////////////////
//
// psUnit is generated (bGenerated if that went well): its turn ends, its
// listing is written, it is assembled into its object, and what compiling
// it needed is freed.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::FinishCompile(SCPP65CompiledUnit* psUnit, bool bGenerated)
{
	SCPP65UnitCompile*	psCompile;
	bool				bResult;

	psCompile = psUnit->psCompile;
	bResult = psCompile->bResult && bGenerated;
	psCompile->cGenerator.GetMissingCallees(&psUnit->aszMissing);
	psUnit->szErrors.Append(psCompile->cGenerator.GetErrors()->Text());
	psUnit->szWarnings.Append(psCompile->cGenerator.GetWarnings()->Text());
	psCompile->cGenerator.Kill();
	if (meTarget == CPPT_W65C816)
	{
		//Its turn is over even if it didn't get to it (an error before the unit was finished).
		mcImports.EndTurn(psUnit->uiTurn);
	}
	if (!mszListings.Empty())
	{
		bResult &= WriteListing(psUnit->pcFile->ShortName(), &psCompile->cAssembly, &psUnit->szErrors);
	}
	psUnit->uiLazySkipped += psCompile->cParser.NumLazySkipped();
	psUnit->uiLazyLoaded += psCompile->cParser.NumLazyLoaded();
	psUnit->uiLazyRead += psCompile->cParser.NumLazyRead();
	psCompile->cParser.Kill();

	//The syntax tree and tokens aren't needed to assemble.
	psCompile->cTree.Kill();
	psCompile->cTokeniser.Kill();

	bResult &= Assemble(psUnit->pcObject, &psCompile->cAssembly, NULL, &psUnit->szErrors);
	psCompile->cAssembly.Kill();
	if (psUnit->pcSpillStore == &psUnit->cSpillStore)
	{
		psUnit->cSpillStore.Kill();
	}
	psUnit->bResult &= bResult;

	//The file's tokens and source are freed (the tokens' memory kept by the workspace for the next unit).
	psUnit->pcFile->GetTokens()->SetMallocators(NULL);
	psUnit->pcFile->Unload();
	GiveBackWorkspace(psCompile->pcWorkspace);
	free(psCompile);
	psUnit->psCompile = NULL;
	UnitFinished();
}


//////////////////////////////////////////////////////////////////////////
//
// CIRImports' resume function: a unit that waited for its turn is given to
// the jobs, to go on in its turn or (bStale) to be generated again.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::ResumeUnit(void* pvBuilder, void* pvUnit, bool bStale)
{
	CCPP65Builder*	pcBuilder;

	pcBuilder = (CCPP65Builder*)pvBuilder;
	pcBuilder->mpcUnitJobs->Run(bStale ? RestartUnitJob : ResumeUnitJob, pvUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// A job: the unit goes on in its turn (Resume).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::ResumeUnitJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcBuilder->Resume(psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// A job: the unit is generated again before its turn (RestartGenerating).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::RestartUnitJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcBuilder->RestartGenerating(psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// In psUnit's turn, after it waited for it: it goes on where it was
// (eStage).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::Resume(SCPP65CompiledUnit* psUnit)
{
	if (psUnit->eStage == CPP65US_Deciding)
	{
		DecideAgain(psUnit);
	}
	else
	{
		ContinueGenerating(psUnit);
	}
}
