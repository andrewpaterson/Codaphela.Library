/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
void CMapStringUnknown::Init(bool bKillElements, bool bOverwriteExisting)
{
	CMapCommonUnknown::Init(bKillElements, bOverwriteExisting);
	mcMap.Init(true, bOverwriteExisting);
	mcMap.SetDataFreeCallback(this);
	mcMap.SetDataIOCallback(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::Kill(void)
{
	mcMap.Kill();
	CMapCommonUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::Put(const char* szKey, CUnknown* pcValue)
{
	CUnknown**	ppcValue;
	CUnknown*	pcOldValue;

	if (StrEmpty(szKey))
	{
		return false;
	}

	ppcValue = mcMap.Get(szKey);
	if (ppcValue)
	{
		if (!IsOverwriteExisting())
		{
			return false;
		}

		//Replace the value in place.  Removing the node and adding a new one would kill the value even if it is
		//the one being put again.
		pcOldValue = *ppcValue;
		*ppcValue = pcValue;
		if (IsKillElements() && pcOldValue && (pcOldValue != pcValue))
		{
			pcOldValue->Kill();
		}
		return true;
	}

	ppcValue = mcMap.Put(szKey);
	if (ppcValue)
	{
		*ppcValue = pcValue;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnknown* CMapStringUnknown::Get(const char* szKey)
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
size CMapStringUnknown::NumElements(void)
{
	return mcMap.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::Save(CFileWriter* pcFileWriter)
{
	ReturnOnFalse(pcFileWriter->WriteInt16(muiFlags));
	return mcMap.Write(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::Load(CFileReader* pcFileReader)
{
	ReturnOnFalse(pcFileReader->ReadInt16(&muiFlags));
	return mcMap.Read(pcFileReader, this, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapStringUnknown::GetSortedSize(void)
{
	return mcMap.GetSortedSize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapStringUnknown::GetHoldingSize(void)
{
	return mcMap.GetHoldingSize();
}


//////////////////////////////////////////////////////////////////////////
//
// The entry an iteration step found (bExists): its key and value, which
// the map holds a pointer to.
//
//////////////////////////////////////////////////////////////////////////
static bool MapStringUnknownEntry(bool bExists, char* szKey, CUnknown** ppcValue, char** pszKey, CUnknown** ppcData)
{
	if (bExists)
	{
		*pszKey = szKey;
		*ppcData = *ppcValue;
	}
	return bExists;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::StartIteration(SMapIterator* psIterator, char** ppcKey, CUnknown** ppcData)
{
	char*		szKey;
	CUnknown**	ppcValue;

	return MapStringUnknownEntry(mcMap.StartIteration(psIterator, (void**)&szKey, NULL, (void**)&ppcValue, NULL), szKey, ppcValue, ppcKey, ppcData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::Iterate(SMapIterator* psIterator, char** ppcKey, CUnknown** ppcData)
{
	char*		szKey;
	CUnknown**	ppcValue;

	return MapStringUnknownEntry(mcMap.Iterate(psIterator, (void**)&szKey, NULL, (void**)&ppcValue, NULL), szKey, ppcValue, ppcKey, ppcData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::FreeData(void* pvData)
{
	SMNode*		psNode;
	CUnknown*	pcValue;

	psNode = (SMNode*)pvData;
	if (muiFlags & MAP_COMMON_KILL_ELEMENT)
	{
		pcValue = *((CUnknown**)mcMap.GetValue(psNode));
		if (pcValue)
		{
			pcValue->Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::WriteData(CFileWriter* pcFile, void* pvData)
{
	void*		pvValue;
	void*		pvKey;
	SMNode*		psNode;

	psNode = (SMNode*)pvData;
	mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);

	ReturnOnFalse(pcFile->WriteData(pvKey, psNode->iKeySize));
	return gcUnknowns.SaveUnknown(pcFile, *((CUnknown**)pvValue));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapStringUnknown::ReadData(CFileReader* pcFile, void* pvData)
{
	void*		pvValue;
	void*		pvKey;
	SMNode*		psNode;

	psNode = (SMNode*)pvData;
	mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);

	ReturnOnFalse(pcFile->ReadData(pvKey, psNode->iKeySize));
	return gcUnknowns.LoadUnknown(pcFile, (CUnknown**)pvValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapStringUnknown::Pack(void)
{
	mcMap.Pack();
}


