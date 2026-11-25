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
#ifndef __C_P_P_PROGRAM_LINKER_H__
#define __C_P_P_PROGRAM_LINKER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "Assembler65Lib/Asm65Object.h"
#include "Assembler65Lib/Asm65Linker.h"
#include "CPPBuild.h"


class CLibrary;
class CTranslationUnit;


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


//The objects of a program and how they are linked (CCPP65Builder has one): the start up, the assembly and the
//compiled sources' and units' objects in the order they were added, the libraries some units are linked as, and the
//linker with its settings (where the text goes and must end, missing labels stubbed, what nothing uses left out,
//functions loaded lazily).  A function C++ calls and tests for an exception must be compiled C++ (CheckTested).
class CCPPProgramLinker
{
protected:
	CCPPBuild*						mpcBuild;			//The target, and whether floats are forbidden.
	CChars*							mpszErrors;			//Where the link's and the assembler's errors go.
	CCPPCompileStatistics*			mpcStatistics;		//What linking is timed in.
	CArrayCPP65LinkLibraries		macLinkLibraries;
	uint32							muiLimit;
	uint32							muiTextAddress;
	bool							mbStubMissing;
	bool							mbRemoveUnused;
	bool							mbLazy;
	uint32							muiLazySectorSize;
	uint32							muiLazyFirstSector;
	CAsm65Library					mcObjects;
	CArrayCPP65Objects				macObjects;
	CAsm65Library					mcLinkWithObjects;
	CAsm65Linker					mcLinker;
	CArrayChars						maszTested;			//The functions the compiled units call by name and test for an exception...
	CArrayTemplate<CAsm65Object*>	mapcCompiled;		//...and the objects that were compiled from C++: a tested function must be in one (CheckTested).

public:
	void				Init(CCPPBuild* pcBuild, CChars* pszErrors, CCPPCompileStatistics* pcStatistics);
	void				Kill(void);

	void				SetStubMissing(bool bStubMissing);
	void				SetRemoveUnused(bool bRemoveUnused);
	void				SetLazy(uint32 uiSectorSize, uint32 uiFirstSector);
	void				SetLimit(uint32 uiLimit);
	void				SetTextAddress(uint32 uiTextAddress);
	void				ForbidFloats(void);

	void				AddLinkLibrary(CLibrary* pcLibrary, const char* szName);
	CAsm65Library*		GetLinkLibrary(CLibrary* pcLibrary);
	bool				AddObject(const char* szName, const char* szAssembly, bool bResident = false, const char* szDirectory = NULL, const char* szSourceFile = NULL);
	bool				AddObject(const char* szName, CAsm65Source* pcSource, bool bResident = false, const char* szDirectory = NULL, const char* szSourceFile = NULL);
	bool				AddCompiledObject(const char* szName, const char* szAssembly);
	CAsm65Object*		AddUnitObject(CTranslationUnit* pcFile);
	void				AddCompiled(CAsm65Object* pcObject, CArrayChars* paszTested);
	CArrayChars*		GetTested(void);
	bool				Assemble(CAsm65Object* pcObject, const char* szAssembly, const char* szDirectory = NULL, const char* szSourceFile = NULL);
	bool				Assemble(CAsm65Object* pcObject, CAsm65Source* pcSource, const char* szDirectory, CChars* pszErrors, const char* szSourceFile = NULL);
	void				ResetLinker(void);
	bool				AddLinkWithObject(const char* szName, const char* szAssembly);
	bool				LinkObjects(void);

	CAsm65Output*		GetOutput(void);
	CAsm65Linker*		GetLinker(void);
	const char*			GetMap(void);
	size				NumMissing(void);
	char*				GetMissing(size uiIndex);
	void				GetMissingNames(CChars* pszDest);
	size				NumUnitObjects(void) const;
	CAsm65Object*		GetUnitObject(size uiIndex);
	size				NumUnits(void);
	CAsm65LineTable*	GetLineTable(void);
	CAsm65DebugInfo*	GetDebugInfo(void);
	bool				GetSymbol(const char* szName, int64* plliValue);
	CArrayUint8*		GetLazyImage(void);

protected:
	bool				CheckTested(void);
};


#endif // __C_P_P_PROGRAM_LINKER_H__
