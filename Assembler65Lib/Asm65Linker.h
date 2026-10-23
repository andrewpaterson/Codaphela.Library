/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65_LINKER_H__
#define __ASM65_LINKER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArraySize.h"
#include "Asm65Object.h"
#include "Asm65Output.h"
#include "Asm65Errors.h"


#define ASM65_DEFAULT_TEXT_ADDRESS	0x10000

//The library file: a PackFile holding "Library" (name and object count) and each
//object's ChunkFile as "Objects/0", "Objects/1" ...
#define ASM65_LIBRARY_MAGIC		"Asm65Library"
#define ASM65_LIBRARY_VERSION	1


//Objects the linker includes only when they define a name that is still needed.
//The library owns its objects.
class CAsm65Library
{
protected:
	CChars							mszName;
	CArrayTemplatePtr<CAsm65Object>	mapcObjects;

public:
	void			Init(const char* szName);
	void			Kill(void);

	char*			GetName(void);
	CAsm65Object*	AddObject(const char* szName);
	size			NumObjects(void);
	CAsm65Object*	GetObject(size uiIndex);

	bool			Write(CAbstractFile* pcFile);
	bool			Read(CAbstractFile* pcFile);
};


//An included object: its sections are uiFirstSection onwards in the linker's list.
struct SAsm65LinkObject
{
	CAsm65Object*	pcObject;
	size			uiFirstSection;
};


//An input section's place.  A dropped COMDAT copy is not kept; a section nothing uses
//is not used (SetRemoveUnused).  Its relocations are uiNumRelocations indices from
//uiFirstRelocation in the linker's maiRelocations.
struct SAsm65LinkSection
{
	uint32	uiAddress;
	bool	bKept;
	bool	bUsed;
	size	uiFirstRelocation;
	size	uiNumRelocations;
};


//A global definition (iObject is -1 for the linker's own symbols).  bPlaced is false
//when its section was removed.
struct SAsm65LinkSymbol
{
	CChars	szName;
	int32	iObject;
	uint32	uiSymbol;
	bool	bWeak;
	bool	bPlaced;
	int64	iValue;
};


//A name some included object uses but does not define.  bStrong when any use is not weak.
struct SAsm65LinkReference
{
	bool	bStrong;
	size	uiObject;
};


//Links objects (and the library members they need) into bytes at absolute addresses.
//Sections are placed by kind: text, rodata, init, fini, data then bss.  Text starts at
//$10000 and each kind follows the one before unless SetAddress fixes it.  The linker
//defines __<kind>_start and __<kind>_end for each kind and __end after the last.
class CAsm65Linker
{
protected:
	CAsm65Errors							mcErrors;
	CArrayTemplatePtr<CAsm65Object>			mapcInputs;
	CArrayTemplatePtr<CAsm65Library>		mapcLibraries;

	CArrayTemplate<SAsm65LinkObject>		masObjects;
	CArrayTemplate<SAsm65LinkSection>		masSections;
	CArrayTemplate<SAsm65LinkSymbol>		masSymbols;
	CMapStringTemplate<uint32>				mcSymbolIndices;
	CMapStringTemplate<SAsm65LinkReference>	mcReferences;
	CArrayChars								maszReferenced;
	CMapStringTemplate<SAsm65LinkReference>	mcUsedReferences;
	CArrayChars								maszUsedReferenced;
	CArraySize								maiRelocations;
	CMapStringTemplate<uint32>				mcGroups;
	CArrayChars								maszMissing;
	CArraySize								maiPlacement;
	CAsm65Object							mcStubs;

	uint32									mauiAddress[ASK65_NumKinds];
	bool									mabFixed[ASK65_NumKinds];
	uint32									mauiStart[ASK65_NumKinds];
	uint32									mauiEnd[ASK65_NumKinds];
	uint32									muiLimit;
	bool									mbStubMissing;
	bool									mbRemoveUnused;

	CAsm65Output							mcOutput;
	CChars									mszMap;

public:
	void			Init(void);
	void			Kill(void);

	void			AddObject(CAsm65Object* pcObject);
	void			AddLibrary(CAsm65Library* pcLibrary);
	void			SetAddress(EAsm65SectionKind eKind, uint32 uiAddress);
	void			SetLimit(uint32 uiLimit);
	void			SetStubMissing(bool bStubMissing);
	void			SetRemoveUnused(bool bRemoveUnused);

	bool			Link(void);

	CAsm65Output*	GetOutput(void);
	char*			GetMap(void);
	char*			GetErrors(void);
	size			NumErrors(void);
	bool			GetSymbol(const char* szName, int64* piValue);
	size			NumMissing(void);
	char*			GetMissing(size uiIndex);

protected:
	void			Include(CAsm65Object* pcObject);
	bool			IsIncluded(CAsm65Object* pcObject);
	bool			Wanted(CAsm65Object* pcObject);
	void			PullLibraries(void);
	bool			IsLinkerSymbol(const char* szName);
	void			FindMissing(CMapStringTemplate<SAsm65LinkReference>* pcReferences, CArrayChars* paszReferenced);
	void			AddReference(CMapStringTemplate<SAsm65LinkReference>* pcReferences, CArrayChars* paszReferenced, const char* szName, bool bStrong, size uiObject);
	void			IndexRelocations(void);
	void			MarkUsed(void);
	void			UseSection(size uiSection, CArraySize* paiWork);
	void			UseTarget(size uiObject, SAsm65Relocation* psRelocation, CArraySize* paiWork);
	bool			IsPlaced(SAsm65LinkSection* psLinkSection);
	void			AddStubs(void);
	void			Layout(void);
	void			DefineLinkerSymbols(void);
	void			ResolveSymbols(void);
	bool			Target(size uiObject, SAsm65Relocation* psRelocation, int64* piValue);
	void			Relocate(void);
	void			WriteMap(void);
	void			AppendMapSections(const char* szTitle, bool bRemoved);
	void			AppendMapSection(size uiObject, size uiSection, bool bPlaced);
	size			ObjectOfSection(size uiSection);
	size			SectionIndex(size uiObject, size uiSection);

	SAsm65LinkSymbol*	GetGlobal(const char* szName);
	void			AddGlobal(const char* szName, int32 iObject, uint32 uiSymbol, bool bWeak, int64 iValue);
	bool			SymbolAddress(size uiObject, SAsm65ObjectSymbol* psSymbol, int64* piValue);
	SAsm65LinkSection*	GetLinkSection(size uiObject, size uiSection);
	void			Error(CAsm65Object* pcObject, SAsm65Relocation* psRelocation, const char* szMessage, const char* szDetail);
};


#endif // __ASM65_LINKER_H__
