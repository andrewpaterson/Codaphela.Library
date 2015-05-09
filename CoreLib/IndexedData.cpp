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
	mbWriteThrough = pcConfig->mbWriteThrough;
	
	mcTemp.Init();

	mcDurableFileControl.Init(pcConfig->mszWorkingDirectory, pcConfig->mbDurable);
	mcDurableFileControl.MakeDir(pcConfig->mszWorkingDirectory);

	InitIndices(pcConfig);
	mcDataFiles.Init(&mcDurableFileControl, "DAT");
	mcDataFiles.Open();

	if (pcConfig->miObjectsCacheSize != 0)
	{
		mcDataCache.Init(pcConfig->miObjectsCacheSize);
		mbCaching = TRUE;
	}
	else
	{
		mcDataCache.Zero();
		mbCaching = FALSE;
	}

	mbTransient = pcConfig->mbTransient;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Kill(void)
{
	if (!mbTransient)
	{
		if (!mcDurableFileControl.IsDurable())
		{
			KillNonTransientNonDurable();
		}
		else
		{
			DurableBegin();
			Uncache();
			CloseFiles();
			DurableEnd();
		}
	}
	else
	{
		KillTransient();
	}

	KillEnd();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::KillEnd(void)
{
	mcDurableFileControl.Kill();

	if (mbCaching)
	{
		mcDataCache.Kill();
	}

	mcIndicesAccess.Kill();
	mcIndicesFile.Kill();
	mcDataFiles.Kill();
	mcIndices.Kill();
	mcTemp.Kill();
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::KillTransient(void)
{
	mcDataFiles.Close();
	RemoveFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::KillNonTransientNonDurable(void)
{
	Flush(TRUE);
	CloseFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::CloseFiles(void)
{
	BOOL bResult;
	
	bResult = mcIndices.Save();
	bResult &= mcDataFiles.Close();
	bResult &= mcIndicesFile.Close();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::RemoveFiles(void)
{
	BOOL	bResult;
	
	bResult = mcDataFiles.RemoveFiles();
	bResult &= mcIndices.RemoveFile();
	return bResult;
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
	CIndexedDataDescriptor		cDescriptor;
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
BOOL CIndexedData::Write(CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp)
{
	BOOL	bWritten;
	BOOL	bResult;

	pcDescriptor->TimeStamp(uiTimeStamp);
	bResult = CacheWrite(pcDescriptor, pvData, &bWritten);
	if (bResult)
	{
		if (!bWritten && mbWriteThrough)
		{
			bResult = mcDataFiles.WriteData(pcDescriptor, pvData);
			return bResult;
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
BOOL CIndexedData::CacheRead(CIndexedDataDescriptor* pcDescriptor)
{
	BOOL					bResult;
	CMemoryCacheAllocation	cPreAllocated;

	if (mbCaching)
	{
		cPreAllocated.Init(pcDescriptor->GetDataSize());
		if (mcDataCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
		{
			bResult = WriteEvictedData(cPreAllocated.GetEvictedArray());
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			bResult = mcDataCache.Allocate(pcDescriptor, &cPreAllocated);
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}
			
			bResult = mcDataFiles.Read(pcDescriptor, pcDescriptor->GetCache());
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
BOOL CIndexedData::CacheWrite(CIndexedDataDescriptor* pcDescriptor, void* pvData, BOOL* pbWritten)
{
	BOOL					bResult;
	CMemoryCacheAllocation	cPreAllocated;

	if (mbCaching)
	{
		cPreAllocated.Init(pcDescriptor->GetDataSize());
		if (mcDataCache.PreAllocate(&cPreAllocated))  //PreAllocate ensures there will be enough space in the cache.
		{
			*pbWritten = FALSE;
			bResult = WriteEvictedData(cPreAllocated.GetEvictedArray());
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			bResult = mcDataCache.Allocate(pcDescriptor, &cPreAllocated);
			if (!bResult)
			{
				cPreAllocated.Kill();
				return FALSE;
			}

			memcpy_fast(pcDescriptor->GetCache(), pvData, pcDescriptor->GetDataSize());

			EvictOverlappingFromCache(cPreAllocated.GetEvictedArray());
			cPreAllocated.Kill();
			return TRUE;
		}
		else
		{
			//There wasn't enough space in the cache... the object is written immediately.
			bResult = mcDataFiles.WriteData(pcDescriptor, pvData);
			*pbWritten = TRUE;

			cPreAllocated.Kill();
			return bResult;
		}
	}
	else
	{
		*pbWritten = TRUE;
		bResult = mcDataFiles.WriteData(pcDescriptor, pvData);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::EvictFromCache(CIndexedDataDescriptor* pcDescriptor)
{
	SIndexedCacheDescriptor*	psExisting;
	BOOL						bResult;

	if (pcDescriptor->IsCached())
	{
		psExisting = mcDataCache.GetHeader(pcDescriptor->GetCache());
		bResult = WriteEvictedData(pcDescriptor, psExisting);
		mcDataCache.Invalidate(psExisting);
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
BOOL CIndexedData::EvictFromCache(SIndexedCacheDescriptor* psExisting)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = mcIndices.Get(&cDescriptor, psExisting->oi);
	if (!bResult)
	{
		return FALSE;
	}

	cDescriptor.Cache(NULL);
	bResult = mcIndices.Set(&cDescriptor);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::EvictOverlappingFromCache(CArrayIntAndPointer* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psDesc;
	int							iType;
	BOOL						bResult;

	bResult = TRUE;
	for (i = 0; i < papsEvictedIndexedCacheDescriptors->NumElements(); i++)
	{
		if (papsEvictedIndexedCacheDescriptors->Get(i, (void**)&psDesc, &iType))
		{
			bResult &= EvictFromCache(psDesc);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::InvalidateData(CIndexedDataDescriptor* pcDescriptor)
{
	if (mbCaching)
	{
		mcDataCache.Invalidate(pcDescriptor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::WriteEvictedData(SIndexedCacheDescriptor* psCached)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

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
BOOL CIndexedData::WriteEvictedData(CIndexedDataDescriptor* pcDescriptor, SIndexedCacheDescriptor* psCached)
{
	void*	pvData;
	BOOL	bResult;

	if (psCached->iFlags & CACHE_DESCRIPTOR_FLAG_DIRTY)
	{
		pvData = RemapSinglePointer(psCached, sizeof(SIndexedCacheDescriptor));
		bResult = mcDataFiles.WriteData(pcDescriptor, pvData);
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
BOOL CIndexedData::WriteEvictedData(CArrayIntAndPointer* papsEvictedIndexedCacheDescriptors)
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
BOOL CIndexedData::ClearDescriptorCache(SIndexedCacheDescriptor* psCached)
{
	CIndexedDataDescriptor		cDescriptor;
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
unsigned int CIndexedData::Size(OIndex oi)
{
	BOOL						bResult;
	CIndexedDataDescriptor		cDescriptor;

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
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

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
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return SetData(&cDescriptor, pvData, uiTimeStamp);
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
BOOL CIndexedData::SetData(CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp)
{
	BOOL	bResult;
	BOOL	bUpdated;

	if (pcDescriptor->IsCached())
	{
		bUpdated = mcDataCache.Update(pcDescriptor, pvData);
		if (bUpdated && mbWriteThrough)
		{
			bResult = mcDataFiles.WriteData(pcDescriptor, pvData);
		}
		else
		{
			bResult = TRUE;
		}
		pcDescriptor->TimeStamp(uiTimeStamp);
		mcIndices.Set(pcDescriptor);
		return bResult;
	}
	else
	{
		bResult = Write(pcDescriptor, pvData, uiTimeStamp);
		mcIndices.Set(pcDescriptor);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (bResult)
	{
		return SetData(&cDescriptor, pvData, uiDataSize, uiTimeStamp);
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
BOOL CIndexedData::SetData(CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL	bResult;
	OIndex	oi;

	if (pcDescriptor->GetDataSize() == uiDataSize)
	{
		return SetData(pcDescriptor, pvData, uiTimeStamp);
	}
	else
	{
		oi = pcDescriptor->GetIndex();
		InvalidateData(pcDescriptor);
		pcDescriptor->Init(oi, uiDataSize);

		bResult = Write(pcDescriptor, pvData, uiTimeStamp);
		mcIndices.Set(pcDescriptor);
		return bResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL					bResult;
	CIndexedDataDescriptor	cDescriptor;

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
BOOL CIndexedData::CompareDiskToMemory(CIndexedDataDescriptor* pcDescriptor, void* pvData)
{
	//This function tells the disk whether it must update itself because the cached value has changed.
	//It also timestamps the descriptor of the changed data.
	unsigned int	uiDataSize;
	void*			pvTemp;

	if (pcDescriptor->HasFile())
	{
		uiDataSize = pcDescriptor->GetDataSize();
		pvTemp = mcTemp.Size(uiDataSize);

		mcDataFiles.Read(pcDescriptor, pvTemp);
		if (memcmp(pvTemp, pvData, uiDataSize) == 0)
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
BOOL CIndexedData::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return mcIndices.Get(pcDescriptor, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Get(OIndex oi, void* pvData)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		return FALSE;
	}

	bResult = GetData(&cDescriptor, pvData);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedData::Get(OIndex oi, int* piDataSize)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;
	filePos					iDataSize;
	void*					pvData;

	bResult = GetDescriptor(oi, &cDescriptor);
	if (!bResult)
	{
		return NULL;
	}

	iDataSize = cDescriptor.GetDataSize();
	SafeAssign(piDataSize, (int)iDataSize);

	pvData = malloc((size_t)iDataSize);
	if (pvData)
	{
		bResult = GetData(&cDescriptor, pvData);
		if (bResult)
		{
			return pvData;
		}
		else
		{
			free(pvData);
			return NULL;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::GetData(CIndexedDataDescriptor* pcDescriptor, void* pvData)
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
				return mcDataFiles.Read(pcDescriptor, pvData);
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
BOOL CIndexedData::Contains(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = GetDescriptor(oi, &cDescriptor);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Remove(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	//This is not correct.  Removed objects must be marked as removed until all transactions are finished.
	bResult = mcIndices.Get(&cDescriptor, oi);
	if (bResult)
	{
		if (cDescriptor.IsCached())
		{
			InvalidateData(&cDescriptor);
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
	mcDurableFileControl.Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::DurableEnd(void)
{
	mcDurableFileControl.End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Flush(BOOL bClearCache)
{
	SIndexedCacheDescriptor*	psCached;
	BOOL						bAnyFailed;
	BOOL						bResult;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		psCached = mcDataCache.StartIteration();
		while (psCached)
		{
			bResult = WriteEvictedData(psCached);
			if (!bResult)
			{
				bAnyFailed = TRUE;
			}
			psCached = mcDataCache.Iterate(psCached);
		}
		if (bClearCache && !bAnyFailed)
		{
			mcDataCache.Clear();
		}
		return !bAnyFailed;
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
BOOL CIndexedData::Uncache(void)
{
	SIndexedCacheDescriptor*	psCached;
	BOOL						bAnyFailed;
	BOOL						bResult;

	if (mbCaching)
	{
		bAnyFailed = FALSE;
		psCached = mcDataCache.StartIteration();
		while (psCached)
		{
			bResult = ClearDescriptorCache(psCached);
			if (!bResult)
			{
				bAnyFailed = TRUE;
			}
			psCached = mcDataCache.Iterate(psCached);
		}
		mcDataCache.Clear();
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
		return mcDataCache.NumCached();
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
int CIndexedData::NumCached(int iSize)
{
	if (mbCaching)
	{
		return mcDataCache.NumCached(iSize);
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
	return mcDataFiles.NumFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedData::NumInFile(int iDataSize)
{
	return mcDataFiles.NumInFile(iDataSize);
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
unsigned int CIndexedData::TestGetCachedObjectSize(OIndex oi)
{
	SIndexedCacheDescriptor*	psDesc;

	psDesc = mcDataCache.TestGetDescriptor(oi);
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
int CIndexedData::TestNumCachedIndexes(void) { return (int)mcIndices.NumCachedDatas(); }

int CIndexedData::TestIndexedDescriptorsLength(void) { return (int)mcIndices.Length(); }

int CIndexedData::TestNumIgnoredCacheElements(void) { return mcDataCache.NumIgnored(); }

CIndexAccess* CIndexedData::TestGetIndexAccess(void) { return mcIndicesAccess.GetAccess(); }

void CIndexedData::AddFile(CDurableFile* pcFile) { mcDurableFileControl.AddFile(pcFile); }

CDurableFileController* CIndexedData::GetDurableFileControl(void) { return &mcDurableFileControl; }

BOOL CIndexedData::IsCaching(void) { return mbCaching; } 

BOOL CIndexedData::IsTransient(void) { return mbTransient; }

BOOL CIndexedData::IsDurable(void) { return mcDurableFileControl.IsDurable(); }

