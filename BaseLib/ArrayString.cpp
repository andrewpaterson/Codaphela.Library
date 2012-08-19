/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "ArrayString.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayString::Init(int iChunkSize, BOOL bFake)
{
	mcArray.Init(iChunkSize);
	mbFaked = bFake;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayString::Kill(void)
{
	int		i;
	CChars*	pcChars;

	if (!mbFaked)
	{
		for (i = 0; i < mcArray.NumElements(); i++)
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
void CArrayString::ReInit(void)
{
	int iChunkSize;

	iChunkSize = mcArray.ChunkSize();
	Kill();
	Init(iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayString::Add(CChars cChars)
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
CChars* CArrayString::Add(char* szString)
{
	CChars*		pcChars2;

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
CChars* CArrayString::Add(char* szString, int iStartInclusive, int iEndExclusive)
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
CChars* CArrayString::Add(char* szText, char* szLastCharInclusive)
{
	CChars*		pcChars2;

	pcChars2 = mcArray.Add();
	if (!mbFaked)
	{
		pcChars2->Init(szText, 0, (int)(szLastCharInclusive-szText)+1);
	}
	else
	{
		pcChars2->Fake(szText, 0, (int)(szLastCharInclusive-szText)+1);
	}
	return pcChars2;	
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayString::AddIfUnique(char* szString)
{
	int			i;
	CChars*		pcExisting;

	if (!mbFaked)
	{
		for (i = 0; i < mcArray.NumElements(); i++)
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
CChars* CArrayString::AddIfUnique(CChars cChars)
{
	int			i;
	CChars*		pcExisting;

	if (!mbFaked)
	{	for (i = 0; i < mcArray.NumElements(); i++)
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
CChars* CArrayString::InsertIntoSorted(char* szText, char* szLastCharInclusive)
{
	CChars*		pcChars2;
	CChars		szTemp;
	char		c;
	BOOL		bResult;
	int			iIndex;

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
		pcChars2->Init(szText, 0, (int)(szLastCharInclusive-szText)+1);
		return pcChars2;	
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars*	CArrayString::Add(void)
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
void CArrayString::Remove(CChars* pcChars)
{
	Remove(pcChars->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayString::Remove(char* szString)
{
	int			i;
	CChars*		pcChars2;

	for (i = 0; i < mcArray.NumElements(); i++)
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
void CArrayString::Remove(int iIndex)
{
	CChars*		pcChars;

	pcChars = mcArray.Get(iIndex);
	pcChars->Kill();
	mcArray.RemoveAt(iIndex, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayString::Get(int iIndex)
{
	return mcArray.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayString::Tail(void)
{
	if (NumElements() > 0)
	{
		return Get(NumElements()-1);
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
char* CArrayString::GetText(int iIndex)
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
int CArrayString::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayString::GetIndex(char* szStart)
{
	int			i;
	CChars*		psz;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psz = mcArray.Get(i);
		if (psz->Equals(szStart))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayString::GetIndex(char* szStart, int iLen)
{
	int			i;
	CChars*		psz;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psz = mcArray.Get(i);
		if (psz->Equals(szStart, iLen))
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayString::FindInSorted(char* szString, BOOL bCaseSensitive)
{
	CChars	szFake;

	szFake.Fake(szString);
	return FindInSorted(&szFake, bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayString::FindInSorted(CChars* szString, BOOL bCaseSensitive)
{
	int(*Func)(const void*, const void*);
	int		iIndex;
	BOOL	bResult;

	if (bCaseSensitive)
	{
		Func = CompareChars;
	}
	else
	{
		Func = CompareCharsIgnoreCase;
	}
	bResult = mcArray.FindInSorted(szString, Func, &iIndex);
	if (!bResult)
	{
		return -1;
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
void CArrayString::Copy(CArrayString* pcSource)
{
	int		i;
	CChars*	psz;

	for (i = 0; i < pcSource->NumElements(); i++)
	{
		psz = pcSource->Get(i);
		Add(*psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayString::Equals(CArrayString* pcOther)
{
	int		i;
	CChars*	pszThis;
	CChars*	pszOther;

	if (mcArray.NumElements() != pcOther->mcArray.NumElements())
	{
		return FALSE;
	}

	for (i = 0; i < NumElements(); i++)
	{
		pszThis = Get(i);
		pszOther = pcOther->Get(i);
		if (!pszThis->Equals(*pszOther))
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayString::Contains(char* szText)
{
	int		iIndex;

	iIndex = GetIndex(szText);
	if (iIndex != -1)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayString::QuickSort(BOOL bIgnoreCase)
{
	if (bIgnoreCase)
	{
		mcArray.QuickSort(&CompareCharsIgnoreCase);
	}
	else
	{
		mcArray.QuickSort(&CompareChars);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayString::BubbleSort(BOOL bIgnoreCase)
{
	if (bIgnoreCase)
	{
		mcArray.BubbleSort(&CompareCharsIgnoreCase);
	}
	else
	{
		mcArray.BubbleSort(&CompareChars);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayString::Finalise(void)
{
	mcArray.Finalise();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayString::Dump(void)
{
	int			i;
	CChars*		psz;
	CChars		sz;

	sz.Init(1024);
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psz = mcArray.Get(i);
		sz.Append(psz);
		sz.AppendNewLine();
	}
	sz.Dump();
	sz.Kill();
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayString::Split(char* szString, char cSplitter)
{
	CChars			szTemp;

	szTemp.Init(szString);
	if (!szTemp.Empty())
	{
		szTemp.Split(this, cSplitter);
		szTemp.Kill();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayString::RemoveTail(void)
{
	if (NumElements() > 0)
	{
		Remove(NumElements()-1);
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

