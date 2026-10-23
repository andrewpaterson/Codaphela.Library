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
	return strcmp(psLeft->szName, psRight->szName);
}


//////////////////////////////////////////////////////////////////////////
//
// A field of uiBytes holds the value signed or unsigned.
//
//////////////////////////////////////////////////////////////////////////
static bool FitsField(int64 iValue, uint8 uiBytes, bool bSigned)
{
	int64	iLimit;

	if (uiBytes >= 8)
	{
		return true;
	}
	iLimit = (int64)1 << (8 * uiBytes);
	if (bSigned)
	{
		return (iValue >= -(iLimit / 2)) && (iValue < iLimit / 2);
	}
	return (iValue >= 0) && (iValue < iLimit);
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
		pcObject = *mapcObjects.Get(i);
		pcObject->Kill();
		free(pcObject);
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

	pcObject = (CAsm65Object*)malloc(sizeof(CAsm65Object));
	pcObject->Init(szName);
	mapcObjects.Add(&pcObject);
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
	return *mapcObjects.Get(uiIndex);
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
	mapcInputs.Add(&pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
// A library's objects are linked when they define a name that is still needed.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AddLibrary(CAsm65Library* pcLibrary)
{
	mapcLibraries.Add(&pcLibrary);
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
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::Link(void)
{
	size	i;

	for (i = 0; i < mapcInputs.NumElements(); i++)
	{
		Include(*mapcInputs.Get(i));
	}
	PullLibraries();
	FindMissing();
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
	SAsm65LinkReference*	psReference;
	SAsm65LinkReference		sReference;
	uint32*					puiGroup;
	uint32					uiObject;
	uint32					uiIndex;
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
			psReference = mcReferences.Get(psSymbol->szName.Text());
			if (psReference == NULL)
			{
				sReference.bStrong = psSymbol->eBinding != AB65_Weak;
				sReference.uiObject = uiObject;
				mcReferences.Put(psSymbol->szName.Text(), &sReference);
				maszReferenced.Add(psSymbol->szName.Text());
			}
			else if (psSymbol->eBinding != AB65_Weak)
			{
				psReference->bStrong = true;
			}
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
			uiIndex = (uint32)masSymbols.NumElements();
			psGlobal = masSymbols.Add();
			psGlobal->szName.Init(psSymbol->szName.Text());
			psGlobal->iObject = (int32)uiObject;
			psGlobal->uiSymbol = (uint32)i;
			psGlobal->bWeak = psSymbol->eBinding == AB65_Weak;
			psGlobal->iValue = 0;
			mcSymbolIndices.Put(psSymbol->szName.Text(), &uiIndex);
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
			pcLibrary = *mapcLibraries.Get(i);
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
// __end and __<kind>_start / __<kind>_end.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::IsLinkerSymbol(const char* szName)
{
	int			i;
	size		uiLength;
	const char*	szKind;

	if (strcmp(szName, "__end") == 0)
	{
		return true;
	}
	if ((szName[0] != '_') || (szName[1] != '_'))
	{
		return false;
	}
	for (i = 0; i < ASK65_NumKinds; i++)
	{
		szKind = Asm65SectionKindName((EAsm65SectionKind)i);
		uiLength = strlen(szKind);
		if ((strncmp(&szName[2], szKind, uiLength) == 0) && ((strcmp(&szName[2 + uiLength], "_start") == 0) || (strcmp(&szName[2 + uiLength], "_end") == 0)))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Names needed (not only weakly) that nothing defines.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::FindMissing(void)
{
	size					i;
	char*					szName;
	SAsm65LinkReference*	psReference;

	for (i = 0; i < maszReferenced.NumElements(); i++)
	{
		szName = maszReferenced.GetText(i);
		psReference = mcReferences.Get(szName);
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
				if ((psSection->eKind != iKind) || !psLinkSection->bKept)
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
			Asm65AppendHex(&szMessage, muiLimit, 8);
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
	int					iKind;
	int					iEnd;
	uint32				uiEnd;
	uint32				uiIndex;
	CChars				szName;
	SAsm65LinkSymbol*	psGlobal;

	uiEnd = 0;
	for (iKind = 0; iKind < ASK65_NumKinds * 2 + 1; iKind++)
	{
		szName.Init("__");
		if (iKind == ASK65_NumKinds * 2)
		{
			szName.Append("end");
		}
		else
		{
			szName.Append(Asm65SectionKindName((EAsm65SectionKind)(iKind / 2)));
			szName.Append((iKind % 2 == 0) ? "_start" : "_end");
		}

		if (GetGlobal(szName.Text()) == NULL)
		{
			uiIndex = (uint32)masSymbols.NumElements();
			psGlobal = masSymbols.Add();
			psGlobal->szName.Init(szName.Text());
			psGlobal->iObject = -1;
			psGlobal->uiSymbol = 0;
			psGlobal->bWeak = false;
			if (iKind == ASK65_NumKinds * 2)
			{
				for (iEnd = 0; iEnd < ASK65_NumKinds; iEnd++)
				{
					if (mauiEnd[iEnd] > uiEnd)
					{
						uiEnd = mauiEnd[iEnd];
					}
				}
				psGlobal->iValue = uiEnd;
			}
			else
			{
				psGlobal->iValue = (iKind % 2 == 0) ? mauiStart[iKind / 2] : mauiEnd[iKind / 2];
			}
			mcSymbolIndices.Put(szName.Text(), &uiIndex);
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
	SAsm65ObjectSymbol*	psSymbol;

	for (i = 0; i < masSymbols.NumElements(); i++)
	{
		psGlobal = masSymbols.Get(i);
		if (psGlobal->iObject < 0)
		{
			continue;
		}
		psSymbol = masObjects.Get(psGlobal->iObject)->pcObject->GetSymbol(psGlobal->uiSymbol);
		psGlobal->iValue = psSymbol->iValue;
		if (psSymbol->iSection >= 0)
		{
			psGlobal->iValue += GetLinkSection(psGlobal->iObject, psSymbol->iSection)->uiAddress;
		}
	}
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
		*piValue += psSymbol->iValue;
		if (psSymbol->iSection >= 0)
		{
			psLinkSection = GetLinkSection(uiObject, psSymbol->iSection);
			if (!psLinkSection->bKept)
			{
				Error(pcObject, psRelocation, "Refers to a dropped COMDAT section", psSymbol->szName.Text());
				return false;
			}
			*piValue += psLinkSection->uiAddress;
		}
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
			if (!psLinkSection->bKept)
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
			if (!FitsField(iValue, psRelocation->uiBytes, psRelocation->bSigned))
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
		Asm65AppendHex(&mszMap, psLinkSection->uiAddress, 8);
		mszMap.Append(" ");
		Asm65AppendHex(&mszMap, psSection->uiSize, 8);
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

	asSymbols.Init();
	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSymbols(); j++)
		{
			psSymbol = psObject->pcObject->GetSymbol(j);
			if ((psSymbol->iSection == ASM65_SECTION_UNDEFINED) || ((psSymbol->iSection >= 0) && !GetLinkSection(i, psSymbol->iSection)->bKept))
			{
				continue;
			}
			psMapSymbol = asSymbols.Add();
			psMapSymbol->iValue = psSymbol->iValue;
			if (psSymbol->iSection >= 0)
			{
				psMapSymbol->iValue += GetLinkSection(i, psSymbol->iSection)->uiAddress;
			}
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
		Asm65AppendHex(&mszMap, (uint64)psMapSymbol->iValue, 8);
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
// A global's address after Link (or a linker symbol such as __end).
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::GetSymbol(const char* szName, int64* piValue)
{
	SAsm65LinkSymbol*	psGlobal;

	psGlobal = GetGlobal(szName);
	if (psGlobal == NULL)
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
