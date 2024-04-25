/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "PointerRemapper.h"
#include "FileUtil.h"
#include "FastFunctions.h"
#include "PointerFunctions.h"
#include "Logger.h"
#include "IndexedData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Init(CDurableFileController* pcController, CLifeInit<CIndexedDataConfig> cConfig)
{
	mpcConfig = NULL;
	cConfig.ConfigureLife(&mcConfig, &mpcConfig);
	CIndexedDataCommon::Init(mpcConfig->GetIndexedDataEvictionUserCallback());

	meWriteThrough = mpcConfig->GetWriteThrough();
	mpcDurableFileControl = pcController;

	mcIndices.Init(this, mpcDurableFileControl, mpcConfig->GetSubdirectory(), mpcConfig->GetIndexCacheSize(), meWriteThrough, mpcConfig->GetEvictionStrategy(), mpcConfig->GetIndexTreeEvictionUserCallback(), mpcConfig->GetIndexTreeDataOrderer());
	mcData.Init(mpcDurableFileControl, mpcConfig->GetSubdirectory(), "DAT", "Files.IDX", "_Files.IDX", mpcConfig->GetDataCacheSize(), meWriteThrough, this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::Kill(void)
{
	bool bResult;

	mpcDurableFileControl = NULL;

	bResult = mcData.Kill();
	bResult &= mcIndices.Kill();
	mcConfig.Kill();

	return bResult;;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction)
{
	return mcIndices.Get(pcDescriptor, oi, bNoEviction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, bool bNoEviction)
{
	return mcIndices.Set(pcDescriptor, oi, bNoEviction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize)
{
	return mcIndices.SetCache(pvCache, uiDataSize, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::RemoveDescriptor(OIndex oi)
{
	return mcIndices.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::Flush(void)
{
	return Flush(false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::Flush(bool bClearDataCache)
{
	bool bRresult;

	if (meWriteThrough == IWT_No)
	{
		bRresult &= mcData.Flush(bClearDataCache);
		bRresult = mcIndices.Flush();
		return bRresult;
	}
	else if (meWriteThrough == IWT_Yes)
	{
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Don't know how to flush data with Write Through [IWT_Unknown].", NULL);
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::IsFlushed(void)
{
	bool bIndicesFlushed;
	bool bDataFlushed;

	bIndicesFlushed = mcIndices.IsFlushed();
	bDataFlushed = mcData.IsFlushed();

	return bIndicesFlushed && bDataFlushed;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedData::NumIndices(void)
{
	return mcIndices.NumIndices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::IsDirty(OIndex oi)
{
	CIndexedDataDescriptor		cKeyDescriptor;
	SIndexedCacheDescriptor*	psDataDescriptor;
	bool						bInMemory;

	bInMemory = mcIndices.GetIfInMemory(&cKeyDescriptor, oi);
	if (bInMemory)
	{
		if (cKeyDescriptor.GetCache())
		{
			psDataDescriptor = (SIndexedCacheDescriptor*)RemapSinglePointer(cKeyDescriptor.GetCache(), -(ptrdiff_t)sizeof(SIndexedCacheDescriptor));
			if (psDataDescriptor->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
			{
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::DescriptorEvicted(OIndex oi, void* pvCache, unsigned int uiDataSize)
{
	bool						bResult;

	if (mpcIndexedDataEvictionCallback)
	{
		bResult = UpdateDescriptorCache(oi, NULL, 0);
		bResult &= mpcIndexedDataEvictionCallback->IndexEvicted(oi, pvCache, uiDataSize);
		return bResult;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::EvictKey(OIndex oi)
{
	return mcIndices.Evict(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return mcData.Evict(oi, pcDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::FlushKey(OIndex oi)
{
	return mcIndices.Flush(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::DumpIndex(void)
{
	mcIndices.Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::ValidateIndex(void)
{
	return mcIndices.ValidateIndex();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedData::ValidateConfigInitialised(void)
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
bool CIndexedData::ValidateConfigKilled(void)
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
OIndex CIndexedData::StartIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	OIndex					oi;
	bool					bResult;
	size_t					iDescriptorSize;

	oi = mcIndices.StartIteration(psIterator, &cDescriptor, &iDescriptorSize, sizeof(CIndexedDataDescriptor));
	if (oi != INVALID_O_INDEX)
	{
		bResult = GetData(oi, &cDescriptor, piDataSize, pvData, iMaxDataSize);
		if (!bResult)
		{
			oi = INVALID_O_INDEX;
		}
	}
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedData::Iterate(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	OIndex					oi;
	bool					bResult;
	size_t					iDescriptorSize;

	oi = mcIndices.Iterate(psIterator, &cDescriptor, &iDescriptorSize, sizeof(CIndexedDataDescriptor));
	if (oi != INVALID_O_INDEX)
	{
		bResult = GetData(oi, &cDescriptor, piDataSize, pvData, iMaxDataSize);
		if (!bResult)
		{
			oi = INVALID_O_INDEX;
		}
	}
	return oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedData::NumIndicesCached(void) { return mcIndices.NumIndicesCached(); }
int64 CIndexedData::NumIndicesCached(size_t iSize) { return mcIndices.NumIndicesCached(iSize); }
CDurableFileController* CIndexedData::GetDurableFileControl(void) { return mpcDurableFileControl; }
bool CIndexedData::IsDurable(void) { return mpcDurableFileControl->IsDurable(); }
size_t CIndexedData::GetIndiciesSystemMemorySize(void) { return mcIndices.GetSystemMemorySize(); }
size_t CIndexedData::GetDataSystemMemorySize(void) { return mcData.GetSystemMemorySize(); }
uint8 CIndexedData::GetRootFlags(void) { return mcIndices.GetRootFlags();  }

