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
#include "IndexedData.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Init(char* szWorkingDirectory, unsigned int uiCacheSize, BOOL bDurable)
{
	CIndexedConfig	cConfig;

	cConfig.OptimiseForStreaming(szWorkingDirectory);
	cConfig.SetDurable(bDurable);
	cConfig.SetObjectCacheSize(uiCacheSize);

	Init(&cConfig);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Init(CIndexedConfig* pcConfig)
{
	mbDurable = pcConfig->mbDurable;
	mbWriteThrough = pcConfig->mbWriteThrough;
	mpvTemp = NULL;
	muiTempSize = 0;

	mcDurableFileControl.Init(pcConfig->mszWorkingDirectory, mbDurable);
	mcDurableFileControl.MakeDir(pcConfig->mszWorkingDirectory);

	InitIndices(pcConfig);
	mcObjectFiles.Init(&mcDurableFileControl, "DAT");

	if (pcConfig->miObjectsCacheSize != 0)
	{
		mcObjectCache.Init(pcConfig->miObjectsCacheSize);
		mbCaching = TRUE;
	}
	else
	{
		mcObjectCache.Zero();
		mbCaching = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Kill(void)
{
	DurableBegin();

	if (!mbDurable)
	{
		Flush();
	}
	else
	{
		Uncache();
	}
	mcIndices.Save();

	DurableEnd();
	mcDurableFileControl.Kill();

	if (mbCaching)
	{
		mcObjectCache.Kill();
	}

	mcIndicesAccess.Kill();
	mcIndicesFile.Kill();
	mcObjectFiles.Kill();
	mcIndices.Kill();
	SafeFree(mpvTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::InitIndices(CIndexedConfig* pcConfig)
{
	CChars			szName;
	CChars			szRewrite;

	szName.Init(pcConfig->mszWorkingDirectory);
	szName.Append(FILE_SEPARATOR[0]);
	szName.Append("Indices.DAT");
	szRewrite.Init(pcConfig->mszWorkingDirectory);
	szRewrite.Append(FILE_SEPARATOR[0]);
	szRewrite.Append("_Indices.DAT");

	mcIndicesFile.Init(&mcDurableFileControl, szName.Text(), szRewrite.Text());
	mcIndicesAccess.Init(	&mcIndicesFile, 
							this, 
							pcConfig->mbDirtyTesting, 
							pcConfig->miIndicesSecondLevelWidth, 
							pcConfig->miIndicesThirdLevelWidth, 
							pcConfig->miIndicesNumSecondLevelChunks, 
							pcConfig->miIndicesNumThirdLevelChunks);
	mcIndices.Init(&mcIndicesAccess);

	szName.Kill();
	szRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedData::Flags(OIndex oi)
{
	CIndexDescriptor		cDescriptor;
	BOOL						bResult;

	bResult = mcIndices.Get(&cDescriptor, oi);
	if (bResult)
	{
		return cDescriptor.GetUserFlags();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::IsCaching(void)
{
	return mbCaching;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Write(CIndexDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp)
{
	BOOL	bWritten;
	BOOL	bResult;

	pcDescriptor->TimeStamp(uiTimeStamp);
	bResult = CacheWrite(pcDescriptor, pvData, &bWritten);
	if (bResult)
	{
		if (!bWritten && mbWriteThrough)
		{
			return WriteData(pcDescriptor, pvData);
		}
		else
		{
			return TRUE;
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
BOOL CIndexedData::CacheRead(CIndexDescriptor* pcDescriptor)
{
	void*					pvData;
	BOOL					bResult;
	CMemoryCacheAllocation	cPreAllocated;

	if (mbCaching)
	{
		cPreAllocated.Init(pcDescriptor->GetDataSize());
		if (mcObjectCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
		{
			bResult = WriteEvictedData(cPreAllocated.GetEvictedArray());
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}
			pvData = mcObjectCache.Allocate(pcDescriptor);  //Remove all Allocate(CIndexDescriptor*)... it's already preallocated.
			if (!pvData)
			{
				cPreAllocated.Kill();
				return FALSE;
			}
			
			bResult = mcObjectFiles.Read(pcDescriptor, pvData);
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}
			EvictOverlappingFromCache(cPreAllocated.GetEvictedArray());
			cPreAllocated.Kill();
			return TRUE;
		}
		else
		{
			return TRUE;
		}
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
BOOL CIndexedData::CacheWrite(CIndexDescriptor* pcDescriptor, void* pvData, BOOL* pbWritten)
{
	BOOL					bResult;
	CMemoryCacheAllocation	cPreAllocated;

	if (mbCaching)
	{
		cPreAllocated.Init(pcDescriptor->GetDataSize());
		if (mcObjectCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
		{
			*pbWritten = FALSE;
			bResult = WriteEvictedData(cPreAllocated.GetEvictedArray());
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			bResult = mcObjectCache.Allocate(pcDescriptor, pvData);  //Remove all Allocate(CIndexDescriptor*)... it's already preallocated.
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			EvictOverlappingFromCache(cPreAllocated.GetEvictedArray());
			cPreAllocated.Kill();
			return TRUE;
		}
		else
		{
			//There wasn't enough space in the cache... the object is written immediately.
			bResult = WriteData(pcDescriptor, pvData);
			*pbWritten = TRUE;

			cPreAllocated.Kill();
			return bResult;
		}
	}
	else
	{
		*pbWritten = TRUE;
		return WriteData(pcDescriptor, pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::EvictFromCache(CIndexDescriptor* pcDescriptor)
{
	SIndexedCacheDescriptor* psExisting;

	if (pcDescriptor->IsCached())
	{
		psExisting = mcObjectCache.GetHeader(pcDescriptor->GetCache());
		WriteEvictedData(pcDescriptor, psExisting);
		mcObjectCache.Invalidate(psExisting);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::EvictFromCache(SIndexedCacheDescriptor* psExisting)
{
	CIndexDescriptor	cDescriptor;
	BOOL				bResult;

	bResult = mcIndices.Get(&cDescriptor, psExisting->oi);
	cDescriptor.Cache(NULL);
	mcIndices.Set(&cDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::EvictOverlappingFromCache(CArrayPointer* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psDesc;
	int							iType;

	for (i = 0; i < papsEvictedIndexedCacheDescriptors->NumElements(); i++)
	{
		if (papsEvictedIndexedCacheDescriptors->Get(i, (void**)&psDesc, &iType))
		{
			EvictFromCache(psDesc);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Invalidate(CIndexDescriptor* pcDescriptor)
{
	if (mbCaching)
	{
		mcObjectCache.Invalidate(pcDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::WriteData(CIndexDescriptor* pcDescriptor, void* pvData)
{
	if (pcDescriptor->HasFile())
	{
		return mcObjectFiles.WriteExisting(pcDescriptor, pvData);
	}
	else
	{
		return mcObjectFiles.WriteNew(pcDescriptor, pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::WriteEvictedData(CIndexDescriptor* pcDescriptor, SIndexedCacheDescriptor* psCached)
{
	void*				pvData;

	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		pvData = RemapSinglePointer(psCached, sizeof(SIndexedCacheDescriptor));
		WriteData(pcDescriptor, pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::WriteEvictedData(SIndexedCacheDescriptor* psCached)
{
	CIndexDescriptor	cDescriptor;
	BOOL				bResult;

	bResult = mcIndices.Get(&cDescriptor, psCached->oi);
	if (!bResult)
	{
		return FALSE;
	}

	//We can assume the cache is clear of the data because it has been evicted.
	cDescriptor.Cache(NULL);

	WriteEvictedData(&cDescriptor, psCached);
	return mcIndices.Set(&cDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::ClearDescriptorCache(SIndexedCacheDescriptor* psCached)
{
	CIndexDescriptor		cDescriptor;
	BOOL						bResult;

	bResult = mcIndices.Get(&cDescriptor, psCached->oi);
	if (!bResult)
	{
		return FALSE;
	}

	//We can assume the cache is clear of the data because it has been evicted.
	cDescriptor.Cache(NULL);

	//These can never ever be unequal.  But just in case...
	if (cDescriptor.GetDataSize() == psCached->iDataSize)
	{
		return mcIndices.Set(&cDescriptor);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::WriteEvictedData(CArrayPointer* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psCached;
	BOOL						bResult;

	for (i = 0; i < papsEvictedIndexedCacheDescriptors->NumElements(); i++)
	{
		psCached = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		bResult = WriteEvictedData(psCached);
		if (!bResult)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedData::Size(OIndex oi)
{
	BOOL						bResult;
	CIndexDescriptor		cDescriptor;

	bResult = mcIndices.Get(&cDescriptor, oi);
	if (bResult)
	{
		return cDescriptor.GetDataSize();
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Add(OIndex oi, void* pvData, unsigned int iDataSize, unsigned int uiTimeStamp)
{
	CIndexDescriptor	cDescriptor;
	BOOL				bResult;

	//It doesn't matter what the result was.
	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		//Can't add an oi that already exists.
		return FALSE;
	}

	//This init clears the file index.  This means CompareDiskToMemory() will not try and read it to test for changes.
	cDescriptor.Init(oi, iDataSize);

	bResult = Write(&cDescriptor, pvData, uiTimeStamp);
	mcIndices.Set(&cDescriptor);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Set(OIndex oi, void* pvData, unsigned int uiTimeStamp)
{
	CIndexDescriptor	cDescriptor;
	BOOL				bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		if (cDescriptor.IsCached())
		{
			mcObjectCache.Update(&cDescriptor, pvData);
			cDescriptor.TimeStamp(uiTimeStamp);
			mcIndices.Set(&cDescriptor);
			return TRUE;
		}
		else
		{
			bResult = Write(&cDescriptor, pvData, uiTimeStamp);
			mcIndices.Set(&cDescriptor);
			return bResult;
		}
	}
	else
	{
		//Can't set if the oi doesn't exist.
		return FALSE;
	}
}




//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	CIndexDescriptor	cDescriptor;
	BOOL				bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		if (cDescriptor.GetDataSize() == uiDataSize)
		{
			if (cDescriptor.IsCached())
			{
				mcObjectCache.Update(&cDescriptor, pvData);
				cDescriptor.TimeStamp(uiTimeStamp);
				mcIndices.Set(&cDescriptor);
				return TRUE;
			}
			else
			{
				bResult = Write(&cDescriptor, pvData, uiTimeStamp);
				mcIndices.Set(&cDescriptor);
				return bResult;
			}
		}
		else
		{
			Invalidate(&cDescriptor);
			cDescriptor.Init(oi, uiDataSize);

			bResult = Write(&cDescriptor, pvData, uiTimeStamp);
			mcIndices.Set(&cDescriptor);
			return bResult;
		}
	}
	else
	{
		//Can't set if the oi doesn't exist.
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL				bResult;
	CIndexDescriptor	cDescriptor;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return Set(oi, pvData, uiDataSize, uiTimeStamp);
	}
	else
	{
		return Add(oi, pvData, uiDataSize, uiTimeStamp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::CompareDiskToMemory(CIndexDescriptor* pcDescriptor, void* pvData)
{
	//This function tells the disk whether it must update itself because the cached value has changed.
	//It also timestamps the descriptor of the changed data.
	unsigned int	uiDataSize;

	if (pcDescriptor->HasFile())
	{
		uiDataSize = pcDescriptor->GetDataSize();
		if (pcDescriptor->GetDataSize() > muiTempSize)
		{
			SafeFree(mpvTemp);
			mpvTemp = malloc(uiDataSize);
			muiTempSize = uiDataSize;
		}

		mcObjectFiles.Read(pcDescriptor, mpvTemp);
		if (memcmp(mpvTemp, pvData, uiDataSize) == 0)
		{
			return TRUE;
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
BOOL CIndexedData::GetDescriptor(OIndex oi, CIndexDescriptor* pcDescriptor)
{
	return mcIndices.Get(pcDescriptor, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Get(OIndex oi, void* pvData)
{
	CIndexDescriptor	cDescriptor;
	BOOL				bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		return FALSE;
	}

	GetData(&cDescriptor, pvData);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::GetData(CIndexDescriptor* pcDescriptor, void* pvData)
{
	BOOL	bResult;

	//The descriptor has always been set prior to calling this.
	if (pcDescriptor->IsCached())
	{
		memcpy_fast(pvData, pcDescriptor->GetCache(), pcDescriptor->GetDataSize());
		return TRUE;
	}
	else
	{
		bResult = CacheRead(pcDescriptor);
		if (bResult)
		{
			if (pcDescriptor->IsCached())
			{
				memcpy_fast(pvData, pcDescriptor->GetCache(), pcDescriptor->GetDataSize());
				return mcIndices.Set(pcDescriptor);
			}
			else
			{
				return mcObjectFiles.Read(pcDescriptor, pvData);
			}
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Remove(OIndex oi)
{
	CIndexDescriptor	cDescriptor;
	BOOL				bResult;

	//This is not correct.  Removed objects must be marked as removed until all transactions are finished.
	bResult = mcIndices.Get(&cDescriptor, oi);
	if (bResult)
	{
		if (cDescriptor.IsCached())
		{
			Invalidate(&cDescriptor);
		}
		mcIndices.Remove(oi);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::DurableBegin(void)
{
	if (mbDurable)
	{
		mcDurableFileControl.Begin();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::DurableEnd(void)
{
	if (mbDurable)
	{
		mcDurableFileControl.End();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Flush(void)
{
	SIndexedCacheDescriptor*	psCached;
	BOOL						bAnyFailed;
	BOOL						bResult;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		psCached = mcObjectCache.GetFirst();
		while (psCached)
		{
			bResult = WriteEvictedData(psCached);
			if (!bResult)
			{
				bAnyFailed = TRUE;
			}
			psCached = mcObjectCache.GetNext(psCached);
		}
		mcObjectCache.Clear();
		return bAnyFailed;
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
BOOL CIndexedData::Uncache(void)
{
	SIndexedCacheDescriptor*	psCached;
	BOOL						bAnyFailed;
	BOOL						bResult;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		psCached = mcObjectCache.GetFirst();
		while (psCached)
		{
			bResult = ClearDescriptorCache(psCached);
			if (!bResult)
			{
				bAnyFailed = TRUE;
			}
			psCached = mcObjectCache.GetNext(psCached);
		}
		mcObjectCache.Clear();
		return bAnyFailed;
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
int CIndexedData::NumCached(void)
{
	if (mbCaching)
	{
		return mcObjectCache.NumCached();
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
int CIndexedData::NumFiles(void)
{
	return mcObjectFiles.NumFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedData::NumInFile(int iDataSize)
{
	return mcObjectFiles.NumInFile(iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedData::NumElements(void)
{
	return mcIndices.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::TestNumCachedIndexes(void)
{
	return (int)mcIndices.NumCachedDatas();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::TestIndexedDescriptorsLength(void)
{
	return (int)mcIndices.Length();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::TestNumIgnoredCacheElements(void)
{
	return mcObjectCache.NumIgnored();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexAccess* CIndexedData::TestGetIndexAccess(void)
{
	return mcIndicesAccess.GetAccess();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::AddFile(CDurableFile* pcFile)
{
	mcDurableFileControl.AddFile(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedData::TestGetCachedObjectSize(OIndex oi)
{
	SIndexedCacheDescriptor*	psDesc;

	psDesc = mcObjectCache.TestGetDescriptor(oi);
	if (psDesc)
	{
		return sizeof(SIndexedCacheDescriptor) + psDesc->iDataSize;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFileController* CIndexedData::GetDurableFileControl(void)
{
	return &mcDurableFileControl;
}

