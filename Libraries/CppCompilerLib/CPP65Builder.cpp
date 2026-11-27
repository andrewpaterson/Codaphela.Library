/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <time.h>
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "BaseLib/ConstructorCall.h"
#include "VisualStudioProject.h"
#include "Assembler65Lib/Asm65C64Assembler.h"
#include "Assembler65Lib/AsmW65C816Assembler.h"
#include "CPP65IRFrontEnd.h"
#include "CPPPreprocessor.h"
#include "HeaderFileMap.h"
#include "HeaderFiles.h"
#include "CppTokeniser.h"
#include "CppParser.h"
#include "CPPUnit.h"
#include "CPP65Builder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::Init(void)
{
	mcBuild.Init();
	mcProject.Init(false);
	muiUnits = 0;
	mbCompiled = false;
	mszErrors.Init();
	mszWarnings.Init();
	mcBuild.SetWarnings(&mszWarnings);
	mcProgram.Init(&mcBuild, &mszErrors, &mcStatistics);
	mcBuild.SetTested(mcProgram.GetTested());
	mcSchedule.Init(&mcBuild, &mcProgram, &mcProject, &mszErrors, &mszWarnings, &mcStatistics);
	maszOutsideProjects.Init();
	maszOutsideLibraries.Init();
	mbRuntime = false;
	mcHeaderCache.Init();
	mcProject.SetHeaderCache(&mcHeaderCache);
	mcStatistics.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::Kill(void)
{
	mcSchedule.Kill();
	mcStatistics.Kill();
	mcHeaderCache.Kill();
	maszOutsideLibraries.Kill();
	maszOutsideProjects.Kill();
	mszWarnings.Kill();
	mszErrors.Kill();
	mcProgram.Kill();
	mcProject.Kill();
	mcBuild.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A directory searched for #include files: by sources, and by the project's units.  A system directory is
// searched for <file>, the others for "file".  False if szDirectory doesn't exist.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddIncludeDirectory(const char* szDirectory, bool bIncludeSubDirectories, bool bSystem)
{
	mcBuild.AddIncludeDirectory(szDirectory, bIncludeSubDirectories, bSystem);
	return mcProject.AddIncludeDirectory((char*)szDirectory, bIncludeSubDirectories, bSystem);
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
		mcBuild.GetTarget()->AddDefines(pcLibrary->AddConfiguration((char*)mcBuild.GetTarget()->GetConfigurationName()), mcBuild.GetOptions());
		if (bLinkAsLibrary)
		{
			mcProgram.AddLinkLibrary(pcLibrary, szLibrary);
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
			pcLinkLibrary = mcProgram.GetLinkLibrary(pcLibrary);
			//With a line table its lines that write bytes are source lines, as compiled code's .lines are.
			szSourceFile.Init(szPath);
			cFileUtil.RemoveLastFromPath(&szPath);
			if (pcLinkLibrary)
			{
				mcProgram.Assemble(pcLinkLibrary->AddObject(szFile), cFile.Text(), szPath.Text(), mcBuild.HasLineTable() ? szSourceFile.Text() : NULL);
			}
			else
			{
				mcProgram.AddObject(szFile, cFile.Text(), false, szPath.Text(), mcBuild.HasLineTable() ? szSourceFile.Text() : NULL);
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
		//A C++ unit is found from its library's directory (the first unit's): one that isn't there is an error, not a
		//unit compiled to nothing.
		szPath.Init(pcLibrary->GetBaseDir());
		cFileUtil.AppendToPath(&szPath, szFile);
		if (cFileUtil.Exists(szPath.Text()))
		{
			pcLibrary->AddTranslationUnit((char*)szFile);
			muiUnits++;
		}
		else
		{
			mszErrors.Append("Could not read ")->Append(szPath.Text())->Append(".\n");
		}
		szPath.Kill();
	}
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The target's runtime (AddRuntime) and its maths tables, in Libc65/MathTbls
// (CCPPTarget::GetMathTableUnits: the W65C816's, Garth Wilson's MathMult
// for the runtime's multiply (SetMathTables) and the soft float's, and
// MathInvert for the soft float's divide).  Each is linked only when it is
// used.  A target with none needs none.  A program whose tables are in
// memory already (an HCP65 program: UserLib's UserMathTables.asm gives
// their addresses) adds Libc65 alone.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddMathTables(const char* szLibc65Directory)
{
	const char**	paszUnits;
	CChars			szDirectory;
	CFileUtil		cFileUtil;
	size			i;

	paszUnits = mcBuild.GetTarget()->GetMathTableUnits();
	if (!paszUnits)
	{
		return;
	}
	AddRuntime(szLibc65Directory);
	szDirectory.Init(szLibc65Directory);
	cFileUtil.AppendToPath(&szDirectory, "MathTbls");
	for (i = 0; paszUnits[i]; i++)
	{
		AddUnit("MathTables", szDirectory.Text(), paszUnits[i], i == 0);
	}
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The back end's runtime routines, in Libc65 (CCPPTarget::GetRuntimeUnits:
// the W65C816's R816_Mul16 ..., its multiplies reading MathMult or not as
// SetMathTables says), linked as a library so only those used are kept.
// Added once, by AddMathTables or AddLibc65.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddRuntime(const char* szLibc65Directory)
{
	const char**	paszUnits;
	size			i;

	paszUnits = mcBuild.GetTarget()->GetRuntimeUnits(mcBuild.GetOptions()->mbMathTables);
	if (!paszUnits || mbRuntime)
	{
		return;
	}
	mbRuntime = true;
	for (i = 0; paszUnits[i]; i++)
	{
		AddUnit("Runtime", szLibc65Directory, paszUnits[i], i == 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Libc65's runtime for the target (CCPPTarget::GetLibc65Units), each unit
// linked only when it is used.  A program that calls the C library or the
// compiler's runtime links with these.  A target with none has none yet.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddLibc65(const char* szLibc65Directory)
{
	const char**	paszUnits;
	size			i;

	paszUnits = mcBuild.GetTarget()->GetLibc65Units();
	if (!paszUnits)
	{
		return;
	}
	AddRuntime(szLibc65Directory);
	for (i = 0; paszUnits[i]; i++)
	{
		AddUnit("Libc65", szLibc65Directory, paszUnits[i], true);
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
// Every C++ source of a Visual Studio project as a unit of the library szLibrary (AddProjectSources), except those
// named in szExcluded (space separated).  Returns false if the project can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddVisualStudioProject(const char* szLibrary, const char* szProjectFile, bool bLinkAsLibrary, const char* szExcluded)
{
	CVisualStudioProject	cProject;
	CArrayChars				aszExcluded;
	CArrayChars				aszSources;
	CChars					szExclude;
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
	aszSources.Init();
	for (i = 0; i < cProject.NumSources(); i++)
	{
		if (!IsExcluded(&aszExcluded, cProject.GetSource(i)))
		{
			aszSources.Add(cProject.GetSource(i));
		}
	}
	AddProjectSources(szLibrary, cProject.GetDirectory(), &aszSources, bLinkAsLibrary);
	aszSources.Kill();
	cProject.Kill();
	aszExcluded.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A project's sources (paths relative to its directory, szDirectory, a full path) as units of the library szLibrary
// (AddUnit), with their W65C816 assembly.  A source outside the project's directory (..\Other\File.cpp, or a full
// path, /Work/Other/File.cpp) is a unit of a library of its own directory, szLibrary.Other, linked as szLibrary is and
// given szLibrary's defines (AddDefine):
// a library's units are found, and their listings named, from the library's directory.  A Visual Studio project's
// sources are added this way (AddVisualStudioProject), and a project described some other way (hcp65build's plain
// command line) gets exactly the same units.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddProjectSources(const char* szLibrary, const char* szDirectory, CArrayChars* paszSources, bool bLinkAsLibrary)
{
	CChars		szUnitDirectory;
	CChars		szFile;
	CChars		szPath;
	CChars		szUnitLibrary;
	CFileUtil	cFileUtil;
	size		i;

	for (i = 0; i < paszSources->NumElements(); i++)
	{
		szUnitDirectory.Init(szDirectory);
		szFile.Init(paszSources->GetText(i));
		szFile.Replace('\\', FILE_SEPARATOR[0]);
		szUnitLibrary.Init(szLibrary);
		if (cFileUtil.IsAbsolutePath(szFile.Text()) || (strstr(szFile.Text(), "..") != NULL))
		{
			if (cFileUtil.IsAbsolutePath(szFile.Text()))
			{
				szPath.Init(szFile);
			}
			else
			{
				szPath.Init(szUnitDirectory);
				cFileUtil.AppendToPath(&szPath, szFile.Text());
			}
			cFileUtil.CollapsePath(&szPath);
			szFile.Kill();
			szUnitDirectory.Kill();
			szFile.Init();
			szUnitDirectory.Init();
			cFileUtil.SplitPath(szPath.Text(), &szFile, &szUnitDirectory);
			szPath.Kill();

			szPath.Init(szUnitDirectory);
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
		AddUnit(szUnitLibrary.Text(), szUnitDirectory.Text(), szFile.Text(), bLinkAsLibrary);
		AddAssemblyUnit(szUnitLibrary.Text(), szUnitDirectory.Text(), szFile.Text(), bLinkAsLibrary);
		szUnitLibrary.Kill();
		szFile.Kill();
		szUnitDirectory.Kill();
	}
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
	pcConfig = pcLibrary ? pcLibrary->GetConfig((char*)mcBuild.GetTarget()->GetConfigurationName()) : NULL;
	if (!pcConfig)
	{
		return false;
	}
	pcConfig->AddDefine(szDefine);

	//And to the libraries of its project's sources in other directories (AddVisualStudioProject).
	for (i = 0; i < maszOutsideProjects.NumElements(); i++)
	{
		if (StringCompare(maszOutsideProjects.GetText(i), szLibrary) == 0)
		{
			pcLibrary = mcProject.GetLibrary(maszOutsideLibraries.GetText(i));
			pcConfig = pcLibrary ? pcLibrary->GetConfig((char*)mcBuild.GetTarget()->GetConfigurationName()) : NULL;
			if (pcConfig)
			{
				pcConfig->AddDefine(szDefine);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A project's unit X.cpp may have the target's assembly next to it (the
// W65C816's XW65C816.asm, CCPPTarget::GetAssemblyUnitSuffix), with the functions
// X.cpp leaves out on the target (#ifndef __W65C816_ASSEMBLY__).  It is
// added with it, unless the C++ is used instead
// (CCPPCompileOptions::SetAssemblyUnits(false)).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::AddAssemblyUnit(const char* szLibrary, const char* szDirectory, const char* szSource, bool bLinkAsLibrary)
{
	CFileUtil	cFileUtil;
	CChars		szUnit;
	CChars		szPath;
	const char*	szExtension;

	if (!mcBuild.GetTarget()->HasAssemblyUnits() || !mcBuild.GetOptions()->mbAssemblyUnits)
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
	szUnit.Append(mcBuild.GetTarget()->GetAssemblyUnitSuffix());
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
	mcProgram.SetStubMissing(bStubMissing);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what nothing uses is left out of the program (the default).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetRemoveUnused(bool bRemoveUnused)
{
	mcProgram.SetRemoveUnused(bRemoveUnused);
}


//////////////////////////////////////////////////////////////////////////
//
// Functions are loaded when they are first called, from a drive with uiSectorSize byte
// sectors from uiFirstSector, into the memory after the core up to the heap.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLazy(uint32 uiSectorSize, uint32 uiFirstSector)
{
	mcProgram.SetLazy(uiSectorSize, uiFirstSector);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnits(void)
{
	return mcProgram.NumUnits();
}


//////////////////////////////////////////////////////////////////////////
//
// The C++ translation units added (AddUnit), which Compile compiles.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnitsToCompile(void) const
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
	mcBuild.SetTarget(eTarget);
}


//////////////////////////////////////////////////////////////////////////
//
// A profile of the program's functions (CAsm65Profile, from a run): on the
// W65C816 those it measured worth it get page aligned frames (without one
// none do), and its hottest calls are inlined first, bigger callees for
// hot ones (CIRInliner::SetProfile).  Set it before compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetFrameProfile(CAsm65Profile* pcProfile)
{
	mcBuild.SetFrameProfile(pcProfile);
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
	mcBuild.SetListingDirectory(szDirectory);
}


//////////////////////////////////////////////////////////////////////////
//
// The classes that what is compiled lays out are written to szFile (once
// the units are compiled, and again with each source's): a line for each
// class (CCPPUnit::AddLayout), each once and in order, so that two builds'
// files can be compared (another target's, or the same sources after a
// change to the layout rules or to a class).  NULL for none.  Set it
// before compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLayoutsFile(const char* szFile)
{
	mcBuild.SetLayoutsFile(szFile);
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
	mcBuild.SetLazyHeaders(bLazyHeaders);
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
	mcSchedule.SetJobs(pcJobs);
}


//////////////////////////////////////////////////////////////////////////
//
// Each unit is reported as soon as it has finished compiling, with its
// errors and warnings (TakeUnitReports), so they can be shown while the
// others compile.  Its errors and warnings are then only in its report,
// not in GetErrors and GetWarnings, which keep what isn't a unit's (the
// link's, say).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetUnitReports(bool bUnitReports)
{
	mcSchedule.SetUnitReports(bUnitReports);
}


//////////////////////////////////////////////////////////////////////////
//
// The units' compiling is timed by phase as well as counted
// (GetStatistics).  Set before the units are compiled.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetTimes(bool bTimes)
{
	mcStatistics.SetTimes(bTimes);
}


//////////////////////////////////////////////////////////////////////////
//
// What compiling the units was counted (and timed, SetTimes) in, added up
// as each finished, and linking.
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileStatistics* CCPP65Builder::GetStatistics(void)
{
	return &mcStatistics;
}


//////////////////////////////////////////////////////////////////////////
//
// How the units are compiled where there is a choice: speed or size, what is
// inlined, whether the target's assembly units are built.  Set them before
// anything is added to the builder: a unit's macros are fixed when it is.
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileOptions* CCPP65Builder::GetOptions(void)
{
	return mcBuild.GetOptions();
}


//////////////////////////////////////////////////////////////////////////
//
// The units of szLibrary (a project) are compiled without the optimiser,
// for a debugger (CCPPCompileOptions::mbDebuggable): every line steps as
// it is written.  The rest are compiled with GetOptions.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetUnoptimised(const char* szLibrary)
{
	mcBuild.SetUnoptimised(szLibrary);
}


//////////////////////////////////////////////////////////////////////////
//
// The units of szLibrary (a project) are compiled with pcOptions in place
// of GetOptions (CCPPBuild::SetLibraryOptions).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLibraryOptions(const char* szLibrary, CCPPCompileOptions* pcOptions)
{
	mcBuild.SetLibraryOptions(szLibrary, pcOptions);
}


//////////////////////////////////////////////////////////////////////////
//
// The units reported since the last time, in the order they finished,
// added to pasDest (KillUnitReports frees them).  Safe while Compile runs
// on another thread.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::TakeUnitReports(CArrayCPP65UnitReports* pasDest)
{
	mcSchedule.TakeUnitReports(pasDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::KillUnitReports(CArrayCPP65UnitReports* pasReports)
{
	CCPPUnitSchedule::KillUnitReports(pasReports);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumLazySkipped(void) const
{
	return mcSchedule.NumLazySkipped();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumLazyLoaded(void) const
{
	return mcSchedule.NumLazyLoaded();
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the lazy declarations loaded were read as parsed before (in
// an earlier unit), not parsed again.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumLazyRead(void) const
{
	return mcSchedule.NumLazyRead();
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the W65C816's units were compiled again because a function
// they call was exported after they were compiled (CompileUnits).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumCompiledAgain(void) const
{
	return mcSchedule.NumCompiledAgain();
}


//////////////////////////////////////////////////////////////////////////
//
// A header a unit preprocessed is kept, and the units after that include
// it with the same macros get its output without preprocessing it
// (CCPPPreprocessor::SetHeaderCache).  The code is the same.  On by default;
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
const CPPHeaderCache* CCPP65Builder::GetHeaderCache(void) const
{
	return &mcHeaderCache;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit, inlining before its turn, missed a
// function another unit exported meanwhile, and was generated again in its
// turn (CIRImports::Missed).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumImportsMissed(void) const
{
	return mcSchedule.NumImportsMissed();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit found before its turn that it had missed
// a function another unit exported while it inlined, and was generated
// again then (CIRImports::Arrive).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumImportsStale(void) const
{
	return mcSchedule.NumImportsStale();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit waited for its turn without a thread (CIRImports::
// Arrive).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumImportsParked(void) const
{
	return mcSchedule.NumImportsParked();
}


//////////////////////////////////////////////////////////////////////////
//
// For testing: every unit is generated twice, as if it missed an
// imported function the first time.  The code is the same.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetForceImportsMissed(bool bForce)
{
	mcSchedule.SetForceImportsMissed(bForce);
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
	return mcSchedule.SetSpillDirectory(szDirectory, uiCacheSize);
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
	mcBuild.SetLineTable(bLineTable);
}


//////////////////////////////////////////////////////////////////////////
//
// A program with no floating point (the HCP65's kernal): floating point
// in the code of a unit not linked as a library is an error at its
// statement (CCPPUnit::SetNoFloats), and a used function of any
// unit (a library's too) that uses the soft float or math.h is an error
// naming the function (CAsm65Linker::ForbidObject).  Call before Link.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetNoFloats(bool bNoFloats)
{
	mcBuild.SetNoFloats(bNoFloats);
	if (bNoFloats)
	{
		mcProgram.ForbidFloats();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether Garth Wilson's product table (MathMult) is in memory when the
// W65C816 program runs (true unless set).  Without it the runtime's
// multiplies are IRW65C816Runtime-MathSafe.asm's and an i8 multiply calls
// R816_Mul16 instead of reading the table in place: for a program that
// runs before the tables are loaded (the HCP65's kernal and operating
// system).  The divides read no table either way.  Call before
// AddMathTables and AddLibc65, which add the runtime.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetMathTables(bool bMathTables)
{
	if (mbRuntime && (mcBuild.GetOptions()->mbMathTables != bMathTables))
	{
		mszErrors.Append("CCPP65Builder::SetMathTables must be called before AddMathTables and AddLibc65.\n");
	}
	mcBuild.GetOptions()->mbMathTables = bMathTables;
}


//////////////////////////////////////////////////////////////////////////
//
// The linked program's source lines and fingerprints (SetLineTable).
//
//////////////////////////////////////////////////////////////////////////
CAsm65LineTable* CCPP65Builder::GetLineTable(void)
{
	return mcProgram.GetLineTable();
}


//////////////////////////////////////////////////////////////////////////
//
// What the last link gives a debugger: its line table and debug records.
//
//////////////////////////////////////////////////////////////////////////
CAsm65DebugInfo* CCPP65Builder::GetDebugInfo(void)
{
	return mcProgram.GetDebugInfo();
}


//////////////////////////////////////////////////////////////////////////
//
// A global's address after Link (see CAsm65Linker::GetSymbol).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::GetSymbol(const char* szName, int64* plliValue)
{
	return mcProgram.GetSymbol(szName, plliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Where the program must end (CPP65_HEAP_ADDRESS unless set).  A program in a memory of more than 2MB has its heap
// above 0020:0000 (Libc65's malloc), so it can end nearer the stack.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetLimit(uint32 uiLimit)
{
	mcProgram.SetLimit(uiLimit);
}


//////////////////////////////////////////////////////////////////////////
//
// Where the program's code starts (CPP65_PROGRAM_ADDRESS unless set): for
// a program that runs somewhere else, such as a ROM image copied into a
// higher bank.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetTextAddress(uint32 uiTextAddress)
{
	mcProgram.SetTextAddress(uiTextAddress);
}


//////////////////////////////////////////////////////////////////////////
//
// The lazy units' sectors, to write to the drive from the first sector given to
// SetLazy.
//
//////////////////////////////////////////////////////////////////////////
CArrayUint8* CCPP65Builder::GetLazyImage(void)
{
	return mcProgram.GetLazyImage();
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
	mcBuild.GetTarget()->AppendStartUp(&szAssembly, mcBuild.GetOptions()->mbExceptions);
	bResult = mcProgram.AddObject("start up", szAssembly.Text(), true);
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
	bResult &= mcProgram.AddCompiledObject(szName, szAssembly.Text());
	szAssembly.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles szSource (AddSource's and LinkWith's) with the builder's
// settings: its line table, its listing (UnitName.lst) when it keeps
// listings, and its classes added to the layouts file when there is one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::GenerateSource(CChars* pszAssembly, const char* szName, const char* szSource)
{
	bool	bResult;

	bResult = mcBuild.Generate(szName, szSource, false, pszAssembly, &mszErrors);
	if (mcBuild.HasListingDirectory())
	{
		bResult &= mcBuild.WriteListing(szName, pszAssembly, &mszErrors);
	}
	return bResult & mcBuild.WriteLayouts(&mszErrors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddAssembly(const char* szName, const char* szAssembly)
{
	return mcProgram.AddObject(szName, szAssembly);
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
	bResult &= mcProgram.LinkObjects();
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

	//The options of the units SetUnoptimised names, set once before the units' threads read them.
	mcBuild.FixOptions();
	bResult = mcSchedule.CompileUnits(muiUnits);
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

	mcProgram.ResetLinker();
	szAssembly.Init();
	bResult = GenerateSource(&szAssembly, szName, szSource);
	bResult &= mcProgram.AddLinkWithObject(szName, szAssembly.Text());
	szAssembly.Kill();

	bResult &= mcProgram.LinkObjects();
	return bResult && mszErrors.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Output* CCPP65Builder::GetOutput(void)
{
	return mcProgram.GetOutput();
}


//////////////////////////////////////////////////////////////////////////
//
// The linker, for the linked program's symbols once Link has run.
//
//////////////////////////////////////////////////////////////////////////
CAsm65Linker* CCPP65Builder::GetLinker(void)
{
	return mcProgram.GetLinker();
}


//////////////////////////////////////////////////////////////////////////
//
// Every stage's errors: preprocessing, tokenising, parsing, generating, assembling and linking.
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65Builder::GetErrors(void) const
{
	return mszErrors.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65Builder::GetWarnings(void) const
{
	return mszWarnings.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65Builder::GetMap(void)
{
	return mcProgram.GetMap();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumMissing(void)
{
	return mcProgram.NumMissing();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPP65Builder::GetMissing(size uiIndex)
{
	return mcProgram.GetMissing(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The objects linked (not those in link libraries): the units' (once
// compiled) and the assembly's, in the order they were added.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnitObjects(void) const
{
	return mcProgram.NumUnitObjects();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CCPP65Builder::GetUnitObject(size uiIndex)
{
	return mcProgram.GetUnitObject(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The labels no object defines, sorted and space separated.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::GetMissingNames(CChars* pszDest)
{
	mcProgram.GetMissingNames(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// What is built and how (CCPPBuild): what a test compiles one source with
// (CCPPBuild::Generate), with the builder's include directories.
//
//////////////////////////////////////////////////////////////////////////
CCPPBuild* CCPP65Builder::GetBuild(void)
{
	return &mcBuild;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit has finished compiling, for showing progress from
// another thread while Compile runs.  A unit compiled again counts
// twice, so it can pass NumUnitsToCompile.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Builder::NumUnitsFinished(void)
{
	return mcSchedule.NumUnitsFinished();
}


