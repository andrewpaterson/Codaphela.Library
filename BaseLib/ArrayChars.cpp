/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "ArrayChars.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::_Init(void)
{
	mcArray._Init();
	mbFaked = false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Init(void)
{
	mcArray.Init();
	mbFaked = false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Init(const char* szFirst, ...)
{
	va_list		vaMarker;
	char*		sz;

	Init();


	if (szFirst != NULL)
	{
		Add(szFirst);

		va_start(vaMarker, szFirst);
		sz = va_arg(vaMarker, char*);
		while (sz != NULL)
		{
			Add(sz);
			sz = va_arg(vaMarker, char*);
		}
		va_end(vaMarker);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Init(CArrayChars* pasz)
{
	size		i;
	CChars*		psz;
	size		uiChars;

	Init();

	uiChars = pasz->NumElements();
	for (i = 0; i < uiChars; i++)
	{
		psz = pasz->Get(i);
		Add(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Fake(void)
{
	mcArray.Init();
	mbFaked = true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Kill(void)
{
	size		i;
	CChars*	pcChars;
	size		uiElements;

	if (!mbFaked)
	{
		uiElements = mcArray.NumElements();
		for (i = 0; i < uiElements; i++)
		{
			pcChars = mcArray.Get(i);
			pcChars->Kill();
		}
	}
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::ReInit(void)
{
	Kill();
	Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Add(CChars cChars)
{
	CChars*		pcChars2;

	pcChars2 = mcArray.Add();
	if (!mbFaked)
	{
		pcChars2->Init(cChars);
	}
	else
	{
		pcChars2->Fake(cChars.Text());
	}
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Add(CChars* pcChars)
{
	CChars* pcChars2;

	pcChars2 = mcArray.Add();
	if (!mbFaked)
	{
		pcChars2->Init(pcChars);
	}
	else
	{
		pcChars2->Fake(pcChars->Text());
	}
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Add(char* szString)
{
	return Add((const char*)szString);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Add(const char* szString)
{
	CChars* pcChars2;

	pcChars2 = mcArray.Add();
	if (!mbFaked)
	{
		pcChars2->Init(szString);
	}
	else
	{
		pcChars2->Fake(szString);
	}
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::AddLength(size uiLength)
{
	CChars* pcChars2;

	pcChars2 = mcArray.Add();
	if (!mbFaked)
	{
		pcChars2->InitLength(uiLength);
	}
	else
	{
		pcChars2->Fake(NULL);
	}
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Add(char* szString, size iStartInclusive, size iEndExclusive)
{
	CChars*		pcChars2;

	pcChars2 = mcArray.Add();
	if (!mbFaked)
	{
		pcChars2->Init(szString, iStartInclusive, iEndExclusive);
	}
	else
	{
		pcChars2->Fake(szString, iStartInclusive, iEndExclusive);
	}
	return pcChars2;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Add(char* szText, char* szLastCharInclusive)
{
	CChars*		pcChars2;

	pcChars2 = mcArray.Add();
	if (!mbFaked)
	{
		pcChars2->Init(szText, 0, (size)(szLastCharInclusive-szText)+1);
	}
	else
	{
		pcChars2->Fake(szText, 0, (size)(szLastCharInclusive-szText)+1);
	}
	return pcChars2;	
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::AddList(char* sz, ...)
{
	va_list		vaMarker;
	char*		szValue;

	if (sz == NULL)
	{
		return;
	}

	Add(sz);

	va_start(vaMarker, sz);
	szValue = va_arg(vaMarker, char*);
	while (szValue != NULL)
	{
		Add(szValue);
		szValue = va_arg(vaMarker, char*);
	}
	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::AddIfUnique(char* szString)
{
	size		i;
	CChars*		pcExisting;
	size		uiElements;

	if (!mbFaked)
	{
		uiElements = mcArray.NumElements();
		for (i = 0; i < uiElements; i++)
		{
			pcExisting = mcArray.Get(i);
			if (pcExisting->Equals(szString))
			{
				return pcExisting;
			}
		}
		return Add(szString);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::AddIfUnique(CChars cChars)
{
	size		i;
	CChars*		pcExisting;
	size		uiElements;

	if (!mbFaked)
	{	
		uiElements = mcArray.NumElements();
		for (i = 0; i < uiElements; i++)
		{
			pcExisting = mcArray.Get(i);
			if (pcExisting->Equals(cChars))
			{
				return pcExisting;
			}
		}
		return Add(cChars);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::InsertIntoSorted(char* szText, char* szLastCharInclusive)
{
	CChars*		pcChars2;
	CChars		szTemp;
	char		c;
	bool		bResult;
	size		iIndex;

	c = szLastCharInclusive[1];
	szLastCharInclusive[1] = 0;
	szTemp.Fake(szText);

	bResult = mcArray.FindInSorted(&szTemp, CompareChars, &iIndex);
	if (bResult)
	{
		szLastCharInclusive[1] = c;
		return mcArray.Get(iIndex);
	}
	else
	{
		szLastCharInclusive[1] = c;
		pcChars2 = mcArray.InsertAt(iIndex);
		pcChars2->Init(szText, 0, (size)(szLastCharInclusive-szText)+1);
		return pcChars2;	
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::InsertIntoSorted(CChars* psz)
{
	CChars*		pcChars2;
	bool		bResult;
	size		iIndex;

	bResult = mcArray.FindInSorted(psz, CompareChars, &iIndex);
	if (bResult)
	{
		return mcArray.Get(iIndex);
	}
	else
	{
		pcChars2 = mcArray.InsertAt(iIndex);
		pcChars2->Init(psz);
		return pcChars2;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars*	CArrayChars::Add(void)
{
	CChars*		pcChars;

	pcChars = mcArray.Add();
	if (!mbFaked)
	{
		pcChars->Init();
	}
	else
	{
		pcChars->Fake(NULL);
	}
	return pcChars;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Remove(CChars* pcChars)
{
	Remove(pcChars->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Remove(char* szString)
{
	size		i;
	CChars*		pcChars2;
	size		uiElements;

	uiElements = mcArray.NumElements();
	for (i = 0; i < uiElements; i++)
	{
		pcChars2 = mcArray.Get(i);
		if (pcChars2->Equals(szString))
		{
			Remove(i);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Remove(size iIndex)
{
	CChars*		pcChars;

	pcChars = mcArray.Get(iIndex);
	pcChars->Kill();
	mcArray.RemoveAt(iIndex, true);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Get(size iIndex)
{
	return mcArray.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Tail(void)
{
	if (NumElements() != 0)
	{
		return Get(NumElements() - 1);
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CArrayChars::GetText(size iIndex)
{
	CChars*		pcChars;

	pcChars = mcArray.SafeGet(iIndex);
	if (pcChars)
	{
		return pcChars->Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayChars::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayChars::GetIndex(char* szStart)
{
	size		i;
	CChars*		psz;
	size		uiElements;

	uiElements = mcArray.NumElements();
	for (i = 0; i < uiElements; i++)
	{
		psz = mcArray.Get(i);
		if (psz->Equals(szStart))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayChars::GetIndex(CChars* psz)
{
	size		i;
	CChars*		pszOther;
	size		uiElements;

	uiElements = mcArray.NumElements();
	for (i = 0; i < uiElements; i++)
	{
		pszOther = mcArray.Get(i);
		if (pszOther->Equals(psz))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayChars::GetSubStringIndex(char* szStart)
{
	size		i;
	CChars*		psz;
	size		uiElements;

	uiElements = mcArray.NumElements();
	for (i = 0; i < uiElements; i++)
	{
		psz = mcArray.Get(i);
		if (psz->Contains(szStart))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayChars::GetIndex(char* szStart, size iLen)
{
	size		i;
	CChars*		psz;
	size		uiElements;

	uiElements = mcArray.NumElements();
	for (i = 0; i < uiElements; i++)
	{
		psz = mcArray.Get(i);
		if (psz->Equals(szStart, iLen))
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayChars::FindInSorted(char* szString, bool bCaseSensitive)
{
	CChars	szFake;

	szFake.Fake(szString);
	return FindInSorted(&szFake, bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayChars::FindInSorted(CChars* szString, bool bCaseSensitive)
{
	DataCompare		fCompare;
	size			iIndex;
	bool			bResult;

	if (bCaseSensitive)
	{
		fCompare = CompareChars;
	}
	else
	{
		fCompare = CompareCharsIgnoreCase;
	}
	bResult = mcArray.FindInSorted(szString, fCompare, &iIndex);
	if (!bResult)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	else
	{
		return iIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Copy(CArrayChars* pcSource)
{
	size		i;
	CChars*		psz;
	size		uiSourceElements;

	uiSourceElements = pcSource->NumElements();
	for (i = 0; i < uiSourceElements; i++)
	{
		psz = pcSource->Get(i);
		Add(*psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayChars::Equals(CArrayChars* pcOther)
{
	size		i;
	CChars*		pszThis;
	CChars*		pszOther;
	size		uiElements;

	if (mcArray.NumElements() != pcOther->mcArray.NumElements())
	{
		return false;
	}

	uiElements = mcArray.NumElements();
	for (i = 0; i < uiElements; i++)
	{
		pszThis = Get(i);
		pszOther = pcOther->Get(i);
		if (!pszThis->Equals(*pszOther))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayChars::Contains(char* szText)
{
	size	iIndex;

	iIndex = GetIndex(szText);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayChars::Contains(CChars* psz)
{
	size	iIndex;

	iIndex = GetIndex(psz);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayChars::ContainsSubString(char* szText)
{
	size	iIndex;

	iIndex = GetSubStringIndex(szText);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::QuickSort(bool bCaseSensitive)
{
	if (bCaseSensitive)
	{
		mcArray.QuickSort(&CompareChars);
	}
	else
	{
		mcArray.QuickSort(&CompareCharsIgnoreCase);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::BubbleSort(bool bCaseSensitive)
{
	if (bCaseSensitive)
	{
		mcArray.BubbleSort(&CompareChars);
	}
	else
	{
		mcArray.BubbleSort(&CompareCharsIgnoreCase);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Shuffle(CRandom* pcRandom)
{
	mcArray.Shuffle(pcRandom);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Finalise(void)
{
	mcArray.Finalise();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Print(CChars* pszDest)
{
	size		i;
	CChars*		psz;
	size		uiElements;

	uiElements = mcArray.NumElements();
	for (i = 0; i < uiElements; i++)
	{
		psz = mcArray.Get(i);
		pszDest->Append(psz);
		pszDest->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayChars::Split(char* szString, char cSplitter)
{
	CChars	szTemp;

	szTemp.Init(szString);
	if (!szTemp.Empty())
	{
		szTemp.Split(this, cSplitter);
		szTemp.Kill();
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CArrayChars::RemoveTail(void)
{
	if (NumElements() != 0)
	{
		Remove(NumElements() - 1);
		return true;
	}
	else
	{
		return false;
	}
}

