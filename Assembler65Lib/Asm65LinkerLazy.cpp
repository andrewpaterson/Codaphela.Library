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
#include "Asm65Assembler.h"
#include "Asm65Linker.h"


//The loader each trampoline calls the first time.  It sets A, B, X and Y to 32 bits, as a function is called (so
//code called with wider registers, like the runtime routines, must be resident), and keeps their values.  It
//reads the unit's sectors after the code loaded so far, applies its relocation records, points the trampoline's
//slot at the target and returns to it, so the target sees its caller's return address.  Its frame from V: Y, X,
//B, A, the caller's V, then the return address into the trampoline: the trampoline's unit entry, offset and
//slot.  A unit's directory entry is its sector, its number of sectors, the bytes they fill, its code bytes, its
//records and its address (0 until it is loaded).  A failed load stops with A -1 and R_LazyError 1 (no room) or
//2 (the drive failed).
static const char* gszLazyLoader =
	"	.section text.R_LazyLoad, keep\n"
	"R_LazyLoad:\n"
	"	wtth (A, B), 32\n"
	"	wtth (X, Y), 32\n"
	"	psh V\n"
	"	psh A\n"
	"	psh B\n"
	"	psh X\n"
	"	psh Y\n"
	"	cpy V, S\n"
	"	cpy X, [21 + V]\n"
	"	cpy Y, [X]\n"
	"	cpy A, [20 + Y]\n"
	"	cmp A, 0\n"
	"	brac z, R_LazyTarget\n"
	"	cpy A, [R_LazyFree]\n"
	"	add A, 3\n"
	"	and A, -4\n"
	"	cpy B, A\n"
	"	add B, [8 + Y]\n"
	"	cmp B, R_LazyEnd\n"
	"	brac c, R_LazyFits\n"
	"	bras z, R_LazyFits\n"
	"	cpy B, 1\n"
	"	bra R_LazyFail\n"
	"R_LazyFits:\n"
	"	cpy B, [Y]\n"
	"	cpy [$FF0000], B\n"
	"	cpy [$FF0004], A\n"
	"	cpy B, [4 + Y]\n"
	"	cpy [$FF0008], B\n"
	"	cpy B, 1\n"
	"	cpy [$FF000C], B\n"
	"	cpy B, [$FF0010]\n"
	"	cmp B, 0\n"
	"	bras z, R_LazyRead\n"
	"	cpy B, 2\n"
	"	bra R_LazyFail\n"
	"R_LazyRead:\n"
	"	psh X\n"
	"	psh Y\n"
	"	cpy X, A\n"
	"	add X, [12 + Y]\n"
	"	cpy B, [16 + Y]\n"
	"	cpy [R_LazyCount], B\n"
	"R_LazyRelocate:\n"
	"	cpy B, [R_LazyCount]\n"
	"	cmp B, 0\n"
	"	bras z, R_LazyRelocated\n"
	"	sub B, 1\n"
	"	cpy [R_LazyCount], B\n"
	"	cpy Y, [X]\n"
	"	cmp Y, 0\n"
	"	bras n, R_LazyRelative\n"
	"	add Y, A\n"
	"	add [Y], A\n"
	"	add X, 8\n"
	"	bra R_LazyRelocate\n"
	"R_LazyRelative:\n"
	"	and Y, $7FFFFFFF\n"
	"	add Y, A\n"
	"	cpy B, [4 + X]\n"
	"	sub B, Y\n"
	"	cpy [Y], B\n"
	"	add X, 8\n"
	"	bra R_LazyRelocate\n"
	"R_LazyRelocated:\n"
	"	pul Y\n"
	"	pul X\n"
	"	cpy [20 + Y], A\n"
	"	cpy B, A\n"
	"	add B, [12 + Y]\n"
	"	cpy [R_LazyFree], B\n"
	"R_LazyTarget:\n"
	"	add A, [4 + X]\n"
	"	cpy Y, [8 + X]\n"
	"	cpy [Y], A\n"
	"	cpy [21 + V], A\n"
	"	pul Y\n"
	"	pul X\n"
	"	pul B\n"
	"	pul A\n"
	"	pul V\n"
	"	ret\n"
	"R_LazyFail:\n"
	"	cpy [R_LazyError], B\n"
	"	cpy A, -1\n"
	"	stp\n"
	"	.section data.R_Lazy, keep\n"
	"	.align 4\n"
	"	.global R_LazyError\n"
	"R_LazyFree:\n"
	"	.long __end\n"
	"R_LazyCount:\n"
	"	.long 0\n"
	"R_LazyError:\n"
	"	.long 0\n";


