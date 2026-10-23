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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/PackFiles.h"
#include "BaseLib/PackFile.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
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
void CAsm65Library::Init(const char* szName)
{
	mszName.Init(szName);
	mapcObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Library::Kill(void)
{
	size			i;
	CAsm65Object*	pcObject;

	for (i = 0; i < mapcObjects.NumElements(); i++)
	{
		pcObject = mapcObjects.GetPtr(i);
		SafeKill(pcObject);
	}
	mapcObjects.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Library::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// An empty object for the caller to assemble into.
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CAsm65Library::AddObject(const char* szName)
{
	CAsm65Object*	pcObject;

	pcObject = NewMalloc<CAsm65Object>();
	pcObject->Init(szName);
	mapcObjects.Add(pcObject);
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Library::NumObjects(void)
{
	return mapcObjects.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAsm65Object* CAsm65Library::GetObject(size uiIndex)
{
	return mapcObjects.GetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// "Objects/3"
//
//////////////////////////////////////////////////////////////////////////
static void ObjectPackName(CChars* psz, size uiIndex)
{
	psz->Init("Objects/");
	psz->Append((uint32)uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the library as a PackFile.  Each object is written as a ChunkFile in
// memory first, then copied into the pack.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Library::Write(CAbstractFile* pcFile)
{
	CPackFiles		cPackFiles;
	CFileBasic		cBasic;
	CPackFile*		pcPackFile;
	CMemoryFile*	pcMemoryFile;
	CChars			szName;
	size			i;
	bool			bResult;

	if (!cPackFiles.Init(pcFile, PFM_Write))
	{
		return false;
	}

	bResult = false;
	pcPackFile = cPackFiles.WriteOpen("Library");
	if (pcPackFile)
	{
		cBasic.Init(pcPackFile);
		bResult = Asm65WriteMagic(&cBasic, ASM65_LIBRARY_MAGIC, ASM65_LIBRARY_VERSION);
		bResult &= mszName.WriteString(&cBasic);
		bResult &= cBasic.WriteInt32((uint32)mapcObjects.NumElements());
		bResult &= cBasic.Close();
		cBasic.Kill();
	}

	for (i = 0; bResult && (i < mapcObjects.NumElements()); i++)
	{
		pcMemoryFile = MemoryFile();
		pcMemoryFile->mbBasicFileMustFree = false;
		bResult = GetObject(i)->Write(pcMemoryFile);

		ObjectPackName(&szName, i);
		pcPackFile = bResult ? cPackFiles.WriteOpen(szName.Text()) : NULL;
		szName.Kill();
		if (pcPackFile)
		{
			cBasic.Init(pcPackFile);
			bResult = cBasic.WriteData(pcMemoryFile->GetBufferPointer(), pcMemoryFile->GetBufferSize());
			bResult &= cBasic.Close();
			cBasic.Kill();
		}
		else
		{
			bResult = false;
		}
		SafeKill(pcMemoryFile);
	}

	cPackFiles.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads a library written by Write into this (empty, initialised) library.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Library::Read(CAbstractFile* pcFile)
{
	CPackFiles		cPackFiles;
	CFileBasic		cBasic;
	CPackFile*		pcPackFile;
	CChars			szName;
	CArrayUint8		cBytes;
	uint32			uiCount;
	uint32			i;
	bool			bResult;
	CAsm65Object*	pcObject;

	if ((mapcObjects.NumElements() != 0) || !cPackFiles.Init(pcFile, PFM_Read))
	{
		return false;
	}

	bResult = false;
	pcPackFile = cPackFiles.ReadOpen("Library");
	if (pcPackFile)
	{
		cBasic.Init(pcPackFile);
		bResult = Asm65ReadMagic(&cBasic, ASM65_LIBRARY_MAGIC, ASM65_LIBRARY_VERSION);
		bResult = bResult && Asm65ReadText(&cBasic, &mszName);
		bResult = bResult && cBasic.ReadInt32(&uiCount);
		cBasic.Close();
		cBasic.Kill();
	}

	for (i = 0; bResult && (i < uiCount); i++)
	{
		ObjectPackName(&szName, i);
		pcPackFile = cPackFiles.ReadOpen(szName.Text());
		szName.Kill();
		if (pcPackFile == NULL)
		{
			bResult = false;
			break;
		}

		cBytes.Init();
		cBasic.Init(pcPackFile);
		cBytes.Resize((size)cBasic.Size());
		bResult = (cBytes.NumElements() == 0) || cBasic.ReadData(cBytes.GetData(), cBytes.NumElements());
		cBasic.Close();
		cBasic.Kill();

		pcObject = AddObject("");
		bResult = bResult && pcObject->Read(MemoryFile(cBytes.GetData(), cBytes.NumElements()));
		cBytes.Kill();
	}

	cPackFiles.Kill();
	return bResult;
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
	mbStubMissing = false;
	mbRemoveUnused = false;

	mcOutput.Init();
	mszMap.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The objects and libraries added belong to the caller.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Kill(void)
{
	size	i;

	mszMap.Kill();
	mcOutput.Kill();

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
// Names that nothing defines become the address of a stp instead of errors.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetStubMissing(bool bStubMissing)
{
	mbStubMissing = bStubMissing;
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
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::Link(void)
{
	size	i;

	for (i = 0; i < mapcInputs.NumElements(); i++)
	{
		Include(mapcInputs.GetPtr(i));
	}
	PullLibraries();
	if (mbRemoveUnused)
	{
		MarkUsed();
		FindMissing(&mcUsedReferences, &maszUsedReferenced);
	}
	else
	{
		FindMissing(&mcReferences, &maszReferenced);
	}
	AddStubs();
	if (mcErrors.NumErrors() != 0)
	{
		return false;
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
	uint32					uiObject;
	size					i;
	CChars					szMessage;

	uiObject = (uint32)masObjects.NumElements();
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
		psLinkSection->uiFirstRelocation = 0;
		psLinkSection->uiNumRelocations = 0;
		if (!psSection->szGroup.Empty())
		{
			puiGroup = mcGroups.Get(psSection->szGroup.Text());
			if (puiGroup == NULL)
			{
				mcGroups.Put(psSection->szGroup.Text(), &uiObject);
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
// True when the object defines a name an included object needs (not only weakly)
// that nothing defines yet.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::Wanted(CAsm65Object* pcObject)
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
		psReference = mcReferences.Get(psSymbol->szName.Text());
		if (psReference && psReference->bStrong && (GetGlobal(psSymbol->szName.Text()) == NULL))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Includes library objects until no more are needed (a library object can need a
// name from an earlier library).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::PullLibraries(void)
{
	bool			bChanged;
	size			i;
	size			j;
	CAsm65Library*	pcLibrary;
	CAsm65Object*	pcObject;

	do
	{
		bChanged = false;
		for (i = 0; i < mapcLibraries.NumElements(); i++)
		{
			pcLibrary = mapcLibraries.GetPtr(i);
			for (j = 0; j < pcLibrary->NumObjects(); j++)
			{
				pcObject = pcLibrary->GetObject(j);
				if (!IsIncluded(pcObject) && Wanted(pcObject))
				{
					Include(pcObject);
					bChanged = true;
				}
			}
		}
	}
	while (bChanged);
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
// Marks the used sections (see SetRemoveUnused) and notes the names they need.
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

	aiWork.Init();
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSections(); j++)
		{
			psSection = psObject->pcObject->GetSection(j);
			if (psSection->bKeep || (((psSection->eKind == ASK65_Init) || (psSection->eKind == ASK65_Fini)) && (psSection->iAssociated < 0)))
			{
				UseSection(psObject->uiFirstSection + j, &aiWork);
			}
		}
	}

	do
	{
		while (aiWork.NumElements() > 0)
		{
			uiSection = (size)aiWork.Pop();
			for (uiObject = masObjects.NumElements() - 1; masObjects.Get(uiObject)->uiFirstSection > uiSection; uiObject--)
			{
			}
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
					UseSection(psObject->uiFirstSection + j, &aiWork);
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
		UseSection(masObjects.Get(uiObject)->uiFirstSection + psRelocation->iTargetSection, paiWork);
		return;
	}

	psSymbol = pcObject->GetSymbol(psRelocation->uiTargetSymbol);
	if ((psSymbol->eBinding == AB65_Local) && (psSymbol->iSection != ASM65_SECTION_UNDEFINED))
	{
		if (psSymbol->iSection >= 0)
		{
			UseSection(masObjects.Get(uiObject)->uiFirstSection + psSymbol->iSection, paiWork);
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
			UseSection(masObjects.Get(psGlobal->iObject)->uiFirstSection + psSymbol->iSection, paiWork);
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

	cAssembler.Init();
	cAssembler.Assemble("stp");
	psSegment = cAssembler.GetOutput()->GetSegment(0);
	psSection = mcStubs.GetSection(mcStubs.AddSection("text", "", ASK65_Text));
	psSection->cBytes.Copy(&psSegment->cBytes);
	psSection->uiSize = (uint32)psSection->cBytes.NumElements();
	psSection->uiAlignment = 2;
	cAssembler.Kill();

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
// Places the kept sections kind by kind, each at its alignment.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::Layout(void)
{
	uint64				uiAddress;
	uint32				uiAlignment;
	int					iKind;
	int					iOther;
	size				i;
	size				j;
	SAsm65LinkObject*	psObject;
	SAsm65Section*		psSection;
	SAsm65LinkSection*	psLinkSection;
	CChars				szMessage;

	uiAddress = mauiAddress[ASK65_Text];
	for (iKind = 0; iKind < ASK65_NumKinds; iKind++)
	{
		if (mabFixed[iKind])
		{
			uiAddress = mauiAddress[iKind];
		}
		mauiStart[iKind] = (uint32)uiAddress;
		for (i = 0; i < masObjects.NumElements(); i++)
		{
			psObject = masObjects.Get(i);
			for (j = 0; j < psObject->pcObject->NumSections(); j++)
			{
				psSection = psObject->pcObject->GetSection(j);
				psLinkSection = GetLinkSection(i, j);
				if ((psSection->eKind != iKind) || !IsPlaced(psLinkSection))
				{
					continue;
				}
				uiAlignment = psSection->uiAlignment;
				uiAddress = (uiAddress + uiAlignment - 1) & ~(uint64)(uiAlignment - 1);
				psLinkSection->uiAddress = (uint32)uiAddress;
				uiAddress += psSection->uiSize;
				maiPlacement.Add((int)(psObject->uiFirstSection + j));
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
// value.  False when its section is a dropped COMDAT copy.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::SymbolAddress(size uiObject, SAsm65ObjectSymbol* psSymbol, int64* piValue)
{
	SAsm65LinkSection*	psLinkSection;

	*piValue = psSymbol->iValue;
	if (psSymbol->iSection < 0)
	{
		return true;
	}
	psLinkSection = GetLinkSection(uiObject, psSymbol->iSection);
	*piValue += psLinkSection->uiAddress;
	return IsPlaced(psLinkSection);
}


//////////////////////////////////////////////////////////////////////////
//
// The relocation's target plus its addend.  An undefined weak name is 0.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::Target(size uiObject, SAsm65Relocation* psRelocation, int64* piValue)
{
	CAsm65Object*		pcObject;
	SAsm65ObjectSymbol*	psSymbol;
	SAsm65LinkSymbol*	psGlobal;
	SAsm65LinkSection*	psLinkSection;
	int64				iSymbol;

	pcObject = masObjects.Get(uiObject)->pcObject;
	*piValue = psRelocation->iAddend;
	if (psRelocation->iTargetSection != ASM65_TARGET_SYMBOL)
	{
		psLinkSection = GetLinkSection(uiObject, psRelocation->iTargetSection);
		if (!psLinkSection->bKept)
		{
			Error(pcObject, psRelocation, "Refers to a dropped COMDAT section", pcObject->GetSection(psRelocation->iTargetSection)->szName.Text());
			return false;
		}
		*piValue += psLinkSection->uiAddress;
		return true;
	}

	psSymbol = pcObject->GetSymbol(psRelocation->uiTargetSymbol);
	if ((psSymbol->eBinding == AB65_Local) && (psSymbol->iSection != ASM65_SECTION_UNDEFINED))
	{
		if (!SymbolAddress(uiObject, psSymbol, &iSymbol))
		{
			Error(pcObject, psRelocation, "Refers to a dropped COMDAT section", psSymbol->szName.Text());
			return false;
		}
		*piValue += iSymbol;
		return true;
	}

	psGlobal = GetGlobal(psSymbol->szName.Text());
	if (psGlobal)
	{
		*piValue += psGlobal->iValue;
		return true;
	}
	if (psSymbol->eBinding == AB65_Weak)
	{
		return true;
	}
	Error(pcObject, psRelocation, "Undefined symbol", psSymbol->szName.Text());
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies every kept section, writes its relocated fields, then writes the copies to
// the output in address order.  The objects are not changed.
//
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
			pcBytes->Copy(&psObject->pcObject->GetSection(j)->cBytes);
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
			if (!IsPlaced(psLinkSection))
			{
				continue;
			}
			pcBytes = acBytes.Get(psObject->uiFirstSection + psRelocation->uiSection);
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
}


//////////////////////////////////////////////////////////////////////////
//
// The placed sections, dropped COMDAT copies and every symbol by address.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::WriteMap(void)
{
	CArrayTemplate<SAsm65MapSymbol>	asSymbols;
	SAsm65MapSymbol*				psMapSymbol;
	SAsm65LinkObject*				psObject;
	SAsm65Section*					psSection;
	SAsm65LinkSection*				psLinkSection;
	SAsm65ObjectSymbol*				psSymbol;
	SAsm65LinkSymbol*				psGlobal;
	size							i;
	size							j;
	size							uiObject;
	bool							bDropped;
	bool							bRemoved;
	int64							iValue;

	mszMap.Append("Sections\n");
	for (i = 0; i < maiPlacement.NumElements(); i++)
	{
		for (uiObject = masObjects.NumElements() - 1; masObjects.Get(uiObject)->uiFirstSection > (size)maiPlacement.GetValue(i); uiObject--)
		{
		}
		psObject = masObjects.Get(uiObject);
		psSection = psObject->pcObject->GetSection(maiPlacement.GetValue(i) - psObject->uiFirstSection);
		psLinkSection = masSections.Get(maiPlacement.GetValue(i));
		mszMap.Append("  ");
		AppendAddress(&mszMap, psLinkSection->uiAddress);
		mszMap.Append(" ");
		AppendAddress(&mszMap, psSection->uiSize);
		mszMap.Append(" ");
		mszMap.Append(psSection->szName.Text());
		mszMap.Append(" (");
		mszMap.Append(psObject->pcObject->GetName());
		mszMap.Append(")\n");
	}

	bDropped = false;
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSections(); j++)
		{
			if (GetLinkSection(i, j)->bKept)
			{
				continue;
			}
			if (!bDropped)
			{
				mszMap.Append("Dropped COMDAT sections\n");
				bDropped = true;
			}
			psSection = psObject->pcObject->GetSection(j);
			mszMap.Append("  ");
			mszMap.Append(psSection->szName.Text());
			mszMap.Append(" comdat ");
			mszMap.Append(psSection->szGroup.Text());
			mszMap.Append(" (");
			mszMap.Append(psObject->pcObject->GetName());
			mszMap.Append(")\n");
		}
	}

	bRemoved = false;
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSections(); j++)
		{
			psLinkSection = GetLinkSection(i, j);
			if (!psLinkSection->bKept || psLinkSection->bUsed)
			{
				continue;
			}
			if (!bRemoved)
			{
				mszMap.Append("Removed unused sections\n");
				bRemoved = true;
			}
			psSection = psObject->pcObject->GetSection(j);
			mszMap.Append("  ");
			AppendAddress(&mszMap, psSection->uiSize);
			mszMap.Append(" ");
			mszMap.Append(psSection->szName.Text());
			mszMap.Append(" (");
			mszMap.Append(psObject->pcObject->GetName());
			mszMap.Append(")\n");
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
	return masSections.Get(masObjects.Get(uiObject)->uiFirstSection + uiSection);
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
