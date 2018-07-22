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
void CIndexedData::Init(char* szWorkingDirectory, char* szRewriteDirectory, unsigned int uiCacheSize)
{
	CIndexedConfig	cConfig;

	cConfig.OptimiseForStreaming(szWorkingDirectory);
	cConfig.mszRewriteDirectory = szRewriteDirectory;
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
	
	mcDurableFileControl.Init(pcConfig->mszWorkingDirectory, pcConfig->mszRewriteDirectory);

	mcDurableFileControl.Begin();

	InitIndices(pcConfig->mbDirtyTesting);
	mcData.Init(&mcDurableFileControl, "DAT", "Files.IDX", "_Files.IDX", pcConfig->miObjectsCacheSize, pcConfig->mbWriteThrough, this);

	mcDurableFileControl.End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Kill(void)
{
	if (mcDurableFileControl.IsDurable())
	{
		DurableBegin();
		mcData.Flush(FALSE);
		DurableEnd();
	}
	else
	{
		DurableBegin();
		Flush(TRUE);
		DurableEnd();
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

	mcData.Kill();

	mcIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::InitIndices(BOOL bDirtyTesting)
{
	mcIndices.Init(&mcDurableFileControl, bDirtyTesting);
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
BOOL CIndexedData::EvictFromCache(CIndexedDataDescriptor* pcDescriptor)
{
	return mcData.EvictFromCache(pcDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::EvictFromCache(OIndex oi)
{
	CIndexedDataDescriptor	cDescriptor;
	BOOL					bResult;

	bResult = mcIndices.Get(&cDescriptor, oi);
	if (!bResult)
	{
		return FALSE;
	}

	cDescriptor.Cache(NULL);
	bResult = mcIndices.Set(&cDescriptor, oi);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	int							i;
	SIndexedCacheDescriptor*	psDesc;
	BOOL						bResult;
	int							iNumElements;

	bResult = TRUE;
	iNumElements = papsEvictedIndexedCacheDescriptors->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psDesc = (SIndexedCacheDescriptor*)papsEvictedIndexedCacheDescriptors->GetPtr(i);
		if (psDesc != NULL)
		{
			bResult &= EvictFromCache(psDesc->oi);
		}
	}
	return bResult;
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
	cDescriptor.Init(iDataSize);

	bResult = mcData.SetData(oi, &cDescriptor, pvData, uiTimeStamp);
	mcIndices.Set(&cDescriptor, oi);
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
		return SetData(oi, &cDescriptor, pvData, uiTimeStamp);
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
BOOL CIndexedData::SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiTimeStamp)
{
	BOOL	bResult;

	if (pcDescriptor->IsCached())
	{
		bResult = mcData.SetData(pcDescriptor, pvData);
		mcIndices.Set(pcDescriptor, oi);
		return bResult;
	}
	else
	{
		bResult = mcData.SetData(oi, pcDescriptor, pvData, uiTimeStamp);
		mcIndices.Set(pcDescriptor, oi);
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
		return SetData(oi, &cDescriptor, pvData, uiDataSize, uiTimeStamp);
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
BOOL CIndexedData::SetData(OIndex oi, CIndexedDataDescriptor* pcDescriptor, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp)
{
	BOOL	bResult;

	if (pcDescriptor->GetDataSize() == uiDataSize)
	{
		return SetData(oi, pcDescriptor, pvData, uiTimeStamp);
	}
	else
	{
		mcData.InvalidateData(pcDescriptor);
		pcDescriptor->Init(uiDataSize);

		bResult = mcData.SetData(oi, pcDescriptor, pvData, uiTimeStamp);
		mcIndices.Set(pcDescriptor, oi);
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
BOOL CIndexedData::GetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return mcIndices.Get(pcDescriptor, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::SetDescriptor(OIndex oi, CIndexedDataDescriptor* pcDescriptor)
{
	return mcIndices.Set(pcDescriptor, oi);
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

	bResult = mcData.GetData(oi, &cDescriptor, pvData);
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
		bResult = mcData.GetData(oi, &cDescriptor, pvData);
		if (bResult)
		{
			return pvData;
		}
		else
		{
			SafeFree(pvData);
			return NULL;
		}
	}
	return NULL;
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

	bResult = mcIndices.Get(&cDescriptor, oi);
	if (bResult)
	{
		if (cDescriptor.IsCached())
		{
			mcData.InvalidateData(&cDescriptor);
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
	BOOL bRresult;
	bRresult = mcIndices.Flush();
	bRresult &= mcData.Flush(bClearCache);
	return bRresult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::NumCached(void)
{
	return mcData.NumCached();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::NumCached(int iSize)
{
	return mcData.NumCached(iSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::NumFiles(void)
{
	return mcData.NumFiles();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedData::NumData(int iDataSize)
{
	return mcData.NumData(iDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int64 CIndexedData::NumElements(void)
{
	return mcIndices.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedData::TestGetCachedObjectSize(OIndex oi) {	return  mcData.TestGetCachedObjectSize(oi); }
int CIndexedData::TestNumCachedIndexes(void) { return (int)mcIndices.NumCachedDatas(); }
int CIndexedData::TestNumIgnoredCacheElements(void) { return mcData.TestNumIgnoredCacheElements(); }
CDurableFileController* CIndexedData::GetDurableFileControl(void) { return &mcDurableFileControl; }
BOOL CIndexedData::IsCaching(void) { return mcData.IsCaching(); } 
BOOL CIndexedData::IsDurable(void) { return mcDurableFileControl.IsDurable(); }

