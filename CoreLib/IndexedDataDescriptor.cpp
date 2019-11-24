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
void CIndexedDataDescriptor::Init(unsigned int uiDataSize)
{
	//This is memset to zero to ensure unused bytes due to compiler struct alignment are set to zero also.
	memset(this, 0, sizeof(CIndexedDataDescriptor));

	msFileDescriptor.muiDataSize = uiDataSize;

	msFileDescriptor.mcFileIndex.Init();

	mpvCache = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(unsigned int uiDataSize, CFilePosIndex* pcFilePosIndex)
{
	//This is memset to zero to ensure unused bytes due to compiler struct alignment are set to zero also.
	memset(this, 0, sizeof(CIndexedDataDescriptor));

	msFileDescriptor.muiDataSize = uiDataSize;

	msFileDescriptor.mcFileIndex.Init(pcFilePosIndex->miFile, pcFilePosIndex->mulliFilePos);

	mpvCache = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(unsigned int uiDataSize, void* pvCache)
{
	//This is memset to zero to ensure unused bytes due to compiler struct alignment are set to zero also.
	memset(this, 0, sizeof(CIndexedDataDescriptor));

	msFileDescriptor.muiDataSize = uiDataSize;

	msFileDescriptor.mcFileIndex.Init();

	mpvCache = pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Init(unsigned int uiDataSize, CFilePosIndex* pcFilePosIndex, void* pvCache)
{
	//This is memset to zero to ensure unused bytes due to compiler struct alignment are set to zero also.
	memset(this, 0, sizeof(CIndexedDataDescriptor));

	msFileDescriptor.muiDataSize = uiDataSize;

	msFileDescriptor.mcFileIndex.Init(pcFilePosIndex->miFile, pcFilePosIndex->mulliFilePos);

	mpvCache = pvCache;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataDescriptor::GetDataSize(void)
{
	return msFileDescriptor.muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::SetDataSize(unsigned int uiDataSize)
{
	msFileDescriptor.muiDataSize = uiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::Cache(void* pvCache)
{
	mpvCache = pvCache;
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
unsigned int CIndexedDataDescriptor::GetDataIndexInFile(void)
{
	return (unsigned int)(msFileDescriptor.mcFileIndex.mulliFilePos / msFileDescriptor.muiDataSize);
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
	return bUpdated;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::GetFileDataIndex(CFileDataIndex* pcFileDataIndex)
{
	pcFileDataIndex->Init(GetFileIndex(), GetDataIndexInFile());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataDescriptor::GetFileIndex(void) { return msFileDescriptor.mcFileIndex.miFile; }
filePos CIndexedDataDescriptor::GetPositionInFile(void) { return msFileDescriptor.mcFileIndex.mulliFilePos; }
CFilePosIndex* CIndexedDataDescriptor::GetFilePosIndex(void) { return &msFileDescriptor.mcFileIndex; }

