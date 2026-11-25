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
#ifndef __C_P_P_65_BUILDER_H__
#define __C_P_P_65_BUILDER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "Assembler65Lib/Asm65Object.h"
#include "Assembler65Lib/Asm65Linker.h"
#include "Project.h"
#include "CPPTarget.h"
#include "Assembler65Lib/Asm65Profile.h"
#include "CPP65Assembly.h"
#include "CPPCompileStatistics.h"
#include "CPPBuild.h"
#include "CPPProgramLinker.h"
#include "CPPUnitSchedule.h"


//Builds a program for a target (SetTarget, before anything is added: the 65C64 unless set, or the W65C816, whose
//sections are kept inside 64KB banks and only the vectors and their stubs are in bank 0).  Each part is an object: the start up, C++ sources given as text, assembly, and the
//translation units of a project.  They are linked in the order they were added (the project's units last, when
//Link compiles them) with the text at CPP65_PROGRAM_ADDRESS (unless SetTextAddress), ending below CPP65_HEAP_ADDRESS.  A project library
//can instead be linked as a library, so only the units something uses are linked.  A label no object
//defines is stubbed with stp (unless SetStubMissing(false)), so a call to it stops the program; NumMissing and
//GetMissing list them.  Functions, globals and string literals nothing uses are left out (unless
//SetRemoveUnused(false)): the start up and the init and fini entries of used globals are what is kept.  With
//SetLazy the functions are loaded from a drive when they are first called (only the start up and the runtime
//routines are resident); GetLazyImage is what to write to the drive.  Compile then LinkWith links the compiled
//objects again and again, each time with another source (a main, for example), so a large project can be run as
//several programs that each fit.  On the W65C816 small functions are inlined across the project's units (CIRImports):
//a unit compiled before one whose function it calls is compiled again at the end.  With SetJobs several units are
//preprocessed and compiled at once; what is built is the same as compiling them one at a time.
class CCPP65Builder
{
protected:
	CCPPUnitSchedule				mcSchedule;			//What compiles the project's units.
	CCPPProgramLinker				mcProgram;			//The objects and how they are linked.
	CCPPBuild						mcBuild;			//What is built and how: the target, options, include directories, listings.
	CProject						mcProject;
	size							muiUnits;
	bool							mbCompiled;
	CChars							mszErrors;
	CChars							mszWarnings;		//The generator's warnings for every unit compiled; they don't fail a build.
	CArrayChars						maszOutsideProjects;	//A project with a source outside its directory, and that
	CArrayChars						maszOutsideLibraries;	//source's library (AddVisualStudioProject), each pair at an index.
	bool							mbRuntime;			//The target's runtime units have been added (AddRuntime).
	CPPHeaderCache					mcHeaderCache;			//Headers' preprocessed output (SetHeaderCache).
	CCPPCompileStatistics			mcStatistics;			//The units' counts and times, and the builder's own (linking).

public:
	void					Init(void);
	void					Kill(void);
	void					SetTarget(ECPPTarget eTarget);
	void					SetFrameProfile(CAsm65Profile* pcProfile);
	void					SetListingDirectory(const char* szDirectory);
	void					SetLayoutsFile(const char* szFile);
	bool					SetSpillDirectory(const char* szDirectory, lsize uiCacheSize);
	void					SetLazyHeaders(bool bLazyHeaders);
	void					SetJobs(CJobs* pcJobs);
	void					SetUnitReports(bool bUnitReports);
	void					SetTimes(bool bTimes);
	CCPPCompileStatistics*	GetStatistics(void);
	CCPPCompileOptions*		GetOptions(void);
	void					SetUnoptimised(const char* szLibrary);
	void					TakeUnitReports(CArrayCPP65UnitReports* pasDest);
	static void				KillUnitReports(CArrayCPP65UnitReports* pasReports);
	size					NumLazySkipped(void) const;
	size					NumLazyLoaded(void) const;
	size					NumLazyRead(void) const;
	size					NumCompiledAgain(void) const;
	void					SetHeaderCache(bool bHeaderCache);
	const CPPHeaderCache*	GetHeaderCache(void) const;
	size					NumImportsMissed(void) const;
	size					NumImportsStale(void) const;
	size					NumImportsParked(void) const;
	void					SetForceImportsMissed(bool bForce);
	void					SetLineTable(bool bLineTable);
	void					SetNoFloats(bool bNoFloats);
	void					SetMathTables(bool bMathTables);
	CAsm65LineTable*	GetLineTable(void);
	CAsm65DebugInfo*	GetDebugInfo(void);

	bool					AddIncludeDirectory(const char* szDirectory, bool bIncludeSubDirectories, bool bSystem);
	void					AddUnit(const char* szLibrary, const char* szDirectory, const char* szFile, bool bLinkAsLibrary = false);
	void					AddMathTables(const char* szLibc65Directory);
	void					AddRuntime(const char* szLibc65Directory);
	void					AddLibc65(const char* szLibc65Directory);
	bool					AddVisualStudioProject(const char* szLibrary, const char* szProjectFile, bool bLinkAsLibrary, const char* szExcluded = NULL);
	void					AddProjectSources(const char* szLibrary, const char* szDirectory, CArrayChars* paszSources, bool bLinkAsLibrary);
	bool					AddDefine(const char* szLibrary, const char* szDefine);
	void					SetStubMissing(bool bStubMissing);
	void					SetRemoveUnused(bool bRemoveUnused);
	void					SetLazy(uint32 uiSectorSize, uint32 uiFirstSector);
	void					SetLimit(uint32 uiLimit);
	void					SetTextAddress(uint32 uiTextAddress);

	bool					AddStartUp(void);
	bool					AddSource(const char* szName, const char* szSource);
	bool					AddAssembly(const char* szName, const char* szAssembly);
	bool					Link(void);
	bool					Compile(void);
	bool					LinkWith(const char* szName, const char* szSource);

	CAsm65Output*			GetOutput(void);
	CAsm65Linker*			GetLinker(void);
	const char*				GetErrors(void) const;
	const char*				GetWarnings(void) const;
	const char*				GetMap(void);
	size					NumMissing(void);
	char*					GetMissing(size uiIndex);
	size					NumUnitObjects(void) const;
	CAsm65Object*			GetUnitObject(size uiIndex);
	void					GetMissingNames(CChars* pszDest);
	size					NumUnits(void);
	size					NumUnitsToCompile(void) const;
	size					NumUnitsFinished(void);
	bool					GetSymbol(const char* szName, int64* plliValue);
	CArrayUint8*			GetLazyImage(void);

	CCPPBuild*				GetBuild(void);

protected:
	void					AddAssemblyUnit(const char* szLibrary, const char* szDirectory, const char* szSource, bool bLinkAsLibrary);
	bool					GenerateSource(CChars* pszAssembly, const char* szName, const char* szSource);
};


#endif // __C_P_P_65_BUILDER_H__

