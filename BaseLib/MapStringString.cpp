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
#include "MapStringString.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::Kill(void)
{
	int			i;
	CChars*		psKey;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		psKey = (CChars*)mcArray.GetPtr(i);
		PrivateFreeNode(psKey);
	}

	mcArray.Kill();
	Func = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CMapStringString::PrivateAllocateNode(char* szKey, char* szValue)
{
	CChars*	sz;
	CChars* szData;

	sz = (CChars*)malloc(sizeof(CChars) + sizeof(CChars));
	sz->Init(szKey);

	szData = PrivateGetDataForKey(sz);
	szData->Init(szValue);

	return sz;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::PrivateFreeNode(CChars* psKey)
{
	CChars* szData;

	szData = PrivateGetDataForKey(psKey);
	szData->Kill();
	psKey->Kill();
	free(psKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::Put(CChars* psKey, CChars* psValue)
{
	Put(psKey->Text(), psValue->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::Put(char* psKey, char* psValue)
{
	CChars*		ps;
	CChars*		psData;
	int			iIndex;
	CChars		szFake;

	szFake.Fake(psKey);
	iIndex = GetIndex(&szFake);
	if (iIndex != -1)
	{
		CMapTemplate<CChars, CChars>::GetAtIndex(iIndex, &ps, &psData);
		psData->Kill();
		psData->Init(psValue);
	}
	else
	{
		ps = PrivateAllocateNode(psKey, psValue);
		mcArray.InsertIntoSorted(Func, ps, -1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CChars* CMapStringString::GetWithKey(CChars* psKey)
{
	return CMapStringTemplate<CChars>::GetWithKey(psKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CMapStringString::GetWithKey(char* psKey)
{
	CChars*	pszKey;

	pszKey = CMapStringTemplate<CChars>::GetWithKey(psKey);
	if (pszKey)
	{
		return pszKey->Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringString::GetAtIndex(int iIndex, CChars** ppsKey, CChars** ppsData)
{
	return CMapStringTemplate<CChars>::GetAtIndex(iIndex, ppsKey, ppsData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::Remove(CChars* szKey)
{
	Remove(szKey->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::Remove(char* szKey)
{
	CChars		szFake;
	CChars*		ps;
	int			iIndex;

	szFake.Fake(szKey);
	iIndex = GetIndex(&szFake);
	if (iIndex != -1)
	{
		ps = (CChars*)mcArray.GetPtr(iIndex);
		PrivateFreeNode(ps);
		mcArray.RemoveAt(iIndex, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMapStringString::Dump(void)
{
	int		i;
	CChars*	psKey;
	CChars*	psValue;
	CChars	sz;

	sz.Init(1024);
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		if (GetAtIndex(i, &psKey, &psValue))
		{
			sz.Append(psKey->Text());
			sz.Append(" -> ");
			sz.Append(psValue->Text());
			sz.AppendNewLine();
		}
	}
	sz.Dump();
	sz.Kill();
}

