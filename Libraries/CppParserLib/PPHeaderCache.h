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
#ifndef __P_P_HEADER_CACHE_H__
#define __P_P_HEADER_CACHE_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/Mutex.h"
#include "PreprocessedTokens.h"
#include "PPDirective.h"
#include "NameSet.h"


class CHeaderFiles;


//Which macro a name was when a header looked it up: none (pcFile NULL and uiHash 0), the one #defined at a line and
//column of a file (any unit's #define there is the same), or a predefined one (no file) with these contents.
struct SPPDefineIdentity
{
	CSourceFile*	pcFile;
	int32			iLine;
	int32			iColumn;
	uint64			uiHash;

	void	Init(void);
	bool	Equals(SPPDefineIdentity* psOther);
};


//A macro name a header looked up before it defined (or undefined) it itself, and what it was.
struct SPPHeaderQuery
{
	const char*			szName;		//Not NULL terminated.
	uint32				uiLength;
	SPPDefineIdentity	sIdentity;
};


//A file with #pragma once a header included before it pragma'd it itself, and whether it was skipped.
struct SPPHeaderPragmaQuery
{
	CSourceFile*	pcFile;
	bool			bOnce;
};


enum EPPHeaderEvent
{
	PPHE_Define,
	PPHE_Undef,
	PPHE_PragmaOnce,
};


//What a header did besides output tokens, done again when it is replayed: a #define or #undef (the directive, in the
//file, read again) or a #pragma once.
struct SPPHeaderEvent
{
	uint8			eType;			//EPPHeaderEvent.
	CSourceFile*	pcFile;
	CPPDirective*	pcDirective;	//PPHE_Define and PPHE_Undef.
};


//A name a header defined or undefined (its includer's later look ups of it don't depend on the includer's macros).
struct SPPHeaderName
{
	const char*		szName;			//Not NULL terminated.
	uint32			uiLength;
};


struct SPPCachedHeader;


//A run of a header's own output, or a header it included (psChild, cached on its own).
struct SPPHeaderSegment
{
	SPPCachedHeader*	psChild;
	uint32				uiFirstToken;
	uint32				uiNumTokens;
	uint32				uiFirstExpansion;
	uint32				uiNumExpansions;
	uint32				uiFirstInactive;
	uint32				uiNumInactive;
	uint32				uiFirstDirective;
	uint32				uiNumDirectives;
	uint32				uiFirstEvent;
	uint32				uiNumEvents;
};


typedef CArrayTemplate<SPPHeaderQuery>			CArrayPPHeaderQueries;
typedef CArrayTemplate<SPPHeaderPragmaQuery>	CArrayPPHeaderPragmaQueries;
typedef CArrayTemplate<SPPHeaderEvent>			CArrayPPHeaderEvents;
typedef CArrayTemplate<SPPHeaderName>			CArrayPPHeaderNames;
typedef CArrayTemplate<SPPHeaderSegment>		CArrayPPHeaderSegments;
typedef CArrayTemplate<CSourceFile*>			CArrayPPHeaderFiles;


//A header's preprocessed output, as one unit made it (with what it included), which another unit that includes the
//header with the same macros (asQueries, asPragmaQueries) gets without preprocessing it (CPreprocessor's
//ReplayHeader).  The tokens' and expansions' text that isn't in a source file is in mcStrings; expansion indices are
//relative to the segment's first expansion.
struct SPPCachedHeader
{
	CSourceFile*					pcFile;
	CHeaderFiles*					pcHeaderNameMap;		//Where it was found (#include_next goes on from there).
	uint64							uiContext;				//The include directories and the includers' directories, hashed.

	CArrayPPHeaderQueries			asQueries;
	CArrayPPHeaderPragmaQueries		asPragmaQueries;
	CArrayPPHeaderNames				asDefined;				//Names it (or a header it included) defined or undefined.
	CArrayPPHeaderFiles				apcPragmaOnced;			//Files it (or a header it included) pragma'd once.

	CArrayPPHeaderSegments			asSegments;
	CArrayPreprocessedTokens		asTokens;
	CArrayMacroExpansions			asExpansions;
	CArrayInactiveRegions			asInactiveRegions;
	CArrayDirectiveLines			asDirectiveLines;
	CArrayPPHeaderEvents			asEvents;
	uint8							uiEndPendingFlags;		//Spacing of a macro at its end that expanded to nothing.
	size							uiTotalTokens;			//With its children's.
	CMemoryStackExtended			mcStrings;

	SPPCachedHeader*				psNext;					//Another with the same file and context.

	void	Init(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext);
	void	Kill(void);
	char*	AddString(const char* szText, size uiLength);
};


//Headers' preprocessed output (SPPCachedHeader) kept by the builder for all its units, by the header and context.
//Units preprocessed at the same time share it: Get, and following psNext, only while it is locked (Lock).  A header
//is made whole before it is added (Publish), and doesn't change after.  They are freed by Kill.
class CPPHeaderCache
{
protected:
	CMapLongTemplate<SPPCachedHeader*>	mcHeaders;
	CArrayTemplatePtr<SPPCachedHeader>	mapsHeaders;
	CMutex								mcMutex;
	size								muiHits;
	size								muiMisses;
	size								muiReplayedTokens;

public:
	void				Init(void);
	void				Kill(void);

	void				Lock(void);
	void				Unlock(void);
	SPPCachedHeader*	Get(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext);
	SPPCachedHeader*	New(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext);
	void				Publish(SPPCachedHeader* psHeader);
	void				Discard(SPPCachedHeader* psHeader);
	void				Count(size uiHits, size uiMisses, size uiReplayedTokens);

	size				NumHeaders(void);
	size				NumHits(void);
	size				NumMisses(void);
	size				NumReplayedTokens(void);

protected:
	static int64		Key(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext);
};


#endif // __P_P_HEADER_CACHE_H__
