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
#include "Asm65Library.h"
#include "Asm65Output.h"
#include "Asm65Errors.h"


#define ASM65_DEFAULT_TEXT_ADDRESS	0x10000

//A lazy unit's relocation record is two 32 bit values: the field's offset in the unit (with
//ASM65_LAZY_RELATIVE set when the field is relative) and, for a relative field, its target.
#define ASM65_LAZY_RELATIVE			0x80000000
#define ASM65_LAZY_RECORD_BYTES		8
#define ASM65_LAZY_DIRECTORY_BYTES	24

//An included object: its sections are uiFirstSection onwards in the linker's list.
struct SAsm65LinkObject
{
	CAsm65Object*	pcObject;
	size			uiFirstSection;
};


//An input section's place.  A dropped COMDAT copy is not kept; a section nothing uses
//is not used (SetRemoveUnused).  Its relocations are uiNumRelocations indices from
//uiFirstRelocation in the linker's maiRelocations.  A section loaded lazily is
//iUnit (-1 when it is in the core).
struct SAsm65LinkSection
{
	uint32	uiAddress;
	bool	bKept;
	bool	bUsed;
	int32	iUnit;
	size	uiFirstRelocation;
	size	uiNumRelocations;
};


//A section loaded when it is first called (SetLazy): its sectors on the drive and its
//relocation records.
struct SAsm65LinkUnit
{
	size	uiSection;
	uint32	uiSector;
	uint32	uiSectors;
	uint32	uiRecords;
};


//A place in a unit that the core refers to: calls and addresses go to its trampoline.
struct SAsm65LinkTrampoline
{
	int32	iUnit;
	uint32	uiOffset;
};


//How a field in a lazy unit is written.
enum EAsm65LazyField
{
	ALF65_Error,
	ALF65_Value,		//The value is written now.
	ALF65_Base,			//The value (an offset in the unit) has the unit's address added when it is loaded.
	ALF65_Relative,		//The value is a target; the field is the target minus its own address when loaded.
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
//A section with an address (.section name, at address) is placed there first and the
//others flow around it.  With SetBankSize (the W65C816's $10000) a section no bigger
//than a bank is never placed across the end of one, and only sections with an address
//are in bank 0 (it is the stack and the direct page, plus the vectors and their stubs).
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

	bool									mbLazy;
	uint32									muiLazyEnd;
	uint32									muiSectorSize;
	uint32									muiFirstSector;
	CArrayTemplatePtr<CAsm65Object>			mapcResidents;
	CArrayTemplate<SAsm65LinkUnit>			masUnits;
	CArrayTemplate<SAsm65LinkTrampoline>	masTrampolines;
	CAsm65Object							mcLazy;
	size									muiLazyObject;
	CArrayUint8								mcLazyImage;

	uint32									mauiAddress[ASK65_NumKinds];
	bool									mabFixed[ASK65_NumKinds];
	uint32									mauiStart[ASK65_NumKinds];
	uint32									mauiEnd[ASK65_NumKinds];
	uint32									muiLimit;
	uint32									muiBankSize;
	bool									mbStubMissing;
	int										miStubOpcode;	//The stub's one byte stp (SetStubOpcode); -1: the 65C64's stp.
	bool									mbRemoveUnused;

	CAsm65Output							mcOutput;
	CChars									mszMap;

public:
	void				Init(void);
	void				Kill(void);

	void				AddObject(CAsm65Object* pcObject);
	void				AddResidentObject(CAsm65Object* pcObject);
	void				AddLibrary(CAsm65Library* pcLibrary);
	void				SetAddress(EAsm65SectionKind eKind, uint32 uiAddress);
	void				SetLimit(uint32 uiLimit);
	void				SetBankSize(uint32 uiBankSize);
	void				SetStubMissing(bool bStubMissing);
	void				SetStubOpcode(uint8 uiOpcode);
	void				SetRemoveUnused(bool bRemoveUnused);
	void				SetLazy(uint32 uiCodeEnd, uint32 uiSectorSize, uint32 uiFirstSector);

	bool				Link(void);

	CAsm65Output*		GetOutput(void);
	char*				GetMap(void);
	char*				GetErrors(void);
	size				NumErrors(void);
	bool				GetSymbol(const char* szName, int64* piValue);
	size				NumMissing(void);
	char*				GetMissing(size uiIndex);
	size				NumUnits(void);
	CArrayUint8*		GetLazyImage(void);

protected:
	void				Include(CAsm65Object* pcObject);
	bool				IsIncluded(CAsm65Object* pcObject);
	bool				Wanted(CAsm65Object* pcObject, CMapStringTemplate<SAsm65LinkReference>* pcReferences);
	bool				PullLibraries(CMapStringTemplate<SAsm65LinkReference>* pcReferences);
	bool				IsLinkerSymbol(const char* szName);
	void				FindMissing(CMapStringTemplate<SAsm65LinkReference>* pcReferences, CArrayChars* paszReferenced);
	void				AddReference(CMapStringTemplate<SAsm65LinkReference>* pcReferences, CArrayChars* paszReferenced, const char* szName, bool bStrong, size uiObject);
	void				IndexRelocations(void);
	void				MarkUsed(void);
	void				UseSection(size uiSection, CArraySize* paiWork);
	void				UseTarget(size uiObject, SAsm65Relocation* psRelocation, CArraySize* paiWork);
	bool				IsPlaced(SAsm65LinkSection* psLinkSection);
	bool				IsCore(SAsm65LinkSection* psLinkSection);
	void				AddStubs(void);
	void				Layout(void);
	void				LayoutAt(void);
	uint64				FlowAddress(uint64 uiAddress, SAsm65Section* psSection);
	bool				InBank(uint64 uiAddress, uint32 uiSize);
	void				DefineLinkerSymbols(void);
	void				ResolveSymbols(void);
	bool				TargetPlace(size uiObject, SAsm65Relocation* psRelocation, int64* piSection, int64* piValue);
	bool				Target(size uiObject, SAsm65Relocation* psRelocation, int64* piValue);
	void				Relocate(void);
	void				WriteMap(void);
	void				AppendMapSections(const char* szTitle, bool bRemoved);
	void				AppendMapSection(size uiObject, size uiSection, bool bPlaced);
	size				ObjectOfSection(size uiSection);
	size				SectionIndex(size uiObject, size uiSection);

	SAsm65LinkSymbol*	GetGlobal(const char* szName);
	void				AddGlobal(const char* szName, int32 iObject, uint32 uiSymbol, bool bWeak, int64 iValue);
	bool				SymbolAddress(size uiObject, SAsm65ObjectSymbol* psSymbol, int64* piValue);
	SAsm65LinkSection*	GetLinkSection(size uiObject, size uiSection);

	//Asm65LinkerLazy.cpp: units loaded when they are first called.
	bool				IsResident(CAsm65Object* pcObject);
	void				PlanLazy(void);
	EAsm65LazyField		LazyField(size uiObject, SAsm65Relocation* psRelocation, int32 iUnit, bool bPlan, int64* piValue);
	int32				FindTrampoline(int32 iUnit, uint32 uiOffset);
	int32				PutTrampoline(int32 iUnit, uint32 uiOffset);
	int64				TrampolineAddress(int32 iTrampoline);
	void				AssembleLazyObject(void);
	void				BuildUnitImages(void);
	void				PadLazyImage(uint32 uiSector);
	SAsm65Section*		UnitSection(SAsm65LinkUnit* psUnit);
	void				Error(CAsm65Object* pcObject, SAsm65Relocation* psRelocation, const char* szMessage, const char* szDetail);
};


#endif // __ASM65_LINKER_H__
