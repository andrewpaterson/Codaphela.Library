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
#include <time.h>
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "Project.h"
#include "CPPPreprocessor.h"
#include "HeaderFileMap.h"
#include "HeaderFiles.h"
#include "CppTokeniser.h"
#include "CppParser.h"
#include "CPPUnit.h"
#include "IRBlock.h"
#include "CPPBuild.h"


static bool			gbCPP65CheckDebugCode = false;		//Generate compiles again without debug value markers and compares the code.
static bool			gbCPP65CheckingDebugCode = false;	//It is doing so: no check of the check.
//What debug value markers may change (and a fingerprint's time).
static const char*	gaszCPP65MarkerDirectives[] = { ".dvariable", ".drange", ".dinline", ".dinspan", ".dtype", ".dmember", ".fingerprint", NULL };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::Init(void)
{
	mpcTarget = GetCPPTarget(CPPT_65C64);
	macIncludeDirectories.Init();
	mcOptions.Init();
	maszUnoptimised.Init();
	mcUnoptimisedOptions.Init();
	maszOwnLibraries.Init();
	macOwnOptions.Init();
	mpcFrameProfile = NULL;
	mbListing = false;
	mszListings.Init();
	mszLayouts.Init();
	maszLayouts.Init();
	mbLineTable = false;
	mlliCompileTime = 0;
	mbNoFloats = false;
	mbLazyHeaders = true;
	mpszWarnings = NULL;
	mpaszExternals = NULL;
	mpaszTested = NULL;
	mpcStatistics = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::Kill(void)
{
	size	i;

	maszLayouts.Kill();
	mszLayouts.Kill();
	mszListings.Kill();
	macOwnOptions.Kill();
	maszOwnLibraries.Kill();
	maszUnoptimised.Kill();
	for (i = 0; i < macIncludeDirectories.NumElements(); i++)
	{
		macIncludeDirectories.Get(i)->szDirectory.Kill();
	}
	macIncludeDirectories.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The processor to build for.  Set it before adding anything.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetTarget(ECPPTarget eTarget)
{
	mpcTarget = GetCPPTarget(eTarget);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPTarget* CCPPBuild::GetTarget(void)
{
	return mpcTarget;
}


//////////////////////////////////////////////////////////////////////////
//
// A directory searched for #include files.  A system directory is searched
// for <file>, the others for "file".
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::AddIncludeDirectory(const char* szDirectory, bool bIncludeSubDirectories, bool bSystem)
{
	SCPP65IncludeDirectory*	psInclude;

	psInclude = macIncludeDirectories.Add();
	psInclude->szDirectory.Init(szDirectory);
	psInclude->bIncludeSubDirectories = bIncludeSubDirectories;
	psInclude->bSystem = bSystem;
}


//////////////////////////////////////////////////////////////////////////
//
// How the units are compiled where there is a choice: speed or size, what
// is inlined, whether the target's assembly units are built.
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileOptions* CCPPBuild::GetOptions(void)
{
	return &mcOptions;
}


//////////////////////////////////////////////////////////////////////////
//
// The units of szLibrary (a project) are compiled without the optimiser,
// for a debugger (CCPPCompileOptions::mbDebuggable): every line steps as
// it is written.  The rest are compiled with GetOptions.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetUnoptimised(const char* szLibrary)
{
	maszUnoptimised.AddIfUnique(szLibrary);
}


//////////////////////////////////////////////////////////////////////////
//
// The units of szLibrary (a project) are compiled with what pcOptions has
// that is about one project's code (CCPPCompileOptions::SetOwn: its
// warnings, the conversions it is allowed, whether it is for a debugger),
// and GetOptions' for the rest, which is the whole program's.  Given
// again, they replace those given before.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetLibraryOptions(const char* szLibrary, CCPPCompileOptions* pcOptions)
{
	size	uiIndex;

	uiIndex = maszOwnLibraries.GetIndex(szLibrary);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		maszOwnLibraries.Add(szLibrary);
		macOwnOptions.Add(pcOptions);
	}
	else
	{
		*macOwnOptions.Get(uiIndex) = *pcOptions;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The options of the units SetUnoptimised names, set once before the
// units' threads read them: GetOptions, or the library's own, without
// the optimiser.  And those of the libraries with their own: GetOptions
// as they are now, with the library's own.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::FixOptions(void)
{
	CCPPCompileOptions	cOptions;
	size				ui;

	mcUnoptimisedOptions = mcOptions;
	mcUnoptimisedOptions.mbOptimise = false;
	mcUnoptimisedOptions.mbDebuggable = true;
	for (ui = 0; ui < maszOwnLibraries.NumElements(); ui++)
	{
		cOptions = mcOptions;
		cOptions.SetOwn(macOwnOptions.Get(ui));
		*macOwnOptions.Get(ui) = cOptions;
		if (maszUnoptimised.Contains(maszOwnLibraries.GetText(ui)))
		{
			macOwnOptions.Get(ui)->mbOptimise = false;
			macOwnOptions.Get(ui)->mbDebuggable = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The options a unit of pcLibrary is compiled with: its own
// (SetLibraryOptions), else GetOptions, unless the library is one
// SetUnoptimised named.
//
//////////////////////////////////////////////////////////////////////////
CCPPCompileOptions* CCPPBuild::LibraryOptions(CLibrary* pcLibrary)
{
	size	ui;

	for (ui = 0; pcLibrary && (ui < maszOwnLibraries.NumElements()); ui++)
	{
		if (pcLibrary->IsNamed(maszOwnLibraries.GetText(ui)))
		{
			return macOwnOptions.Get(ui);
		}
	}
	for (ui = 0; pcLibrary && (ui < maszUnoptimised.NumElements()); ui++)
	{
		if (pcLibrary->IsNamed(maszUnoptimised.GetText(ui)))
		{
			return &mcUnoptimisedOptions;
		}
	}
	return &mcOptions;
}


//////////////////////////////////////////////////////////////////////////
//
// A profile of the program's functions (CAsm65Profile, from a run): on the
// W65C816 those it measured worth it get page aligned frames (without one
// none do), and its hottest calls are inlined first, bigger callees for
// hot ones (CIRInliner::SetProfile).
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetFrameProfile(CAsm65Profile* pcProfile)
{
	mpcFrameProfile = pcProfile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Profile* CCPPBuild::GetFrameProfile(void)
{
	return mpcFrameProfile;
}


//////////////////////////////////////////////////////////////////////////
//
// The code has comments giving the C++ source line each piece of it is for
// (CCPPUnit::SetListing), without a listing being written.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetListing(bool bListing)
{
	mbListing = bListing;
}


//////////////////////////////////////////////////////////////////////////
//
// Each unit compiled is also written to szDirectory as a listing: its
// assembly with comments giving the C++ source line each piece of code is
// for (UnitName.lst).  The code is the same as without.  NULL for none.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetListingDirectory(const char* szDirectory)
{
	mszListings.Kill();
	mszListings.Init(szDirectory ? szDirectory : "");
	mbListing = !mszListings.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::HasListing(void)
{
	return mbListing;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::HasListingDirectory(void)
{
	return !mszListings.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPBuild::GetListingDirectory(void)
{
	return mszListings.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// The classes that what is compiled lays out are written to szFile (once
// the units are compiled, and again with each source's): a line for each
// class (CCPPUnit::AddLayout), each once and in order, so that two builds'
// files can be compared (another target's, or the same sources after a
// change to the layout rules or to a class).  NULL for none.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetLayoutsFile(const char* szFile)
{
	mszLayouts.Kill();
	mszLayouts.Init(szFile ? szFile : "");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::HasLayouts(void)
{
	return !mszLayouts.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
// A unit's classes, added to those laid out, sorted and each once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::AddLayouts(CArrayChars* paszLayouts)
{
	size	ui;

	for (ui = 0; ui < paszLayouts->NumElements(); ui++)
	{
		maszLayouts.InsertIntoSorted(paszLayouts->Get(ui));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The units are compiled with their C++ source lines and functions'
// fingerprints (now as their compile time), so after linking the line
// table gives the source line of an address: a profile by line.  Assembly
// units (.asm) give their lines that write bytes.  The code is the same as
// without.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetLineTable(bool bLineTable)
{
	mbLineTable = bLineTable;
	mlliCompileTime = (int64)time(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::HasLineTable(void)
{
	return mbLineTable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPBuild::GetCompileTime(void)
{
	return mlliCompileTime;
}


//////////////////////////////////////////////////////////////////////////
//
// A program with no floating point (the HCP65's kernal): floating point
// in the code of a unit not linked as a library is an error at its
// statement (CCPPUnit::SetNoFloats).
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetNoFloats(bool bNoFloats)
{
	mbNoFloats = bNoFloats;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::HasNoFloats(void)
{
	return mbNoFloats;
}


//////////////////////////////////////////////////////////////////////////
//
// A header declaration (a class, enum, typedef, template or function) a
// unit parsed is not parsed by the units after until they use one of its
// names.  The code is the same.  On by default; false parses every header
// declaration in every unit.
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetLazyHeaders(bool bLazyHeaders)
{
	mbLazyHeaders = bLazyHeaders;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::HasLazyHeaders(void)
{
	return mbLazyHeaders;
}


//////////////////////////////////////////////////////////////////////////
//
// Generate's warnings are appended to pszWarnings (NULL for none kept).
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetWarnings(CChars* pszWarnings)
{
	mpszWarnings = pszWarnings;
}


//////////////////////////////////////////////////////////////////////////
//
// The labels Generate's source uses but does not define are added to
// paszExternals (NULL for none kept).
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetExternals(CArrayChars* paszExternals)
{
	mpaszExternals = paszExternals;
}


//////////////////////////////////////////////////////////////////////////
//
// The functions Generate's source calls by name and tests for an
// exception are added to paszTested (CCPPUnit::GetTestedCallees).
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetTested(CArrayChars* paszTested)
{
	mpaszTested = paszTested;
}


//////////////////////////////////////////////////////////////////////////
//
// What Generate is counted and timed in (NULL for nothing).
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetStatistics(CCPPCompileStatistics* pcStatistics)
{
	mpcStatistics = pcStatistics;
}


//////////////////////////////////////////////////////////////////////////
//
// For tests: each Generate with a line table also compiles the source
// without debug value markers (CCPPCompileOptions::mbDebugMarkers), and it
// is an error if the code differs: they must change no code
// (DebuggerFPlan.txt F1).
//
//////////////////////////////////////////////////////////////////////////
void CCPPBuild::SetCheckDebugCode(bool bCheck)
{
	gbCPP65CheckDebugCode = bCheck;
}


//////////////////////////////////////////////////////////////////////////
//
// The length of szPrefix if szLine starts with it, else 0.
//
//////////////////////////////////////////////////////////////////////////
static size CPP65Prefix(const char* szLine, const char* szPrefix)
{
	size	i;

	for (i = 0; szPrefix[i] != '\0'; i++)
	{
		if (szLine[i] != szPrefix[i])
		{
			return 0;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an assembly line is one debug value markers may change: where
// variables and inlined calls are and the types they have (.dvariable,
// .drange, .dinline, .dinspan, .dtype, .dmember), a debug label
// (IR_DEBUG_LABEL) or a comment.
//
//////////////////////////////////////////////////////////////////////////
static bool CPP65IsMarkerLine(const char* szLine)
{
	size	i;
	size	uiLength;

	while ((*szLine == ' ') || (*szLine == '\t'))
	{
		szLine++;
	}
	if ((*szLine == '\0') || (*szLine == ';') || (CPP65Prefix(szLine, IR_DEBUG_LABEL) != 0))
	{
		return true;
	}
	for (i = 0; gaszCPP65MarkerDirectives[i]; i++)
	{
		uiLength = CPP65Prefix(szLine, gaszCPP65MarkerDirectives[i]);
		if ((uiLength != 0) && ((szLine[uiLength] == ' ') || (szLine[uiLength] == '\t') || (szLine[uiLength] == '\0')))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// szAssembly's lines that markers mustn't change (not CPP65IsMarkerLine):
// the code, and the rest of what a debugger is told.
//
//////////////////////////////////////////////////////////////////////////
static void CPP65CodeLines(const char* szAssembly, CArrayChars* paszDest)
{
	CArrayChars	aszLines;
	size		i;

	aszLines.Init();
	aszLines.Split(szAssembly, '\n');
	for (i = 0; i < aszLines.NumElements(); i++)
	{
		if (!CPP65IsMarkerLine(aszLines.GetText(i)))
		{
			paszDest->Add(aszLines.GetText(i));
		}
	}
	aszLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// SetCheckDebugCode's check: the source compiled again without debug
// value markers (and without a listing's comments, warnings, externals,
// tested functions or counts); an error naming the first line of code
// that differs.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::CheckDebugCode(const char* szName, const char* szSource, bool bProgram, CChars* pszAssembly, CChars* pszErrors)
{
	CCPPCompileOptions		cOptions;
	bool					bListing;
	CChars*					pszWarnings;
	CArrayChars*			paszExternals;
	CArrayChars*			paszTested;
	CCPPCompileStatistics*	pcStatistics;
	CChars					szPlain;
	CChars					szPlainErrors;
	CArrayChars				aszDebug;
	CArrayChars				aszPlain;
	size					i;
	bool					bResult;

	cOptions = mcOptions;
	bListing = mbListing;
	pszWarnings = mpszWarnings;
	paszExternals = mpaszExternals;
	paszTested = mpaszTested;
	pcStatistics = mpcStatistics;
	mcOptions.mbDebugMarkers = false;
	mbListing = false;
	mpszWarnings = NULL;
	mpaszExternals = NULL;
	mpaszTested = NULL;
	mpcStatistics = NULL;
	szPlain.Init();
	szPlainErrors.Init();
	gbCPP65CheckingDebugCode = true;
	bResult = Generate(szName, szSource, bProgram, &szPlain, &szPlainErrors);
	gbCPP65CheckingDebugCode = false;
	mcOptions = cOptions;
	mbListing = bListing;
	mpszWarnings = pszWarnings;
	mpaszExternals = paszExternals;
	mpaszTested = paszTested;
	mpcStatistics = pcStatistics;

	aszDebug.Init();
	aszPlain.Init();
	CPP65CodeLines(pszAssembly->Text(), &aszDebug);
	CPP65CodeLines(szPlain.Text(), &aszPlain);
	for (i = 0; bResult && (i < aszDebug.NumElements()) && (i < aszPlain.NumElements()); i++)
	{
		if (!aszDebug.Get(i)->Equals(aszPlain.Get(i)))
		{
			pszErrors->Append("Debug value markers changed the code, line ")->Append((int)i + 1)->Append(": [")->Append(aszDebug.GetText(i))->Append("] without it [")->Append(aszPlain.GetText(i))->Append("]\n");
			bResult = false;
		}
	}
	if (bResult && (aszDebug.NumElements() != aszPlain.NumElements()))
	{
		pszErrors->Append("Debug value markers changed the code's length: ")->Append((int)aszDebug.NumElements())->Append(" lines without it ")->Append((int)aszPlain.NumElements())->Append("\n");
		bResult = false;
	}
	aszPlain.Kill();
	aszDebug.Kill();
	szPlainErrors.Kill();
	szPlain.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses, parses and generates szSource with the include
// directories, for the target with the options.  As a program (with its
// start up) if bProgram, otherwise as an object.  The assembly is appended
// to pszAssembly and every stage's errors to pszErrors; the warnings,
// externals, tested functions and counts go where SetWarnings,
// SetExternals, SetTested and SetStatistics said; on the W65C816 the frame
// profile's measured functions get aligned frames; with a listing the code
// has comments with the C++ source lines it is for, with a line table
// .line and .fingerprint directives (now as its compile time), and with a
// layouts file the classes it lays out are added to those written.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::Generate(const char* szName, const char* szSource, bool bProgram, CChars* pszAssembly, CChars* pszErrors)
{
	CTranslationUnit				cFile;
	CCPPPreprocessor				cPreprocessor;
	CCPPTokeniser					cTokeniser;
	CCPPSyntaxTree					cTree;
	CCPPParser						cParser;
	CCPPUnit						cGenerator;
	CHeaderFileMap					cHeaderFileMap;
	CArrayTemplate<CHeaderFiles>	acHeaderFiles;
	SCPP65IncludeDirectory*			psInclude;
	CConfig							cConfig;
	size							uiIncludes;
	size							i;
	bool							bResult;

	cHeaderFileMap.Init();
	uiIncludes = macIncludeDirectories.NumElements();
	//All added before any is used: the preprocessor keeps their addresses.
	acHeaderFiles.Init();
	for (i = 0; i < uiIncludes; i++)
	{
		acHeaderFiles.Add();
	}
	for (i = 0; i < uiIncludes; i++)
	{
		psInclude = macIncludeDirectories.Get(i);
		acHeaderFiles.Get(i)->Init(psInclude->szDirectory.Text(), &cHeaderFileMap, psInclude->bIncludeSubDirectories, psInclude->bSystem);
	}

	cConfig.Init((char*)mpcTarget->GetConfigurationName());
	mpcTarget->AddDefines(&cConfig, &mcOptions);
	cFile.Init((char*)szName, NULL, false);
	cFile.SetContents((char*)szSource);
	cPreprocessor.Init(&cConfig);
	for (i = 0; i < uiIncludes; i++)
	{
		cPreprocessor.AddIncludeDirectory(acHeaderFiles.Get(i));
	}
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);
	pszErrors->Append(cPreprocessor.GetErrors()->Text());
	if (mpszWarnings)
	{
		mpszWarnings->Append(cPreprocessor.GetWarnings()->Text());
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

	cGenerator.Init(&cTree, mpcTarget->GetTarget());
	cGenerator.SetOptions(&mcOptions);
	cGenerator.SetStatistics(mpcStatistics);
	cGenerator.SetFrameProfile(mpcFrameProfile);
	cGenerator.SetListing(mbListing);
	cGenerator.SetLayouts(HasLayouts());
	cGenerator.SetLineTable(mbLineTable, (int64)time(NULL));
	cGenerator.SetNoFloats(mbNoFloats);
	bResult &= bProgram ? cGenerator.GenerateProgram(pszAssembly) : cGenerator.Generate(pszAssembly);
	if (mpaszExternals)
	{
		cGenerator.GetExternals(mpaszExternals);
	}
	if (mpaszTested)
	{
		cGenerator.GetTestedCallees(mpaszTested);
	}
	if (HasLayouts())
	{
		cGenerator.GetLayouts(&maszLayouts);
	}
	pszErrors->Append(cGenerator.GetErrors()->Text());
	if (mpszWarnings)
	{
		mpszWarnings->Append(cGenerator.GetWarnings()->Text());
	}
	cGenerator.Kill();
	if (bResult && mbLineTable && gbCPP65CheckDebugCode && !gbCPP65CheckingDebugCode)
	{
		bResult = CheckDebugCode(szName, szSource, bProgram, pszAssembly, pszErrors);
	}

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
// Writes the classes laid out so far to the layouts file (SetLayoutsFile),
// if there is one and there are any.  Its error goes to pszErrors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::WriteLayouts(CChars* pszErrors)
{
	CTextFile	cFile;
	size		i;
	bool		bResult;

	if (mszLayouts.Empty() || (maszLayouts.NumElements() == 0))
	{
		return true;
	}
	cFile.Init();
	for (i = 0; i < maszLayouts.NumElements(); i++)
	{
		cFile.mcText.Append(maszLayouts.GetText(i));
		cFile.mcText.AppendNewLine();
	}
	bResult = cFile.Write(mszLayouts.Text());
	cFile.Kill();
	if (!bResult)
	{
		pszErrors->Append("Can't write the classes' layouts to ")->Append(mszLayouts)->AppendNewLine();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a unit's assembly (with its source comments) to the listings
// directory as the unit's name with .lst for its extension.  A unit with
// no code (all of it left out by #if) has no listing.  Its error goes to
// pszErrors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::WriteListing(const char* szUnit, CChars* pszAssembly, CChars* pszErrors)
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
		pszErrors->Append("Can't write the listing ");
		pszErrors->Append(szPath);
		pszErrors->AppendNewLine();
	}
	szPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// As WriteListing(CChars*), a line at a time.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPBuild::WriteListing(const char* szUnit, CCPP65Assembly* pcAssembly, CChars* pszErrors)
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
bool CCPPBuild::ListingName(const char* szUnit, CChars* pszPath)
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
