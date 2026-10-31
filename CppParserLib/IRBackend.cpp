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
#include "IRBackend.h"


//////////////////////////////////////////////////////////////////////////
//
// piLabels is the counter labels are numbered from (the generator's).
// Function sections have no options until a back end sets them.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Init(CIRModule* pcModule, int* piLabels)
{
	mpcModule = pcModule;
	mpiLabels = piLabels;
	mpcFunction = NULL;
	mpszOut = NULL;
	muiSource = 0;
	mbSourceMarkers = false;
	mcSourceMarkers.Init();
	mpcErrors = NULL;
	mbResult = true;
	maiBlockLabels.Init();
	miFrameBytes = 0;
	mszSectionOptions = "";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Kill(void)
{
	maiBlockLabels.Kill();
	mcSourceMarkers.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Every function (each a text section of its own, its name followed by
// mszSectionOptions) and global (a data section) of the module, for the
// target's assembler (AssembleObject).
//
//////////////////////////////////////////////////////////////////////////
bool CIRBackend::GenerateModule(CChars* pszDest, CCPPErrors* pcErrors)
{
	size			ui;
	CIRFunction*	pcFunction;
	char*			szName;
	bool			bResult;

	bResult = true;
	for (ui = 0; ui < mpcModule->NumFunctions(); ui++)
	{
		pcFunction = mpcModule->GetFunction(ui);
		szName = mpcModule->GetSymbolName(pcFunction->muiSymbol);
		pszDest->Append("\t\t.section text.");
		pszDest->Append(szName);
		pszDest->Append(mszSectionOptions);
		pszDest->AppendNewLine();
		Binding(szName, pcFunction->muiAttributes, pszDest);
		bResult &= GenerateFunction(pcFunction, szName, pszDest, pcErrors);
	}
	for (ui = 0; ui < mpcModule->NumGlobals(); ui++)
	{
		GenerateGlobal(mpcModule->GetGlobal(ui), pszDest);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// .global or .weak, or nothing for the unit's own label.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Binding(const char* szName, uint32 uiAttributes, CChars* pszDest)
{
	if (uiAttributes & IRA_Local)
	{
		return;
	}
	pszDest->Append((uiAttributes & IRA_Weak) ? "\t\t.weak " : "\t\t.global ");
	pszDest->Append(szName);
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A global's data section, alignment, binding, label and items.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::GenerateGlobal(CIRGlobal* pcGlobal, CChars* pszDest)
{
	size			ui;
	size			uiByte;
	SIRDataItem*	psItem;
	char*			szName;
	int				iBytes;

	szName = mpcModule->GetSymbolName(pcGlobal->muiSymbol);
	pszDest->Append("\t\t.section data.");
	pszDest->Append(szName);
	pszDest->AppendNewLine();
	if (pcGlobal->muiAlign > 1)
	{
		pszDest->Append("\t\t.align ");
		pszDest->Append(pcGlobal->muiAlign);
		pszDest->AppendNewLine();
	}
	Binding(szName, pcGlobal->muiAttributes, pszDest);
	pszDest->Append(szName);
	pszDest->Append(':');
	pszDest->AppendNewLine();

	for (ui = 0; ui < pcGlobal->masItems.NumElements(); ui++)
	{
		psItem = pcGlobal->masItems.Get(ui);
		switch (psItem->uiKind)
		{
		case IRDK_Value:
		case IRDK_Symbol:
			iBytes = mpcModule->GetSize(psItem->sType);
			pszDest->Append((iBytes == 1) ? "\t\t.byte " : (iBytes == 2) ? "\t\t.word " : (iBytes == 4) ? "\t\t.long " : "\t\t.quad ");
			if (psItem->uiKind == IRDK_Symbol)
			{
				pszDest->Append(mpcModule->GetSymbolName((uint32)psItem->ulliValue));
				if (psItem->lliOffset != 0)
				{
					pszDest->Append(psItem->lliOffset > 0 ? " + " : " - ");
					pszDest->Append(psItem->lliOffset > 0 ? psItem->lliOffset : -psItem->lliOffset);
				}
			}
			else
			{
				pszDest->Append(IRSignExtend(psItem->sType, psItem->ulliValue));
			}
			pszDest->AppendNewLine();
			break;
		case IRDK_Bytes:
			for (uiByte = 0; uiByte < psItem->uiLength; uiByte++)
			{
				pszDest->Append(((uiByte % 16) == 0) ? "\t\t.byte " : ", ");
				pszDest->Append((int32)*pcGlobal->macBytes.Get(psItem->uiStart + uiByte));
				if (((uiByte % 16) == 15) || (uiByte + 1 == psItem->uiLength))
				{
					pszDest->AppendNewLine();
				}
			}
			break;
		case IRDK_Zero:
			if (psItem->ulliValue > 0)
			{
				pszDest->Append("\t\t.space ");
				pszDest->Append(psItem->ulliValue);
				pszDest->AppendNewLine();
			}
			break;
		}
	}
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// An error in the function being written ("@name: message"); the
// function's result is false.  Returns false.
//
//////////////////////////////////////////////////////////////////////////
bool CIRBackend::Error(const char* szMessage, const char* szDetail)
{
	CChars	szError;

	szError.Init("@");
	szError.Append(mpcModule->GetSymbolName(mpcFunction->muiSymbol));
	szError.Append(": ");
	szError.Append(szMessage);
	if (mpcErrors)
	{
		mpcErrors->Add(NULL, -1, -1, -1, szError.Text(), szDetail);
	}
	szError.Kill();
	mbResult = false;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The marker line (IR_SOURCE_MARKER) before the code of an instruction or
// a jump with uiSource (0 for none) when it isn't the last one's.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Source(uint32 uiSource)
{
	if (!mbSourceMarkers || (uiSource == 0) || (uiSource == muiSource))
	{
		return;
	}
	muiSource = uiSource;
	mpszOut->Append(IR_SOURCE_MARKER);
	mpszOut->Append((int64)muiSource);
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether Source writes markers (for a listing): they are taken out while
// the text passes run, so the code is the same as without them.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::SetSourceMarkers(bool bSourceMarkers)
{
	mbSourceMarkers = bSourceMarkers;
}


//////////////////////////////////////////////////////////////////////////
//
// The next label number from the shared counter.
//
//////////////////////////////////////////////////////////////////////////
int32 CIRBackend::NewLabel(void)
{
	(*mpiLabels)++;
	return *mpiLabels;
}


//////////////////////////////////////////////////////////////////////////
//
// A label line, "L" and its number.
//
//////////////////////////////////////////////////////////////////////////
void CIRBackend::Label(int32 iLabel)
{
	mpszOut->Append('L');
	mpszOut->Append(iLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
}

