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
void CIndexedData::Init(char* szWorkingDirectory, char* szRewriteDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough)
{
	Init(szWorkingDirectory, szRewriteDirectory, uiDataCacheSize, uiIndexCacheSize, eWriteThrough, NULL, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::Init(char* szWorkingDirectory, char* szRewriteDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, EIndexWriteThrough eWriteThrough, CEvictionCallback* pcIndexEvictionUserCallback, CEvictionCallback* pcEvictionUserCallback)
{
	CIndexedDataCommon::Init(pcEvictionUserCallback);

	mcDurableFileControl.Init(szWorkingDirectory, szRewriteDirectory);

	meWriteThrough = eWriteThrough;

	mcDurableFileControl.Begin();

	InitIndices(&mcDurableFileControl, TRUE, uiIndexCacheSize, eWriteThrough, pcIndexEvictionUserCallback);
	mcData.Init(&mcDurableFileControl, "DAT", "Files.IDX", "_Files.IDX", uiDataCacheSize, eWriteThrough, this);

	mcDurableFileControl.End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::Kill(void)
{
	if (mcDurableFileControl.IsDurable())
	{
		mcDurableFileControl.Begin();
		mcData.Flush(FALSE);
		mcDurableFileControl.End();
	}
	else
	{
		mcDurableFileControl.Begin();
		Flush(TRUE);
		mcDurableFileControl.End();
	}

	mcDurableFileControl.Kill();

	mcData.Kill();

	mcIndices.Kill();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedData::InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting, size_t uiCutoff, EIndexWriteThrough eWriteThrough, CEvictionCallback* pcIndexEvictionUserCallback)
{
	mcIndices.Init(this, pcDurableFileControl, bDirtyTesting, uiCutoff, eWriteThrough, IKR_Yes, pcIndexEvictionUserCallback);
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
BOOL CIndexedData::Flush(BOOL bClearDataCache)
{
	BOOL bRresult;

	if (!meWriteThrough)
	{
		bRresult = mcIndices.Flush();
		bRresult &= mcData.Flush(bClearDataCache);
		return bRresult;
	}
	return TRUE;
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
			psDataDescriptor = (SIndexedCacheDescriptor*)RemapSinglePointer(cKeyDescriptor.GetCache(), -(int)(sizeof(SIndexedCacheDescriptor)));
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
BOOL CIndexedData::DurableBegin(void)
{
	return mcDurableFileControl.Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::DurableEnd(void)
{
	return mcDurableFileControl.End();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedData::DescriptorsEvicted(CArrayVoidPtr* papsEvictedIndexedCacheDescriptors)
{
	if (papsEvictedIndexedCacheDescriptors->NumElements() > 0)
	{
		return CIndexedDataCommon::DescriptorsEvicted(papsEvictedIndexedCacheDescriptors);
	}
	return TRUE;
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
void CIndexedData::DumpIndex(void)
{
	mcIndices.Dump();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::NumIndicesCached(void) { return (int)mcIndices.NumCachedDatas(); }
CDurableFileController* CIndexedData::GetDurableFileControl(void) { return &mcDurableFileControl; }
BOOL CIndexedData::IsDurable(void) { return mcDurableFileControl.IsDurable(); }
size_t CIndexedData::GetIndiciesSystemMemorySize(void) { return mcIndices.GetSystemMemorySize(); }
size_t CIndexedData::GetDataSystemMemorySize(void) { return mcData.GetSystemMemorySize(); }
unsigned char CIndexedData::GetRootFlags(void) { return mcIndices.GetRootFlags();  }