//////////////////////////////////////////////////////////////////////////
//
// Loads code when it is first called: every text section of the objects that aren't
// resident (AddResidentObject) is a unit on a drive with uiSectorSize byte sectors,
// from uiFirstSector (GetLazyImage is what to write there).  Units are loaded after the
// core, up to uiCodeEnd, and stay loaded.  The core refers to a unit through
// trampolines, so code, function pointers, vtables and init tables all work before a
// unit is loaded.  The simulator's drive is CAsm65Memory::SetDrive.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::SetLazy(uint32 uiCodeEnd, uint32 uiSectorSize, uint32 uiFirstSector)
{
	mbLazy = true;
	muiLazyEnd = uiCodeEnd;
	muiSectorSize = uiSectorSize;
	muiFirstSector = uiFirstSector;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Linker::NumUnits(void)
{
	return masUnits.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The units' sectors, to write to the drive from the first sector given to SetLazy.
//
//////////////////////////////////////////////////////////////////////////
CArrayUint8* CAsm65Linker::GetLazyImage(void)
{
	return &mcLazyImage;
}


//////////////////////////////////////////////////////////////////////////
//
// The resident objects and the linker's own are never loaded lazily.
//
//////////////////////////////////////////////////////////////////////////
bool CAsm65Linker::IsResident(CAsm65Object* pcObject)
{
	size	i;

	if ((pcObject == &mcStubs) || (pcObject == &mcLazy))
	{
		return true;
	}
	for (i = 0; i < mapcResidents.NumElements(); i++)
	{
		if (mapcResidents.GetPtr(i) == pcObject)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the units (each placed text section that isn't resident or in a resident
// object), counts their relocation records, makes a trampoline for every place the
// core or another unit refers to and for every global in a unit, gives the units
// their sectors and adds the loader.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::PlanLazy(void)
{
	SAsm65LinkObject*		psObject;
	SAsm65Section*			psSection;
	SAsm65LinkSection*		psLinkSection;
	SAsm65LinkUnit*			psUnit;
	SAsm65ObjectSymbol*		psSymbol;
	SAsm65Relocation*		psRelocation;
	int64					iValue;
	uint32					uiSector;
	uint32					uiBytes;
	size					i;
	size					j;

	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		if (IsResident(psObject->pcObject))
		{
			continue;
		}
		for (j = 0; j < psObject->pcObject->NumSections(); j++)
		{
			psSection = psObject->pcObject->GetSection(j);
			psLinkSection = GetLinkSection(i, j);
			if ((psSection->eKind == ASK65_Text) && !psSection->bResident && IsPlaced(psLinkSection))
			{
				psLinkSection->iUnit = (int32)masUnits.NumElements();
				psUnit = masUnits.Add();
				psUnit->uiSection = SectionIndex(i, j);
				psUnit->uiSector = 0;
				psUnit->uiSectors = 0;
				psUnit->uiRecords = 0;
			}
		}
	}

	for (i = 0; i < masObjects.NumElements(); i++)
	{
		psObject = masObjects.Get(i);
		for (j = 0; j < psObject->pcObject->NumSymbols(); j++)
		{
			psSymbol = psObject->pcObject->GetSymbol(j);
			if ((psSymbol->eBinding != AB65_Local) && (psSymbol->iSection >= 0))
			{
				psLinkSection = GetLinkSection(i, psSymbol->iSection);
				if (psLinkSection->iUnit >= 0)
				{
					PutTrampoline(psLinkSection->iUnit, (uint32)psSymbol->iValue);
				}
			}
		}
		for (j = 0; j < psObject->pcObject->NumRelocations(); j++)
		{
			psRelocation = psObject->pcObject->GetRelocation(j);
			psLinkSection = GetLinkSection(i, psRelocation->uiSection);
			if (!IsPlaced(psLinkSection))
			{
				continue;
			}
			switch (LazyField(i, psRelocation, psLinkSection->iUnit, true, &iValue))
			{
			case ALF65_Base:
			case ALF65_Relative:
				masUnits.Get(psLinkSection->iUnit)->uiRecords++;
				break;
			default:
				break;
			}
		}
	}

	uiSector = muiFirstSector;
	for (i = 0; i < masUnits.NumElements(); i++)
	{
		psUnit = masUnits.Get(i);
		uiBytes = UnitSection(psUnit)->uiSize + psUnit->uiRecords * ASM65_LAZY_RECORD_BYTES;
		psUnit->uiSector = uiSector;
		psUnit->uiSectors = (uiBytes + muiSectorSize - 1) / muiSectorSize;
		uiSector += psUnit->uiSectors;
	}

	AssembleLazyObject();
}


//////////////////////////////////////////////////////////////////////////
//
// How a field in iUnit (-1 for the core) that psRelocation writes is done.  A place
// in the same unit reached through its section or a local label stays in the unit:
// relative, it is worked out now; absolute, it has the unit's address added when the
// unit is loaded.  Anything else is a value in the core (a unit's place is its
// trampoline, made now if bPlan): written now, unless the field is relative and in a
// unit, when it is worked out when the unit is loaded.  Fields worked out when a unit
// is loaded must be 32 bits.
//
//////////////////////////////////////////////////////////////////////////
EAsm65LazyField CAsm65Linker::LazyField(size uiObject, SAsm65Relocation* psRelocation, int32 iUnit, bool bPlan, int64* piValue)
{
	CAsm65Object*		pcObject;
	SAsm65LinkSection*	psTarget;
	int64				iSection;
	int32				iTrampoline;
	bool				bInside;

	pcObject = masObjects.Get(uiObject)->pcObject;
	if (!TargetPlace(uiObject, psRelocation, &iSection, piValue))
	{
		return ALF65_Error;
	}

	psTarget = (iSection >= 0) ? masSections.Get((size)iSection) : NULL;
	bInside = (psRelocation->iTargetSection != ASM65_TARGET_SYMBOL) || (pcObject->GetSymbol(psRelocation->uiTargetSymbol)->eBinding == AB65_Local);
	if (psTarget && (iUnit >= 0) && (psTarget->iUnit == iUnit) && bInside)
	{
		if (psRelocation->bRelative)
		{
			*piValue -= psRelocation->uiOffset;
			return ALF65_Value;
		}
		if (psRelocation->uiBytes != 4)
		{
			Error(pcObject, psRelocation, "A relocation loaded with a lazy unit must be 32 bits", NULL);
			return ALF65_Error;
		}
		return ALF65_Base;
	}

	if (psTarget && (psTarget->iUnit >= 0))
	{
		if (bPlan)
		{
			PutTrampoline(psTarget->iUnit, (uint32)*piValue);
		}
		else
		{
			iTrampoline = FindTrampoline(psTarget->iUnit, (uint32)*piValue);
			*piValue = TrampolineAddress(iTrampoline);
		}
	}
	else if (psTarget)
	{
		*piValue += psTarget->uiAddress;
	}

	if ((iUnit >= 0) && psRelocation->bRelative)
	{
		if (psRelocation->uiBytes != 4)
		{
			Error(pcObject, psRelocation, "A relocation loaded with a lazy unit must be 32 bits", NULL);
			return ALF65_Error;
		}
		return ALF65_Relative;
	}
	return ALF65_Value;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CAsm65Linker::FindTrampoline(int32 iUnit, uint32 uiOffset)
{
	size					i;
	SAsm65LinkTrampoline*	psTrampoline;

	for (i = 0; i < masTrampolines.NumElements(); i++)
	{
		psTrampoline = masTrampolines.Get(i);
		if ((psTrampoline->iUnit == iUnit) && (psTrampoline->uiOffset == uiOffset))
		{
			return (int32)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CAsm65Linker::PutTrampoline(int32 iUnit, uint32 uiOffset)
{
	int32					iTrampoline;
	SAsm65LinkTrampoline*	psTrampoline;

	iTrampoline = FindTrampoline(iUnit, uiOffset);
	if (iTrampoline < 0)
	{
		iTrampoline = (int32)masTrampolines.NumElements();
		psTrampoline = masTrampolines.Add();
		psTrampoline->iUnit = iUnit;
		psTrampoline->uiOffset = uiOffset;
	}
	return iTrampoline;
}


//////////////////////////////////////////////////////////////////////////
//
// R_LazyT<n> in the loader's object.
//
//////////////////////////////////////////////////////////////////////////
int64 CAsm65Linker::TrampolineAddress(int32 iTrampoline)
{
	CChars	szName;
	int32	iSymbol;
	int64	iValue;

	szName.Init("R_LazyT");
	szName.Append(iTrampoline);
	iSymbol = mcLazy.FindSymbol(szName.Text());
	szName.Kill();
	iValue = 0;
	if (iSymbol >= 0)
	{
		SymbolAddress(muiLazyObject, mcLazy.GetSymbol(iSymbol), &iValue);
	}
	return iValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The loader, the trampolines, their slots and the units' directory, assembled and
// included as a resident object.  A trampoline jumps through its slot, which points
// back into the trampoline (at its call to the loader) until the unit is loaded.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::AssembleLazyObject(void)
{
	CAsm65Assembler			cAssembler;
	CChars					szSource;
	SAsm65LinkTrampoline*	psTrampoline;
	SAsm65LinkUnit*			psUnit;
	size					i;

	szSource.Init(gszLazyLoader);
	szSource.Append("R_LazyEnd = ");
	szSource.Append(muiLazyEnd);
	szSource.AppendNewLine();

	szSource.Append("	.section text.R_LazyTrampolines, keep\n");
	for (i = 0; i < masTrampolines.NumElements(); i++)
	{
		psTrampoline = masTrampolines.Get(i);
		szSource.Append("R_LazyT")->Append((uint32)i)->Append(":\n");
		szSource.Append("	jmp [R_LazyS")->Append((uint32)i)->Append("]\n");
		szSource.Append("R_LazyL")->Append((uint32)i)->Append(":\n");
		szSource.Append("	jsr R_LazyLoad\n");
		szSource.Append("	.long R_LazyU")->Append(psTrampoline->iUnit)->Append(", ")->Append(psTrampoline->uiOffset)->Append(", R_LazyS")->Append((uint32)i)->AppendNewLine();
	}

	szSource.Append("	.section data.R_LazySlots, keep\n	.align 4\n");
	for (i = 0; i < masTrampolines.NumElements(); i++)
	{
		szSource.Append("R_LazyS")->Append((uint32)i)->Append(":\n");
		szSource.Append("	.long R_LazyL")->Append((uint32)i)->AppendNewLine();
	}
	for (i = 0; i < masUnits.NumElements(); i++)
	{
		psUnit = masUnits.Get(i);
		szSource.Append("R_LazyU")->Append((uint32)i)->Append(":\n");
		szSource.Append("	.long ")->Append(psUnit->uiSector)->Append(", ")->Append(psUnit->uiSectors)->Append(", ");
		szSource.Append(psUnit->uiSectors * muiSectorSize)->Append(", ");
		szSource.Append(UnitSection(psUnit)->uiSize)->Append(", ");
		szSource.Append(psUnit->uiRecords)->Append(", 0\n");
	}

	cAssembler.Init();
	cAssembler.AssembleObject(szSource.Text(), &mcLazy);
	if (cAssembler.NumErrors() != 0)
	{
		mcErrors.Add("link", 0, 0, "The lazy loader did not assemble", cAssembler.GetErrors());
	}
	cAssembler.Kill();
	szSource.Kill();

	muiLazyObject = masObjects.NumElements();
	Include(&mcLazy);
}


//////////////////////////////////////////////////////////////////////////
//
// Each unit's sectors: its bytes with the fields written that can be, then its
// relocation records.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::BuildUnitImages(void)
{
	CArrayUint8				cBytes;
	CArrayUint8				cRecords;
	SAsm65LinkObject*		psObject;
	SAsm65LinkSection*		psLinkSection;
	SAsm65LinkUnit*			psUnit;
	SAsm65Relocation*		psRelocation;
	int64					iValue;
	uint32					uiField;
	size					i;
	size					j;
	size					uiStart;
	uint8					k;
	CChars					szValue;

	for (i = 0; i < masUnits.NumElements(); i++)
	{
		psUnit = masUnits.Get(i);
		psObject = masObjects.Get(ObjectOfSection(psUnit->uiSection));
		cBytes.Init();
		cBytes.Copy(&UnitSection(psUnit)->cBytes);
		cRecords.Init();

		uiStart = psObject->uiFirstSection;
		for (j = 0; j < psObject->pcObject->NumRelocations(); j++)
		{
			psRelocation = psObject->pcObject->GetRelocation(j);
			if (uiStart + psRelocation->uiSection != psUnit->uiSection)
			{
				continue;
			}
			psLinkSection = masSections.Get(psUnit->uiSection);
			switch (LazyField(ObjectOfSection(psUnit->uiSection), psRelocation, psLinkSection->iUnit, false, &iValue))
			{
			case ALF65_Error:
				continue;
			case ALF65_Base:
				uiField = psRelocation->uiOffset;
				break;
			case ALF65_Relative:
				uiField = psRelocation->uiOffset | ASM65_LAZY_RELATIVE;
				break;
			default:
				if (!Asm65Fits(iValue, psRelocation->uiBytes, psRelocation->bSigned))
				{
					szValue.Init();
					szValue.Append(iValue);
					Error(psObject->pcObject, psRelocation, "Relocated value does not fit", szValue.Text());
					szValue.Kill();
					continue;
				}
				for (k = 0; k < psRelocation->uiBytes; k++)
				{
					cBytes.SetValue(psRelocation->uiOffset + k, (uint8)((uint64)iValue >> (8 * k)));
				}
				continue;
			}

			for (k = 0; k < 4; k++)
			{
				cRecords.Add((uint8)(uiField >> (8 * k)));
			}
			for (k = 0; k < 4; k++)
			{
				cRecords.Add((uint8)((uint64)iValue >> (8 * k)));
				if (psRelocation->bRelative)
				{
					cBytes.SetValue(psRelocation->uiOffset + k, 0);
				}
				else
				{
					cBytes.SetValue(psRelocation->uiOffset + k, (uint8)((uint64)iValue >> (8 * k)));
				}
			}
		}

		PadLazyImage(psUnit->uiSector);
		for (j = 0; j < cBytes.NumElements(); j++)
		{
			mcLazyImage.Add(cBytes.GetValue(j));
		}
		for (j = 0; j < cRecords.NumElements(); j++)
		{
			mcLazyImage.Add(cRecords.GetValue(j));
		}
		cRecords.Kill();
		cBytes.Kill();
	}
	if (masUnits.NumElements() > 0)
	{
		PadLazyImage(masUnits.Tail()->uiSector + masUnits.Tail()->uiSectors);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Zeros up to uiSector.
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Linker::PadLazyImage(uint32 uiSector)
{
	while (mcLazyImage.NumElements() < (size)(uiSector - muiFirstSector) * muiSectorSize)
	{
		mcLazyImage.Add(0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The object section a unit is.
//
//////////////////////////////////////////////////////////////////////////
SAsm65Section* CAsm65Linker::UnitSection(SAsm65LinkUnit* psUnit)
{
	size	uiObject;

	uiObject = ObjectOfSection(psUnit->uiSection);
	return masObjects.Get(uiObject)->pcObject->GetSection(psUnit->uiSection - masObjects.Get(uiObject)->uiFirstSection);
}
