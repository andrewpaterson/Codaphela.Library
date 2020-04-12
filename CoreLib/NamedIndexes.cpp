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
void CNamedIndexes::Init(CDurableFileController* pcDurableFileController, size_t uiCutoff, EIndexWriteThrough eWriteThrough)
{
	Init(pcDurableFileController, uiCutoff, eWriteThrough, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::Init(CDurableFileController* pcDurableFileController, size_t uiCutoff, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback)
{
	mcIndexTree.Init(pcDurableFileController, uiCutoff, pcEvictionCallback, &mcEvictionStrategy, this, eWriteThrough, IKR_No);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Kill(void)
{
	return mcIndexTree.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize)
{
	if (iDataSize != sizeof(OIndex))
	{
		return gcLogger.Error2(__METHOD__, "Data evicted was not an OIndex.", NULL);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Put(char* szName, OIndex oi, BOOL bFailOnExisting)
{
	int		iKeySize;
	BOOL	bExists;

	if (szName)
	{
		iKeySize = strlen(szName);
		if (bFailOnExisting)
		{
			bExists = mcIndexTree.HasKey(szName, iKeySize);
			if (bExists)
			{
				return FALSE;
			}
		}
		return mcIndexTree.Put(szName, iKeySize, &oi, sizeof(OIndex));
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Put(CChars* szName, OIndex oi, BOOL bFailOnExisting)
{
	BOOL	bExists;

	if (bFailOnExisting)
	{
		bExists = mcIndexTree.HasKey(szName->Text(), szName->Length());
		if (bExists)
		{
			return FALSE;
		}
	}
	return mcIndexTree.Put(szName->Text(), szName->Length(), &oi, sizeof(OIndex));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::GetIndex(char* szName)
{
	int		iKeySize;
	OIndex	iResult;
	BOOL	bExists;

	if (szName)
	{
		iKeySize = strlen(szName);
		bExists = mcIndexTree.Get(szName, iKeySize, &iResult, NULL);
		if (bExists)
		{
			return iResult;
		}
	}
	return INVALID_O_INDEX;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CNamedIndexes::GetIndex(CChars* szName)
{
	OIndex	iResult;
	BOOL	bExists;

	bExists = mcIndexTree.Get(szName->Text(), szName->Length(), &iResult, NULL);
	if (bExists)
	{
		return iResult;
	}
	return INVALID_O_INDEX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(char* szName)
{
	int iKeySize;

	if (szName)
	{
		iKeySize = strlen(szName);
		return mcIndexTree.Remove(szName, iKeySize);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Remove(CChars* szName)
{
	return mcIndexTree.Remove(szName->Text(), szName->Length());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CNamedIndexes::NumElements(void)
{
	return mcIndexTree.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::Flush(void) 
{
	return mcIndexTree.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short CNamedIndexes::IndexTreeDataSize(unsigned short uiSourceSize)
{
	if (uiSourceSize != 0)
	{
		return sizeof(OIndex);
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
BOOL CNamedIndexes::IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize)
{
	memcpy_fast(pvDataBuffer, pvSource, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CNamedIndexes::IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize)
{
	memset_fast(pvDest, 0, uiDestDataSize);
	memcpy_fast(pvDest, pvDataBuffer, iFileDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNamedIndexes::Dump(void)
{
	mcIndexTree.Dump();
}

