/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Unknowns.h"
#include "MapStringUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::Init(BOOL bKillElements, BOOL bOverwriteExisting, int iChunkSize)
{
	CMapCommon::Init(bKillElements, bOverwriteExisting);
	mcMap.Init(iChunkSize, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::Kill(void)
{
	int				i;
	void*			pv;
	CUnknown**		ppcUnknown;

	if (miFlags & MAP_COMMOM_KILL_ELEMENT)
	{
		for (i = 0; i < mcMap.mcArray.NumElements(); i++)
		{
			pv = mcMap.mcArray.GetPtr(i);
			ppcUnknown = mcMap.PrivateGetDataForKey((CChars*)pv);
			if (ppcUnknown)
			{
				(*ppcUnknown)->Kill();
			}
		}
	}
	
	mcMap.Kill();
	CMapCommon::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringUnknown::Save(CFileWriter* pcFile)
{
	int			i;
	CChars*		psz;
	CUnknown**	ppcUnknown;
	BOOL		bCaseSensitive;

	bCaseSensitive = mcMap.IsCaseSensitive();

	ReturnOnFalse(pcFile->WriteInt(miFlags));
	ReturnOnFalse(pcFile->WriteArrayTemplateHeader(&mcMap.mcArray));
	ReturnOnFalse(pcFile->WriteInt(mcMap.miKeySize));
	ReturnOnFalse(pcFile->WriteBool(bCaseSensitive));

	for (i = 0; i < mcMap.NumElements(); i++)
	{
		mcMap.GetAtIndex(i, &psz, &ppcUnknown);
		ReturnOnFalse(pcFile->WriteString(psz));
		ReturnOnFalse((*ppcUnknown)->SaveHeader(pcFile));
		ReturnOnFalse((*ppcUnknown)->Save(pcFile));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringUnknown::Load(CFileReader* pcFile)
{
	int				i;
	CChars			szKey;
	CUnknown**		ppcUnknown;
	BOOL			bCaseSensitive;
	CUnknown*		pcUnknown;
	STypedPointer*	psType;

	ReturnOnFalse(pcFile->ReadInt(&miFlags));
	ReturnOnFalse(pcFile->ReadArrayTemplateHeader(&mcMap.mcArray));
	ReturnOnFalse(pcFile->ReadInt(&mcMap.miKeySize));
	ReturnOnFalse(pcFile->ReadBool(&bCaseSensitive));
	
	mcMap.mcArray.InitFromHeader();
	mcMap.SetCaseSensitive(bCaseSensitive);

	for (i = 0; i < mcMap.NumElements(); i++)
	{
		psType = mcMap.mcArray.CArrayTemplate::Get(i);
		ReturnOnFalse(pcFile->ReadString(&szKey));
		psType->iType = -1;
		psType->pvData = mcMap.PrivateAllocateNode(szKey.Text());
		szKey.Kill();
		ppcUnknown = mcMap.PrivateGetDataForKey((CChars*)psType->pvData);

		pcUnknown = gcUnknowns.AddFromHeader(pcFile);
		if (!pcUnknown)
		{
			return FALSE;
		}
		ReturnOnFalse(pcUnknown->Load(pcFile));
		*ppcUnknown = pcUnknown;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringUnknown::Put(char* szKey, CUnknown* pcValue)
{
	CUnknown**	ppcNew;
	CUnknown**	ppcExisting;

	if (szKey)
	{
		ppcNew = mcMap.Put(szKey);
		if (ppcNew)
		{
			*ppcNew = pcValue;
			return TRUE;
		}
		else
		{
			if (miFlags & MAP_COMMOM_OVERWRITE)
			{
				ppcExisting = mcMap.GetWithKey(szKey);
				if (miFlags & MAP_COMMOM_KILL_ELEMENT)
				{
					(*ppcExisting)->Kill();
				}
				*ppcExisting = pcValue;
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CMapStringUnknown::Get(char* szKey)
{
	CUnknown**	ppcValue;

	ppcValue = mcMap.GetWithKey(szKey);
	if (ppcValue)
	{
		return *ppcValue;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CMapStringUnknown::NumElements(void)
{
	return mcMap.NumElements();
}

