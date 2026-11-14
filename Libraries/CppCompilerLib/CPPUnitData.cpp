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
#include "CPPUnitData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::Init(void)
{
	mszData.Init();
	maszStrings.Init();
	miStartUps = 0;
	mbDataBss = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::Kill(void)
{
	size	i;

	for (i = 0; i < maszStrings.NumElements(); i++)
	{
		maszStrings.Get(i)->Kill();
	}
	maszStrings.Kill();
	mszData.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// ".section kind.name" and szOptions (", comdat" for code or data every unit that
// uses it writes: inline, template and generated functions, vtables, thunks and
// runtime routines, so the linker keeps one copy).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::AppendSection(CChars* psz, const char* szKind, CChars* pszName, const char* szOptions)
{
	psz->Append("\t\t.section ");
	psz->Append(szKind);
	psz->Append('.');
	psz->Append(pszName);
	if (szOptions)
	{
		psz->Append(szOptions);
	}
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// The label is visible to the other objects.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::AppendGlobal(CChars* psz, CChars* pszLabel)
{
	psz->Append("\t\t.global ");
	psz->Append(pszLabel);
	psz->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A data item's section (data.label, or bss.label if bBss, and szOptions),
// alignment, .global (if bGlobal) and label.  Its data follows (only .space
// in bss).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::BeginData(CChars* pszLabel, int iAlignment, bool bGlobal, const char* szOptions, bool bBss)
{
	AppendSection(&mszData, bBss ? "bss" : "data", pszLabel, szOptions);
	mbDataBss = bBss;
	if (iAlignment > 1)
	{
		mszData.Append("\t\t.align ");
		mszData.Append(iAlignment);
		mszData.AppendNewLine();
	}
	if (bGlobal)
	{
		AppendGlobal(&mszData, pszLabel);
	}
	mszData.Append(pszLabel);
	mszData.Append(':');
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// iBytes of a bss item.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::AppendSpace(int iBytes)
{
	mszData.Append("\t\t.space ");
	mszData.Append(iBytes);
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A value of iBytes.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::AppendData(int iBytes, int64 lliValue)
{
	switch (iBytes)
	{
	case 1:
		mszData.Append("\t\t.byte ");
		mszData.Append((int64)(lliValue & 0xFF));
		break;
	case 2:
		mszData.Append("\t\t.word ");
		mszData.Append((int64)(lliValue & 0xFFFF));
		break;
	case 8:
		mszData.Append("\t\t.quad ");
		mszData.Append(lliValue);
		break;
	case 16:
		mszData.Append("\t\t.quad ");
		mszData.Append(lliValue);
		mszData.AppendNewLine();
		mszData.Append("\t\t.quad 0");
		break;
	default:
		mszData.Append("\t\t.long ");
		mszData.Append((int64)(lliValue & 0xFFFFFFFF));
		break;
	}
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// iBytes of zeros.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::AppendZeros(int iBytes)
{
	int		i;

	for (i = 0; i + 4 <= iBytes; i += 4)
	{
		mszData.Append("\t\t.long 0");
		mszData.AppendNewLine();
	}
	for (; i < iBytes; i++)
	{
		mszData.Append("\t\t.byte 0");
		mszData.AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A start up routine's entry in the init (or fini) table, kept only while the global's data section is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::AppendStartUpEntry(const char* szTable, CChars* pszLabel, CChars* pszData)
{
	CChars	szOptions;

	szOptions.Init(mbDataBss ? ", associated bss." : ", associated data.");
	szOptions.Append(pszData);
	AppendSection(&mszData, szTable, pszLabel, szOptions.Text());

	//Aligned, so an entry never crosses the end of a W65C816 bank (the linker would move it on, leaving a gap the start
	//up would read as an entry).
	mszData.Append("\t\t.align 4\n");
	mszData.Append("\t\t.long ");
	mszData.Append(pszLabel);
	mszData.AppendNewLine();
	szOptions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// String literals are Str and a number, zero terminated.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitData::AppendStrings(CChars* pszDest)
{
	size	i;
	CChars	szLabel;

	for (i = 0; i < maszStrings.NumElements(); i++)
	{
		szLabel.Init("Str");
		szLabel.Append((int)i);
		AppendSection(pszDest, "rodata", &szLabel, NULL);
		szLabel.Kill();
		pszDest->Append("Str");
		pszDest->Append((int)i);
		pszDest->Append(':');
		pszDest->AppendNewLine();
		pszDest->Append("\t\t.byte ");
		pszDest->Append(maszStrings.Get(i));
		pszDest->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiLength bytes and a zero as string data, labelled Str and the number returned.
//
//////////////////////////////////////////////////////////////////////////
int CCPPUnitData::AddString(char* szBytes, size uiLength)
{
	CChars*	pszBytes;
	size	i;

	pszBytes = maszStrings.Add();
	pszBytes->Init();
	for (i = 0; i < uiLength; i++)
	{
		pszBytes->Append((int)(uint8)szBytes[i]);
		pszBytes->Append(", ");
	}
	pszBytes->Append('0');
	return (int)maszStrings.NumElements() - 1;
}

