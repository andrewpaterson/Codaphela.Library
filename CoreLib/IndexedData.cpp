/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
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
BOOL CIndexedData::Kill(void)
{
	BOOL bResult;

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
BOOL CIndexedData::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction)
{
	return mcIndices.Get(pcDescriptor, oi, bNoEviction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor, BOOL bNoEviction)
{
	return mcIndices.Set(pcDescriptor, oi, bNoEviction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::UpdateDescriptorCache(OIndex oi, void* pvCache, unsigned int uiDataSize)
{
	return mcIndices.SetCache(pvCache, uiDataSize, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::RemoveDescriptor(OIndex oi)
{
	return mcIndices.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Flush(void)
{
	return Flush(FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Flush(BOOL bClearDataCache)
{
	BOOL bRresult;

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
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::IsFlushed(void)
{
	BOOL bIndicesFlushed;
	BOOL bDataFlushed;

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
BOOL CIndexedData::IsDirty(OIndex oi)
{
	CIndexedDataDescriptor		cKeyDescriptor;
	SIndexedCacheDescriptor*	psDataDescriptor;
	BOOL						bInMemory;

	bInMemory = mcIndices.GetIfInMemory(&cKeyDescriptor, oi);
	if (bInMemory)
	{
		if (cKeyDescriptor.GetCache())
		{
			psDataDescriptor = (SIndexedCacheDescriptor*)RemapSinglePointer(cKeyDescriptor.GetCache(), -(ptrdiff_t)sizeof(SIndexedCacheDescriptor));
			if (psDataDescriptor->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
			{
				return TRUE;
			}
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::DescriptorEvicted(OIndex oi, void* pvCache, unsigned int uiDataSize)
{
	BOOL						bResult;

	if (mpcIndexedDataEvictionCallback)
	{
		bResult = UpdateDescriptorCache(oi, NULL, 0);
		bResult &= mpcIndexedDataEvictionCallback->IndexEvicted(oi, pvCache, uiDataSize);
		return bResult;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::EvictKey(OIndex oi)
{
	return mcIndices.Evict(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::EvictData(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return mcData.Evict(oi, pcDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::FlushKey(OIndex oi)
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
BOOL CIndexedData::ValidateIndex(void)
{
	return mcIndices.ValidateIndex();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::ValidateConfigInitialised(void)
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
BOOL CIndexedData::ValidateConfigKilled(void)
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
OIndex CIndexedData::StartIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	OIndex					oi;
	BOOL					bResult;
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
	BOOL					bResult;
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
BOOL CIndexedData::IsDurable(void) { return mpcDurableFileControl->IsDurable(); }
size_t CIndexedData::GetIndiciesSystemMemorySize(void) { return mcIndices.GetSystemMemorySize(); }
size_t CIndexedData::GetDataSystemMemorySize(void) { return mcData.GetSystemMemorySize(); }
unsigned char CIndexedData::GetRootFlags(void) { return mcIndices.GetRootFlags();  }

