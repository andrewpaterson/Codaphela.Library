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
#include "NamedIndexes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::Init(CDurableFileController* pcController, CLifeInit<CNamedIndexesConfig> cConfig)
{
	mpcConfig = NULL;
	cConfig.ConfigureLife(&mcConfig, &mpcConfig);
	mcIndexTree.Init(pcController, mpcConfig->GetSubDirectory(), mpcConfig->GetIndexCacheSize(), mpcConfig->GetIndexTreeEvictionCallback(), mpcConfig->GetEvictionStrategy(), this, mpcConfig->GetWriteThrough(), IKR_No, mpcConfig->GetIndexTreeDataOrderer());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Kill(void)
{
	BOOL	bResult;

	bResult = mcIndexTree.Kill();
	mcConfig.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	if (iDataSize != sizeof(OIndex))
	{
		return gcLogger.Error2(__METHOD__, "Data evicted was not an OIndex.", NULL);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Add(char* szName, OIndex oi)
{
	int		iKeySize;

	if (szName != NULL)
	{
		iKeySize = strlen(szName);
		return Add(szName, iKeySize, oi);
	}
	else
	{
		return Add(NULL, 0, oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Add(CChars* szName, OIndex oi)
{
	if (szName != NULL)
	{
		return Add(szName->Text(), szName->Length(), oi);
	}
	else
	{
		return Add(NULL, 0, oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Add(char* szName, int iNameLength, OIndex oi)
{
	BOOL	bExists;

	if ((iNameLength != 0) && (szName != NULL))
	{
		if (!IsValidIndex(oi))
		{
			return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "] with index [", IndexToString(oi), "].", NULL);
		}

		bExists = mcIndexTree.HasKey(szName, iNameLength);
		if (bExists)
		{
			return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "] and index [", IndexToString(oi), "].  It already exists.", NULL);
		}
		return mcIndexTree.Put(szName, iNameLength, &oi, sizeof(OIndex));
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Add index [", IndexToString(oi), "] with empty name.", NULL);;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Set(char* szName, OIndex oi)
{
	int		iKeySize;
	BOOL	bExists;

	if (!StrEmpty(szName))
	{
		iKeySize = strlen(szName);
		bExists = mcIndexTree.HasKey(szName, iKeySize);
		if (bExists)
		{
			return gcLogger.Error2(__METHOD__, " Cannot Set Name [", szName, "] and index [", IndexToString(oi), "].  The Name does not exist.", NULL);
		}
		return mcIndexTree.Put(szName, iKeySize, &oi, sizeof(OIndex));
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set index [", IndexToString(oi), "] with empty name.", NULL);;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Set(CChars* szName, OIndex oi)
{
	BOOL	bExists;

	if ((szName != NULL) && (!szName->Empty()))
	{
		bExists = mcIndexTree.HasKey(szName->Text(), szName->Length());
		if (!bExists)
		{
			return gcLogger.Error2(__METHOD__, " Cannot Set Name [", szName->Text(), "] and index [", IndexToString(oi), "].  The Name does not exist.", NULL);
		}
		return mcIndexTree.Put(szName->Text(), szName->Length(), &oi, sizeof(OIndex));
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Set index [", IndexToString(oi), "] with empty name.", NULL);;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Put(char* szName, OIndex oi)
{
	int		iKeySize;

	if (!StrEmpty(szName))
	{
		iKeySize = strlen(szName);
		return mcIndexTree.Put(szName, iKeySize, &oi, sizeof(OIndex));
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Put index [", IndexToString(oi), "] with empty name.", NULL);;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Put(CChars* szName, OIndex oi)
{
	if ((szName != NULL) && (!szName->Empty()))
	{
		return mcIndexTree.Put(szName->Text(), szName->Length(), &oi, sizeof(OIndex));
	}
	else
	{
		return gcLogger.Error2(__METHOD__, " Cannot Put index [", IndexToString(oi), "] with empty name.", NULL);;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::Get(char* szName)
{
	int		iKeySize;
	OIndex	iResult;
	BOOL	bExists;

	if (!StrEmpty(szName))
	{
		iKeySize = strlen(szName);
		bExists = mcIndexTree.Get(szName, iKeySize, &iResult, NULL, sizeof(OIndex));
		if (bExists)
		{
			return iResult;
		}
	}
	return INVALID_O_INDEX;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::Get(CChars* szName)
{
	OIndex	iResult;
	BOOL	bExists;

	if ((szName != NULL) && !szName->Empty())
	{
		bExists = mcIndexTree.Get(szName->Text(), szName->Length(), &iResult, NULL, sizeof(OIndex));
		if (bExists)
		{
			return iResult;
		}
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(char* szName)
{
	int iKeySize;

	if (szName)
	{
		iKeySize = strlen(szName);
		return mcIndexTree.Remove(szName, iKeySize);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(CChars* szName)
{
	return mcIndexTree.Remove(szName->Text(), szName->Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexes::NumElements(void)
{
	return mcIndexTree.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CNamedIndexes::NumElementsCached(void)
{
	return mcIndexTree.NumMemoryElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Flush(void) 
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::StartIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	int		iKeySize;
	BOOL	bExists;
	size_t	iDataSize;

	bExists = mcIndexTree.StartIteration(psIterator, szKey, &iKeySize, MAX_KEY_SIZE, poi, &iDataSize, sizeof(OIndex));
	if (bExists)
	{
		if (szKey)
		{
			szKey[iKeySize] = '\0';
		}
	}
	else
	{
		SafeAssign(poi, INVALID_O_INDEX);
		if (szKey)
		{
			szKey[0] = '\0';
		}
	}
	return bExists;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Iterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	int		iKeySize;
	BOOL	bExists;
	size_t	iDataSize;

	bExists = mcIndexTree.Iterate(psIterator, szKey, &iKeySize, MAX_KEY_SIZE, poi, &iDataSize, sizeof(OIndex));
	if (bExists)
	{
		if (szKey)
		{
			szKey[iKeySize] = '\0';
		}
	}
	else
	{
		SafeAssign(poi, INVALID_O_INDEX);
		if (szKey)
		{
			szKey[0] = '\0';
		}
	}
	return bExists;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CNamedIndexes::IndexTreeDataSize(uint16 uiSourceSize)
{
	if (uiSourceSize != 0)
	{
		return sizeof(OIndex);
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize)
{
	memcpy_fast(pvDataBuffer, pvSource, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize)
{
	memset_fast(pvDest, 0, uiDestDataSize);
	memcpy_fast(pvDest, pvDataBuffer, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::DumpIndex(void)
{
	mcIndexTree.Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::ValidateIndex(void)
{
	return mcIndexTree.ValidateIndexTree();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::ValidateConfigInitialised(void)
{
	if (!mcConfig.IsInitialised())
	{
		return gcLogger.Error2(__METHOD__, " IndexedData config is not initialised.", NULL);
	}
	if (!mcConfig.HasLifeCycleObject())
	{
		return gcLogger.Error2(__METHOD__, " IndexedData config has [NULL] life cycle object.", NULL);
	}
	if (mpcConfig == NULL)
	{
		return gcLogger.Error2(__METHOD__, " IndexedData config is [NULL].", NULL);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::ValidateConfigKilled(void)
{
	if (!mcConfig.IsKilled())
	{
		return gcLogger.Error2(__METHOD__, " IndexedData config is not killed", NULL);
	}
	if (mcConfig.HasLifeCycleObject())
	{
		return gcLogger.Error2(__METHOD__, " IndexedData config has [!NULL] life cycle object.", NULL);
	}
	if (mpcConfig != NULL)
	{
		return gcLogger.Error2(__METHOD__, " IndexedData config is [!NULL].", NULL);
	}
	return TRUE;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CNamedIndexes::GetSystemMemorySize(void)
{
	return mcIndexTree.GetSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size_t CNamedIndexes::GetUserMemorySize(void)
{
	return mcIndexTree.GetUserMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CNamedIndexes::GetRootFlags(void)
{
	return mcIndexTree.GetRoot()->GetFlags();
}

