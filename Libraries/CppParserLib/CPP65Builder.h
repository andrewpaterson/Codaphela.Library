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
#ifndef __C_P_P_65_BUILDER_H__
#define __C_P_P_65_BUILDER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "Assembler65Lib/Asm65Object.h"
#include "Assembler65Lib/Asm65Linker.h"
#include "Project.h"
#include "CPPTarget.h"
#include "IRImports.h"
#include "Assembler65Lib/Asm65816Profile.h"
#include "CPP65Assembly.h"
#include "CPPHeaderIndex.h"


struct SCPP65IncludeDirectory
{
	CChars	szDirectory;
	bool	bIncludeSubDirectories;
	bool	bSystem;
};


typedef CArrayTemplate<SCPP65IncludeDirectory>	CArrayCPP65IncludeDirectories;


//A project library whose units are linked as a library: a unit is only linked if another object needs it.
struct SCPP65LinkLibrary
{
	CLibrary*		pcLibrary;
	CAsm65Library	cObjects;
};


typedef CArrayTemplate<SCPP65LinkLibrary>	CArrayCPP65LinkLibraries;


//An object given to the linker, again for each LinkWith.
struct SCPP65Object
{
	CAsm65Object*	pcObject;
	bool			bResident;
};


typedef CArrayTemplate<SCPP65Object>	CArrayCPP65Objects;


//A translation unit compiled by CompileUnits: its object, and the functions it calls that no unit had exported
//when it was compiled (it is compiled again if one is exported later).
struct SCPP65CompiledUnit
{
	CTranslationUnit*	pcFile;
	CAsm65Object*		pcObject;
	CArrayChars			aszMissing;
};


typedef CArrayTemplate<SCPP65CompiledUnit>	CArrayCPP65CompiledUnits;


//Builds a program for a target (SetTarget, before anything is added: the 65C64 unless set, or the W65C816, whose
//sections are kept inside 64KB banks and only the vectors and their stubs are in bank 0).  Each part is an object: the start up, C++ sources given as text, assembly, and the
//translation units of a project.  They are linked in the order they were added (the project's units last, when
//Link compiles them) with the text at CPP65_PROGRAM_ADDRESS, ending below CPP65_HEAP_ADDRESS.  A project library
//can instead be linked as a library, so only the units something uses are linked.  A label no object
//defines is stubbed with stp (unless SetStubMissing(false)), so a call to it stops the program; NumMissing and
//GetMissing list them.  Functions, globals and string literals nothing uses are left out (unless
//SetRemoveUnused(false)): the start up and the init and fini entries of used globals are what is kept.  With
//SetLazy the functions are loaded from a drive when they are first called (only the start up and the runtime
//routines are resident); GetLazyImage is what to write to the drive.  Compile then LinkWith links the compiled
//objects again and again, each time with another source (a main, for example), so a large project can be run as
//several programs that each fit.  On the W65C816 small functions are inlined across the project's units (CIRImports):
//a unit compiled before one whose function it calls is compiled again at the end.
class CCPP65Builder
{
protected:
	CArrayCPP65IncludeDirectories	macIncludeDirectories;
	CArrayCPP65LinkLibraries		macLinkLibraries;
	CProject						mcProject;
	ECPPTarget						meTarget;
	size							muiUnits;
	uint32							muiLimit;
	bool							mbCompiled;
	bool							mbStubMissing;
	bool							mbRemoveUnused;
	bool							mbLazy;
	uint32							muiLazySectorSize;
	uint32							muiLazyFirstSector;
	CAsm65Library					mcObjects;
	CArrayCPP65Objects				macObjects;
	CAsm65Library					mcLinkWithObjects;
	CAsm65Linker					mcLinker;
	CChars							mszErrors;
	CChars							mszWarnings;		//The generator's warnings for every unit compiled; they don't fail a build.
	CIRImports						mcImports;			//The units' small functions, for inlining in the others.
	CAsm65816Profile*					mpcFrameProfile;	//Measured functions, for the W65C816's aligned frames, or NULL.
	CChars							mszListings;		//Where each unit's listing is written (SetListingDirectory), empty for none.
	bool							mbLineTable;		//Units are compiled with .line and .fingerprint (SetLineTable).
	int64							mlliCompileTime;	//When SetLineTable was called (seconds since 1970).
	CSpillStore*					mpcSpillStore;			//Where the units' functions' code waits to be assembled (SetSpillDirectory).
	CMemorySpillStore				mcMemorySpillStore;
	CIndexTreeSpillStore			mcIndexTreeSpillStore;
	bool							mbLazyHeaders;			//Header declarations are parsed when a unit first uses them (SetLazyHeaders).
	CCPPHeaderIndex					mcHeaderIndex;			//The header declarations the units have parsed.
	size							muiLazySkipped;			//The units' header declarations left unparsed (SetLazyHeaders)...
	size							muiLazyLoaded;			//...and parsed when used.

public:
	void			Init(void);
	void			Kill(void);
	void			SetTarget(ECPPTarget eTarget);
	void			SetFrameProfile(CAsm65816Profile* pcProfile);
	void			SetListingDirectory(const char* szDirectory);
	bool			SetSpillDirectory(const char* szDirectory, lsize uiCacheSize);
	void			SetLazyHeaders(bool bLazyHeaders);
	size			NumLazySkipped(void);
	size			NumLazyLoaded(void);
	void			SetLineTable(bool bLineTable);
	CAsm65LineTable*	GetLineTable(void);

