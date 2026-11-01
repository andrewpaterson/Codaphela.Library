/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "IndexedDataDescriptor.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(lsize uiDataSize, CFilePosIndex* pcFilePosIndex)
{
	//The descriptor is written to the index file whole: its padding must be zero.
	memset(this, 0, sizeof(CIndexedDataDescriptor));
	msFileDescriptor.muiDataSize = (uint32)uiDataSize;
	msFileDescriptor.mcFileIndex.Init(pcFilePosIndex->miFile, pcFilePosIndex->mulliFilePos);

	mpvCache = NULL;
	muiCacheDataSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(lsize uiDataSize, void* pvCache)
{
	memset(this, 0, sizeof(CIndexedDataDescriptor));
	msFileDescriptor.muiDataSize = 0;
	msFileDescriptor.mcFileIndex.Init();

	mpvCache = pvCache;
	muiCacheDataSize = uiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(lsize uiFileDataSize, CFilePosIndex* pcFilePosIndex, lsize uiCacheDataSize, void* pvCache)
{
	memset(this, 0, sizeof(CIndexedDataDescriptor));
	if (pcFilePosIndex->HasFile())
	{
		msFileDescriptor.muiDataSize = (uint32)uiFileDataSize;
	}
	else
	{
		msFileDescriptor.muiDataSize = 0;
	}
	msFileDescriptor.mcFileIndex.Init(pcFilePosIndex->miFile, pcFilePosIndex->mulliFilePos);

	if (pvCache != NULL)
	{
		muiCacheDataSize = uiCacheDataSize;
	}
	else
	{
		muiCacheDataSize = 0;
	}
	mpvCache = pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CIndexedDataDescriptor::GetFileDataSize(void)
{
	return msFileDescriptor.muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CIndexedDataDescriptor::GetCacheDataSize(void)
{
	return muiCacheDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CIndexedDataDescriptor::GetDataSize(void)
{
	if (mpvCache != NULL)
	{
		return muiCacheDataSize;
	}
	else if (HasFile())
	{
		return msFileDescriptor.muiDataSize;
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
void CIndexedDataDescriptor::Cache(void* pvCache, lsize uiDataSize)
{
	mpvCache = pvCache;
	muiCacheDataSize = uiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::ClearCache(void)
{
	mpvCache = NULL;
	muiCacheDataSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataDescriptor::HasFile(void)
{
	return msFileDescriptor.mcFileIndex.HasFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataDescriptor::IsCached(void)
{
	return FixBool(mpvCache);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexedDataDescriptor::GetCache(void)
{
	return mpvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::SetIndexes(int iFileIndex, filePos iIndexInFile)
{
	msFileDescriptor.mcFileIndex.SetIndex(iFileIndex, iIndexInFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexedDataDescriptor::Update(CIndexedDataDescriptor* pcNew)
{
	bool	bUpdated;

	bUpdated = false;
	if (pcNew->msFileDescriptor.mcFileIndex.miFile != msFileDescriptor.mcFileIndex.miFile)
	{
		msFileDescriptor.mcFileIndex.miFile = pcNew->msFileDescriptor.mcFileIndex.miFile;
		bUpdated = true;
	}
	if (pcNew->msFileDescriptor.mcFileIndex.mulliFilePos != msFileDescriptor.mcFileIndex.mulliFilePos)
	{
		msFileDescriptor.mcFileIndex.mulliFilePos = pcNew->msFileDescriptor.mcFileIndex.mulliFilePos;
		bUpdated = true;
	}
	if (pcNew->msFileDescriptor.muiDataSize != msFileDescriptor.muiDataSize)
	{
		msFileDescriptor.muiDataSize = pcNew->msFileDescriptor.muiDataSize;
		bUpdated = true;
	}

	mpvCache = pcNew->GetCache();
	muiCacheDataSize = pcNew->muiCacheDataSize;
	return bUpdated;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CFileDataIndex CIndexedDataDescriptor::GetFileDataIndex(void)
{
	CFileDataIndex	cDataIndex;

	cDataIndex = msFileDescriptor.mcFileIndex.ToFileDataIndex(msFileDescriptor.muiDataSize);
	return cDataIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataDescriptor::GetFileIndex(void) { return msFileDescriptor.mcFileIndex.miFile; }
filePos CIndexedDataDescriptor::GetPositionInFile(void) { return msFileDescriptor.mcFileIndex.mulliFilePos; }
CFilePosIndex* CIndexedDataDescriptor::GetFilePosIndex(void) { return &msFileDescriptor.mcFileIndex; }

