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
#include "BaseLib/StringHelper.h"
#include "Project.h"
#include "CPPUnit.h"
#include "CPPProgramLinker.h"


//////////////////////////////////////////////////////////////////////////
//
// The objects are linked for pcBuild's target; the errors go to pszErrors
// and linking is timed in pcStatistics.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::Init(CCPPBuild* pcBuild, CChars* pszErrors, CCPPCompileStatistics* pcStatistics)
{
	mpcBuild = pcBuild;
	mpszErrors = pszErrors;
	mpcStatistics = pcStatistics;
	macLinkLibraries.Init();
	muiLimit = CPP65_HEAP_ADDRESS;
	muiTextAddress = CPP65_PROGRAM_ADDRESS;
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
	maszTested.Init();
	mapcCompiled.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::Kill(void)
{
	size	i;

	mapcCompiled.Kill();
	maszTested.Kill();
	mcLinker.Kill();
	mcLinkWithObjects.Kill();
	macObjects.Kill();
	mcObjects.Kill();
	for (i = 0; i < macLinkLibraries.NumElements(); i++)
	{
		macLinkLibraries.Get(i)->cObjects.Kill();
	}
	macLinkLibraries.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcLibrary's units are linked as the library szName: only those another
// object needs.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::AddLinkLibrary(CLibrary* pcLibrary, const char* szName)
{
	SCPP65LinkLibrary*	psLinkLibrary;

	psLinkLibrary = macLinkLibraries.Add();
	psLinkLibrary->pcLibrary = pcLibrary;
	psLinkLibrary->cObjects.Init(szName);
}


//////////////////////////////////////////////////////////////////////////
//
// A compiled source's object, assembled from szAssembly and linked: C++'s,
// so the functions C++ tests may be in it (CheckTested).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPProgramLinker::AddCompiledObject(const char* szName, const char* szAssembly)
{
	bool	bResult;

	bResult = AddObject(szName, szAssembly);
	mapcCompiled.Add(&macObjects.Tail()->pcObject);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What a compiled unit tests (paszTested), and that its object (if it has
// one) is C++'s, for CheckTested.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::AddCompiled(CAsm65Object* pcObject, CArrayChars* paszTested)
{
	size	ui;

	for (ui = 0; ui < paszTested->NumElements(); ui++)
	{
		maszTested.AddIfUnique(paszTested->GetText(ui));
	}
	if (pcObject)
	{
		mapcCompiled.Add(&pcObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The functions compiled sources call by name and test, for a source
// compiled outside the units (CCPPBuild::SetTested).
//
//////////////////////////////////////////////////////////////////////////
CArrayChars* CCPPProgramLinker::GetTested(void)
{
	return &maszTested;
}


//////////////////////////////////////////////////////////////////////////
//
// LinkWith's source's object, from szAssembly, in place of the last
// one's, linked after the others (ResetLinker first).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPProgramLinker::AddLinkWithObject(const char* szName, const char* szAssembly)
{
	bool	bResult;

	mcLinkWithObjects.Kill();
	mcLinkWithObjects.Init("link with");
	bResult = Assemble(mcLinkWithObjects.AddObject(szName), szAssembly);
	mcLinker.AddObject(mcLinkWithObjects.GetObject(0));
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a label no object defines is stubbed (the default) or is a link error.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::SetStubMissing(bool bStubMissing)
{
	mbStubMissing = bStubMissing;
	mcLinker.SetStubMissing(bStubMissing);
}

//////////////////////////////////////////////////////////////////////////
//
// Whether what nothing uses is left out of the program (the default).
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::SetRemoveUnused(bool bRemoveUnused)
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
void CCPPProgramLinker::SetLazy(uint32 uiSectorSize, uint32 uiFirstSector)
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
size CCPPProgramLinker::NumUnits(void)
{
	return mcLinker.NumUnits();
}

//////////////////////////////////////////////////////////////////////////
//
// The linked program's source lines and fingerprints (SetLineTable).
//
//////////////////////////////////////////////////////////////////////////
CAsm65LineTable* CCPPProgramLinker::GetLineTable(void)
{
	return mcLinker.GetLineTable();
}

//////////////////////////////////////////////////////////////////////////
//
// What the last link gives a debugger: its line table and debug records.
//
//////////////////////////////////////////////////////////////////////////
CAsm65DebugInfo* CCPPProgramLinker::GetDebugInfo(void)
{
	return mcLinker.GetDebugInfo();
}

//////////////////////////////////////////////////////////////////////////
//
// A global's address after Link (see CAsm65Linker::GetSymbol).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPProgramLinker::GetSymbol(const char* szName, int64* plliValue)
{
	return mcLinker.GetSymbol(szName, plliValue);
}

//////////////////////////////////////////////////////////////////////////
//
// Where the program must end (CPP65_HEAP_ADDRESS unless set).  A program in a memory of more than 2MB has its heap
// above 0020:0000 (Libc65's malloc), so it can end nearer the stack.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::SetLimit(uint32 uiLimit)
{
	muiLimit = uiLimit;
}

//////////////////////////////////////////////////////////////////////////
//
// Where the program's code starts (CPP65_PROGRAM_ADDRESS unless set): for
// a program that runs somewhere else, such as a ROM image copied into a
// higher bank.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::SetTextAddress(uint32 uiTextAddress)
{
	muiTextAddress = uiTextAddress;
}

//////////////////////////////////////////////////////////////////////////
//
// The lazy units' sectors, to write to the drive from the first sector given to
// SetLazy.
//
//////////////////////////////////////////////////////////////////////////
CArrayUint8* CCPPProgramLinker::GetLazyImage(void)
{
	return mcLinker.GetLazyImage();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Output* CCPPProgramLinker::GetOutput(void)
{
	return mcLinker.GetOutput();
}

//////////////////////////////////////////////////////////////////////////
//
// The linker, for the linked program's symbols once Link has run.
//
//////////////////////////////////////////////////////////////////////////
CAsm65Linker* CCPPProgramLinker::GetLinker(void)
{
	return &mcLinker;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPProgramLinker::GetMap(void)
{
	return mcLinker.GetMap();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPProgramLinker::NumMissing(void)
{
	return mcLinker.NumMissing();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPProgramLinker::GetMissing(size uiIndex)
{
	return mcLinker.GetMissing(uiIndex);
}

//////////////////////////////////////////////////////////////////////////
//
// The objects linked (not those in link libraries): the units' (once
// compiled) and the assembly's, in the order they were added.
//
//////////////////////////////////////////////////////////////////////////
size CCPPProgramLinker::NumUnitObjects(void) const
{
	return macObjects.NumElements();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CCPPProgramLinker::GetUnitObject(size uiIndex)
{
	return macObjects.Get(uiIndex)->pcObject;
}

//////////////////////////////////////////////////////////////////////////
//
// The labels no object defines, sorted and space separated.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::GetMissingNames(CChars* pszDest)
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
void CCPPProgramLinker::ForbidFloats(void)
{
	static const char*	aszFloatUnits[] = { "softfloatW65C816.asm", "softfloat.asm", "softfloat.cpp", "math.cpp", NULL };
	size				i;

	for (i = 0; aszFloatUnits[i]; i++)
	{
		mcLinker.ForbidObject(aszFloatUnits[i], "Floating point is used, and this build has none (CCPPProgramLinker::SetNoFloats: the kernal has no floats)");
	}
}

//////////////////////////////////////////////////////////////////////////
//
// A new linker with the builder's settings and objects, for LinkWith.
//
//////////////////////////////////////////////////////////////////////////
void CCPPProgramLinker::ResetLinker(void)
{
	SCPP65Object*	psObject;
	size			i;

	mpszErrors->Kill();
	mpszErrors->Init();
	mcLinker.Kill();
	mcLinker.Init();
	mcLinker.SetStubMissing(mbStubMissing);
	mcLinker.SetRemoveUnused(mbRemoveUnused);
	if (mpcBuild->HasNoFloats())
	{
		ForbidFloats();
	}
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
// Exceptions (ExceptionsPlan.txt): a function that throws returns with a
// flag set, and compiled C++ returns with it clear when it doesn't.  An
// assembly routine leaves it as it falls, so C++ that tests a call of one
// would take what it finds for an exception.  Every function the compiled
// units call by name and test must therefore be one a compiled unit
// defines; one that is linked from anything else is an error, which says
// what to do: declare it noexcept, and its calls aren't tested.  (A function
// nothing defines is the linker's to report, or a stub.)
//
//////////////////////////////////////////////////////////////////////////
bool CCPPProgramLinker::CheckTested(void)
{
	CAsm65Object*	pcObject;
	const char*		szName;
	size			ui;
	size			uiObject;
	size			uiMissing;
	int32			iSymbol;
	bool			bCompiled;
	bool			bMissing;
	bool			bResult;

	bResult = true;
	for (ui = 0; ui < maszTested.NumElements(); ui++)
	{
		szName = maszTested.GetText(ui);
		bCompiled = false;
		for (uiObject = 0; !bCompiled && (uiObject < mapcCompiled.NumElements()); uiObject++)
		{
			pcObject = *mapcCompiled.Get(uiObject);
			iSymbol = pcObject->FindSymbol(szName);
			bCompiled = (iSymbol >= 0) && (pcObject->GetSymbol((size)iSymbol)->iSection >= 0);
		}
		bMissing = false;
		for (uiMissing = 0; !bCompiled && !bMissing && (uiMissing < mcLinker.NumMissing()); uiMissing++)
		{
			bMissing = strcmp(mcLinker.GetMissing(uiMissing), szName) == 0;
		}
		if (!bCompiled && !bMissing)
		{
			mpszErrors->Append(szName)->Append(" is assembly and is called by C++ that tests for an exception: declare it noexcept.\n");
			bResult = false;
		}
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPProgramLinker::LinkObjects(void)
{
	const char*	szPrefix;
	bool		bResult;
	size		i;

	//The libraries are only added now, when their array won't move them any more.
	for (i = 0; i < macLinkLibraries.NumElements(); i++)
	{
		mcLinker.AddLibrary(&macLinkLibraries.Get(i)->cObjects);
	}
	mcLinker.SetAddress(ASK65_Text, muiTextAddress);
	mcLinker.SetLimit(muiLimit);
	mcLinker.SetBankSize(mpcBuild->GetTarget()->GetBankSize());
	if (mpcBuild->GetTarget()->GetStubOpcode() >= 0)
	{
		mcLinker.SetStubOpcode((uint8)mpcBuild->GetTarget()->GetStubOpcode());
	}
	mpcStatistics->Begin(CPPCP_Link);
	bResult = mcLinker.Link();
	mpcStatistics->End(CPPCP_Link);
	mpszErrors->Append(mcLinker.GetErrors());
	bResult &= CheckTested();

	//A runtime routine isn't stubbed: its program would stop at the first multiply.
	szPrefix = mpcBuild->GetTarget()->GetRuntimePrefix();
	for (i = 0; szPrefix && (i < mcLinker.NumMissing()); i++)
	{
		if (strncmp(mcLinker.GetMissing(i), szPrefix, strlen(szPrefix)) == 0)
		{
			mpszErrors->Append("The ")->Append(mpcBuild->GetTarget()->GetConfigurationName())->Append(" runtime routine ")->Append(mcLinker.GetMissing(i))->Append(" isn't linked (CCPPProgramLinker::AddMathTables or AddLibc65 adds the runtime).\n");
			bResult = false;
		}
	}
	return bResult;
}

//////////////////////////////////////////////////////////////////////////
//
// Assembles one object and adds it to the link.  .inchex file names are
// relative to szDirectory.  szSourceFile, the assembly's own file, makes
// its lines that write bytes source lines (CAsm65BaseAssembler::SetSourceFile).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPProgramLinker::AddObject(const char* szName, const char* szAssembly, bool bResident, const char* szDirectory, const char* szSourceFile)
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
bool CCPPProgramLinker::AddObject(const char* szName, CAsm65Source* pcSource, bool bResident, const char* szDirectory, const char* szSourceFile)
{
	CAsm65Object*	pcObject;
	SCPP65Object*	psObject;
	bool			bResult;

	pcObject = mcObjects.AddObject(szName);
	bResult = Assemble(pcObject, pcSource, szDirectory, mpszErrors, szSourceFile);
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
bool CCPPProgramLinker::Assemble(CAsm65Object* pcObject, const char* szAssembly, const char* szDirectory, const char* szSourceFile)
{
	CAsm65TextSource	cSource;
	bool				bResult;

	cSource.Init(szAssembly);
	bResult = Assemble(pcObject, &cSource, szDirectory, mpszErrors, szSourceFile);
	cSource.Kill();
	return bResult;
}

//////////////////////////////////////////////////////////////////////////
//
// Assembles pcSource into pcObject with the target's assembler.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPProgramLinker::Assemble(CAsm65Object* pcObject, CAsm65Source* pcSource, const char* szDirectory, CChars* pszErrors, const char* szSourceFile)
{
	return mpcBuild->GetTarget()->Assemble(pcSource, pcObject, szDirectory, szSourceFile, pszErrors);
}

//////////////////////////////////////////////////////////////////////////
//
// The library a project library's units are linked as, or NULL if they are linked as objects.
//
//////////////////////////////////////////////////////////////////////////
CAsm65Library* CCPPProgramLinker::GetLinkLibrary(CLibrary* pcLibrary)
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
// A unit's (empty) object for pcFile, added to its link library or to the
// objects linked, after the units before it.
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CCPPProgramLinker::AddUnitObject(CTranslationUnit* pcFile)
{
	CAsm65Library*	pcLinkLibrary;
	SCPP65Object*	psObject;
	CAsm65Object*	pcObject;

	pcLinkLibrary = GetLinkLibrary(pcFile->GetLibrary());
	if (pcLinkLibrary)
	{
		return pcLinkLibrary->AddObject(pcFile->ShortName());
	}
	pcObject = mcObjects.AddObject(pcFile->ShortName());
	psObject = macObjects.Add();
	psObject->pcObject = pcObject;
	psObject->bResident = false;
	mcLinker.AddObject(pcObject);
	return pcObject;
}
