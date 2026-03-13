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
	CUnknown**	ppcUnknownKey;
	CUnknown**	ppcUnknownValue;
	size		iNumElements;
	void*		pvValue;
	void*		pvKey;
	SMNode*		psNode;

	ReturnOnFalse(pcFileWriter->WriteInt16(miFlags));
	ReturnOnFalse(mcMap.WriteExceptData(pcFileWriter));

	iNumElements = mcMap.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psNode = mcMap.WriteSizes(pcFileWriter, i);
		if (!psNode)
		{
			return false;
		}

		mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);
		ppcUnknownKey = (CUnknown**)pvKey;
		ppcUnknownValue = (CUnknown**)pvValue;
		ReturnOnFalse((*ppcUnknownKey)->SaveHeader(pcFileWriter));
		ReturnOnFalse((*ppcUnknownKey)->Save(pcFileWriter));
		ReturnOnFalse((*ppcUnknownValue)->SaveHeader(pcFileWriter));
		ReturnOnFalse((*ppcUnknownValue)->Save(pcFileWriter));
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
	CUnknown*		pcUnknownKey;
	CUnknown*		pcUnknownValue;
	CUnknown**		ppcUnknownKey;
	CUnknown**		ppcUnknownValue;
	size			iNumElements;
	void*			pvKey;
	void*			pvValue;
	SMNode*			psNode;

	ReturnOnFalse(pcFileReader->ReadInt16(&miFlags));
	if (!mcMap.ReadExceptData(pcFileReader, ComparePtrPtr))
	{
		return false;
	}

	iNumElements = mcMap.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psNode = mcMap.ReadSizes(pcFileReader, i);
		if (!psNode)
		{
			return false;
		}

		mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);
		ppcUnknownKey = (CUnknown**)pvKey;
		ppcUnknownValue = (CUnknown**)pvValue;
		pcUnknownKey = gcUnknowns.AddFromHeader(pcFileReader);
		if (!pcUnknownKey)
		{
			return false;
		}
		ReturnOnFalse(pcUnknownKey->Load(pcFileReader));
		*ppcUnknownKey = pcUnknownKey;

		pcUnknownValue = gcUnknowns.AddFromHeader(pcFileReader);
		if (!pcUnknownValue)
		{
			return false;
		}
		ReturnOnFalse(pcUnknownValue->Load(pcFileReader));
		*ppcUnknownValue = pcUnknownValue;
	}

	//Sorting is necessary here because the map is keyed by a pointer and the key pointers may not be allocated consecutively.
	mcMap.Sort();
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


