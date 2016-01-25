/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/Numbers.h"
#include "IndexedDataDescriptor.h"
#include "IndexedConfig.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::Manual(char* szWorkingDirectory, 
							BOOL bDurable, 
							BOOL bDirtyTesting, 
							BOOL bWriteThrough, 
							int iIndicesSecondLevelWidth, 
							int iIndicesThirdLevelWidth, 
							int iIndicesNumSecondLevelChunks, 
							int iIndicesNumThirdLevelChunks, 
							int iIndicesMemoryChunkSize, 
							int iObjectsCacheSize)
{
	mszWorkingDirectory = szWorkingDirectory;
	mbDurable = bDurable;
	mbDirtyTesting = bDirtyTesting;
	mbWriteThrough = bWriteThrough;
	miIndicesSecondLevelWidth = iIndicesSecondLevelWidth;
	miIndicesThirdLevelWidth = iIndicesThirdLevelWidth;
	miIndicesNumSecondLevelChunks = iIndicesNumSecondLevelChunks; 
	miIndicesNumThirdLevelChunks = iIndicesNumThirdLevelChunks;
	miIndicesMemoryChunkSize = iIndicesMemoryChunkSize;
	miObjectsCacheSize = iObjectsCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::OptimiseForStreaming(char* szWorkingDirectory)
{
	//This configuration is used for streaming in large games maps etc...
	//Writing and read caching are not important.

	mszWorkingDirectory = szWorkingDirectory;
	mbDurable = FALSE;
	mbDirtyTesting = TRUE;
	mbWriteThrough = FALSE;
	miIndicesSecondLevelWidth = 131072;
	miIndicesThirdLevelWidth = 2048;
	miIndicesNumSecondLevelChunks = 8; 
	miIndicesNumThirdLevelChunks = 1024;
	miIndicesMemoryChunkSize = miIndicesNumThirdLevelChunks * sizeof(CIndexedDataDescriptor);
	miObjectsCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::OptimiseForGameGraph(char* szWorkingDirectory)
{
	//This configuration is used where write performance is important.
	//The database will become corrupt if it is not closed.

	mszWorkingDirectory = szWorkingDirectory;
	mbDurable = FALSE;
	mbDirtyTesting = TRUE;
	mbWriteThrough = FALSE;
	miIndicesSecondLevelWidth = 131072;
	miIndicesThirdLevelWidth = 2048;
	miIndicesNumSecondLevelChunks = 8; 
	miIndicesNumThirdLevelChunks = 1024;
	miIndicesMemoryChunkSize = miIndicesNumThirdLevelChunks * sizeof(CIndexedDataDescriptor);
	miObjectsCacheSize = 128 MB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::OptimiseForTransactions(char* szWorkingDirectory)
{
	//This configuration is used where database consistency is important.
	//Changes are immediately written to disk atomically.

	mszWorkingDirectory = szWorkingDirectory;
	mbDurable = TRUE;
	mbDirtyTesting = TRUE;
	mbWriteThrough = TRUE;
	miIndicesSecondLevelWidth = 131072;
	miIndicesThirdLevelWidth = 2048;
	miIndicesNumSecondLevelChunks = 8; 
	miIndicesNumThirdLevelChunks = 1024;
	miIndicesMemoryChunkSize = miIndicesNumThirdLevelChunks * sizeof(CIndexedDataDescriptor);
	miObjectsCacheSize = 128 MB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetDurable(BOOL bDurable)
{
	if (bDurable)
	{
		mbDurable = TRUE;
		mbDirtyTesting = TRUE;
		mbWriteThrough = TRUE;
	}
	else
	{
		mbDurable = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetDirtyTesting(BOOL bDirtyTesting)
{
	if (!mbDirtyTesting)
	{
		mbDirtyTesting = FALSE;
		mbDurable = FALSE;
	}
	else
	{
		mbDirtyTesting = TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetWriteThrough(BOOL bWriteThrough)
{
	if (bWriteThrough)
	{
		mbWriteThrough = TRUE;
	}
	else
	{
		mbWriteThrough = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::DisableObjectCaching(void)
{
	miObjectsCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetObjectCacheSize(int iObjectsCacheSize)
{
	miObjectsCacheSize = iObjectsCacheSize;
}

