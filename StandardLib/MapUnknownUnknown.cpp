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
	mcMap.SetDataFreeCallback(this);
	mcMap.SetDataIOCallback(this);
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
void CMapUnknownUnknown::ReInit(void)
{
	mcMap.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::Save(CFileWriter* pcFileWriter)
{
	ReturnOnFalse(WriteMapUnknownHeader(pcFileWriter));
	return mcMap.WriteMapBlockElements(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::Load(CFileReader* pcFileReader)
{
	bool	bResult;

	ReturnOnFalse(ReadMapUnknownHeader(pcFileReader));
	bResult = mcMap.ReadMapBlockElements(pcFileReader);
	if (bResult)
	{
		mcMap.Sort();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::WriteMapUnknownHeader(CFileWriter* pcFileWriter)
{
	ReturnOnFalse(pcFileWriter->WriteInt16(miFlags));
	return mcMap.WriteMapBlockHeader(pcFileWriter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::ReadMapUnknownHeader(CFileReader* pcFileReader)
{
	ReturnOnFalse(pcFileReader->ReadInt16(&miFlags));
	return mcMap.ReadMapBlockHeader(pcFileReader, &ComparePtrPtr, this, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::ReadMapUnknownHeader(CFileReader* pcFileReader, CDataIO* pcDataIO, CDataFree* pcDataFree)
{
	ReturnOnFalse(pcFileReader->ReadInt16(&miFlags));
	return mcMap.ReadMapBlockHeader(pcFileReader, &ComparePtrPtr, pcDataIO, pcDataFree);
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
SMNode* CMapUnknownUnknown::GetNode(CUnknown* pcKey)
{
	return mcMap.GetNode(&pcKey, sizeof(CUnknown*));
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
size CMapUnknownUnknown::NonNullElements(void)
{
	return mcMap.NonNullElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::Remove(CUnknown* pcKey)
{
	return mcMap.Remove(pcKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::SetKey(SMapIterator* psIter, CUnknown* pcNewKey)
{
	SMNode*		psNode;
	void*		pvValue;
	void*		pvKey;
	CUnknown**	ppcOldKey;

	psNode = mcMap.GetNode(psIter);
	mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);
	ppcOldKey = (CUnknown**)pvKey;
	*ppcOldKey = pcNewKey;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::SetValue(SMapIterator* psIter, CUnknown* pcNewValue)
{
	SMNode*		psNode;
	void*		pvValue;
	void*		pvKey;
	CUnknown**	ppcOldValue;

	psNode = mcMap.GetNode(psIter);
	mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);
	ppcOldValue = (CUnknown**)pvValue;
	*ppcOldValue = pcNewValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::StartIteration(SMapIterator* psIterator, CUnknown** ppcKey, CUnknown** ppcValue)
{
	CUnknown**	ppcK;
	CUnknown**	ppcV;
	bool		bResult;

	bResult = mcMap.StartIteration(psIterator, (void**)&ppcK, NULL, (void**)&ppcV, NULL);
	if (bResult)
	{
		*ppcKey = *ppcK;
		*ppcValue = *ppcV;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::Iterate(SMapIterator* psIterator, CUnknown** ppcKey, CUnknown** ppcValue)
{
	CUnknown**	ppcK;
	CUnknown**	ppcV;
	bool		bResult;

	bResult = mcMap.Iterate(psIterator, (void**)&ppcK, NULL, (void**)&ppcV, NULL);
	if (bResult)
	{
		*ppcKey = *ppcK;
		*ppcValue = *ppcV;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::FreeData(void* pvData)
{
	CUnknown*	pcKey;
	CUnknown*	pcValue;
	SMNode*		psNode;

	psNode = (SMNode*)pvData;
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
bool CMapUnknownUnknown::WriteData(CFileWriter* pcFile, void* pvData)
{
	CUnknown**	ppcUnknownKey;
	CUnknown**	ppcUnknownValue;
	void*		pvValue;
	void*		pvKey;
	bool		bResult;
	SMNode*		psNode;

	psNode = (SMNode*)pvData;
	mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);
	ppcUnknownValue = (CUnknown**)pvValue;
	ppcUnknownKey = (CUnknown**)pvKey;

	bResult = (*ppcUnknownKey)->SaveHeader(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = (*ppcUnknownKey)->Save(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = (*ppcUnknownValue)->SaveHeader(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = (*ppcUnknownValue)->Save(pcFile);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::ReadData(CFileReader* pcFile, void* pvData)
{
	CUnknown**	ppcUnknownKey;
	CUnknown*	pcUnknownKey;
	CUnknown**	ppcUnknownValue;
	CUnknown*	pcUnknownValue;
	void*		pvValue;
	void*		pvKey;
	bool		bResult;
	SMNode*		psNode;

	psNode = (SMNode*)pvData;

	mcMap.RemapKeyAndData(psNode, &pvKey, &pvValue);

	ppcUnknownKey = (CUnknown**)pvKey;
	pcUnknownKey = gcUnknowns.AddFromHeader(pcFile);
	if (!pcUnknownKey)
	{
		return false;
	}
	bResult = pcUnknownKey->Load(pcFile);
	if (!bResult)
	{
		return false;
	}
	*ppcUnknownKey = pcUnknownKey;

	ppcUnknownValue = (CUnknown**)pvValue;
	pcUnknownValue = gcUnknowns.AddFromHeader(pcFile);
	if (!pcUnknownValue)
	{
		return false;
	}
	bResult = pcUnknownValue->Load(pcFile);
	if (!bResult)
	{
		return false;
	}
	*ppcUnknownValue = pcUnknownValue;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::Pack(void)
{
	mcMap.Pack();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMapUnknownUnknown::Sort(void)
{
	return mcMap.Sort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMapUnknownUnknown::IsSorted(void)
{
	return mcMap.IsSorted();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CMapPtrPtr* CMapUnknownUnknown::GetPointerMap(void)
{
	return &mcMap;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::Print(CChars* psz)
{
	SMapIterator	sIter;
	CUnknown*		pcKey;
	CUnknown*		pcValue;
	bool			bResult;
	size			ui;
	CChars			szNumber;

	ui = 0;
	bResult = StartIteration(&sIter, &pcKey, &pcValue);
	while (bResult)
	{
		szNumber.Init();
		szNumber.Append(ui);
		szNumber.RightAlign(' ', 5);
		psz->Append(szNumber);
		szNumber.Kill();
		psz->Append(" ");
		psz->Append(pcKey->ClassName());
		psz->Append(":[");
		pcKey->Print(psz);
		psz->Append("] -> ");
		psz->Append(pcValue->ClassName());
		psz->Append(":[");
		pcValue->Print(psz);
		psz->Append("]");
		psz->AppendNewLine();
		bResult = Iterate(&sIter, &pcKey, &pcValue);
		ui++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMapUnknownUnknown::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}

