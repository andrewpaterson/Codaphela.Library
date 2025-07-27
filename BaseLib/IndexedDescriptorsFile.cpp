#include "Logger.h"
#include "IndexedDataCommon.h"
#include "IndexedDescriptorsFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy)
{
	Init(mpcIndexedData, pcDurableFileController, szSubDirectory, uiIndexCacheSize, eWriteThrough, cEvictionStrategy, NULL, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
{
	CIndexTreeEvictionCallback*	pcCallback;

	mpcIndexedData = pcIndexedData;
	if (pcEvictionCallback)
	{
		mcEvictionCallbackWrapper.Init(pcEvictionCallback, this);
		pcCallback = &mcEvictionCallbackWrapper;
	}
	else
	{
		mcEvictionCallbackWrapper.Init(NULL, NULL);
		pcCallback = this;
	}
	mcIndexTree.Init(pcDurableFileController, szSubDirectory, uiIndexCacheSize, pcCallback, cEvictionStrategy, this, eWriteThrough, IKR_Yes, cDataOrderer);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Kill(void)
{
	bool bResult;

	bResult = mcIndexTree.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Remove(OIndex oi)
{
	return mcIndexTree.Remove((uint8*)&oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mcIndexTree.Get((uint8*)&oi, sizeof(OIndex), pcDescriptor, NULL, sizeof(CIndexedDataDescriptor));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi, bool bNoEviction)
{
	if (!bNoEviction)
	{
		return mcIndexTree.Get((uint8*)&oi, sizeof(OIndex), pcDescriptor, NULL, sizeof(CIndexedDataDescriptor));
	}
	else
	{
		return mcIndexTree.GetWithoutEviction((uint8*)&oi, sizeof(OIndex), pcDescriptor, NULL, sizeof(CIndexedDataDescriptor));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mcIndexTree.Put((uint8*)&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi, bool bNoEviction)
{
	if (!bNoEviction)
	{
		return mcIndexTree.Put((uint8*)&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
	}
	else
	{
		return mcIndexTree.PutWithoutEviction((uint8*)&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::SetCache(void* pvCache, uint32 uiDataSize, OIndex oi)
{
	CIndexTreeNode*			pcNode;
	CIndexedDataDescriptor*	pcDescriptor;

	pcNode = mcIndexTree.GetMemoryNode((uint8*)&oi, sizeof(OIndex));
	if (pcNode)
	{
		pcDescriptor = (CIndexedDataDescriptor*)pcNode->GetDataPtr();
		pcDescriptor->Cache(pvCache, uiDataSize);
		return true;
	}
	return false;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDescriptorsFile::NumElements(void)
{
	return NumIndices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDescriptorsFile::NumIndices(void)
{
	return mcIndexTree.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedDescriptorsFile::NumIndicesCached(void)
{
	return mcIndexTree.NumMemoryElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedDescriptorsFile::NumIndicesCached(size iSize)
{
	return mcIndexTree.NumMemoryElements(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedDescriptorsFile::NumDatas(void)
{
	return mpcIndexedData->NumDatas();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Flush(void)
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::IsFlushed(void)
{
	return mcIndexTree.IsFlushed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	CIndexTreeNodeFile*		pcNode;
	CIndexedDataDescriptor*	pcReturn;

	pcNode = mcIndexTree.GetMemoryNode((uint8*)&oi, sizeof(OIndex));
	if (pcNode)
	{
		pcReturn = (CIndexedDataDescriptor*)pcNode->GetDataPtr();
		if (pcDescriptor)
		{
			memcpy(pcDescriptor, pcReturn, sizeof(CIndexedDataDescriptor));
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Evict(OIndex oi)
{
	return mcIndexTree.Evict((uint8*)&oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::Flush(OIndex oi)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::IndexTreeNodeEvicted(void* pvKey, size iKeySize, void* pvData, size iDataSize)
{
	OIndex	oi;

	if (iKeySize != sizeof(OIndex))
	{
		return gcLogger.Error2(__METHOD__, " Key evicted was not an OIndex.", NULL);
	}

	oi = *((OIndex*)pvKey);
	return mpcIndexedData->EvictData(oi, (CIndexedDataDescriptor*)pvData);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedDescriptorsFile::GetSystemMemorySize(void)
{
	return mcIndexTree.GetSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedDescriptorsFile::GetRootFlags(void)
{
	return mcIndexTree.GetRootFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Dump(void)
{
	mcIndexTree.Dump();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::ValidateIndex(void)
{
	return mcIndexTree.ValidateIndexTree();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexedDescriptorsFile::IndexTreeDataSize(size uiSourceSize)
{
	if (uiSourceSize != 0)
	{
		return sizeof(SIndexedFileDataDescriptor);
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
bool CIndexedDescriptorsFile::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, size iFileDataSize, size uiSourceDataSize)
{
	memcpy_fast(pvDataBuffer, pvSource, iFileDataSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDescriptorsFile::IndexTreeReadData(void* pvDest, void* pvDataBuffer, size uiDestDataSize, size iFileDataSize)
{
	memset_fast(pvDest, 0, uiDestDataSize);
	memcpy_fast(pvDest, pvDataBuffer, iFileDataSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedDescriptorsFile::StartIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	OIndex	oi;
	size	iKeySize;
	bool	bExists;

	bExists = mcIndexTree.StartIteration(psIterator, (uint8*)&oi, &iKeySize, sizeof(OIndex), pvData, piDataSize, iMaxDataSize);
	if (bExists)
	{
		return oi;
	}
	else
	{
		return INVALID_O_INDEX;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedDescriptorsFile::Iterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize)
{
	OIndex	oi;
	size	iKeySize;
	bool	bExists;

	bExists = mcIndexTree.Iterate(psIterator, (uint8*)&oi, &iKeySize, sizeof(OIndex), pvData, piDataSize, iMaxDataSize);
	if (bExists)
	{
		return oi;
	}
	else
	{
		return INVALID_O_INDEX;
	}
}


