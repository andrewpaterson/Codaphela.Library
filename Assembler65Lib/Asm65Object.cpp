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
#include <string.h>
#include "Asm65Object.h"


static const char* gaszSectionKinds[ASK65_NumKinds] = { "text", "rodata", "init", "fini", "data", "bss" };


//////////////////////////////////////////////////////////////////////////
//
// The kind is the name up to its first '.' after the start.
//
//////////////////////////////////////////////////////////////////////////
bool Asm65SectionKind(const char* szName, size uiLength, EAsm65SectionKind* peKind)
{
	size	uiKindLength;
	int		i;

	for (uiKindLength = 0; uiKindLength < uiLength; uiKindLength++)
	{
		if (szName[uiKindLength] == '.')
		{
			break;
		}
	}
	for (i = 0; i < ASK65_NumKinds; i++)
	{
		if ((strlen(gaszSectionKinds[i]) == uiKindLength) && (memcmp(gaszSectionKinds[i], szName, uiKindLength) == 0))
		{
			*peKind = (EAsm65SectionKind)i;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* Asm65SectionKindName(EAsm65SectionKind eKind)
{
	return gaszSectionKinds[eKind];
}


//////////////////////////////////////////////////////////////////////////
//
// $ then iDigits upper case hex digits.
//
//////////////////////////////////////////////////////////////////////////
void Asm65AppendHex(CChars* psz, uint64 uiValue, int iDigits)
{
	static const char	acDigits[] = "0123456789ABCDEF";
	int					i;

	psz->Append('$');
	for (i = iDigits - 1; i >= 0; i--)
	{
		psz->Append(acDigits[(uiValue >> (4 * i)) & 0xF]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::Init(const char* szName)
{
	mszName.Init(szName);
	mcSections.Init();
	mcSymbols.Init();
	mcSymbolIndices.Init(true, true);
	mcRelocations.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::Kill(void)
{
	size			i;
	SAsm65Section*	psSection;

	mcRelocations.Kill();
	mcSymbolIndices.Kill();
	for (i = 0; i < mcSymbols.NumElements(); i++)
	{
		mcSymbols.Get(i)->szName.Kill();
	}
	mcSymbols.Kill();
	for (i = 0; i < mcSections.NumElements(); i++)
	{
		psSection = mcSections.Get(i);
		psSection->cBytes.Kill();
		psSection->szGroup.Kill();
		psSection->szName.Kill();
	}
	mcSections.Kill();
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAsm65Object::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Object::AddSection(const char* szName, const char* szGroup, EAsm65SectionKind eKind)
{
	SAsm65Section*	psSection;

	psSection = mcSections.Add();
	psSection->szName.Init(szName);
	psSection->szGroup.Init(szGroup);
	psSection->eKind = eKind;
	psSection->uiAlignment = 1;
	psSection->uiSize = 0;
	psSection->cBytes.Init();
	return (uint32)(mcSections.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumSections(void)
{
	return mcSections.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Section* CAsm65Object::GetSection(size uiIndex)
{
	return mcSections.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CAsm65Object::FindSection(const char* szName, const char* szGroup)
{
	size			i;
	SAsm65Section*	psSection;

	for (i = 0; i < mcSections.NumElements(); i++)
	{
		psSection = mcSections.Get(i);
		if (psSection->szName.Equals(szName) && psSection->szGroup.Equals(szGroup))
		{
			return (int32)i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Finds the symbol or adds it as undefined and local.
//
//////////////////////////////////////////////////////////////////////////
uint32 CAsm65Object::PutSymbol(const char* szName)
{
	uint32*				puiIndex;
	SAsm65ObjectSymbol*	psSymbol;
	uint32				uiIndex;

	puiIndex = mcSymbolIndices.Get(szName);
	if (puiIndex)
	{
		return *puiIndex;
	}

	uiIndex = (uint32)mcSymbols.NumElements();
	psSymbol = mcSymbols.Add();
	psSymbol->szName.Init(szName);
	psSymbol->iSection = ASM65_SECTION_UNDEFINED;
	psSymbol->iValue = 0;
	psSymbol->eBinding = AB65_Local;
	mcSymbolIndices.Put(szName, &uiIndex);
	return uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumSymbols(void)
{
	return mcSymbols.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65ObjectSymbol* CAsm65Object::GetSymbol(size uiIndex)
{
	return mcSymbols.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CAsm65Object::FindSymbol(const char* szName)
{
	uint32*	puiIndex;

	puiIndex = mcSymbolIndices.Get(szName);
	if (puiIndex)
	{
		return (int32)*puiIndex;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Relocation* CAsm65Object::AddRelocation(void)
{
	SAsm65Relocation*	psRelocation;

	psRelocation = mcRelocations.Add();
	memset(psRelocation, 0, sizeof(SAsm65Relocation));
	return psRelocation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CAsm65Object::NumRelocations(void)
{
	return mcRelocations.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SAsm65Relocation* CAsm65Object::GetRelocation(size uiIndex)
{
	return mcRelocations.Get(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// A readable listing of the sections, symbols and relocations (for tests).
//
//////////////////////////////////////////////////////////////////////////
void CAsm65Object::Dump(CChars* psz)
{
	size				i;
	size				j;
	SAsm65Section*		psSection;
	SAsm65ObjectSymbol*	psSymbol;
	SAsm65Relocation*	psRelocation;
	static const char*	aszBindings[3] = { "local", "global", "weak" };

	for (i = 0; i < mcSections.NumElements(); i++)
	{
		psSection = mcSections.Get(i);
		psz->Append("section ");
		psz->Append((uint32)i);
		psz->Append(" ");
		psz->Append(psSection->szName.Text());
		if (!psSection->szGroup.Empty())
		{
			psz->Append(" comdat ");
			psz->Append(psSection->szGroup.Text());
		}
		psz->Append(" align ");
		psz->Append(psSection->uiAlignment);
		psz->Append(" size ");
		psz->Append(psSection->uiSize);
		for (j = 0; j < psSection->cBytes.NumElements(); j++)
		{
			psz->Append((j % 16 == 0) ? "\n  " : " ");
			Asm65AppendHex(psz, psSection->cBytes.GetValue(j), 2);
		}
		psz->AppendNewLine();
	}

	for (i = 0; i < mcSymbols.NumElements(); i++)
	{
		psSymbol = mcSymbols.Get(i);
		psz->Append("symbol ");
		psz->Append((uint32)i);
		psz->Append(" ");
		psz->Append(psSymbol->szName.Text());
		psz->Append(" ");
		psz->Append(aszBindings[psSymbol->eBinding]);
		if (psSymbol->iSection == ASM65_SECTION_UNDEFINED)
		{
			psz->Append(" undefined");
		}
		else
		{
			if (psSymbol->iSection == ASM65_SECTION_ABSOLUTE)
			{
				psz->Append(" absolute ");
			}
			else
			{
				psz->Append(" section ");
				psz->Append(psSymbol->iSection);
				psz->Append(" + ");
			}
			psz->Append(psSymbol->iValue);
		}
		psz->AppendNewLine();
	}

	for (i = 0; i < mcRelocations.NumElements(); i++)
	{
		psRelocation = mcRelocations.Get(i);
		psz->Append("relocation section ");
		psz->Append(psRelocation->uiSection);
		psz->Append(" + ");
		psz->Append(psRelocation->uiOffset);
		psz->Append(psRelocation->bRelative ? " relative " : " absolute ");
		psz->Append(psRelocation->bSigned ? "signed " : "");
		psz->Append((uint32)psRelocation->uiBytes);
		psz->Append(" bytes = ");
		if (psRelocation->iTargetSection == ASM65_TARGET_SYMBOL)
		{
			psz->Append(mcSymbols.Get(psRelocation->uiTargetSymbol)->szName.Text());
		}
		else
		{
			psz->Append("section ");
			psz->Append(psRelocation->iTargetSection);
		}
		if (psRelocation->iAddend >= 0)
		{
			psz->Append(" + ");
			psz->Append(psRelocation->iAddend);
		}
		else
		{
			psz->Append(" - ");
			psz->Append(-psRelocation->iAddend);
		}
		psz->AppendNewLine();
	}
}
