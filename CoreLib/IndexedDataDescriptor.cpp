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
#include "IndexedDataDescriptor.h"
#include "BaseLib/IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(unsigned int uiDataSize, CFilePosIndex* pcFilePosIndex)
{
	msFileDescriptor.muiDataSize = uiDataSize;
	msFileDescriptor.mcFileIndex.Init(pcFilePosIndex->miFile, pcFilePosIndex->mulliFilePos);

	mpvCache = NULL;
	muiCacheDataSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(unsigned int uiDataSize, void* pvCache)
{
	msFileDescriptor.muiDataSize = 0;
	msFileDescriptor.mcFileIndex.Init();

	mpvCache = pvCache;
	muiCacheDataSize = uiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(unsigned int uiFileDataSize, CFilePosIndex* pcFilePosIndex, unsigned int uiCacheDataSize, void* pvCache)
{
	if (pcFilePosIndex->HasFile())
	{
		msFileDescriptor.muiDataSize = uiFileDataSize;
	}
	else
	{
		msFileDescriptor.muiDataSize = 0;
	}
	msFileDescriptor.mcFileIndex.Init(pcFilePosIndex->miFile, pcFilePosIndex->mulliFilePos);

	if (mpvCache != NULL)
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
unsigned int CIndexedDataDescriptor::GetFileDataSize(void)
{
	return msFileDescriptor.muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataDescriptor::GetCacheDataSize(void)
{
	return muiCacheDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataDescriptor::GetDataSize(void)
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
void CIndexedDataDescriptor::Cache(void* pvCache, unsigned int uiDataSize)
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
BOOL CIndexedDataDescriptor::HasFile(void)
{
	return msFileDescriptor.mcFileIndex.HasFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataDescriptor::IsCached(void)
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
BOOL CIndexedDataDescriptor::Update(CIndexedDataDescriptor* pcNew)
{
	BOOL	bUpdated;

	bUpdated = FALSE;
	if (pcNew->msFileDescriptor.mcFileIndex.miFile != msFileDescriptor.mcFileIndex.miFile)
	{
		msFileDescriptor.mcFileIndex.miFile = pcNew->msFileDescriptor.mcFileIndex.miFile;
		bUpdated = TRUE;
	}
	if (pcNew->msFileDescriptor.mcFileIndex.mulliFilePos != msFileDescriptor.mcFileIndex.mulliFilePos)
	{
		msFileDescriptor.mcFileIndex.mulliFilePos = pcNew->msFileDescriptor.mcFileIndex.mulliFilePos;
		bUpdated = TRUE;
	}
	if (pcNew->msFileDescriptor.muiDataSize != msFileDescriptor.muiDataSize)
	{
		msFileDescriptor.muiDataSize = pcNew->msFileDescriptor.muiDataSize;
		bUpdated = TRUE;
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

