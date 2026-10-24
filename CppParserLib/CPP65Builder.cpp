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
#include "Assembler65Lib/Asm65Assembler.h"
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
	muiUnits = 0;
	mcObjects.Init("objects");
	mcLinker.Init();
	mcLinker.SetStubMissing(true);
	mcLinker.SetRemoveUnused(true);
	mszErrors.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::Kill(void)
{
	size	i;

	mszErrors.Kill();
	mcLinker.Kill();
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
		CCPP65Generator::AddDefines(pcLibrary->AddConfiguration("65C64"));
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
			if (pcLinkLibrary)
			{
				Assemble(pcLinkLibrary->AddObject(szFile), cFile.Text());
			}
			else
			{
				AddObject(szFile, cFile.Text());
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
// Whether a label no object defines is stubbed (the default) or is a link error.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetStubMissing(bool bStubMissing)
{
	mcLinker.SetStubMissing(bStubMissing);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether what nothing uses is left out of the program (the default).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Builder::SetRemoveUnused(bool bRemoveUnused)
{
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
// A global's address after Link (see CAsm65Linker::GetSymbol).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::GetSymbol(const char* szName, int64* piValue)
{
	return mcLinker.GetSymbol(szName, piValue);
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
	CCPP65Generator::AppendStartUp(&szAssembly);
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
	bResult = Generate(&szAssembly, &mszErrors, &macIncludeDirectories, szName, szSource, false, NULL);
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
	size	i;

	bResult = CompileUnits();

	//The libraries are only added now, when their array won't move them any more.
	for (i = 0; i < macLinkLibraries.NumElements(); i++)
	{
		mcLinker.AddLibrary(&macLinkLibraries.Get(i)->cObjects);
	}
	mcLinker.SetAddress(ASK65_Text, CPP65_PROGRAM_ADDRESS);
	mcLinker.SetLimit(CPP65_HEAP_ADDRESS);
	bResult &= mcLinker.Link();
	mszErrors.Append(mcLinker.GetErrors());
	return bResult && mszErrors.Empty();
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
bool CCPP65Builder::Generate(CChars* pszAssembly, CChars* pszErrors, CArrayCPP65IncludeDirectories* pacIncludeDirectories, const char* szName, const char* szSource, bool bProgram, CArrayChars* paszExternals)
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

	cConfig.Init("65C64");
	CCPP65Generator::AddDefines(&cConfig);
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

	cGenerator.Init(&cTree);
	bResult &= bProgram ? cGenerator.GenerateProgram(pszAssembly) : cGenerator.Generate(pszAssembly);
	if (paszExternals)
	{
		cGenerator.GetExternals(paszExternals);
	}
	pszErrors->Append(cGenerator.GetErrors()->Text());
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
// Assembles one object and adds it to the link.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::AddObject(const char* szName, const char* szAssembly, bool bResident)
{
	CAsm65Object*	pcObject;
	bool			bResult;

	pcObject = mcObjects.AddObject(szName);
	bResult = Assemble(pcObject, szAssembly);
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
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::Assemble(CAsm65Object* pcObject, const char* szAssembly)
{
	CAsm65Assembler	cAssembler;
	bool			bResult;

	cAssembler.Init();
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
// Compiles each of the project's units to an object.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Builder::CompileUnits(void)
{
	CTranslationUnit*	pcFile;
	SProcessIter		sIter;
	CCPPTokeniser		cTokeniser;
	CCPPSyntaxTree		cTree;
	CCPPParser			cParser;
	CCPP65Generator		cGenerator;
	CChars				szAssembly;
	CAsm65Library*		pcLinkLibrary;
	size				uiCompiled;
	bool				bResult;

	if (muiUnits == 0)
	{
		return true;
	}

	bResult = mcProject.Process("65C64", &mszErrors);
	uiCompiled = 0;
	for (pcFile = mcProject.Start(&sIter, "65C64"); pcFile; pcFile = mcProject.Iterate(&sIter))
	{
		uiCompiled++;
		cTokeniser.Init();
		bResult &= cTokeniser.Tokenise(pcFile->GetTokens());
		mszErrors.Append(cTokeniser.GetErrors()->Text());
		cTree.Init(&cTokeniser);
		cParser.Init(&cTokeniser, &cTree);
		bResult &= cParser.Parse();
		mszErrors.Append(cTree.GetErrors()->Text());
		cParser.Kill();

		szAssembly.Init();
		cGenerator.Init(&cTree);
		bResult &= cGenerator.Generate(&szAssembly);
		mszErrors.Append(cGenerator.GetErrors()->Text());
		cGenerator.Kill();
		pcLinkLibrary = GetLinkLibrary(pcFile->GetLibrary());
		if (pcLinkLibrary)
		{
			bResult &= Assemble(pcLinkLibrary->AddObject(pcFile->ShortName()), szAssembly.Text());
		}
		else
		{
			bResult &= AddObject(pcFile->ShortName(), szAssembly.Text());
		}
		szAssembly.Kill();

		cTree.Kill();
		cTokeniser.Kill();
	}

	if (uiCompiled != muiUnits)
	{
		mszErrors.Append("Only ")->Append((int)uiCompiled)->Append(" of the ")->Append((int)muiUnits)->Append(" translation units were compiled.\n");
		return false;
	}
	return bResult;
}

