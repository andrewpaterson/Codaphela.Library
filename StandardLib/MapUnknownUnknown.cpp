/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "MapUnknownUnknown.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::Init(bool bKillElements, bool bOverwriteExisting)
{
	CMapCommonUnknown::Init(bKillElements, bOverwriteExisting);
	mcMap.Init();
	mcMap.SetDataFreeCallback(&mcDataFree);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::Kill(void)
{
	mcMap.Kill();
	CMapCommonUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::Save(CFileWriter* pcFileWriter)
{
	size		i;
	CUnknown**	ppcUnknown;
	size		iNumElements;
	void*		pvData;
	size		iDataSize;

	ReturnOnFalse(pcFileWriter->WriteInt16(miFlags));
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
bool CMapUnknownUnknown::Load(CFileReader* pcFileReader)
{
	size			i;
	CUnknown**		ppcUnknown;
	CUnknown*		pcUnknown;
	size			iNumElements;
	void*			pvData;
	size			iDataSize;

	ReturnOnFalse(pcFileReader->ReadInt16(&miFlags));
	if (!mcMap.ReadExceptData(pcFileReader, ComparePtrPtr))
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
bool CMapUnknownUnknown::Put(CUnknown* pcKey, CUnknown* pcValue)
{
	bool		bResult;

	if (pcKey)
	{
		bResult = mcMap.Put(pcKey, pcValue);
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CMapUnknownUnknown::Get(CUnknown* pcKey)
{
	CUnknown* pcValue;

	pcValue = (CUnknown*)mcMap.Get(pcKey);
	return pcValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapUnknownUnknown::NumElements(void)
{
	return mcMap.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapUnknownUnknown::GetSortedSize(void)
{
	return mcMap.GetSortedSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapUnknownUnknown::GetHoldingSize(void)
{
	return mcMap.GetHoldingSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::StartIteration(SMapIterator* psIterator, CUnknown** ppcKey, CUnknown** ppcData)
{
	CUnknown**	ppcK;
	CUnknown**	ppcV;
	bool		bResult;

	bResult = mcMap.StartIteration(psIterator, (void**)&ppcK, NULL, (void**)&ppcV, NULL);
	if (bResult)
	{
		*ppcKey = *ppcK;
		*ppcData = *ppcV;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::Iterate(SMapIterator* psIterator, CUnknown** ppcKey, CUnknown** ppcData)
{
	CUnknown**	ppcK;
	CUnknown**	ppcV;
	bool		bResult;

	bResult = mcMap.Iterate(psIterator, (void**)&ppcK, NULL, (void**)&ppcV, NULL);
	if (bResult)
	{
		*ppcKey = *ppcK;
		*ppcData = *ppcV;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::DataWillBeFreed(SMNode* psNode)
{
	CUnknown*	pcKey;
	CUnknown*	pcValue;

	if (miFlags & MAP_COMMOM_KILL_ELEMENT)
	{
		pcKey = *((CUnknown**)mcMap.GetKey(psNode));
		pcValue = *((CUnknown**)mcMap.GetValue(psNode));
		if (pcValue)
		{
			pcValue->Kill();
		}
		pcKey->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::Pack(void)
{
	mcMap.Pack();
}


