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
void CIndexedData::Init(char* szWorkingDirectory, char* szRewriteDirectory, size_t uiDataCacheSize, size_t uiIndexCacheSize, BOOL bWriteThrough)
{
	CIndexedConfig	cConfig;

	cConfig.OptimiseForStreaming(szWorkingDirectory);
	cConfig.mszRewriteDirectory = szRewriteDirectory;
	cConfig.SetDataCacheSize(uiDataCacheSize);
	cConfig.SetIndexCacheSize(uiIndexCacheSize);
	cConfig.SetWriteThrough(bWriteThrough);

	mcDurableFileControl.Init(cConfig.mszWorkingDirectory, cConfig.mszRewriteDirectory);

	mbWriteThrough = cConfig.mbWriteThrough;

	mcDurableFileControl.Begin();

	InitIndices(&mcDurableFileControl, cConfig.mbDirtyTesting, uiIndexCacheSize, cConfig.mbWriteThrough);
	mcData.Init(&mcDurableFileControl, "DAT", "Files.IDX", "_Files.IDX", cConfig.miDataCacheSize, cConfig.mbWriteThrough, this);

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
void CIndexedData::InitIndices(CDurableFileController* pcDurableFileControl, BOOL bDirtyTesting, size_t uiCutoff, BOOL bWriteThrough)
{
	mcIndices.Init(this, pcDurableFileControl, bDirtyTesting, uiCutoff, bWriteThrough);
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
BOOL CIndexedData::UpdateDescriptorCache(OIndex oi, void* pvCache)
{
	return mcIndices.SetCache(pvCache, oi);
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
BOOL CIndexedData::Flush(BOOL bClearCache)
{
	BOOL bRresult;

	if (!mbWriteThrough)
	{
		bRresult = mcIndices.Flush();
		bRresult &= mcData.Flush(bClearCache);
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
	return CIndexedDataCommon::DescriptorsEvicted(papsEvictedIndexedCacheDescriptors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedData::TestNumCachedIndexes(void) { return (int)mcIndices.NumCachedDatas(); }
CDurableFileController* CIndexedData::GetDurableFileControl(void) { return &mcDurableFileControl; }
BOOL CIndexedData::IsDurable(void) { return mcDurableFileControl.IsDurable(); }

