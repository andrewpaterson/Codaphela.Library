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
void CMapStringUnknown::Init(BOOL bKillElements, BOOL bOverwriteExisting)
{
	CMapCommon::Init(bKillElements, bOverwriteExisting);
	mcMap.Init(TRUE, bOverwriteExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::Kill(void)
{
	CUnknown**		ppcUnknown;
	SMapIterator	sIter;
	char*			szKey;
	BOOL			bResult;

	if (miFlags & MAP_COMMOM_KILL_ELEMENT)
	{
		bResult = mcMap.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&ppcUnknown, NULL);
		while (bResult)
		{
			if (ppcUnknown)
			{
				(*ppcUnknown)->Kill();
			}
			bResult = mcMap.Iterate(&sIter, (void**)&szKey, NULL, (void**)&ppcUnknown, NULL);
		}
	}
	
	mcMap.Kill();
	CMapCommon::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringUnknown::Save(CFileWriter* pcFileWriter)
{
	int			i;
	CUnknown**	ppcUnknown;
	int			iNumElements;
	void*		pvData;
	int			iDataSize;

	ReturnOnFalse(pcFileWriter->WriteInt(miFlags));
	ReturnOnFalse(mcMap.WriteCaseSensitivity(pcFileWriter));
	ReturnOnFalse(mcMap.WriteExceptData(pcFileWriter));

	iNumElements = mcMap.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = mcMap.WriteKey(pcFileWriter, i, &iDataSize);
		if (!pvData)
		{
			return FALSE;
		}
		ppcUnknown = (CUnknown**)pvData;
		ReturnOnFalse((*ppcUnknown)->SaveHeader(pcFileWriter));
		ReturnOnFalse((*ppcUnknown)->Save(pcFileWriter));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMapStringUnknown::Load(CFileReader* pcFileReader)
{
	int				i;
	CUnknown**		ppcUnknown;
	CUnknown*		pcUnknown;
	int				iNumElements;
	void*			pvData;
	int				iDataSize;
	CompareFunc		CaseFunc;

	ReturnOnFalse(pcFileReader->ReadInt(&miFlags));
	CaseFunc = mcMap.ReadCaseSensitivity(pcFileReader);
	if (!mcMap.ReadExceptData(pcFileReader, CaseFunc))
	{
		return FALSE;
	}

	iNumElements = mcMap.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = mcMap.ReadKey(pcFileReader, i, &iDataSize);
		if (!pvData)
		{
			return FALSE;
		}
		ppcUnknown = (CUnknown**)pvData;
		pcUnknown = gcUnknowns.AddFromHeader(pcFileReader);
		if (!pcUnknown)
		{
			return FALSE;
		}
		ReturnOnFalse(pcUnknown->Load(pcFileReader));
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
				ppcExisting = mcMap.Get(szKey);
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

	ppcValue = mcMap.Get(szKey);
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

