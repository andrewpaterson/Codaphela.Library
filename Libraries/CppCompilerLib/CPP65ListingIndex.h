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
#ifndef __C_P_P_65_LISTING_INDEX_H__
#define __C_P_P_65_LISTING_INDEX_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/Chars.h"


//A listing (CCPP65Builder::SetListingDirectory): its name, text and the start of each of its lines.
struct SCPP65Listing
{
	CChars					szName;
	CChars					szText;
	CArrayTemplate<size>	auiLineStarts;
};


//Listing lines uiFirstLine to uiEndLine (exclusive, from 0) of listing uiListing are code for line uiLine (from 0) of
//source file uiFile, in function (label) uiFunction.  The labels just before its .line, lines uiFirstLabel to the .line
//(uiFirstLine - 1), are where it is jumped to; uiFirstLabel is the .line's if there are none.
struct SCPP65ListingRange
{
	size	uiFile;
	size	uiLine;
	size	uiListing;
	size	uiFunction;
	size	uiFirstLabel;
	size	uiFirstLine;
	size	uiEndLine;
};


//A place a source line's code is in: the ranges of one function in one listing (a line can be in pieces in a function,
//like a for's parts).
struct SCPP65ListingPlace
{
	size	uiListing;
	size	uiFunction;
	size	uiFirstRange;	//Of the index's ranges, sorted; the place's are together.
	size	uiNumRanges;
};


typedef CArrayTemplate<SCPP65Listing>		CArrayCPP65Listings;
typedef CArrayTemplate<SCPP65ListingRange>	CArrayCPP65ListingRanges;
typedef CArrayTemplate<SCPP65ListingPlace>	CArrayCPP65ListingPlaces;
typedef CArrayTemplate<size>				CArrayCPP65ListingLines;


//Where each line of the source went in the generated assembly: read from listings written with the line table on
//(CCPP65Builder::SetLineTable), whose .line directives give the source file's full name and line before its code.  A
//source line can be in many places: a header's inline function in each unit that uses it, a function inlined at each
//call, a template in each instantiation.  Code inlined from another unit (the W65C816's imports) is listed under the
//line that calls it.  Source files are found by name whatever their case and slashes.
class CCPP65ListingIndex
{
protected:
	CArrayCPP65Listings			masListings;
	CArrayChars					maszFiles;
	CMapStringTemplate<size>	mcFileIndices;	//By normalised name.
	CArrayChars					maszFunctions;
	CMapStringTemplate<size>	mcFunctionIndices;
	CArrayCPP65ListingRanges	masRanges;
	bool						mbSorted;

public:
	void				Init(void);
	void				Kill(void);

	bool				ReadListing(const char* szFileName);
	void				AddListing(const char* szName, const char* szText);

	void				GetPlaces(const char* szSourceFile, size uiLine, CArrayCPP65ListingPlaces* pasDest);
	void				GetLines(const char* szSourceFile, CArrayCPP65ListingLines* pauiDest);
	void				AppendLines(size uiListing, size uiFirstLine, size uiEndLine, CChars* pszDest);
	void				AppendRange(SCPP65ListingRange* psRange, CChars* pszDest);

	size				NumListings(void);
	char*				GetListingName(size uiListing);
	size				NumListingLines(size uiListing);
	size				NumFiles(void);
	char*				GetFile(size uiFile);
	char*				GetFunction(size uiFunction);
	size				NumRanges(void);
	SCPP65ListingRange*	GetRange(size uiRange);

protected:
	void				IndexListing(size uiListing);
	size				AddFile(const char* szFile);
	size				FindFile(const char* szSourceFile);
	size				FindFirstRange(size uiFile, size uiLine);
	size				AddFunction(const char* szFunction, size uiLength);
	void				EndRange(SCPP65ListingRange* psRange, bool bInRange);
	void				Sort(void);
	void				NormaliseFile(CChars* pszFile);
	bool				ReadLineDirective(const char* szLine, const char* szEnd, CChars* pszFile, size* puiLine);
	bool				StartsWith(const char* szText, const char* szEnd, const char* szPrefix);
};


#endif // __C_P_P_65_LISTING_INDEX_H__

