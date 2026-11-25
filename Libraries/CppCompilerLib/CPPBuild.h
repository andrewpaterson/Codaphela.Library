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
#ifndef __C_P_P_BUILD_H__
#define __C_P_P_BUILD_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "Assembler65Lib/Asm65Profile.h"
#include "CPPTarget.h"
#include "CPPCompileOptions.h"
#include "CPPCompileStatistics.h"
#include "CPP65Assembly.h"


struct SCPP65IncludeDirectory
{
	CChars	szDirectory;
	bool	bIncludeSubDirectories;
	bool	bSystem;
};


typedef CArrayTemplate<SCPP65IncludeDirectory>	CArrayCPP65IncludeDirectories;


class CLibrary;


//What is being built and how (CCPP65Builder has one): the target, the include directories, the options (and those of
//the libraries compiled for a debugger, SetUnoptimised), and what is written beside the code: each unit's listing
//(SetListingDirectory), the classes laid out (SetLayoutsFile), a line table (SetLineTable).  Generate compiles one
//source with it, giving its assembly and errors, and its warnings, the labels it uses and doesn't define, the
//functions it calls and tests and its counts to where SetWarnings, SetExternals, SetTested and SetStatistics say.
class CCPPBuild
{
protected:
	CCPPTarget*						mpcTarget;
	CArrayCPP65IncludeDirectories	macIncludeDirectories;
	CCPPCompileOptions				mcOptions;				//How the units are compiled where there is a choice (GetOptions).
	CArrayChars						maszUnoptimised;		//The libraries (projects) whose units aren't optimised (SetUnoptimised)...
	CCPPCompileOptions				mcUnoptimisedOptions;	//...as mcOptions without the optimiser, for a debugger (FixOptions).
	CAsm65Profile*					mpcFrameProfile;		//Measured functions, for the W65C816's aligned frames, or NULL.
	bool							mbListing;				//The code has comments with its C++ source lines (SetListing)...
	CChars							mszListings;			//...and each unit's listing is written here (SetListingDirectory), empty for none.
	CChars							mszLayouts;				//The file the classes' layouts are written to (SetLayoutsFile), empty for none...
	CArrayChars						maszLayouts;			//...and the classes laid out: a line each, sorted, each once.
	bool							mbLineTable;			//Units are compiled with .line and .fingerprint (SetLineTable)...
	int64							mlliCompileTime;		//...from when SetLineTable was called (seconds since 1970).
	bool							mbNoFloats;				//No floating point (SetNoFloats).
	bool							mbLazyHeaders;			//Header declarations are parsed when a unit first uses them (SetLazyHeaders).
	CChars*							mpszWarnings;			//Where Generate's warnings go, or NULL...
	CArrayChars*					mpaszExternals;			//...the labels it uses but doesn't define...
	CArrayChars*					mpaszTested;			//...the functions it calls by name and tests for an exception...
	CCPPCompileStatistics*			mpcStatistics;			//...and what it is counted and timed in.

public:
	void					Init(void);
	void					Kill(void);

	void					SetTarget(ECPPTarget eTarget);
	CCPPTarget*				GetTarget(void);
	void					AddIncludeDirectory(const char* szDirectory, bool bIncludeSubDirectories, bool bSystem);
	CCPPCompileOptions*		GetOptions(void);
	void					SetUnoptimised(const char* szLibrary);
	void					FixOptions(void);
	CCPPCompileOptions*		LibraryOptions(CLibrary* pcLibrary);
	void					SetFrameProfile(CAsm65Profile* pcProfile);
	CAsm65Profile*			GetFrameProfile(void);
	void					SetListing(bool bListing);
	void					SetListingDirectory(const char* szDirectory);
	bool					HasListing(void);
	bool					HasListingDirectory(void);
	const char*				GetListingDirectory(void);
	void					SetLayoutsFile(const char* szFile);
	bool					HasLayouts(void);
	void					AddLayouts(CArrayChars* paszLayouts);
	void					SetLineTable(bool bLineTable);
	bool					HasLineTable(void);
	int64					GetCompileTime(void);
	void					SetNoFloats(bool bNoFloats);
	bool					HasNoFloats(void);
	void					SetLazyHeaders(bool bLazyHeaders);
	bool					HasLazyHeaders(void);
	void					SetWarnings(CChars* pszWarnings);
	void					SetExternals(CArrayChars* paszExternals);
	void					SetTested(CArrayChars* paszTested);
	void					SetStatistics(CCPPCompileStatistics* pcStatistics);

	bool					Generate(const char* szName, const char* szSource, bool bProgram, CChars* pszAssembly, CChars* pszErrors);
	bool					WriteListing(const char* szUnit, CChars* pszAssembly, CChars* pszErrors);
	bool					WriteListing(const char* szUnit, CCPP65Assembly* pcAssembly, CChars* pszErrors);
	bool					WriteLayouts(CChars* pszErrors);

	static void				SetCheckDebugCode(bool bCheck);

protected:
	bool					CheckDebugCode(const char* szName, const char* szSource, bool bProgram, CChars* pszAssembly, CChars* pszErrors);
	bool					ListingName(const char* szUnit, CChars* pszPath);
};


#endif // __C_P_P_BUILD_H__