	void			AddIncludeDirectory(const char* szDirectory, bool bIncludeSubDirectories, bool bSystem);
	void			AddUnit(const char* szLibrary, const char* szDirectory, const char* szFile, bool bLinkAsLibrary = false);
	void			AddMathTables(const char* szLibc65Directory);
	bool			AddVisualStudioProject(const char* szLibrary, const char* szProjectFile, bool bLinkAsLibrary, const char* szExcluded = NULL);
	void			SetStubMissing(bool bStubMissing);
	void			SetRemoveUnused(bool bRemoveUnused);
	void			SetLazy(uint32 uiSectorSize, uint32 uiFirstSector);
	void			SetLimit(uint32 uiLimit);

	bool			AddStartUp(void);
	bool			AddSource(const char* szName, const char* szSource);
	bool			AddAssembly(const char* szName, const char* szAssembly);
	bool			Link(void);
	bool			Compile(void);
	bool			LinkWith(const char* szName, const char* szSource);

	CAsm65Output*	GetOutput(void);
	char*			GetErrors(void);
	char*			GetWarnings(void);
	char*			GetMap(void);
	size			NumMissing(void);
	char*			GetMissing(size uiIndex);
	void			GetMissingNames(CChars* pszDest);
	size			NumUnits(void);
	bool			GetSymbol(const char* szName, int64* plliValue);
	CArrayUint8*	GetLazyImage(void);

	static bool		Generate(CChars* pszAssembly, CChars* pszErrors, CArrayCPP65IncludeDirectories* pacIncludeDirectories, const char* szName, const char* szSource, bool bProgram, CArrayChars* paszExternals, ECPPTarget eTarget = CPPT_65C64, CChars* pszWarnings = NULL, CAsm65816Profile* pcFrameProfile = NULL, bool bListing = false, bool bLineTable = false);
	CArrayCPP65IncludeDirectories*	GetIncludeDirectories(void);

protected:
	bool			AddObject(const char* szName, const char* szAssembly, bool bResident = false, const char* szDirectory = NULL);
	bool			AddObject(const char* szName, CAsm65Source* pcSource, bool bResident = false, const char* szDirectory = NULL);
	bool			Assemble(CAsm65Object* pcObject, const char* szAssembly, const char* szDirectory = NULL);
	bool			Assemble(CAsm65Object* pcObject, CAsm65Source* pcSource, const char* szDirectory = NULL);
	void			AddW65C816Assembly(const char* szLibrary, const char* szDirectory, const char* szSource, bool bLinkAsLibrary);
	bool			CompileUnits(void);
	bool			CompileUnit(SCPP65CompiledUnit* psUnit);
	bool			GenerateSource(CChars* pszAssembly, const char* szName, const char* szSource);
	bool			WriteListing(const char* szUnit, CChars* pszAssembly);
	bool			WriteListing(const char* szUnit, CCPP65Assembly* pcAssembly);
	bool			ListingName(const char* szUnit, CChars* pszPath);
	CAsm65Library*	GetLinkLibrary(CLibrary* pcLibrary);
	bool			LinkObjects(void);
	void			ResetLinker(void);
};


#endif // __C_P_P_65_BUILDER_H__

