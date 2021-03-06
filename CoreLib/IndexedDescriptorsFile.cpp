#include "BaseLib/Logger.h"
#include "IndexedDataCommon.h"
#include "IndexedDescriptorsFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy)
{
	Init(mpcIndexedData, pcDurableFileController, szSubDirectory, uiIndexCacheSize, eWriteThrough, cEvictionStrategy, NULL, LifeNull<CIndexTreeDataOrderer>());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptorsFile::Init(CIndexedDataCommon* pcIndexedData, CDurableFileController* pcDurableFileController, char* szSubDirectory, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CLifeInit<CIndexTreeEvictionStrategy> cEvictionStrategy, CIndexTreeEvictionCallback* pcEvictionCallback, CLifeInit<CIndexTreeDataOrderer> cDataOrderer)
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
BOOL CIndexedDescriptorsFile::Kill(void)
{
	BOOL bResult;

	bResult = mcIndexTree.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Remove(OIndex oi)
{
	return mcIndexTree.Remove(&oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mcIndexTree.Get(&oi, sizeof(OIndex), pcDescriptor, NULL, sizeof(CIndexedDataDescriptor));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction)
{
	if (!bNoEviction)
	{
		return mcIndexTree.Get(&oi, sizeof(OIndex), pcDescriptor, NULL, sizeof(CIndexedDataDescriptor));
	}
	else
	{
		return mcIndexTree.GetWithoutEviction(&oi, sizeof(OIndex), pcDescriptor, NULL, sizeof(CIndexedDataDescriptor));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mcIndexTree.Put(&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Set(CIndexedDataDescriptor* pcDescriptor, OIndex oi, BOOL bNoEviction)
{
	if (!bNoEviction)
	{
		return mcIndexTree.Put(&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
	}
	else
	{
		return mcIndexTree.PutWithoutEviction(&oi, sizeof(OIndex), pcDescriptor, sizeof(CIndexedDataDescriptor));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::SetCache(void* pvCache, unsigned int uiDataSize, OIndex oi)
{
	CIndexTreeNode*			pcNode;
	CIndexedDataDescriptor*	pcDescriptor;

	pcNode = mcIndexTree.GetMemoryNode(&oi, sizeof(OIndex));
	if (pcNode)
	{
		pcDescriptor = (CIndexedDataDescriptor*)pcNode->GetDataPtr();
		pcDescriptor->Cache(pvCache, uiDataSize);
		return TRUE;
	}
	return FALSE;
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
int CIndexedDescriptorsFile::NumIndicesCached(void)
{
	return mcIndexTree.NumMemoryElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDescriptorsFile::NumIndicesCached(size_t iSize)
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
BOOL CIndexedDescriptorsFile::Flush(void)
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::IsFlushed(void)
{
	return mcIndexTree.IsFlushed();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::GetIfInMemory(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	CIndexTreeNodeFile*		pcNode;
	CIndexedDataDescriptor*	pcReturn;

	pcNode = mcIndexTree.GetMemoryNode(&oi, sizeof(OIndex));
	if (pcNode)
	{
		pcReturn = (CIndexedDataDescriptor*)pcNode->GetDataPtr();
		if (pcDescriptor)
		{
			memcpy(pcDescriptor, pcReturn, sizeof(CIndexedDataDescriptor));
		}
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Evict(OIndex oi)
{
	return mcIndexTree.Evict(&oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::Flush(OIndex oi)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
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
size_t CIndexedDescriptorsFile::GetSystemMemorySize(void)
{
	return mcIndexTree.GetSystemMemorySize();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned char CIndexedDescriptorsFile::GetRootFlags(void)
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
BOOL CIndexedDescriptorsFile::ValidateIndex(void)
{
	return mcIndexTree.ValidateIndexTree();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CIndexedDescriptorsFile::IndexTreeDataSize(uint16 uiSourceSize)
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
BOOL CIndexedDescriptorsFile::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize)
{
	memcpy_fast(pvDataBuffer, pvSource, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptorsFile::IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize)
{
	memset_fast(pvDest, 0, uiDestDataSize);
	memcpy_fast(pvDest, pvDataBuffer, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedDescriptorsFile::StartIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	OIndex	oi;
	int		iKeySize;
	BOOL	bExists;

	bExists = mcIndexTree.StartIteration(psIterator, &oi, &iKeySize, sizeof(OIndex), pvData, piDataSize, iMaxDataSize);
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
OIndex CIndexedDescriptorsFile::Iterate(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize)
{
	OIndex	oi;
	int		iKeySize;
	BOOL	bExists;

	bExists = mcIndexTree.Iterate(psIterator, &oi, &iKeySize, sizeof(OIndex), pvData, piDataSize, iMaxDataSize);
	if (bExists)
	{
		return oi;
	}
	else
	{
		return INVALID_O_INDEX;
	}
}


