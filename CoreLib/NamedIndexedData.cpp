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
#include "BaseLib/Logger.h"
#include "BaseLib/StackMemory.h"
#include "NamedIndexedHeader.h"
#include "NamedIndexedData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexedData::Init(CNamedIndexedDataConfig* pcConfig)
{
	mcData.Init(pcConfig->GetIndexConfig());
	mcNames.Init(pcConfig->GetNamedConfig());
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
BOOL CNamedIndexedData::Add(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	return Add(oi, NULL, 0, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize)
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
BOOL CNamedIndexedData::Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize)
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
BOOL CNamedIndexedData::Add(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize)
{
	CStackMemory<>			cStack;
	CNamedIndexedHeader*	pcHeader;
	size_t					sSize;
	BOOL					bResult;

	if (szName != NULL)
	{
		bResult = mcNames.Add(szName, iNameLength, oi);
		if (!bResult)
		{
			return FALSE;
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
BOOL CNamedIndexedData::Set(OIndex oi, void* pvData)
{
	return Set(oi, pvData, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Set(OIndex oi, void* pvData, unsigned int uiDataSize)
{
	return Set(oi, NULL, 0, pvData, uiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Set(CChars* szName, void* pvData)
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
BOOL CNamedIndexedData::Set(CChars* szName, void* pvData, unsigned int uiDataSize)
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
BOOL CNamedIndexedData::Set(char* szName, void* pvData)
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
BOOL CNamedIndexedData::Set(char* szName, void* pvData, unsigned int uiDataSize)
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
BOOL CNamedIndexedData::Set(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize)
{
	BOOL					bResult;
	CStackMemory<>			cStackSet;
	CNamedIndexedHeader*	pcHeaderSet;
	size_t					sSize;
	CNamedIndexedHeader		cHeaderGet;
	unsigned int			uiOldDataSize;

	if (oi != INVALID_O_INDEX)
	{
		bResult = mcData.Get(oi, &uiOldDataSize, &cHeaderGet, sizeof(CNamedIndexedHeader));
		if (bResult)
		{
			if (uiDataSize == 0)
			{
				uiDataSize = uiOldDataSize;
			}
			sSize = NamedIndexedHeaderSize(iNameLength, uiDataSize);
			pcHeaderSet = (CNamedIndexedHeader*)cStackSet.Init(sSize);
			pcHeaderSet->Init(szName, iNameLength, pvData, uiDataSize);

			bResult = mcData.Set(oi, pvData, uiDataSize);

			cStackSet.Kill();
			return bResult;

		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Put(OIndex oi, void* pvData, unsigned int uiDataSize)
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
BOOL CNamedIndexedData::Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize)
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
BOOL CNamedIndexedData::Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize)
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
BOOL CNamedIndexedData::Put(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize)
{
	OIndex	oiExisting;
	BOOL	bExist;

	oiExisting = mcNames.Get(szName);
	bExist = mcData.Contains(oi);
	if (bExist && oiExisting != INVALID_O_INDEX)
	{
		if (oiExisting != oi)
		{
			gcLogger.Error2(__METHOD__, " Put Index [", IndexToString(oi), "] not equal to existing index [", LongLongToString(oiExisting), "] for name [", szName, "].", NULL);
			return FALSE;
		}
		return Set(oi, pvData, uiDataSize);
	}
	else if (!bExist && oiExisting == INVALID_O_INDEX)
	{
		return Add(oi, szName, iNameLength, pvData, uiDataSize);
	}
	else if (oiExisting)
	{
		gcLogger.Error2(__METHOD__, " Index [", IndexToString(oi), "] does not have an existing name [", szName, "].", NULL);
		return FALSE;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Name [", szName, "] does not have an existing index.", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CNamedIndexedData::Size(OIndex oi)
{
	unsigned int			uiDataSize;
	BOOL					bResult;
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
BOOL CNamedIndexedData::Get(OIndex oi, void* pvData)
{
	return Get(oi, NULL, pvData, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(char* szName, void* pvData)
{
	OIndex	oi;
	BOOL	bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, NULL, pvData, 0);
		return bResult;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	OIndex		oi;
	BOOL		bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, puiDataSize, pvData, uiMaxDataSize);
		return bResult;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(CChars* szName, void* pvData)
{
	OIndex	oi;
	BOOL	bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, NULL, pvData, 0);
		return bResult;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	OIndex		oi;
	BOOL		bResult;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		bResult = Get(oi, puiDataSize, pvData, uiMaxDataSize);
		return bResult;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize)
{
	CStackMemory<>			cStack;
	unsigned int			uiDataSize;
	BOOL					bResult;
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
				return FALSE;
			}
		}
		pvHeaderData = pcHeader->GetData();
 		memcpy_fast(pvData, pvHeaderData, MinDataSize(uiDataSize, uiMaxDataSize));
		cStack.Kill();
		return TRUE;
	}
	else
	{
		cStack.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::GetName(OIndex oi, CChars* szName)
{
	CStackMemory<>			cStack;
	unsigned int			uiDataSize;
	BOOL					bResult;
	CNamedIndexedHeader*	pcHeader;
	
	pcHeader = (CNamedIndexedHeader*)cStack.Init();
	bResult = mcData.Get(oi, &uiDataSize, pcHeader, cStack.GetStackSize());
	if (bResult)
	{
		szName->Append(pcHeader->GetName(), pcHeader->GetNameLength());
		cStack.Kill();
		return TRUE;
	}
	else
	{
		cStack.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength)
{
	CStackMemory<>			cStack;
	unsigned int			uiDataSize;
	BOOL					bResult;
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
		return TRUE;
	}
	else
	{
		SafeAssign(puiNameLength, 0);
		szName[0] = '\0';
		cStack.Kill();
		return FALSE;
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
unsigned int CNamedIndexedData::MinDataSize(unsigned int uiDataSize, unsigned int uiMaxDataSize)
{
	if (uiMaxDataSize == 0)
	{
		return uiDataSize;
	}
	else if (uiMaxDataSize > uiDataSize)
	{
		return uiDataSize;
	}
	else
	{
		return uiMaxDataSize;
	}
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
BOOL CNamedIndexedData::Contains(OIndex oi)
{
	return mcData.Contains(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Contains(char* szName)
{
	OIndex	oi;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		//You might need to get the descriptor and make sure its not NULL.
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Remove(CChars* szName)
{
	OIndex oi;

	oi = mcNames.Get(szName);
	if (oi != INVALID_O_INDEX)
	{
		ReturnOnFalse(mcNames.Remove(szName));
	}
	else
	{
		return FALSE;
	}
	return mcData.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Remove(OIndex oi)
{
	return mcData.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexedData::Flush(BOOL bClearCache)
{
	BOOL	bResult;

	bResult = mcNames.Flush();
	bResult &= mcData.Flush(bClearCache);
	return bResult;
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
int64 CNamedIndexedData::NumIndicesCached(int iSize)
{
	return mcData.NumIndicesCached();
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
BOOL CNamedIndexedData::IsCaching(void)
{
	return mcData.IsCaching();
}

