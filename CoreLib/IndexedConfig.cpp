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
void CIndexedConfig::Manual(char* szWorkingDirectory, char* szRewriteDirectory, BOOL bDirtyTesting, EIndexWriteThrough eWriteThrough, size_t iDataCacheSize, size_t iIndexCacheSize)
{
	mszWorkingDirectory = szWorkingDirectory;
	mszRewriteDirectory = szRewriteDirectory;
	mbDirtyTesting = bDirtyTesting;
	meWriteThrough = eWriteThrough;
	miDataCacheSize = iDataCacheSize;
	miIndexCacheSize = iIndexCacheSize;
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
	mszRewriteDirectory = NULL;
	mbDirtyTesting = TRUE;
	meWriteThrough = IWT_No;
	miDataCacheSize = 0;
	miIndexCacheSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetDirtyTesting(BOOL bDirtyTesting)
{
	mbDirtyTesting = bDirtyTesting;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetWriteThrough(EIndexWriteThrough eWriteThrough)
{
	meWriteThrough = eWriteThrough;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetDataCacheSize(size_t iObjectsCacheSize)
{
	miDataCacheSize = iObjectsCacheSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedConfig::SetIndexCacheSize(size_t iIndexCacheSize)
{

}
