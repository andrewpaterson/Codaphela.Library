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
#include "BaseLib/DataCompare.h"
#include "Unknowns.h"
#include "MapStringUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknownDataFree::Init(CMapStringUnknown* pcMap)
{
	mpcMap = pcMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknownDataFree::DataWillBeFreed(void* pvData)
{
	CUnknown** ppcExisting;

	ppcExisting = (CUnknown**)pvData;

	mpcMap->DataWillBeFreed(*ppcExisting);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::Init(bool bKillElements, bool bOverwriteExisting)
{
	CMapCommon::Init(bKillElements, bOverwriteExisting);
	mcMap.Init(true, bOverwriteExisting);
	mcDataFree.Init(this);
	mcMap.SetDataFreeCallback(&mcDataFree);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::Kill(void)
{
	//CUnknown**		ppcUnknown;
	//SMapIterator	sIter;
	//char*			szKey;
	//bool			bResult;

	//if (miFlags & MAP_COMMOM_KILL_ELEMENT)
	//{
	//	bResult = mcMap.StartIteration(&sIter, (void**)&szKey, NULL, (void**)&ppcUnknown, NULL);
	//	while (bResult)
	//	{
	//		if (ppcUnknown)
	//		{
	//			(*ppcUnknown)->Kill();
	//		}
	//		bResult = mcMap.Iterate(&sIter, (void**)&szKey, NULL, (void**)&ppcUnknown, NULL);
	//	}
	//}
	
	mcMap.Kill();
	CMapCommon::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::Save(CFileWriter* pcFileWriter)
{
	size		i;
	CUnknown**	ppcUnknown;
	size		iNumElements;
	void*		pvData;
	size		iDataSize;

	ReturnOnFalse(pcFileWriter->WriteShort(miFlags));
	ReturnOnFalse(mcMap.WriteCaseSensitivity(pcFileWriter));
	ReturnOnFalse(mcMap.WriteExceptData(pcFileWriter));

	iNumElements = mcMap.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = mcMap.WriteKey(pcFileWriter, i, &iDataSize);
		if (!pvData)
		{
			return false;
		}
		ppcUnknown = (CUnknown**)pvData;
		ReturnOnFalse((*ppcUnknown)->SaveHeader(pcFileWriter));
		ReturnOnFalse((*ppcUnknown)->Save(pcFileWriter));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::Load(CFileReader* pcFileReader)
{
	size			i;
	CUnknown**		ppcUnknown;
	CUnknown*		pcUnknown;
	size			iNumElements;
	void*			pvData;
	size			iDataSize;
	DataCompare		CaseFunc;

	ReturnOnFalse(pcFileReader->ReadShort(&miFlags));
	CaseFunc = mcMap.ReadCaseSensitivity(pcFileReader);
	if (!mcMap.ReadExceptData(pcFileReader, CaseFunc))
	{
		return false;
	}

	iNumElements = mcMap.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pvData = mcMap.ReadKey(pcFileReader, i, &iDataSize);
		if (!pvData)
		{
			return false;
		}
		ppcUnknown = (CUnknown**)pvData;
		pcUnknown = gcUnknowns.AddFromHeader(pcFileReader);
		if (!pcUnknown)
		{
			return false;
		}
		ReturnOnFalse(pcUnknown->Load(pcFileReader));
		*ppcUnknown = pcUnknown;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::Put(char* szKey, CUnknown* pcValue)
{
	CUnknown**	ppcNew;

	if (!StrEmpty(szKey))
	{
		ppcNew = mcMap.Put(szKey);
		if (ppcNew)
		{
			*ppcNew = pcValue;
			return true;
		}
	}
	return false;
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::DataWillBeFreed(CUnknown* pcUnknown)
{
	if (miFlags & MAP_COMMOM_KILL_ELEMENT)
	{
		pcUnknown->Kill();
	}
}

