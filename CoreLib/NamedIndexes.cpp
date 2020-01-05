/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "NamedIndexes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::Init(CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse)
{
	Init(pcDurableFileController, bDirtyTesting, uiCutoff, eWriteThrough, eKeyReverse, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::Init(CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CEvictionCallback* pcEvictionCallback)
{
	//CNamedIndexesEvictionCallback not a CEvictionCallback.
	CEvictionCallback*	pcCallback;

	if (pcEvictionCallback)
	{
		mcEvictionCallbackWrapper.Init(pcEvictionCallback, this);
		pcCallback = &mcEvictionCallbackWrapper;
	}
	else
	{
		mcEvictionCallbackWrapper.Init(NULL, NULL);
		pcCallback = this;
	}
	mcIndexTree.Init(pcDurableFileController, uiCutoff, pcCallback, &mcEvictionStrategy, &mcDescriptorsCallback, eWriteThrough, eKeyReverse);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Kill(void)
{
	mcIndexTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	if (iDataSize != sizeof(OIndex))
	{
		return gcLogger.Error2("Data evicted was not an OIndex.", NULL);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Add(OIndex oi, char* szName, BOOL bFailOnExisting)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Add(OIndex oi, CChars* szName, BOOL bFailOnExisting)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::GetIndex(char* szName)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::GetIndex(CChars* szName)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(char* szName)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(CChars* szName)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CNamedIndexes::NumElements(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Flush(void) 
{
}

