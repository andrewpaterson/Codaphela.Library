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
#include "Logger.h"
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
bool CNamedIndexes::Kill(void)
{
	bool	bResult;

	bResult = mcIndexTree.Kill();
	mcConfig.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexes::IndexTreeNodeEvicted(void* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	if (iDataSize != sizeof(OIndex))
	{
		return gcLogger.Error2(__METHOD__, "Data evicted was not an OIndex.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexes::Add(char* szName, OIndex oi)
{
	size		iKeySize;

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
bool CNamedIndexes::Add(CChars* szName, OIndex oi)
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
bool CNamedIndexes::Add(char* szName, size iNameLength, OIndex oi)
{
	bool	bExists;

	if ((iNameLength != 0) && (szName != NULL))
	{
		if (!IsValidIndex(oi))
		{
			return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "] with index [", IndexToString(oi), "].", NULL);
		}

		bExists = mcIndexTree.HasKey((uint8*)szName, iNameLength);
		if (bExists)
		{
			return gcLogger.Error2(__METHOD__, " Cannot Add Name [", szName, "] and index [", IndexToString(oi), "].  It already exists.", NULL);
		}
		return mcIndexTree.Put((uint8*)szName, iNameLength, &oi, sizeof(OIndex));
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
bool CNamedIndexes::Set(char* szName, OIndex oi)
{
	size	iKeySize;
	bool	bExists;

	if (!StrEmpty(szName))
	{
		iKeySize = strlen(szName);
		bExists = mcIndexTree.HasKey((uint8*)szName, iKeySize);
		if (bExists)
		{
			return gcLogger.Error2(__METHOD__, " Cannot Set Name [", szName, "] and index [", IndexToString(oi), "].  The Name does not exist.", NULL);
		}
		return mcIndexTree.Put((uint8*)szName, iKeySize, &oi, sizeof(OIndex));
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
bool CNamedIndexes::Set(CChars* szName, OIndex oi)
{
	bool	bExists;

	if ((szName != NULL) && (!szName->Empty()))
	{
		bExists = mcIndexTree.HasKey((uint8*)szName->Text(), szName->Length());
		if (!bExists)
		{
			return gcLogger.Error2(__METHOD__, " Cannot Set Name [", szName->Text(), "] and index [", IndexToString(oi), "].  The Name does not exist.", NULL);
		}
		return mcIndexTree.Put((uint8*)szName->Text(), szName->Length(), &oi, sizeof(OIndex));
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
bool CNamedIndexes::Put(char* szName, OIndex oi)
{
	size		iKeySize;

	if (!StrEmpty(szName))
	{
		iKeySize = strlen(szName);
		return mcIndexTree.Put((uint8*)szName, iKeySize, &oi, sizeof(OIndex));
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
bool CNamedIndexes::Put(CChars* szName, OIndex oi)
{
	if ((szName != NULL) && (!szName->Empty()))
	{
		return mcIndexTree.Put((uint8*)szName->Text(), szName->Length(), &oi, sizeof(OIndex));
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
	size	iKeySize;
	OIndex	iResult;
	bool	bExists;

	if (!StrEmpty(szName))
	{
		iKeySize = strlen(szName);
		bExists = mcIndexTree.Get((uint8*)szName, iKeySize, &iResult, NULL, sizeof(OIndex));
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
	bool	bExists;

	if ((szName != NULL) && !szName->Empty())
	{
		bExists = mcIndexTree.Get((uint8*)szName->Text(), szName->Length(), &iResult, NULL, sizeof(OIndex));
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
bool CNamedIndexes::Remove(char* szName)
{
	size iKeySize;

	if (szName)
	{
		iKeySize = strlen(szName);
		return mcIndexTree.Remove((uint8*)szName, iKeySize);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexes::Remove(CChars* szName)
{
	return mcIndexTree.Remove((uint8*)szName->Text(), szName->Length());
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
bool CNamedIndexes::Flush(void) 
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexes::StartIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	size	iKeySize;
	bool	bExists;
	size	iDataSize;

	bExists = mcIndexTree.StartIteration(psIterator, (uint8*)szKey, &iKeySize, MAX_KEY_SIZE, poi, &iDataSize, sizeof(OIndex));
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
bool CNamedIndexes::Iterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi)
{
	size	iKeySize;
	bool	bExists;
	size	iDataSize;

	bExists = mcIndexTree.Iterate(psIterator, (uint8*)szKey, &iKeySize, MAX_KEY_SIZE, poi, &iDataSize, sizeof(OIndex));
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
size CNamedIndexes::IndexTreeDataSize(size uiSourceSize)
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
bool CNamedIndexes::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, size iFileDataSize, size uiSourceDataSize)
{
	memcpy_fast(pvDataBuffer, pvSource, iFileDataSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexes::IndexTreeReadData(void* pvDest, void* pvDataBuffer, size uiDestDataSize, size iFileDataSize)
{
	memset_fast(pvDest, 0, uiDestDataSize);
	memcpy_fast(pvDest, pvDataBuffer, iFileDataSize);
	return true;
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
bool CNamedIndexes::ValidateIndex(void)
{
	return mcIndexTree.ValidateIndexTree();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexes::ValidateConfigInitialised(void)
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNamedIndexes::ValidateConfigKilled(void)
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
	return true;
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNamedIndexes::GetSystemMemorySize(void)
{
	return mcIndexTree.GetSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNamedIndexes::GetUserMemorySize(void)
{
	return mcIndexTree.GetUserMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CNamedIndexes::GetRootFlags(void)
{
	return mcIndexTree.GetRoot()->GetFlags();
}

