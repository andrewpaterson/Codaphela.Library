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
#ifndef __C_P_P_INDEX_H__
#define __C_P_P_INDEX_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/MapStringInt.h"
#include "BaseLib/MapLongTemplate.h"
#include "CPPSymbolKey.h"
#include "CPPSemanticPass.h"


enum ECPPIndexRole
{
	CPPIR_Declaration,
	CPPIR_Definition,		//A function with its body, a class or enum with its members.
	CPPIR_Reference,
};


//SCPPIndexLocation::uiFlags
#define CPPIF_AMBIGUOUS		0x01	//A call the parser can't tell the overload of: it is a reference of each it could be.
#define CPPIF_MACRO			0x02	//A name a macro made, where the macro was used: the text there is the macro's name,
									//ARRAY_BLOCK, not the symbol's, CArrayBlock.


//Where a symbol is named in a file.
struct SCPPIndexLocation
{
	uint32	uiFile;
	int32	iLine;		//0 based.
	int32	iColumn;	//0 based.
	int32	iLength;
	uint8	uiRole;
	uint8	uiFlags;
};


typedef CArrayTemplate<SCPPIndexLocation>	CArrayIndexLocations;


//What the syntax tree says about a name token, while a unit is added.  Zero is nothing known.
struct SCPPIndexToken
{
	CCPPSyntaxDeclarator*	pcDeclarator;	//The function declarator the token names.
	int32					iArguments;		//The callee of a call: its number of arguments plus one.
	bool					bDefinition;	//It names what a definition defines.
};


typedef CArrayTemplate<SCPPIndexToken>	CArrayIndexTokens;


//A name a unit declares or uses, before it is merged into an index.
struct SCPPIndexEntry
{
	uint32			uiKey;		//Index of its key in the unit's keys.
	uint32			uiFile;		//Index of its file in the unit's files.
	int32			iLine;
	int32			iColumn;
	int32			iLength;
	uint8			uiRole;
	uint8			uiFlags;
};


typedef CArrayTemplate<SCPPIndexEntry>	CArrayIndexEntries;


//What one translation unit declares and uses, gathered without touching an index (so units can be gathered on
//several threads at once) and then merged into one (CCPPIndex::Merge).
class CCPPIndexUnit
{
public:
	CArrayIndexEntries	masEntries;
	CArrayChars			maszKeys;		//The different keys.
	CMapStringInt		mcKeys;			//Key to index in maszKeys.
	CArrayInt			maiKinds;		//Each key's ECPPSymbolKind.
	CArrayChars			maszFiles;
	CMapStringInt		mcFiles;		//File name to index in maszFiles.

	void	Init(void);
	void	Kill(void);

	void	Add(const char* szKey, ECPPSymbolKind eKind, SPreprocessedToken* psSource, ECPPIndexRole eRole, uint8 uiFlags);
};


//A symbol of the whole solution: the same in every translation unit that declares or uses it.
class CCPPIndexSymbol
{
public:
	CChars					mszKey;
	ECPPSymbolKind			meKind;
	CArrayIndexLocations	masLocations;

	void	Init(const char* szKey, ECPPSymbolKind eKind);
	void	Kill(void);

	size				NumLocations(void);
	SCPPIndexLocation*	GetLocation(size uiIndex);
	SCPPIndexLocation*	GetDefinition(void);
	SCPPIndexLocation*	GetDeclaration(void);
};


typedef CArrayTemplatePtr<CCPPIndexSymbol>	CArrayIndexSymbols;


//What every translation unit added to it declares and uses, by symbol (CCPPSymbolKey): for Go to Definition, Find All
//References and the refactorings built on them.  A header seen by many units is only indexed once.
class CCPPIndex
{
protected:
	CArrayChars				maszFiles;
	CMapStringInt			mcFiles;		//File name to index in maszFiles.
	CArrayIndexSymbols		mapcSymbols;
	CMapStringInt			mcKeys;			//Key to index in mapcSymbols.
	CMapLongTemplate<int32>	mcLocations;	//A name's place (LocationKey) to its symbol's index.

public:
	void				Init(void);
	void				Kill(void);

	void				AddUnit(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, CCPPSemanticPass* pcPass = NULL);
	static void			Gather(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, CCPPIndexUnit* pcDest, CCPPSemanticPass* pcPass = NULL);
	void				Merge(CCPPIndexUnit* pcUnit);

	size				NumSymbols(void);
	CCPPIndexSymbol*	GetSymbol(size uiIndex);
	CCPPIndexSymbol*	FindSymbol(const char* szKey);
	CCPPIndexSymbol*	FindSymbolAt(const char* szFileName, int iLine, int iColumn);
	size				NumFiles(void);
	char*				GetFileName(size uiFile);
	char*				Print(CChars* pszDest);

protected:
	static void			GatherToken(CCPPSymbolKey* pcKeys, SCPPTokenSymbol* psSymbol, SCPPIndexToken* psToken, SPreprocessedToken* psSource, uint8 uiFlags, CCPPIndexUnit* pcDest);
	static bool			GetMacroUse(CCPPTokeniser* pcTokeniser, SPreprocessedToken* psSource, SPreprocessedToken* psDest);
	static void			GatherResolved(CCPPSymbolKey* pcKeys, SCPPResolvedName* psResolved, SPreprocessedToken* psSource, CCPPIndexUnit* pcDest);
	static void			GatherReferenceToOverload(CCPPSymbolKey* pcKeys, CCPPSymbol* pcSymbol, int iArguments, SPreprocessedToken* psSource, CCPPIndexUnit* pcDest);
	static void			FindNames(CCPPSyntax* pcNode, CArrayIndexTokens* pasTokens, bool bInDefinition);
	int					GetSymbolIndex(const char* szKey, ECPPSymbolKind eKind);
	uint32				GetFileIndex(const char* szFileName);
	bool				AddLocation(int iSymbol, SCPPIndexLocation* psLocation);
};


int64 GetCPPIndexLocationKey(uint32 uiFile, int iLine, int iColumn);


#endif // __C_P_P_INDEX_H__

