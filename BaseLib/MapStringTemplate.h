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
#ifndef __MAP_STRING_TEMPLATE_H__
#define __MAP_STRING_TEMPLATE_H__
#include "MapTemplate.h"
#include "Chars.h"
#include "FileReader.h"

template<class D>
class CMapStringTemplate : public CMapTemplate<CChars, D>
{
public:
	void	Init(int iChunkSize, BOOL bCaseSensitive = TRUE);
	void	Kill(void);

	D*		GetWithKey(CChars* psKey);
	D*		GetWithKey(char* szKey);
	D*		GetWithKey(char* psKey, int iLength);
	D*		GetWithKeyAssumeDuplicates(CChars* psKey);
	BOOL	GetWithKeyNextDuplicate(CChars* psLastKey, int iLastIndex, D** ppsData);
	BOOL	GetAtIndex(int iIndex, CChars** ppsKey, D** ppsData);
	D*		GetData(int iIndex);

	D*		Put(CChars* psKey);
	D*		Put(char* szKey);
	void	Put(CChars* psKey, D* psData);
	void	Put(char* szKey, D* psData);
	D*		PutAllowDuplicates(char* szKey);
	D*		PutAllowDuplicates(CChars* psKey);
	void	PutAllowDuplicates(char* szKey, D* pvData);
	void	PutAllowDuplicates(CChars* psKey, D* pvData);

	void	Remove(CChars* szKey);
	void	Remove(char* szKey);

	BOOL	IsCaseSensitive(void);
	void	SetCaseSensitive(BOOL bCaseSensitive);

	D*		LoadNode(CFileReader* pcFile, int iNode);

protected:
	CChars*	AllocateNode(char* szText);
	void	FreeNode(CChars* psKey);
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::Init(int iChunkSize, BOOL bCaseSensitive)
{
	if (bCaseSensitive)
	{
		CMapTemplate<CChars, D>::Init(iChunkSize, CompareChars);
	}
	else
	{
		CMapTemplate<CChars, D>::Init(iChunkSize, CompareCharsIgnoreCase);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::Kill(void)
{
	int			i;
	CChars*		psData;

	for (i = 0; i < this->mcArray.NumElements(); i++)
	{
		psData = (CChars*)this->mcArray.GetPtr(i);
		FreeNode(psData);
	}

	this->mcArray.Kill();
	this->Func = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
CChars* CMapStringTemplate<D>::AllocateNode(char* szText)
{
	CChars*	sz;

	sz = (CChars*)Malloc(sizeof(CChars) + sizeof(D));
	sz->Init(szText);
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::FreeNode(CChars* psKey)
{
	psKey->Kill();
	Free(psKey);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::GetWithKey(char* psKey)
{
	CChars	sz;

	sz.Fake(psKey);
	return CMapTemplate<CChars, D>::GetWithKey(&sz);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::GetWithKey(char* psKey, int iLength)
{
	CChars	sz;
	char	c;
	D*		pD;

	c = psKey[iLength];
	psKey[iLength] = '\0';
	sz.Fake(psKey);
	pD = CMapTemplate<CChars, D>::GetWithKey(&sz);
	psKey[iLength] = c;
	return pD;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::GetWithKey(CChars* psSearch)
{
	return CMapTemplate<CChars, D>::GetWithKey(psSearch);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CMapStringTemplate<D>::GetAtIndex(int iIndex, CChars** ppsKey, D** ppsData)
{
	return CMapTemplate<CChars, D>::GetAtIndex(iIndex, ppsKey, ppsData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::GetData(int iIndex)
{
	CChars*	pcKey;
	D*		ps;

	pcKey = (CChars*)mcArray.GetPtr(iIndex);;
	ps = GetDataForKey(pcKey);
	return ps;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CMapStringTemplate<D>::GetWithKeyNextDuplicate(CChars* psLastKey, int iLastIndex, D** ppsData)
{
	return CMapTemplate<CChars, D>::GetWithKeyNextDuplicate(psLastKey, iLastIndex, ppsData);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::GetWithKeyAssumeDuplicates(CChars* psSearch)
{
	return CMapTemplate<CChars, D>::GetWithKeyAssumeDuplicates(psSearch);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::Put(CChars* psKey)
{
	return Put(psKey->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::Put(char* szKey)
{
	CChars*		ps;
	D*			psData;
	int			iIndex;
	CChars		szFake;

	szFake.Fake(szKey);
	iIndex = this->GetIndex(&szFake);
	if (iIndex != -1)
	{
		return NULL;
	}
	else
	{
		ps = AllocateNode(szKey);
		this->mcArray.InsertIntoSorted(this->Func, ps, -1);
		psData = this->GetDataForKey(ps);
		return psData;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::Put(CChars* psKey, D* psData)
{
	D*	ps;

	ps = Put(psKey);
	if (ps)
	{
		memcpy(ps, psData, sizeof(D));
	}
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::Put(char* psKey, D* psData)
{
	D*	ps;

	ps = Put(psKey);
	if (ps)
	{
		memcpy(ps, psData, sizeof(D));
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::PutAllowDuplicates(CChars* psKey)
{
	return PutAllowDuplicates(psKey->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::PutAllowDuplicates(char* szKey)
{
	CChars*		ps;
	D*			psData;

	ps = AllocateNode(szKey);
	this->mcArray.InsertIntoSorted(this->Func, ps, -1);
	psData = this->GetDataForKey(ps);
	return psData;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::PutAllowDuplicates(char* szKey, D* psData)
{
	D*	ps;

	ps = PutAllowDuplicates(szKey);
	memcpy(ps, psData, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::PutAllowDuplicates(CChars* psKey, D* psData)
{
	D*	ps;

	ps = PutAllowDuplicates(psKey);
	memcpy(ps, psData, sizeof(D));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::Remove(CChars* szKey)
{
	Remove(szKey->Text());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::Remove(char* szKey)
{
	CChars	szFake;
	CChars*	ps;
	int		iIndex;

	szFake.Fake(szKey);
	iIndex = this->GetIndex(&szFake);
	if (iIndex != -1)
	{
		ps = (CChars*)this->mcArray.GetPtr(iIndex);
		FreeNode(ps);
		this->mcArray.RemoveAt(iIndex, 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
BOOL CMapStringTemplate<D>::IsCaseSensitive(void)
{
	return Func == CompareChars;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
void CMapStringTemplate<D>::SetCaseSensitive(BOOL bCaseSensitive)
{
	if (bCaseSensitive)
	{
		Func = CompareChars;
	}
	else
	{
		Func = CompareCharsIgnoreCase;
	}
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class D>
D* CMapStringTemplate<D>::LoadNode(CFileReader* pcFile, int iNode)
{
	CChars			szKey;
	D*				psData;
	STypedPointer*	psType;

	psType = mcArray.CArrayTemplate::Get(iNode);
	if (!pcFile->ReadString(&szKey))
	{
		return NULL;
	}
	psType->iType = -1;
	psType->pvData = AllocateNode(szKey.Text());
	szKey.Kill();
	psData = GetDataForKey((CChars*)psType->pvData);
	return psData;
}


#endif //__MAP_STRING_TEMPLATE_H__

