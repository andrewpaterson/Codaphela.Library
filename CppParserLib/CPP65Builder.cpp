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
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/StringHelper.h"
#include "VisualStudioProject.h"
#include "Assembler65Lib/Asm65Assembler.h"
#include "Assembler65Lib/Asm816Assembler.h"
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::Kill(void)
{
	size	i;

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
			cFileUtil.RemoveLastFromPath(&szPath);
			if (pcLinkLibrary)
			{
				Assemble(pcLinkLibrary->AddObject(szFile), cFile.Text(), szPath.Text());
			}
			else
			{
				AddObject(szFile, cFile.Text(), false, szPath.Text());
			}
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
// szExcluded (space separated).  Returns false if the project can't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddVisualStudioProject(const char* szLibrary, const char* szProjectFile, bool bLinkAsLibrary, const char* szExcluded)
{
	CVisualStudioProject	cProject;
	CArrayChars				aszExcluded;
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
	for (i = 0; i < cProject.NumSources(); i++)
	{
		if (IsExcluded(&aszExcluded, cProject.GetSource(i)))
		{
			continue;
		}
		AddUnit(szLibrary, cProject.GetDirectory(), cProject.GetSource(i), bLinkAsLibrary);
	}
	cProject.Kill();
	aszExcluded.Kill();
	return bResult;
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
// The processor to build for.  Set it before adding anything.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetTarget(ECPPTarget eTarget)
{
	meTarget = eTarget;
}


//////////////////////////////////////////////////////////////////////////
//
// A global's address after Link (see CAsm65Linker::GetSymbol).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::GetSymbol(const char* szName, int64* piValue)
{
	return mcLinker.GetSymbol(szName, piValue);
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
	bResult = Generate(&szAssembly, &mszErrors, &macIncludeDirectories, szName, szSource, false, NULL, meTarget, &mszWarnings);
	bResult &= AddObject(szName, szAssembly.Text());
	szAssembly.Kill();
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
	bResult = Generate(&szAssembly, &mszErrors, &macIncludeDirectories, szName, szSource, false, NULL, meTarget, &mszWarnings);
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
// pszErrors.  The labels it uses but does not define are added to paszExternals if it isn't NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Generate(CChars* pszAssembly, CChars* pszErrors, CArrayCPP65IncludeDirectories* pacIncludeDirectories, const char* szName, const char* szSource, bool bProgram, CArrayChars* paszExternals, ECPPTarget eTarget, CChars* pszWarnings)
{
	CTranslationUnit		cFile;
	CPreprocessor			cPreprocessor;
	CCPPTokeniser			cTokeniser;
	CCPPSyntaxTree			cTree;
	CCPPParser				cParser;
	CCPP65Generator			cGenerator;
	CHeaderFileMap			cHeaderFileMap;
	CHeaderFiles*			acHeaderFiles;
	SCPP65IncludeDirectory*	psInclude;
	CConfig					cConfig;
	size					uiIncludes;
	size					i;
	bool					bResult;

	cHeaderFileMap.Init();
	uiIncludes = pacIncludeDirectories->NumElements();
	acHeaderFiles = (CHeaderFiles*)malloc(uiIncludes * sizeof(CHeaderFiles) + 1);
	for (i = 0; i < uiIncludes; i++)
	{
		psInclude = pacIncludeDirectories->Get(i);
		acHeaderFiles[i].Init(psInclude->szDirectory.Text(), &cHeaderFileMap, psInclude->bIncludeSubDirectories, psInclude->bSystem);
	}

	cConfig.Init((char*)GetCPPTarget(eTarget)->szName);
	CCPP65Generator::AddDefines(&cConfig, eTarget);
	cFile.Init((char*)szName, NULL, false);
	cFile.SetContents((char*)szSource);
	cPreprocessor.Init(&cConfig);
	for (i = 0; i < uiIncludes; i++)
	{
		cPreprocessor.AddIncludeDirectory(&acHeaderFiles[i]);
	}
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	pszErrors->Append(cPreprocessor.GetErrors()->Text());
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
		acHeaderFiles[i].Kill();
	}
	free(acHeaderFiles);
	cHeaderFileMap.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Assembles one object and adds it to the link.  .inchex file names are
// relative to szDirectory.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddObject(const char* szName, const char* szAssembly, bool bResident, const char* szDirectory)
{
	CAsm65Object*	pcObject;
	SCPP65Object*	psObject;
	bool			bResult;

	pcObject = mcObjects.AddObject(szName);
	bResult = Assemble(pcObject, szAssembly, szDirectory);
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
// assembly unit's own).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Assemble(CAsm65Object* pcObject, const char* szAssembly, const char* szDirectory)
{
	CAsm65Assembler		cAssembler;
	CAsm816Assembler	cAssembler816;
	bool				bResult;

	if (meTarget == CPPT_W65C816)
	{
		cAssembler816.Init();
		if (szDirectory)
		{
			cAssembler816.SetDirectory(szDirectory);
		}
		bResult = cAssembler816.AssembleObject(szAssembly, pcObject);
		mszErrors.Append(cAssembler816.GetErrors());
		cAssembler816.Kill();
		return bResult;
	}
	cAssembler.Init();
	if (szDirectory)
	{
		cAssembler.SetDirectory(szDirectory);
	}
	bResult = cAssembler.AssembleObject(szAssembly, pcObject);
	mszErrors.Append(cAssembler.GetErrors());
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
// Compiles the project's units, each into an object (of its library when it
// is linked as one).  On the W65C816 the units' small functions are kept for
// inlining in the units compiled after them (mcImports), and a unit that
// called one before it was exported is compiled again at the end.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::CompileUnits(void)
{
	CTranslationUnit*			pcFile;
	SProcessIter				sIter;
	CArrayCPP65CompiledUnits	asUnits;
	SCPP65CompiledUnit*			psUnit;
	size						uiCompiled;
	size						ui;
	size						uiName;
	bool						bResult;
	bool						bAgain;

	if (muiUnits == 0)
	{
		return true;
	}

	bResult = mcProject.Process((char*)GetCPPTarget(meTarget)->szName, &mszErrors);
	uiCompiled = 0;
	asUnits.Init();
	for (pcFile = mcProject.Start(&sIter, (char*)GetCPPTarget(meTarget)->szName); pcFile; pcFile = mcProject.Iterate(&sIter))
	{
		uiCompiled++;
		psUnit = asUnits.Add();
		psUnit->pcFile = pcFile;
		psUnit->pcObject = NULL;
		psUnit->aszMissing.Init();
		bResult &= CompileUnit(psUnit);
	}

	for (ui = 0; bResult && (meTarget == CPPT_W65C816) && (ui < asUnits.NumElements()); ui++)
	{
		psUnit = asUnits.Get(ui);
		bAgain = false;
		for (uiName = 0; uiName < psUnit->aszMissing.NumElements(); uiName++)
		{
			bAgain |= mcImports.Find(psUnit->aszMissing.GetText(uiName)) != NULL;
		}
		if (bAgain)
		{
			psUnit->aszMissing.ReInit();
			bResult &= CompileUnit(psUnit);
		}
	}
	for (ui = 0; ui < asUnits.NumElements(); ui++)
	{
		asUnits.Get(ui)->aszMissing.Kill();
	}
	asUnits.Kill();

	if (uiCompiled != muiUnits)
	{
		mszErrors.Append("Only ")->Append((int)uiCompiled)->Append(" of the ")->Append((int)muiUnits)->Append(" translation units were compiled.\n");
		return false;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles psUnit's file into its object: a new one, or its old one again.
// The functions it calls that weren't found for inlining go to its
// aszMissing.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::CompileUnit(SCPP65CompiledUnit* psUnit)
{
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPP65Generator		cGenerator;
	CChars				szAssembly;
	CAsm65Library*		pcLinkLibrary;
	bool				bResult;

	cTokeniser.Init();
	bResult = cTokeniser.Tokenise(psUnit->pcFile->GetTokens());
	mszErrors.Append(cTokeniser.GetErrors()->Text());
	cTree.Init(&cTokeniser);
	cParser.Init(&cTokeniser, &cTree);
	bResult &= cParser.Parse();
	mszErrors.Append(cTree.GetErrors()->Text());
	cParser.Kill();

	szAssembly.Init();
	cGenerator.Init(&cTree, meTarget);
	if (meTarget == CPPT_W65C816)
	{
		cGenerator.SetImports(&mcImports);
	}
	bResult &= cGenerator.Generate(&szAssembly);
	cGenerator.GetMissingCallees(&psUnit->aszMissing);
	mszErrors.Append(cGenerator.GetErrors()->Text());
	if (!psUnit->pcObject)
	{
		mszWarnings.Append(cGenerator.GetWarnings()->Text());
	}
	cGenerator.Kill();

	if (psUnit->pcObject)
	{
		psUnit->pcObject->Kill();
		psUnit->pcObject->Init(psUnit->pcFile->ShortName());
		bResult &= Assemble(psUnit->pcObject, szAssembly.Text());
	}
	else
	{
		pcLinkLibrary = GetLinkLibrary(psUnit->pcFile->GetLibrary());
		if (pcLinkLibrary)
		{
			psUnit->pcObject = pcLinkLibrary->AddObject(psUnit->pcFile->ShortName());
			bResult &= Assemble(psUnit->pcObject, szAssembly.Text());
		}
		else
		{
			bResult &= AddObject(psUnit->pcFile->ShortName(), szAssembly.Text());
			psUnit->pcObject = macObjects.Tail()->pcObject;
		}
	}
	szAssembly.Kill();

	cTree.Kill();
	cTokeniser.Kill();
	return bResult;
}

