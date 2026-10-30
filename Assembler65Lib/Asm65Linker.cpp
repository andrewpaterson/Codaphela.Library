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
#include "BaseLib/StringHelper.h"
#include "Asm65Assembler.h"
#include "Asm65Linker.h"


//A symbol in the map.
struct SAsm65MapSymbol
{
	int64		iValue;
	const char*	szName;
	const char*	szObject;
	bool		bGlobal;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CompareMapSymbols(const void* pvLeft, const void* pvRight)
{
	const SAsm65MapSymbol*	psLeft;
	const SAsm65MapSymbol*	psRight;

	psLeft = (const SAsm65MapSymbol*)pvLeft;
	psRight = (const SAsm65MapSymbol*)pvRight;
	if (psLeft->iValue != psRight->iValue)
	{
		return (psLeft->iValue < psRight->iValue) ? -1 : 1;
	}
	return StringCompare(psLeft->szName, psRight->szName);
}


//////////////////////////////////////////////////////////////////////////
//
// $0001001a (BaseLib hex is lower case)
//
//////////////////////////////////////////////////////////////////////////
static void AppendAddress(CChars* psz, uint32 uiAddress)
{
	psz->Append('$');
	psz->AppendHexHiLo(&uiAddress, sizeof(uint32));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Init(void)
{
	int	i;

	mcErrors.Init();
	mapcInputs.Init();
	mapcLibraries.Init();
	masObjects.Init();
	masSections.Init();
	masSymbols.Init();
	mcSymbolIndices.Init(true, true);
	mcReferences.Init(true, true);
	maszReferenced.Init();
	mcUsedReferences.Init(true, true);
	maszUsedReferenced.Init();
	maiRelocations.Init();
	mcGroups.Init(true, true);
	maszMissing.Init();
	maiPlacement.Init();
	mcStubs.Init("linker stubs");
	mapcResidents.Init();
	masUnits.Init();
	masTrampolines.Init();
	mcLazy.Init("lazy loader");
	mcLazyImage.Init();
	mbLazy = false;
	muiLazyEnd = 0;
	muiSectorSize = 0;
	muiFirstSector = 0;
	muiLazyObject = 0;

	for (i = 0; i < ASK65_NumKinds; i++)
	{
		mauiAddress[i] = 0;
		mabFixed[i] = false;
		mauiStart[i] = 0;
		mauiEnd[i] = 0;
	}
	mauiAddress[ASK65_Text] = ASM65_DEFAULT_TEXT_ADDRESS;
	mabFixed[ASK65_Text] = true;
	muiLimit = 0;
	muiBankSize = 0;
	mbStubMissing = false;
	miStubOpcode = -1;
	mbRemoveUnused = false;

	mcOutput.Init();
	mszMap.Init();
	mcLineTable.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The objects and libraries added belong to the caller.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Kill(void)
{
	size	i;

	mcLineTable.Kill();
	mszMap.Kill();
	mcOutput.Kill();

	mcLazyImage.Kill();
	mcLazy.Kill();
	masTrampolines.Kill();
	masUnits.Kill();
	mapcResidents.Kill();
	mcStubs.Kill();
	maiPlacement.Kill();
	maszMissing.Kill();
	mcGroups.Kill();
	maiRelocations.Kill();
	maszUsedReferenced.Kill();
	mcUsedReferences.Kill();
	maszReferenced.Kill();
	mcReferences.Kill();
	mcSymbolIndices.Kill();
	for (i = 0; i < masSymbols.NumElements(); i++)
	{
		masSymbols.Get(i)->szName.Kill();
	}
	masSymbols.Kill();
	masSections.Kill();
	masObjects.Kill();
	mapcLibraries.Kill();
	mapcInputs.Kill();
	mcErrors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Objects are always linked, in the order they are added.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AddObject(CAsm65Object* pcObject)
{
	mapcInputs.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// An object whose code is always in the core, never loaded lazily (SetLazy): a start
// up, interrupt handlers.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AddResidentObject(CAsm65Object* pcObject)
{
	mapcInputs.Add(pcObject);
	mapcResidents.Add(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// A library's objects are linked when they define a name that is still needed.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AddLibrary(CAsm65Library* pcLibrary)
{
	mapcLibraries.Add(pcLibrary);
}


//////////////////////////////////////////////////////////////////////////
//
// Places a kind of section at an address instead of after the kind before it.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetAddress(EAsm65SectionKind eKind, uint32 uiAddress)
{
	mauiAddress[eKind] = uiAddress;
	mabFixed[eKind] = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The program must end at or below uiLimit (0 for no limit).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetLimit(uint32 uiLimit)
{
	muiLimit = uiLimit;
}


//////////////////////////////////////////////////////////////////////////
//
// Sections no bigger than a bank are kept inside one (0 for no banks).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetBankSize(uint32 uiBankSize)
{
	muiBankSize = uiBankSize;
}


//////////////////////////////////////////////////////////////////////////
//
// Names that nothing defines become the address of a stp instead of errors.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetStubMissing(bool bStubMissing)
{
	mbStubMissing = bStubMissing;
}


//////////////////////////////////////////////////////////////////////////
//
// The stub's stp for another processor (the W65C816's is $DB): the 65C64's
// stp is different bytes, which a W65C816 runs as something else.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetStubOpcode(uint8 uiOpcode)
{
	miStubOpcode = uiOpcode;
}


//////////////////////////////////////////////////////////////////////////
//
// Links only the sections that are used: keep sections and init and fini sections
// (not associated with another section), the sections their relocations reach, and
// so on.  An associated section is used when the section it is associated with is.
// A name only unused sections need is not missing.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetRemoveUnused(bool bRemoveUnused)
{
	mbRemoveUnused = bRemoveUnused;
}


//////////////////////////////////////////////////////////////////////////
//
// Includes the objects and the library objects they need, works out what is used
// (SetRemoveUnused), stubs or reports what is missing, places the sections, works out
// the symbols and writes the relocated bytes to the output and the map.  Call once.
// When unused sections are removed, a library object is only included for a name a
// used section needs.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::Link(void)
{
	size	i;

	for (i = 0; i < mapcInputs.NumElements(); i++)
	{
		Include(mapcInputs.GetPtr(i));
	}
	if (mbRemoveUnused)
	{
		do
		{
			MarkUsed();
		}
		while (PullLibraries(&mcUsedReferences));
		FindMissing(&mcUsedReferences, &maszUsedReferenced);
	}
	else
	{
		PullLibraries(&mcReferences);
		FindMissing(&mcReferences, &maszReferenced);
	}
	AddStubs();
	if (mcErrors.NumErrors() != 0)
	{
		return false;
	}
	if (mbLazy)
	{
		PlanLazy();
		if (mcErrors.NumErrors() != 0)
		{
			return false;
		}
	}

	Layout();
	DefineLinkerSymbols();
	ResolveSymbols();
	if (mcErrors.NumErrors() != 0)
	{
		return false;
	}

	Relocate();
	WriteMap();
	BuildLineTable();
	return mcErrors.NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the object's sections (dropping COMDAT copies already kept from an earlier
// object), its global definitions and the names it needs.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Include(CAsm65Object* pcObject)
{
	SAsm65LinkObject*		psObject;
	SAsm65LinkSection*		psLinkSection;
	SAsm65Section*			psSection;
	SAsm65ObjectSymbol*		psSymbol;
	SAsm65LinkSymbol*		psGlobal;
	uint32*					puiGroup;
	uint32					uiGroupObject;
	size					uiObject;
	size					i;
	CChars					szMessage;

	uiObject = masObjects.NumElements();
	psObject = masObjects.Add();
	psObject->pcObject = pcObject;
	psObject->uiFirstSection = masSections.NumElements();

	for (i = 0; i < pcObject->NumSections(); i++)
	{
		psSection = pcObject->GetSection(i);
		psLinkSection = masSections.Add();
		psLinkSection->uiAddress = 0;
		psLinkSection->bKept = true;
		psLinkSection->bUsed = true;
		psLinkSection->iUnit = -1;
		psLinkSection->uiFirstRelocation = 0;
		psLinkSection->uiNumRelocations = 0;
		if (!psSection->szGroup.Empty())
		{
			puiGroup = mcGroups.Get(psSection->szGroup.Text());
			if (puiGroup == NULL)
			{
				uiGroupObject = (uint32)uiObject;
				mcGroups.Put(psSection->szGroup.Text(), &uiGroupObject);
			}
			else if (*puiGroup != uiObject)
			{
				psLinkSection->bKept = false;
			}
		}
	}

	for (i = 0; i < pcObject->NumSymbols(); i++)
	{
		psSymbol = pcObject->GetSymbol(i);
		if (psSymbol->iSection == ASM65_SECTION_UNDEFINED)
		{
			AddReference(&mcReferences, &maszReferenced, psSymbol->szName.Text(), psSymbol->eBinding != AB65_Weak, uiObject);
			continue;
		}
		if (psSymbol->eBinding == AB65_Local)
		{
			continue;
		}
		if ((psSymbol->iSection >= 0) && !GetLinkSection(uiObject, psSymbol->iSection)->bKept)
		{
			continue;
		}

		psGlobal = GetGlobal(psSymbol->szName.Text());
		if (psGlobal == NULL)
		{
			AddGlobal(psSymbol->szName.Text(), (int32)uiObject, (uint32)i, psSymbol->eBinding == AB65_Weak, 0);
		}
		else if (psGlobal->bWeak && (psSymbol->eBinding == AB65_Global))
		{
			psGlobal->iObject = (int32)uiObject;
			psGlobal->uiSymbol = (uint32)i;
			psGlobal->bWeak = false;
		}
		else if (!psGlobal->bWeak && (psSymbol->eBinding == AB65_Global))
		{
			szMessage.Init("Symbol is also defined in ");
			szMessage.Append(masObjects.Get(psGlobal->iObject)->pcObject->GetName());
			mcErrors.Add(pcObject->GetName(), 0, 0, szMessage.Text(), psSymbol->szName.Text());
			szMessage.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::IsIncluded(CAsm65Object* pcObject)
{
	size	i;

	for (i = 0; i < masObjects.NumElements(); i++)
	{
		if (masObjects.Get(i)->pcObject == pcObject)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True when the object defines a name in pcReferences that is needed (not only
// weakly) and that nothing defines yet.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::Wanted(CAsm65Object* pcObject, CMapStringTemplate<SAsm65LinkReference>* pcReferences)
{
	size					i;
	SAsm65ObjectSymbol*		psSymbol;
	SAsm65LinkReference*	psReference;

	for (i = 0; i < pcObject->NumSymbols(); i++)
	{
		psSymbol = pcObject->GetSymbol(i);
		if ((psSymbol->iSection == ASM65_SECTION_UNDEFINED) || (psSymbol->eBinding == AB65_Local))
		{
			continue;
		}
		psReference = pcReferences->Get(psSymbol->szName.Text());
		if (psReference && psReference->bStrong && (GetGlobal(psSymbol->szName.Text()) == NULL))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Includes the library objects pcReferences needs, until no more are needed (a
// library object can need a name from an earlier library).  Returns true if any were
// included.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::PullLibraries(CMapStringTemplate<SAsm65LinkReference>* pcReferences)
{
	bool			bChanged;
	bool			bIncluded;
	size			i;
	size			j;
	CAsm65Library*	pcLibrary;
	CAsm65Object*	pcObject;

	bIncluded = false;
	do
	{
		bChanged = false;
		for (i = 0; i < mapcLibraries.NumElements(); i++)
		{
			pcLibrary = mapcLibraries.GetPtr(i);
			for (j = 0; j < pcLibrary->NumObjects(); j++)
			{
				pcObject = pcLibrary->GetObject(j);
				if (!IsIncluded(pcObject) && Wanted(pcObject, pcReferences))
				{
					Include(pcObject);
					bChanged = true;
					bIncluded = true;
				}
			}
		}
	}
	while (bChanged);
	return bIncluded;
}


//////////////////////////////////////////////////////////////////////////
//
// The linker's own symbols: __<kind>_start and __<kind>_end for each kind
// (iIndex 0 to 2 * ASK65_NumKinds - 1) then __end.
//
//////////////////////////////////////////////////////////////////////////
static void LinkerSymbolName(CChars* psz, int iIndex)
{
	psz->Init("__");
	if (iIndex == ASK65_NumKinds * 2)
	{
		psz->Append("end");
	}
	else
	{
		psz->Append(Asm65SectionKindName((EAsm65SectionKind)(iIndex / 2)));
		psz->Append((iIndex % 2 == 0) ? "_start" : "_end");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::IsLinkerSymbol(const char* szName)
{
	int		i;
	CChars	szLinkerName;
	bool	bResult;

	bResult = false;
	for (i = 0; (i <= ASK65_NumKinds * 2) && !bResult; i++)
	{
		LinkerSymbolName(&szLinkerName, i);
		bResult = szLinkerName.Equals(szName);
		szLinkerName.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Names needed (not only weakly) that nothing defines.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::FindMissing(CMapStringTemplate<SAsm65LinkReference>* pcReferences, CArrayChars* paszReferenced)
{
	size					i;
	char*					szName;
	SAsm65LinkReference*	psReference;

	for (i = 0; i < paszReferenced->NumElements(); i++)
	{
		szName = paszReferenced->GetText(i);
		psReference = pcReferences->Get(szName);
		if (!psReference->bStrong || GetGlobal(szName) || IsLinkerSymbol(szName))
		{
			continue;
		}
		maszMissing.Add(szName);
		if (!mbStubMissing)
		{
			mcErrors.Add(masObjects.Get(psReference->uiObject)->pcObject->GetName(), 0, 0, "Undefined symbol", szName);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Notes that uiObject needs szName (bStrong when not only weakly).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AddReference(CMapStringTemplate<SAsm65LinkReference>* pcReferences, CArrayChars* paszReferenced, const char* szName, bool bStrong, size uiObject)
{
	SAsm65LinkReference*	psReference;
	SAsm65LinkReference		sReference;

	psReference = pcReferences->Get(szName);
	if (psReference == NULL)
	{
		sReference.bStrong = bStrong;
		sReference.uiObject = uiObject;
		pcReferences->Put(szName, &sReference);
		paszReferenced->Add(szName);
	}
	else if (bStrong)
	{
		psReference->bStrong = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Groups the relocations by section: each section's are uiNumRelocations indices
// (into its object's relocations) from uiFirstRelocation in maiRelocations.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::IndexRelocations(void)
{
	size				i;
	size				j;
	size				uiNext;
	SAsm65LinkObject*	psObject;
	SAsm65LinkSection*	psLinkSection;
	SAsm65Relocation*	psRelocation;

	for (i = 0; i < masSections.NumElements(); i++)
	{
		masSections.Get(i)->uiNumRelocations = 0;
	}
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumRelocations(); j++)
		{
			GetLinkSection(i, psObject->pcObject->GetRelocation(j)->uiSection)->uiNumRelocations++;
		}
	}

	uiNext = 0;
	for (i = 0; i < masSections.NumElements(); i++)
	{
		psLinkSection = masSections.Get(i);
		psLinkSection->uiFirstRelocation = uiNext;
		uiNext += psLinkSection->uiNumRelocations;
		psLinkSection->uiNumRelocations = 0;
	}

	maiRelocations.Resize(uiNext);
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumRelocations(); j++)
		{
			psRelocation = psObject->pcObject->GetRelocation(j);
			psLinkSection = GetLinkSection(i, psRelocation->uiSection);
			maiRelocations.SetValue(psLinkSection->uiFirstRelocation + psLinkSection->uiNumRelocations, (int)j);
			psLinkSection->uiNumRelocations++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the used sections (see SetRemoveUnused) and notes the names they need that
// no included object defines.  It starts again each time it is called.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::MarkUsed(void)
{
	CArraySize			aiWork;
	SAsm65LinkObject*	psObject;
	SAsm65Section*		psSection;
	SAsm65LinkSection*	psLinkSection;
	size				uiSection;
	size				uiObject;
	size				i;
	size				j;
	bool				bAdded;

	IndexRelocations();
	for (i = 0; i < masSections.NumElements(); i++)
	{
		masSections.Get(i)->bUsed = false;
	}
	mcUsedReferences.Kill();
	mcUsedReferences.Init(true, true);
	maszUsedReferenced.ReInit();

	aiWork.Init();
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSections(); j++)
		{
			psSection = psObject->pcObject->GetSection(j);
			if (psSection->bKeep || (((psSection->eKind == ASK65_Init) || (psSection->eKind == ASK65_Fini)) && (psSection->iAssociated < 0)))
			{
				UseSection(SectionIndex(i, j), &aiWork);
			}
		}
	}

	do
	{
		while (aiWork.NumElements() > 0)
		{
			uiSection = (size)aiWork.Pop();
			uiObject = ObjectOfSection(uiSection);
			psObject = masObjects.Get(uiObject);
			psLinkSection = masSections.Get(uiSection);
			for (i = 0; i < psLinkSection->uiNumRelocations; i++)
			{
				UseTarget(uiObject, psObject->pcObject->GetRelocation(maiRelocations.GetValue(psLinkSection->uiFirstRelocation + i)), &aiWork);
			}
		}

		//Associated sections whose section is used.
		bAdded = false;
		for (i = 0; i < masObjects.NumElements(); i++)
		{
			psObject = masObjects.Get(i);
			for (j = 0; j < psObject->pcObject->NumSections(); j++)
			{
				psSection = psObject->pcObject->GetSection(j);
				psLinkSection = GetLinkSection(i, j);
				if (!psLinkSection->bUsed && (psSection->iAssociated >= 0) && GetLinkSection(i, psSection->iAssociated)->bUsed)
				{
					UseSection(SectionIndex(i, j), &aiWork);
					bAdded = true;
				}
			}
		}
	}
	while (bAdded);
	aiWork.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::UseSection(size uiSection, CArraySize* paiWork)
{
	SAsm65LinkSection*	psLinkSection;

	psLinkSection = masSections.Get(uiSection);
	if (psLinkSection->bKept && !psLinkSection->bUsed)
	{
		psLinkSection->bUsed = true;
		paiWork->Add((int)uiSection);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Uses the section a relocation reaches, or notes the name it needs if no object
// defines it.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::UseTarget(size uiObject, SAsm65Relocation* psRelocation, CArraySize* paiWork)
{
	CAsm65Object*		pcObject;
	SAsm65ObjectSymbol*	psSymbol;
	SAsm65LinkSymbol*	psGlobal;

	pcObject = masObjects.Get(uiObject)->pcObject;
	if (psRelocation->iTargetSection != ASM65_TARGET_SYMBOL)
	{
		UseSection(SectionIndex(uiObject, psRelocation->iTargetSection), paiWork);
		return;
	}

	psSymbol = pcObject->GetSymbol(psRelocation->uiTargetSymbol);
	if ((psSymbol->eBinding == AB65_Local) && (psSymbol->iSection != ASM65_SECTION_UNDEFINED))
	{
		if (psSymbol->iSection >= 0)
		{
			UseSection(SectionIndex(uiObject, psSymbol->iSection), paiWork);
		}
		return;
	}

	psGlobal = GetGlobal(psSymbol->szName.Text());
	if (psGlobal == NULL)
	{
		AddReference(&mcUsedReferences, &maszUsedReferenced, psSymbol->szName.Text(), psSymbol->eBinding != AB65_Weak, uiObject);
		return;
	}
	if (psGlobal->iObject >= 0)
	{
		psSymbol = masObjects.Get(psGlobal->iObject)->pcObject->GetSymbol(psGlobal->uiSymbol);
		if (psSymbol->iSection >= 0)
		{
			UseSection(SectionIndex(psGlobal->iObject, psSymbol->iSection), paiWork);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A kept and used section is placed.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::IsPlaced(SAsm65LinkSection* psLinkSection)
{
	return psLinkSection->bKept && psLinkSection->bUsed;
}


//////////////////////////////////////////////////////////////////////////
//
// A placed section that is in the core (not a lazy unit).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::IsCore(SAsm65LinkSection* psLinkSection)
{
	return IsPlaced(psLinkSection) && (psLinkSection->iUnit < 0);
}


//////////////////////////////////////////////////////////////////////////
//
// One stp that every missing name is defined as.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AddStubs(void)
{
	CAsm65Assembler		cAssembler;
	SAsm65Segment*		psSegment;
	SAsm65Section*		psSection;
	SAsm65ObjectSymbol*	psSymbol;
	size				i;

	if (!mbStubMissing || (maszMissing.NumElements() == 0))
	{
		return;
	}

	psSection = mcStubs.GetSection(mcStubs.AddSection("text", "", ASK65_Text));
	if (miStubOpcode >= 0)
	{
		psSection->cBytes.Add((uint8)miStubOpcode);
	}
	else
	{
		cAssembler.Init();
		cAssembler.Assemble("stp");
		psSegment = cAssembler.GetOutput()->GetSegment(0);
		psSection->cBytes.Copy(&psSegment->cBytes);
		cAssembler.Kill();
	}
	psSection->uiSize = (uint32)psSection->cBytes.NumElements();
	psSection->uiAlignment = 2;

	for (i = 0; i < maszMissing.NumElements(); i++)
	{
		psSymbol = mcStubs.GetSymbol(mcStubs.PutSymbol(maszMissing.GetText(i)));
		psSymbol->iSection = 0;
		psSymbol->iValue = 0;
		psSymbol->eBinding = AB65_Global;
	}
	Include(&mcStubs);
}


//////////////////////////////////////////////////////////////////////////
//
// Places the core's sections kind by kind, each at its alignment, and checks the kinds
// don't overlap or pass the limit.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Layout(void)
{
	uint64				uiAddress;
	int					iKind;
	int					iOther;
	size				i;
	size				j;
	SAsm65LinkObject*	psObject;
	SAsm65Section*		psSection;
	SAsm65LinkSection*	psLinkSection;
	CChars				szMessage;
	bool				bFirst;

	LayoutAt();
	uiAddress = mauiAddress[ASK65_Text];
	for (iKind = 0; iKind < ASK65_NumKinds; iKind++)
	{
		if (mabFixed[iKind])
		{
			uiAddress = mauiAddress[iKind];
		}
		//The kind starts where its first section goes (after aligning it or moving it to a bank): a table of entries
		//(init, fini) must start on its first entry.
		mauiStart[iKind] = (uint32)uiAddress;
		bFirst = true;
		for (i = 0; i < masObjects.NumElements(); i++)
		{
			psObject = masObjects.Get(i);
			for (j = 0; j < psObject->pcObject->NumSections(); j++)
			{
				psSection = psObject->pcObject->GetSection(j);
				psLinkSection = GetLinkSection(i, j);
				if ((psSection->eKind != iKind) || !IsCore(psLinkSection) || psSection->bAt)
				{
					continue;
				}
				uiAddress = FlowAddress(uiAddress, psSection);
				psLinkSection->uiAddress = (uint32)uiAddress;
				if (bFirst)
				{
					mauiStart[iKind] = (uint32)uiAddress;
					bFirst = false;
				}
				if ((muiBankSize != 0) && (uiAddress < muiBankSize) && (psSection->uiSize != 0))
				{
					//Bank 0 is the stack and direct page (the user, 2026-10-25), and the vectors and their stubs placed with at.
					mcErrors.Add(psObject->pcObject->GetName(), 0, 0, "Only a section with an address (at) can be in bank 0", psSection->szName.Text());
				}
				uiAddress += psSection->uiSize;
				maiPlacement.Add((int)SectionIndex(i, j));
			}
		}
		if (uiAddress > 0xFFFFFFFFULL)
		{
			mcErrors.Add("link", 0, 0, "Sections end past $FFFFFFFF", Asm65SectionKindName((EAsm65SectionKind)iKind));
			uiAddress = 0xFFFFFFFFULL;
		}
		mauiEnd[iKind] = (uint32)uiAddress;
	}

	for (iKind = 0; iKind < ASK65_NumKinds; iKind++)
	{
		for (iOther = iKind + 1; iOther < ASK65_NumKinds; iOther++)
		{
			if ((mauiStart[iKind] < mauiEnd[iKind]) && (mauiStart[iOther] < mauiEnd[iOther]) &&
				(mauiStart[iKind] < mauiEnd[iOther]) && (mauiStart[iOther] < mauiEnd[iKind]))
			{
				szMessage.Init("Sections overlap: ");
				szMessage.Append(Asm65SectionKindName((EAsm65SectionKind)iKind));
				szMessage.Append(" and ");
				szMessage.Append(Asm65SectionKindName((EAsm65SectionKind)iOther));
				mcErrors.Add("link", 0, 0, szMessage.Text());
				szMessage.Kill();
			}
		}
		if ((muiLimit != 0) && (mauiStart[iKind] < mauiEnd[iKind]) && (mauiEnd[iKind] > muiLimit))
		{
			szMessage.Init("Sections end past the limit ");
			AppendAddress(&szMessage, muiLimit);
			mcErrors.Add("link", 0, 0, szMessage.Text(), Asm65SectionKindName((EAsm65SectionKind)iKind));
			szMessage.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Sections with an address go there.  They must not overlap each other or, with banks,
// cross the end of one.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::LayoutAt(void)
{
	size				i;
	size				j;
	size				k;
	SAsm65LinkObject*	psObject;
	SAsm65Section*		psSection;
	SAsm65LinkSection*	psLinkSection;
	SAsm65Section*		psOther;
	SAsm65LinkSection*	psOtherLink;
	size				uiOther;

	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSections(); j++)
		{
			psSection = psObject->pcObject->GetSection(j);
			psLinkSection = GetLinkSection(i, j);
			if (!psSection->bAt || !IsCore(psLinkSection))
			{
				continue;
			}
			psLinkSection->uiAddress = psSection->uiAt;
			if ((uint64)psSection->uiAt + psSection->uiSize > 0x100000000ULL)
			{
				mcErrors.Add(psObject->pcObject->GetName(), 0, 0, "Section ends past $FFFFFFFF", psSection->szName.Text());
			}
			if (!InBank(psSection->uiAt, psSection->uiSize))
			{
				mcErrors.Add(psObject->pcObject->GetName(), 0, 0, "Section crosses the end of a bank", psSection->szName.Text());
			}
			for (k = 0; k < maiPlacement.NumElements(); k++)
			{
				uiOther = maiPlacement.GetValue(k);
				psOtherLink = masSections.Get(uiOther);
				psOther = masObjects.Get(ObjectOfSection(uiOther))->pcObject->GetSection(uiOther - masObjects.Get(ObjectOfSection(uiOther))->uiFirstSection);
				if ((psSection->uiSize != 0) && (psOther->uiSize != 0) &&
					((uint64)psSection->uiAt < (uint64)psOtherLink->uiAddress + psOther->uiSize) && ((uint64)psOtherLink->uiAddress < (uint64)psSection->uiAt + psSection->uiSize))
				{
					mcErrors.Add(psObject->pcObject->GetName(), 0, 0, "Section overlaps another section with an address", psSection->szName.Text());
				}
			}
			maiPlacement.Add((int)SectionIndex(i, j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Where a section goes at or after uiAddress: aligned, not across the end of a bank (unless
// it is bigger than one), and not over a section with an address.
//
//////////////////////////////////////////////////////////////////////////
uint64 CAsm65Linker::FlowAddress(uint64 uiAddress, SAsm65Section* psSection)
{
	uint32				uiAlignment;
	size				k;
	size				uiAt;
	SAsm65Section*		psAt;
	SAsm65LinkSection*	psAtLink;
	SAsm65LinkObject*	psObject;
	bool				bMoved;

	uiAlignment = psSection->uiAlignment;
	do
	{
		bMoved = false;
		uiAddress = (uiAddress + uiAlignment - 1) & ~(uint64)(uiAlignment - 1);
		if (!InBank(uiAddress, psSection->uiSize))
		{
			uiAddress = (uiAddress / muiBankSize + 1) * muiBankSize;
			bMoved = true;
		}
		for (k = 0; k < maiPlacement.NumElements(); k++)
		{
			uiAt = maiPlacement.GetValue(k);
			psObject = masObjects.Get(ObjectOfSection(uiAt));
			psAt = psObject->pcObject->GetSection(uiAt - psObject->uiFirstSection);
			psAtLink = masSections.Get(uiAt);
			if (psAt->bAt && (psAt->uiSize != 0) && (psSection->uiSize != 0) &&
				(uiAddress < (uint64)psAtLink->uiAddress + psAt->uiSize) && ((uint64)psAtLink->uiAddress < uiAddress + psSection->uiSize))
			{
				uiAddress = (uint64)psAtLink->uiAddress + psAt->uiSize;
				bMoved = true;
			}
		}
	}
	while (bMoved);
	return uiAddress;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether uiSize bytes at uiAddress stay in one bank (always, without banks or when
// they are more than a bank).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::InBank(uint64 uiAddress, uint32 uiSize)
{
	if ((muiBankSize == 0) || (uiSize == 0) || (uiSize > muiBankSize))
	{
		return true;
	}
	return (uiAddress / muiBankSize) == ((uiAddress + uiSize - 1) / muiBankSize);
}


//////////////////////////////////////////////////////////////////////////
//
// __<kind>_start, __<kind>_end and __end, unless an object defines them.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::DefineLinkerSymbols(void)
{
	int		i;
	uint32	uiEnd;
	uint32	uiValue;
	CChars	szName;

	uiEnd = 0;
	for (i = 0; i < ASK65_NumKinds; i++)
	{
		if (mauiEnd[i] > uiEnd)
		{
			uiEnd = mauiEnd[i];
		}
	}

	for (i = 0; i <= ASK65_NumKinds * 2; i++)
	{
		if (i == ASK65_NumKinds * 2)
		{
			uiValue = uiEnd;
		}
		else
		{
			uiValue = (i % 2 == 0) ? mauiStart[i / 2] : mauiEnd[i / 2];
		}
		LinkerSymbolName(&szName, i);
		if (GetGlobal(szName.Text()) == NULL)
		{
			AddGlobal(szName.Text(), -1, 0, false, uiValue);
		}
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::ResolveSymbols(void)
{
	size				i;
	SAsm65LinkSymbol*	psGlobal;

	for (i = 0; i < masSymbols.NumElements(); i++)
	{
		psGlobal = masSymbols.Get(i);
		if (psGlobal->iObject >= 0)
		{
			psGlobal->bPlaced = SymbolAddress(psGlobal->iObject, masObjects.Get(psGlobal->iObject)->pcObject->GetSymbol(psGlobal->uiSymbol), &psGlobal->iValue);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A defined symbol's value: its section's address plus its offset, or its absolute
// value.  A symbol in a lazy unit is its trampoline.  False when its section is not
// placed (a dropped COMDAT copy or removed), or in a unit with no trampoline there.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::SymbolAddress(size uiObject, SAsm65ObjectSymbol* psSymbol, int64* piValue)
{
	SAsm65LinkSection*	psLinkSection;
	int32				iTrampoline;

	*piValue = psSymbol->iValue;
	if (psSymbol->iSection < 0)
	{
		return true;
	}
	psLinkSection = GetLinkSection(uiObject, psSymbol->iSection);
	if (psLinkSection->iUnit >= 0)
	{
		iTrampoline = FindTrampoline(psLinkSection->iUnit, (uint32)psSymbol->iValue);
		if (iTrampoline < 0)
		{
			return false;
		}
		*piValue = TrampolineAddress(iTrampoline);
		return true;
	}
	*piValue += psLinkSection->uiAddress;
	return IsPlaced(psLinkSection);
}


//////////////////////////////////////////////////////////////////////////
//
// Where the relocation's target is: an offset (with the addend) in a section
// (*piSection, an index into masSections) or a value (*piSection is -1).  An undefined
// weak name is 0.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::TargetPlace(size uiObject, SAsm65Relocation* psRelocation, int64* piSection, int64* piValue)
{
	CAsm65Object*		pcObject;
	SAsm65ObjectSymbol*	psSymbol;
	SAsm65LinkSymbol*	psGlobal;
	size				uiDefiner;

	pcObject = masObjects.Get(uiObject)->pcObject;
	*piSection = -1;
	*piValue = psRelocation->iAddend;
	if (psRelocation->iTargetSection != ASM65_TARGET_SYMBOL)
	{
		if (!GetLinkSection(uiObject, psRelocation->iTargetSection)->bKept)
		{
			Error(pcObject, psRelocation, "Refers to a dropped COMDAT section", pcObject->GetSection(psRelocation->iTargetSection)->szName.Text());
			return false;
		}
		*piSection = (int64)SectionIndex(uiObject, psRelocation->iTargetSection);
		return true;
	}

	psSymbol = pcObject->GetSymbol(psRelocation->uiTargetSymbol);
	uiDefiner = uiObject;
	if ((psSymbol->eBinding != AB65_Local) || (psSymbol->iSection == ASM65_SECTION_UNDEFINED))
	{
		psGlobal = GetGlobal(psSymbol->szName.Text());
		if (psGlobal == NULL)
		{
			//The linker's own symbols are values in the core, defined once the core is laid out.
			if ((psSymbol->eBinding == AB65_Weak) || IsLinkerSymbol(psSymbol->szName.Text()))
			{
				return true;
			}
			Error(pcObject, psRelocation, "Undefined symbol", psSymbol->szName.Text());
			return false;
		}
		if (psGlobal->iObject < 0)
		{
			*piValue += psGlobal->iValue;
			return true;
		}
		uiDefiner = (size)psGlobal->iObject;
		psSymbol = masObjects.Get(uiDefiner)->pcObject->GetSymbol(psGlobal->uiSymbol);
	}

	*piValue += psSymbol->iValue;
	if (psSymbol->iSection >= 0)
	{
		if (!GetLinkSection(uiDefiner, psSymbol->iSection)->bKept)
		{
			Error(pcObject, psRelocation, "Refers to a dropped COMDAT section", psSymbol->szName.Text());
			return false;
		}
		*piSection = (int64)SectionIndex(uiDefiner, psSymbol->iSection);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The relocation's target plus its addend as an address: a place in a lazy unit is
// its trampoline.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::Target(size uiObject, SAsm65Relocation* psRelocation, int64* piValue)
{
	SAsm65LinkSection*	psLinkSection;
	int64				iSection;
	int32				iTrampoline;

	if (!TargetPlace(uiObject, psRelocation, &iSection, piValue))
	{
		return false;
	}
	if (iSection < 0)
	{
		return true;
	}
	psLinkSection = masSections.Get((size)iSection);
	if (psLinkSection->iUnit < 0)
	{
		*piValue += psLinkSection->uiAddress;
		return true;
	}
	iTrampoline = FindTrampoline(psLinkSection->iUnit, (uint32)*piValue);
	if (iTrampoline < 0)
	{
		Error(masObjects.Get(uiObject)->pcObject, psRelocation, "Refers inside a lazy unit", NULL);
		return false;
	}
	*piValue = TrampolineAddress(iTrampoline);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies every core section, writes its relocated fields, then writes the copies to
// the output in address order, then builds the lazy units' images.  The objects are
// not changed.
//
//////////////////////////////////////////////////////////////////////////
//
// The line table: each placed section's source lines at their addresses,
// then a line of 0 (no line) at the end of a section that has any, so code
// after it isn't taken for its last line.  The fingerprints of the objects
// with a placed section.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::BuildLineTable(void)
{
	SAsm65LinkObject*	psObject;
	SAsm65LinkSection*	psLinkSection;
	SAsm65SourceLine*	psSourceLine;
	SAsm65Fingerprint*	psFingerprint;
	CAsm65Object*		pcObject;
	CArrayUint8			auiHasLines;
	size				i;
	size				j;
	bool				bPlaced;

	mcLineTable.Clear();
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		pcObject = psObject->pcObject;
		if (pcObject->NumSourceLines() == 0)
		{
			continue;
		}
		auiHasLines.Init();
		auiHasLines.GrowToAtLeastNumElements(pcObject->NumSections(), true, 0);
		bPlaced = false;
		for (j = 0; j < pcObject->NumSourceLines(); j++)
		{
			psSourceLine = pcObject->GetSourceLine(j);
			psLinkSection = GetLinkSection(i, psSourceLine->uiSection);
			if (IsPlaced(psLinkSection))
			{
				mcLineTable.AddLine(psLinkSection->uiAddress + psSourceLine->uiOffset, pcObject->GetSourceFile(psSourceLine->uiFile), psSourceLine->uiLine);
				auiHasLines.SetValue(psSourceLine->uiSection, 1);
				bPlaced = true;
			}
		}
		for (j = 0; j < pcObject->NumSections(); j++)
		{
			if (auiHasLines.GetValue(j))
			{
				psLinkSection = GetLinkSection(i, j);
				mcLineTable.AddLine(psLinkSection->uiAddress + pcObject->GetSection(j)->uiSize, "", 0);
			}
		}
		auiHasLines.Kill();
		for (j = 0; bPlaced && (j < pcObject->NumFingerprints()); j++)
		{
			psFingerprint = pcObject->GetFingerprint(j);
			mcLineTable.AddFingerprint(psFingerprint->szLabel.Text(), psFingerprint->uiHash, psFingerprint->iTime);
		}
	}
	mcLineTable.Sort();
}


//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Relocate(void)
{
	CArrayTemplate<CArrayUint8>	acBytes;
	CArrayUint8*				pcBytes;
	SAsm65LinkObject*			psObject;
	SAsm65Section*				psSection;
	SAsm65LinkSection*			psLinkSection;
	SAsm65Relocation*			psRelocation;
	int64						iValue;
	size						i;
	size						j;
	size						uiGlobal;
	uint8						k;
	CChars						szValue;

	acBytes.Init();
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSections(); j++)
		{
			pcBytes = acBytes.Add();
			pcBytes->Init();
			if (IsCore(GetLinkSection(i, j)))
			{
				pcBytes->Copy(&psObject->pcObject->GetSection(j)->cBytes);
			}
		}
	}

	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumRelocations(); j++)
		{
			psRelocation = psObject->pcObject->GetRelocation(j);
			psSection = psObject->pcObject->GetSection(psRelocation->uiSection);
			psLinkSection = GetLinkSection(i, psRelocation->uiSection);
			if (!IsCore(psLinkSection))
			{
				continue;
			}
			pcBytes = acBytes.Get(SectionIndex(i, psRelocation->uiSection));
			if ((uint64)psRelocation->uiOffset + psRelocation->uiBytes > pcBytes->NumElements())
			{
				Error(psObject->pcObject, psRelocation, "Relocation is outside its section", psSection->szName.Text());
				continue;
			}
			if (!Target(i, psRelocation, &iValue))
			{
				continue;
			}
			if (psRelocation->bRelative)
			{
				iValue -= (int64)psLinkSection->uiAddress + psRelocation->uiOffset;
			}
			if (!Asm65ApplyForm(psRelocation->uiForm, (int64)psLinkSection->uiAddress + psRelocation->uiOffset, &iValue))
			{
				Error(psObject->pcObject, psRelocation, "Address is not in the bank of the instruction using it", psSection->szName.Text());
				continue;
			}
			if (!Asm65Fits(iValue, psRelocation->uiBytes, psRelocation->bSigned))
			{
				szValue.Init();
				szValue.Append(iValue);
				szValue.Append(" in ");
				szValue.Append((uint32)psRelocation->uiBytes);
				szValue.Append(psRelocation->bSigned ? " signed bytes" : " bytes");
				Error(psObject->pcObject, psRelocation, "Relocated value does not fit", szValue.Text());
				szValue.Kill();
				continue;
			}
			for (k = 0; k < psRelocation->uiBytes; k++)
			{
				pcBytes->SetValue(psRelocation->uiOffset + k, (uint8)((uint64)iValue >> (8 * k)));
			}
		}
	}

	for (i = 0; i < maiPlacement.NumElements(); i++)
	{
		uiGlobal = maiPlacement.GetValue(i);
		pcBytes = acBytes.Get(uiGlobal);
		psLinkSection = masSections.Get(uiGlobal);
		for (j = 0; j < pcBytes->NumElements(); j++)
		{
			mcOutput.Write(psLinkSection->uiAddress + (uint32)j, pcBytes->GetValue(j));
		}
	}

	for (i = 0; i < acBytes.NumElements(); i++)
	{
		acBytes.Get(i)->Kill();
	}
	acBytes.Kill();

	if (mbLazy)
	{
		BuildUnitImages();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The placed sections, the dropped COMDAT copies, the removed unused sections, the
// lazy units and every placed symbol by address (a symbol in a unit at its trampoline).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::WriteMap(void)
{
	CArrayTemplate<SAsm65MapSymbol>	asSymbols;
	SAsm65MapSymbol*				psMapSymbol;
	SAsm65LinkObject*				psObject;
	SAsm65ObjectSymbol*				psSymbol;
	SAsm65LinkSymbol*				psGlobal;
	size							i;
	size							j;
	size							uiObject;
	size							uiSection;
	int64							iValue;

	mszMap.Append("Sections\n");
	for (i = 0; i < maiPlacement.NumElements(); i++)
	{
		uiSection = (size)maiPlacement.GetValue(i);
		uiObject = ObjectOfSection(uiSection);
		AppendMapSection(uiObject, uiSection - masObjects.Get(uiObject)->uiFirstSection, true);
	}
	AppendMapSections("Dropped COMDAT sections\n", false);
	AppendMapSections("Removed unused sections\n", true);
	if (masUnits.NumElements() > 0)
	{
		mszMap.Append("Lazy units (sector, size)\n");
		for (i = 0; i < masUnits.NumElements(); i++)
		{
			uiSection = masUnits.Get(i)->uiSection;
			uiObject = ObjectOfSection(uiSection);
			mszMap.Append("  ");
			AppendAddress(&mszMap, masUnits.Get(i)->uiSector);
			AppendMapSection(uiObject, uiSection - masObjects.Get(uiObject)->uiFirstSection, false);
		}
	}

	asSymbols.Init();
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSymbols(); j++)
		{
			psSymbol = psObject->pcObject->GetSymbol(j);
			if ((psSymbol->iSection == ASM65_SECTION_UNDEFINED) || !SymbolAddress(i, psSymbol, &iValue))
			{
				continue;
			}
			psMapSymbol = asSymbols.Add();
			psMapSymbol->iValue = iValue;
			psMapSymbol->szName = psSymbol->szName.Text();
			psMapSymbol->szObject = psObject->pcObject->GetName();
			psMapSymbol->bGlobal = psSymbol->eBinding != AB65_Local;
		}
	}
	for (i = 0; i < masSymbols.NumElements(); i++)
	{
		psGlobal = masSymbols.Get(i);
		if (psGlobal->iObject < 0)
		{
			psMapSymbol = asSymbols.Add();
			psMapSymbol->iValue = psGlobal->iValue;
			psMapSymbol->szName = psGlobal->szName.Text();
			psMapSymbol->szObject = "linker";
			psMapSymbol->bGlobal = true;
		}
	}
	asSymbols.Sort(CompareMapSymbols);

	mszMap.Append("Symbols\n");
	for (i = 0; i < asSymbols.NumElements(); i++)
	{
		psMapSymbol = asSymbols.Get(i);
		mszMap.Append("  ");
		AppendAddress(&mszMap, (uint32)psMapSymbol->iValue);
		mszMap.Append(psMapSymbol->bGlobal ? " G " : " L ");
		mszMap.Append(psMapSymbol->szName);
		mszMap.Append(" (");
		mszMap.Append(psMapSymbol->szObject);
		mszMap.Append(")\n");
	}
	asSymbols.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The dropped COMDAT copies (bRemoved false) or the removed unused sections, under
// szTitle when there are any.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AppendMapSections(const char* szTitle, bool bRemoved)
{
	size				i;
	size				j;
	SAsm65LinkSection*	psLinkSection;
	bool				bAny;

	bAny = false;
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		for (j = 0; j < masObjects.Get(i)->pcObject->NumSections(); j++)
		{
			psLinkSection = GetLinkSection(i, j);
			if (bRemoved ? (!psLinkSection->bKept || psLinkSection->bUsed) : psLinkSection->bKept)
			{
				continue;
			}
			if (!bAny)
			{
				mszMap.Append(szTitle);
				bAny = true;
			}
			AppendMapSection(i, j, false);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// "  $address $size name (object)" for a placed section, "  $size name (object)" for
// a removed one, and "  name comdat group (object)" for a dropped COMDAT copy.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AppendMapSection(size uiObject, size uiSection, bool bPlaced)
{
	CAsm65Object*		pcObject;
	SAsm65Section*		psSection;
	SAsm65LinkSection*	psLinkSection;

	pcObject = masObjects.Get(uiObject)->pcObject;
	psSection = pcObject->GetSection(uiSection);
	psLinkSection = GetLinkSection(uiObject, uiSection);
	mszMap.Append("  ");
	if (bPlaced)
	{
		AppendAddress(&mszMap, psLinkSection->uiAddress);
		mszMap.Append(" ");
	}
	if (psLinkSection->bKept)
	{
		AppendAddress(&mszMap, psSection->uiSize);
		mszMap.Append(" ");
		mszMap.Append(psSection->szName.Text());
	}
	else
	{
		mszMap.Append(psSection->szName.Text());
		mszMap.Append(" comdat ");
		mszMap.Append(psSection->szGroup.Text());
	}
	mszMap.Append(" (");
	mszMap.Append(pcObject->GetName());
	mszMap.Append(")\n");
}


//////////////////////////////////////////////////////////////////////////
//
// The object whose sections include uiSection (an index into masSections).
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Linker::ObjectOfSection(size uiSection)
{
	size	uiObject;

	for (uiObject = masObjects.NumElements() - 1; masObjects.Get(uiObject)->uiFirstSection > uiSection; uiObject--)
	{
	}
	return uiObject;
}


//////////////////////////////////////////////////////////////////////////
//
// The index into masSections of an object's section.
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Linker::SectionIndex(size uiObject, size uiSection)
{
	return masObjects.Get(uiObject)->uiFirstSection + uiSection;
}


//////////////////////////////////////////////////////////////////////////
//
// iObject is -1 for the linker's own symbols (iValue is then their value).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AddGlobal(const char* szName, int32 iObject, uint32 uiSymbol, bool bWeak, int64 iValue)
{
	SAsm65LinkSymbol*	psGlobal;
	uint32				uiIndex;

	uiIndex = (uint32)masSymbols.NumElements();
	psGlobal = masSymbols.Add();
	psGlobal->szName.Init(szName);
	psGlobal->iObject = iObject;
	psGlobal->uiSymbol = uiSymbol;
	psGlobal->bWeak = bWeak;
	psGlobal->bPlaced = true;
	psGlobal->iValue = iValue;
	mcSymbolIndices.Put(szName, &uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65LinkSymbol* CAsm65Linker::GetGlobal(const char* szName)
{
	uint32*	puiIndex;

	puiIndex = mcSymbolIndices.Get(szName);
	if (puiIndex == NULL)
	{
		return NULL;
	}
	return masSymbols.Get(*puiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65LinkSection* CAsm65Linker::GetLinkSection(size uiObject, size uiSection)
{
	return masSections.Get(SectionIndex(uiObject, uiSection));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Error(CAsm65Object* pcObject, SAsm65Relocation* psRelocation, const char* szMessage, const char* szDetail)
{
	mcErrors.Add(pcObject->GetName(), psRelocation->uiLine, psRelocation->uiColumn, szMessage, szDetail);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Output* CAsm65Linker::GetOutput(void)
{
	return &mcOutput;
}


//////////////////////////////////////////////////////////////////////////
//
// The placed code's source lines (.line) and the objects' fingerprints,
// after Link.
//
//////////////////////////////////////////////////////////////////////////
CAsm65LineTable* CAsm65Linker::GetLineTable(void)
{
	return &mcLineTable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Linker::GetMap(void)
{
	return mszMap.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Linker::GetErrors(void)
{
	return mcErrors.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Linker::NumErrors(void)
{
	return mcErrors.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
// A global's address after Link (or a linker symbol such as __end).  False for a
// global in a removed section.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::GetSymbol(const char* szName, int64* piValue)
{
	SAsm65LinkSymbol*	psGlobal;

	psGlobal = GetGlobal(szName);
	if ((psGlobal == NULL) || !psGlobal->bPlaced)
	{
		return false;
	}
	*piValue = psGlobal->iValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The names nothing defined (stp stubs with SetStubMissing).
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Linker::NumMissing(void)
{
	return maszMissing.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Linker::GetMissing(size uiIndex)
{
	return maszMissing.GetText(uiIndex);
}
