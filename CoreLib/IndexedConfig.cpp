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
void CIndexedConfig::Manual(char* szWorkingDirectory, char* szRewriteDirectory, BOOL bDirtyTesting, BOOL bWriteThrough, int iObjectsCacheSize)
{
	mszWorkingDirectory = szWorkingDirectory;
	mszRewriteDirectory = szRewriteDirectory;
	mbDirtyTesting = bDirtyTesting;
	mbWriteThrough = bWriteThrough;
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
	mszRewriteDirectory = NULL;
	mbDirtyTesting = TRUE;
	mbWriteThrough = FALSE;
	miObjectsCacheSize = 0;
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
void CIndexedConfig::SetWriteThrough(BOOL bWriteThrough)
{
	mbWriteThrough = bWriteThrough;
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

