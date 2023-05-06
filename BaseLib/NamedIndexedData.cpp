/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "Logger.h"
#include "StackMemory.h"
#include "NamedIndexedHeader.h"
#include "NamedIndexedData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::Init(CDurableFileController* pcController, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig)
{
	mcData.Init(pcController, cIndexConfig);
	mcNames.Init(pcController, cNamedConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::Kill(void)
{
	mcNames.Kill();
	mcData.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Add(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	return Add(oi, NULL, 0, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize)
{
	int	iNameLength;

	if (szName != NULL)
	{
		iNameLength = strlen(szName);
		return Add(oi, szName, iNameLength, pvData, uiDataSize);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot add Index [", IndexToString(oi), "] with [NULL] Name.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize)
{
	if (szName != NULL)
	{
		return Add(oi, szName->Text(), szName->Length(), pvData, uiDataSize);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot add Index [", IndexToString(oi), "] with [NULL] Name.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Add(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize)
{
	CStackMemory<>			cStack;
	CNamedIndexedHeader*	pcHeader;
	size_t					sSize;
	bool					bResult;

	if (szName != NULL)
	{
		if ((pvData == NULL) || (uiDataSize == 0))
		{
			return gcLogger.Error2(__METHOD__, " Cannot add Name [", szName, "] with Index [", IndexToString(oi), "] with empty data.", NULL);
		}

		bResult = mcNames.Add(szName, iNameLength, oi);
		if (!bResult)
		{
			return false;
		}

		sSize = NamedIndexedHeaderSize(iNameLength, uiDataSize);
		pcHeader = (CNamedIndexedHeader*)cStack.Init(sSize);
		pcHeader->Init(szName, iNameLength, pvData, uiDataSize);
		
		bResult = mcData.Add(oi, pcHeader, sSize);

		cStack.Kill();
		return bResult;
	}
	else
	{
		if ((pvData == NULL) || (uiDataSize == 0))
		{
			return gcLogger.Error2(__METHOD__, " Cannot add Index [", IndexToString(oi), "] with empty data.", NULL);
		}

		sSize = NamedIndexedHeaderSize(uiDataSize);
		pcHeader = (CNamedIndexedHeader*)cStack.Init(sSize);
		pcHeader->Init(pvData, uiDataSize);

		bResult = mcData.Add(oi, pcHeader, sSize);

		cStack.Kill();
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Set(OIndex oi, void* pvData)
{
	return Set(oi, pvData, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Set(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	return Set(oi, NULL, 0, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Set(CChars* szName, void* pvData)
{
	OIndex		oi;

	if ((szName != NULL) && !szName->Empty())
	{
		oi = mcNames.Get(szName);
		return Set(oi, szName->Text(), szName->Length(), pvData, 0);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot set with [NULL] Name.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Set(CChars* szName, void* pvData, unsigned int uiDataSize)
{
	OIndex		oi;

	if ((szName != NULL) && !szName->Empty())
	{
		oi = mcNames.Get(szName);
		return Set(oi, szName->Text(), szName->Length(), pvData, uiDataSize);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot set with [NULL] Name.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Set(char* szName, void* pvData)
{
	int			iNameLength;
	OIndex		oi;

	if (!StrEmpty(szName))
	{
		oi = mcNames.Get(szName);
		iNameLength = strlen(szName);
		return Set(oi, szName, iNameLength, pvData, 0);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot set with [NULL] Name.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Set(char* szName, void* pvData, unsigned int uiDataSize)
{
	int			iNameLength;
	OIndex		oi;

	if (!StrEmpty(szName))
	{
		oi = mcNames.Get(szName);
		iNameLength = strlen(szName);
		return Set(oi, szName, iNameLength, pvData, uiDataSize);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot set with [NULL] Name.", NULL);
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Set(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize)
{
	bool					bResult;
	CStackMemory<>			cStackSet;
	CStackMemory<>			cStackGet;
	CNamedIndexedHeader*	pcHeaderSet;
	size_t					sSize;
	CNamedIndexedHeader*	pcHeaderGet;
	unsigned int			uiOldDataSize;
	int						iOldNameLength;

	if (oi != INVALID_O_INDEX)
	{
		pcHeaderGet = (CNamedIndexedHeader*)cStackGet.Init();
		bResult = mcData.Get(oi, &uiOldDataSize, pcHeaderGet, cStackGet.GetStackSize());
		if (bResult)
		{
			if (uiDataSize == 0)
			{
				uiDataSize = uiOldDataSize - pcHeaderGet->GetHeaderSize();
			}
			iOldNameLength = pcHeaderGet->GetNameLength();
			if (iNameLength == 0)
			{
				iNameLength = iOldNameLength;
				szName = pcHeaderGet->GetName();
			}
			else
			{
				if (iNameLength != iOldNameLength)
				{
					cStackGet.Kill();
					return gcLogger.Error2(__METHOD__, " Old Name [", pcHeaderGet->GetName() ,"] may not be different to set Name [", szName,"].", NULL);
				}
			}
			sSize = NamedIndexedHeaderSize(iNameLength, uiDataSize);
			pcHeaderSet = (CNamedIndexedHeader*)cStackSet.Init(sSize);
			pcHeaderSet->Init(szName, iNameLength, pvData, uiDataSize);

			bResult = mcData.Set(oi, pcHeaderSet, sSize);

			cStackGet.Kill();
			cStackSet.Kill();
			return bResult;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Put(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	if (mcData.Contains(oi))
	{
		return Set(oi, pvData, uiDataSize);
	}
	else
	{
		return Add(oi, pvData, uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize)
{
	if (szName != NULL)
	{
		return Put(oi, szName->Text(), szName->Length(), pvData, uiDataSize);
	}
	else
	{
		return Put(oi, NULL, 0, pvData, uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize)
{
	int		iNameLength;

	if (szName != NULL)
	{
		iNameLength = strlen(szName);
		return Put(oi, szName, iNameLength, pvData, uiDataSize);
	}
	else
	{
		return Put(oi, NULL, 0, pvData, uiDataSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Put(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize)
{
	OIndex	oiExisting;
	bool	bExists;

	oiExisting = mcNames.Get(szName);
	bExists = mcData.Contains(oi);
	if (bExists && oiExisting != INVALID_O_INDEX)
	{
		if (oiExisting != oi)
		{
			return gcLogger.Error2(__METHOD__, " Put Index [", IndexToString(oi), "] not equal to existing index [", IndexToString(oiExisting), "] for name [", szName, "].", NULL);
		}
		return Set(oi, pvData, uiDataSize);
	}
	else if (!bExists && oiExisting == INVALID_O_INDEX)
	{
		return Add(oi, szName, iNameLength, pvData, uiDataSize);
	}
	else if (oiExisting != INVALID_O_INDEX)
	{
		return gcLogger.Error2(__METHOD__, " Index [", IndexToString(oi), "] does not have an existing name [", szName, "].", NULL);
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Name [", szName, "] does not have an existing index.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CNamedIndexedData::Size(OIndex oi)
{
	unsigned int			uiDataSize;
	bool					bResult;
	CNamedIndexedHeader		cNamedIndexedHeader;

	bResult = mcData.Get(oi, &uiDataSize, &cNamedIndexedHeader, sizeof(CNamedIndexedHeader));
	if (bResult)
	{
		return uiDataSize - cNamedIndexedHeader.GetHeaderSize();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Get(OIndex oi, void* pvData)
{
	return Get(oi, NULL, pvData, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Get(char* szName, void* pvData)
{
	OIndex	oi;
	bool	bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, NULL, pvData, 0);
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	OIndex		oi;
	bool		bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, puiDataSize, pvData, uiMaxDataSize);
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Get(CChars* szName, void* pvData)
{
	OIndex	oi;
	bool	bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, NULL, pvData, 0);
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	OIndex		oi;
	bool		bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, puiDataSize, pvData, uiMaxDataSize);
		return bResult;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	CStackMemory<>			cStack;
	unsigned int			uiDataSize;
	bool					bResult;
	CNamedIndexedHeader*	pcHeader;
	int						iNameLength;
	void*					pvHeaderData;

	pcHeader = (CNamedIndexedHeader*)cStack.Init();
	bResult = mcData.Get(oi, &uiDataSize, pcHeader, cStack.GetStackSize());
	if (bResult)
	{
		iNameLength = pcHeader->GetNameLength();
		if (uiDataSize > (unsigned int)cStack.GetStackSize())
		{
			cStack.Kill();
			pcHeader = (CNamedIndexedHeader*)cStack.Init(uiDataSize);
			bResult = mcData.Get(oi, &uiDataSize, pcHeader, uiDataSize);
			if (!bResult)
			{
				cStack.Kill();
				return false;
			}
		}

		uiDataSize = uiDataSize - pcHeader->GetHeaderSize();
		SafeAssign(puiDataSize, uiDataSize);
		if (pvData)
		{
			pvHeaderData = pcHeader->GetData();
			
			memcpy(pvData, pvHeaderData, MinDataSize(uiDataSize, uiMaxDataSize));
		}
		cStack.Kill();
		return true;
	}
	else
	{
		cStack.Kill();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::GetName(OIndex oi, CChars* szName)
{
	CStackMemory<>			cStack;
	unsigned int			uiDataSize;
	bool					bResult;
	CNamedIndexedHeader*	pcHeader;
	
	pcHeader = (CNamedIndexedHeader*)cStack.Init();
	bResult = mcData.Get(oi, &uiDataSize, pcHeader, cStack.GetStackSize());
	if (bResult)
	{
		szName->Append(pcHeader->GetName(), pcHeader->GetNameLength());
		cStack.Kill();
		return true;
	}
	else
	{
		cStack.Kill();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength)
{
	CStackMemory<>			cStack;
	unsigned int			uiDataSize;
	bool					bResult;
	CNamedIndexedHeader*	pcHeader;
	int						iNameLength;
	char*					szSourceName;

	pcHeader = (CNamedIndexedHeader*)cStack.Init();
	bResult = mcData.Get(oi, &uiDataSize, pcHeader, cStack.GetStackSize());
	if (bResult)
	{
		szSourceName = pcHeader->GetName();
		iNameLength = pcHeader->GetNameLength();
		strcpy_s(szName, uiMaxNameLength, szSourceName);
		SafeAssign(puiNameLength, iNameLength);
		cStack.Kill();
		return true;
	}
	else
	{
		SafeAssign(puiNameLength, 0);
		szName[0] = '\0';
		cStack.Kill();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CNamedIndexedData::MinNameSize(unsigned int uiNameSize, unsigned int uiMaxNameSize)
{
	return MinDataSize(uiNameSize, uiMaxNameSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexedData::GetIndex(char* szName)
{
	OIndex	oi;

	oi = mcNames.Get(szName);
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexedData::GetIndex(CChars* szName)
{
	OIndex	oi;

	oi = mcNames.Get(szName);
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Contains(OIndex oi)
{
	return mcData.Contains(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Contains(char* szName)
{
	OIndex	oi;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		//You might need to get the descriptor and make sure its not NULL.
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Contains(CChars* szName)
{
	OIndex	oi;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		//You might need to get the descriptor and make sure its not NULL.
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Remove(CChars* szName)
{
	OIndex oi;

	oi = mcNames.Get(szName);
	if (IsValidIndex(oi))
	{
		if (!mcNames.Remove(szName))
		{
			return false;
		}
		return mcData.Remove(oi);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Remove(char* szName)
{
	OIndex oi;

	oi = mcNames.Get(szName);
	if (IsValidIndex(oi))
	{
		if (!mcNames.Remove(szName))
		{
			return false;
		}
		return mcData.Remove(oi);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Remove(OIndex oi)
{
	CStackMemory<>			cStack;
	unsigned int			uiDataSize;
	bool					bResult;
	CNamedIndexedHeader*	pcHeader;
	int						iNameLength;
	char*					szName;

	pcHeader = (CNamedIndexedHeader*)cStack.Init();
	bResult = mcData.Get(oi, &uiDataSize, pcHeader, cStack.GetStackSize());
	if (bResult)
	{
		szName = pcHeader->GetName();
		iNameLength = pcHeader->GetNameLength();
		if (iNameLength != 0)
		{
			if (!mcNames.Remove(szName))
			{
				cStack.Kill();
				return false;
			}
		}
		cStack.Kill();
		return mcData.Remove(oi);
	}
	else
	{
		cStack.Kill();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Flush(void)
{
	return Flush(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::Flush(bool bClearCache)
{
	bool	bResult;

	if (!IsWriteThrough())
	{
		bResult = mcNames.Flush();
		bResult &= mcData.Flush(bClearCache);
		return bResult;
	}
	else
	{
		//mcData.ClearCache();  //Seriously, what even is the point of "clear cache"?
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexedData::NumIndices(void)
{
	return mcData.NumElements();	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexedData::NumIndicesCached(void)
{
	return mcData.NumIndicesCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexedData::NumIndicesCached(size_t iSize)
{
	return mcData.NumIndicesCached(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexedData::NumDataCached(void)
{
	return mcData.NumDataCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexedData::NumDataCached(size_t iSize)
{
	return mcData.NumDataCached(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexedData::NumNames(void)
{
	return mcNames.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexedData::NumNamesCached(void)
{
	return mcNames.NumElementsCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::IsCaching(void)
{
	return mcData.IsCaching();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::IsWriteThrough(void)
{
	return mcData.IsWriteThrough();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::ValidateIdentifiers(void)
{
	bool bResult;

	bResult = ValidateNameToOidToName();
	bResult &= ValidateOidToNameToOid();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::ValidateOidToNameToOid(void)
{
	SIndexTreeFileIterator	sIter;
	OIndex					oi;
	CStackMemory<>			cStackMemory;
	unsigned int			uiDataSize;
	unsigned int			uiMaxDataSize;
	void*					pvData;
	bool					bResult;
	CNamedIndexedHeader*	pcHeader;
	char*					szName;
	OIndex					oiFromData;

	pvData = cStackMemory.Init();
	uiMaxDataSize = cStackMemory.GetStackSize();

	oi = StartIndexIteration(&sIter, NULL, NULL, 0);
	while (oi != INVALID_O_INDEX)
	{
		bResult = mcData.Get(oi, &uiDataSize, pvData, uiMaxDataSize);
		if (!bResult)
		{
			cStackMemory.Kill();
			return gcLogger.Error2(__METHOD__, " NamedIndexedData corrupt.  Iterated index [0x", LongLongToString(oi, 16), "] but it doesn not exist.", NULL);
		}
		if (uiDataSize > uiMaxDataSize)
		{
			cStackMemory.Kill();
			pvData = cStackMemory.Init(uiDataSize);
			uiMaxDataSize = uiDataSize;

			bResult = mcData.Get(oi, &uiDataSize, pvData, uiMaxDataSize);
		}

		pcHeader = (CNamedIndexedHeader*)pvData;
		if (pcHeader->HasName())
		{
			szName = pcHeader->GetName();
			oiFromData = GetIndex(szName);
			if (oi != oiFromData)
			{
				cStackMemory.Kill();
				return gcLogger.Error2(__METHOD__, " NamedIndexedData corrupt.  Index [0x", LongLongToString(oi, 16), "] maps to name [", szName, "] but the name maps to index [0x", LongLongToString(oiFromData, 16), "].", NULL);
			}
		}

		oi = IndexIterate(&sIter, NULL, NULL, 0);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::ValidateNameToOidToName(void)
{
	SIndexTreeFileIterator	sIter;
	char					szName[MAX_KEY_SIZE + 1];
	OIndex					oi;
	bool					bExists;
	CStackMemory<>			cStackMemory;
	unsigned int			uiDataSize;
	unsigned int			uiMaxDataSize;
	void*					pvData;
	CNamedIndexedHeader*	pcHeader;
	bool					bResult;
	char*					szNameFromData;
	int						iResult;

	pvData = cStackMemory.Init();
	uiMaxDataSize = cStackMemory.GetStackSize();

	memset(szName, 0, MAX_KEY_SIZE + 1);
	bExists = StartNameIteration(&sIter, szName, &oi);
	while (bExists)
	{
		bResult = mcData.Get(oi, &uiDataSize, pvData, uiMaxDataSize);
		if (!bResult)
		{
			cStackMemory.Kill();
			return gcLogger.Error2(__METHOD__, " NamedIndexedData corrupt.  Iterated oid [0x", LongLongToString(oi, 16), "] but it doesn not exist.", NULL);
		}
		if (uiDataSize > uiMaxDataSize)
		{
			cStackMemory.Kill();
			pvData = cStackMemory.Init(uiDataSize);
			uiMaxDataSize = uiDataSize;

			bResult = mcData.Get(oi, &uiDataSize, pvData, uiMaxDataSize);
		}

		pcHeader = (CNamedIndexedHeader*)pvData;
		szNameFromData = pcHeader->GetName();
		iResult = strcmp(szName, szNameFromData);
		if (iResult != 0)
		{
			cStackMemory.Kill();
			return gcLogger.Error2(__METHOD__, " NamedIndexedData corrupt.  Name [", szName, "] maps to index [0x", LongLongToString(oi, 16), "] but the index maps to name [", szNameFromData, "].", NULL);
		}

		bExists = NameIterate(&sIter, szName, &oi);
	}
	cStackMemory.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::ValidateConfigInitialised(void)
{
	bool	bResult;

	bResult = mcData.ValidateConfigInitialised();
	bResult |= mcNames.ValidateConfigInitialised();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::ValidateConfigKilled(void)
{
	bool	bResult;

	bResult = mcData.ValidateConfigKilled();
	bResult |= mcNames.ValidateConfigKilled();

	return bResult; 
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CNamedIndexedData::GetIndiciesSystemMemorySize(void)
{
	return mcData.GetIndiciesSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CNamedIndexedData::GetDataSystemMemorySize(void)
{
	return mcData.GetDataSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CNamedIndexedData::GetNamesSystemMemorySize(void)
{
	return mcNames.GetSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::Dump(void)
{
	CChars	sz;

	sz.Init("---- Names -----")->AppendNewLine()->DumpKill();
	mcNames.DumpIndex();
	sz.Init()->AppendNewLine()->Append("--- Indicies ---")->AppendNewLine()->DumpKill();
	mcData.DumpIndex();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexedData::StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	CStackMemory<8 KB>		cStack;
	CNamedIndexedHeader*	pcHeader;
	void*					pvHeaderData;
	size_t					iMaxDataAndHeaderSize;
	OIndex					oi;
	size_t					iDataSize;

	iMaxDataAndHeaderSize = iMaxDataSize + sizeof(CNamedIndexedHeader) + MAX_KEY_SIZE;
	pcHeader = (CNamedIndexedHeader*)cStack.Init(iMaxDataAndHeaderSize);

	oi = mcData.StartIteration(psIterator, pcHeader, &iDataSize, iMaxDataAndHeaderSize);
	if (oi != INVALID_O_INDEX)
	{
		pvHeaderData = pcHeader->GetData();
		iDataSize = iDataSize - pcHeader->GetHeaderSize();
		SafeAssign(piDataSize, iDataSize);
		if (iMaxDataSize > 0)
		{
			memcpy(pvData, pvHeaderData, MinDataSize(iDataSize, iMaxDataSize));
		}
	}
	
	cStack.Kill();
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexedData::IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	CStackMemory<8 KB>		cStack;
	CNamedIndexedHeader* pcHeader;
	void* pvHeaderData;
	size_t					iMaxDataAndHeaderSize;
	OIndex					oi;
	size_t					iDataSize;

	iMaxDataAndHeaderSize = iMaxDataSize + sizeof(CNamedIndexedHeader) + MAX_KEY_SIZE;
	pcHeader = (CNamedIndexedHeader*)cStack.Init(iMaxDataAndHeaderSize);

	oi = mcData.Iterate(psIterator, pcHeader, &iDataSize, iMaxDataAndHeaderSize);
	if (oi != INVALID_O_INDEX)
	{
		pvHeaderData = pcHeader->GetData();
		iDataSize = iDataSize - pcHeader->GetHeaderSize();
		SafeAssign(piDataSize, iDataSize);
		if (iMaxDataSize > 0)
		{
			memcpy(pvData, pvHeaderData, MinDataSize(iDataSize, iMaxDataSize));
		}
	}

	cStack.Kill();
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	return mcNames.StartIteration(psIterator, szKey, poi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexedData::NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	return mcNames.Iterate(psIterator, szKey, poi);
}

