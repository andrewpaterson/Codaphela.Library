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
#include "ArrayChars.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Init(int iChunkSize, BOOL bFake)
{
	mcArray.Init(iChunkSize);
	mbFaked = bFake;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Kill(void)
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
void CArrayChars::ReInit(void)
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
CChars* CArrayChars::Add(char* szString)
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
CChars* CArrayChars::Add(char* szString, int iStartInclusive, int iEndExclusive)
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
CChars* CArrayChars::AddIfUnique(char* szString)
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
CChars* CArrayChars::AddIfUnique(CChars cChars)
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
CChars* CArrayChars::InsertIntoSorted(char* szText, char* szLastCharInclusive)
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
void CArrayChars::Remove(int iIndex)
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
CChars* CArrayChars::Get(int iIndex)
{
	return mcArray.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CArrayChars::Tail(void)
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
char* CArrayChars::GetText(int iIndex)
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
int CArrayChars::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayChars::GetIndex(char* szStart)
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
int CArrayChars::GetIndex(char* szStart, int iLen)
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
int CArrayChars::FindInSorted(char* szString, BOOL bCaseSensitive)
{
	CChars	szFake;

	szFake.Fake(szString);
	return FindInSorted(&szFake, bCaseSensitive);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CArrayChars::FindInSorted(CChars* szString, BOOL bCaseSensitive)
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
void CArrayChars::Copy(CArrayChars* pcSource)
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
BOOL CArrayChars::Equals(CArrayChars* pcOther)
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
BOOL CArrayChars::Contains(char* szText)
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
void CArrayChars::QuickSort(BOOL bIgnoreCase)
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
void CArrayChars::BubbleSort(BOOL bIgnoreCase)
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
void CArrayChars::Finalise(void)
{
	mcArray.Finalise();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayChars::Dump(void)
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
BOOL CArrayChars::Split(char* szString, char cSplitter)
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
BOOL CArrayChars::RemoveTail(void)
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

