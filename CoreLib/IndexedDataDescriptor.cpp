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

	muiDataSize = uiDataSize;

	mcFileIndex.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataDescriptor::IsAllocated(void)
{
	return muiDataSize != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataDescriptor::GetDataSize(void)
{
	return muiDataSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDataDescriptor::SetDataSize(unsigned int uiDataSize)
{
	muiDataSize = uiDataSize;
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
	return mcFileIndex.HasFile();
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
	mcFileIndex.SetIndex(iFileIndex, iIndexInFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexedDataDescriptor::GetFileIndex(void)
{
	return mcFileIndex.miFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexedDataDescriptor::GetPositionInFile(void)
{
	return mcFileIndex.mulliFilePos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexedDataDescriptor::GetDataIndexInFile(void)
{
	return (unsigned int)(mcFileIndex.mulliFilePos / muiDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDataDescriptor::Update(CIndexedDataDescriptor* pcNew)
{
	BOOL	bUpdated;

	bUpdated = FALSE;
	if (pcNew->mcFileIndex.miFile != mcFileIndex.miFile)
	{
		mcFileIndex.miFile = pcNew->mcFileIndex.miFile;
		bUpdated = TRUE;
	}
	if (pcNew->mcFileIndex.mulliFilePos != mcFileIndex.mulliFilePos)
	{
		mcFileIndex.mulliFilePos = pcNew->mcFileIndex.mulliFilePos;
		bUpdated = TRUE;
	}
	if (pcNew->muiDataSize != muiDataSize)
	{
		muiDataSize = pcNew->muiDataSize;
		bUpdated = TRUE;
	}

	mpvCache = pcNew->GetCache();
	return bUpdated;
}

