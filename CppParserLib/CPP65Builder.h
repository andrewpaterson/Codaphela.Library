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


//Builds a 65C64 program.  Each part is an object: the start up, C++ sources given as text, assembly, and the
//translation units of a project.  They are linked in the order they were added (the project's units last, when
//Link compiles them) with the text at CPP65_PROGRAM_ADDRESS, ending below CPP65_HEAP_ADDRESS.  A project library
//can instead be linked as a library, so only the units something uses are linked.  A label no object
//defines is stubbed with stp (unless SetStubMissing(false)), so a call to it stops the program; NumMissing and
//GetMissing list them.  Functions, globals and string literals nothing uses are left out (unless
//SetRemoveUnused(false)): the start up and the init and fini entries of used globals are what is kept.
class CCPP65Builder
{
protected:
	CArrayCPP65IncludeDirectories	macIncludeDirectories;
	CArrayCPP65LinkLibraries		macLinkLibraries;
	CProject						mcProject;
	size							muiUnits;
	CAsm65Library					mcObjects;
	CAsm65Linker					mcLinker;
	CChars							mszErrors;

public:
	void			Init(void);
	void			Kill(void);

	void			AddIncludeDirectory(const char* szDirectory, bool bIncludeSubDirectories, bool bSystem);
	void			AddUnit(const char* szLibrary, const char* szDirectory, const char* szFile, bool bLinkAsLibrary = false);
	void			SetStubMissing(bool bStubMissing);
	void			SetRemoveUnused(bool bRemoveUnused);

	bool			AddStartUp(void);
	bool			AddSource(const char* szName, const char* szSource);
	bool			AddAssembly(const char* szName, const char* szAssembly);
	bool			Link(void);

	CAsm65Output*	GetOutput(void);
	char*			GetErrors(void);
	char*			GetMap(void);
	size			NumMissing(void);
	char*			GetMissing(size uiIndex);

	static bool		Generate(CChars* pszAssembly, CChars* pszErrors, CArrayCPP65IncludeDirectories* pacIncludeDirectories, const char* szName, const char* szSource, bool bProgram, CArrayChars* paszExternals);
	CArrayCPP65IncludeDirectories*	GetIncludeDirectories(void);

protected:
	bool			AddObject(const char* szName, const char* szAssembly);
	bool			Assemble(CAsm65Object* pcObject, const char* szAssembly);
	bool			CompileUnits(void);
	CAsm65Library*	GetLinkLibrary(CLibrary* pcLibrary);
};


#endif // __C_P_P_65_BUILDER_H__

