/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include <stdlib.h>
#include "BaseLib/FileUtil.h"
#include "SpillStore.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemorySpillStore::Init(void)
{
	mcSpills.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMemorySpillStore::Kill(void)
{
	size	i;

	for (i = 0; i < mcSpills.NumElements(); i++)
	{
		free(mcSpills.GetValue(i)->pvData);
	}
	mcSpills.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMemorySpillStore::MaxDataSize(void) const
{
	return SPILL_STORE_MAX_DATA;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemorySpillStore::Put(const void* pvKey, size uiKeySize, const void* pvData, size uiDataSize)
{
	SMemorySpill*	psSpill;
	void*			pvCopy;

	pvCopy = malloc((uiDataSize > 0) ? uiDataSize : 1);
	if (pvCopy == NULL)
	{
		return false;
	}
	psSpill = mcSpills.FindOrAdd((const char*)pvKey, uiKeySize);
	if (psSpill == NULL)
	{
		free(pvCopy);
		return false;
	}
	memcpy(pvCopy, pvData, uiDataSize);
	free(psSpill->pvData);
	psSpill->pvData = pvCopy;
	psSpill->uiSize = uiDataSize;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemorySpillStore::Get(const void* pvKey, size uiKeySize, void* pvData, size uiMaxDataSize, size* puiDataSize)
{
	SMemorySpill*	psSpill;

	psSpill = mcSpills.Get((const char*)pvKey, uiKeySize);
	if ((psSpill == NULL) || (psSpill->pvData == NULL) || (psSpill->uiSize > uiMaxDataSize))
	{
		return false;
	}
	memcpy(pvData, psSpill->pvData, psSpill->uiSize);
	*puiDataSize = psSpill->uiSize;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMemorySpillStore::Remove(const void* pvKey, size uiKeySize)
{
	SMemorySpill*	psSpill;

	psSpill = mcSpills.Get((const char*)pvKey, uiKeySize);
	if ((psSpill == NULL) || (psSpill->pvData == NULL))
	{
		return false;
	}
	free(psSpill->pvData);
	psSpill->pvData = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// How many keys hold a blob (not counting those removed).
//
//////////////////////////////////////////////////////////////////////////
size CMemorySpillStore::NumKeys(void)
{
	size	i;
	size	uiKeys;

	uiKeys = 0;
	for (i = 0; i < mcSpills.NumElements(); i++)
	{
		if (mcSpills.GetValue(i)->pvData)
		{
			uiKeys++;
		}
	}
	return uiKeys;
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing needs to survive the compiler, so the controller isn't durable.
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeSpillStore::Init(const char* szDirectory, lsize uiCacheSize)
{
	CFileUtil	cFileUtil;

	if (uiCacheSize < SPILL_STORE_MIN_CACHE)
	{
		uiCacheSize = SPILL_STORE_MIN_CACHE;
	}
	mszDirectory.Init(szDirectory);
	cFileUtil.RemoveDir(szDirectory);
	if (!cFileUtil.TouchDir(szDirectory))
	{
		mszDirectory.Kill();
		return false;
	}
	mcController.Init(szDirectory);
	mcController.SetSync(DS_No);
	mcController.Begin();
	mcOrderer.Init();
	mcStrategy.Init(LifeLocal<CIndexTreeDataOrderer>(&mcOrderer));
	mcTree.Init(&mcController, NULL, uiCacheSize, NULL, LifeLocal<CIndexTreeEvictionStrategy>(&mcStrategy), NULL, IWT_No, IKR_No, LifeLocal<CIndexTreeDataOrderer>(&mcOrderer));
	muiCacheSize = uiCacheSize;
	muiUncommitted = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexTreeSpillStore::Kill(void)
{
	CFileUtil	cFileUtil;

	//The directory is removed, so what the tree holds is not written.
	mcController.End();
	mcTree.Discard();
	mcStrategy.Kill();
	mcOrderer.Kill();
	mcController.Kill();
	cFileUtil.RemoveDir(mszDirectory.Text());
	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIndexTreeSpillStore::MaxDataSize(void) const
{
	return MAX_DATA_SIZE - 2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeSpillStore::Put(const void* pvKey, size uiKeySize, const void* pvData, size uiDataSize)
{
	size	uiRecordSize;

	if (uiDataSize > MaxDataSize())
	{
		return false;
	}
	mauiRecord[0] = (uint8)uiDataSize;
	mauiRecord[1] = (uint8)(uiDataSize >> 8);
	memcpy(&mauiRecord[2], pvData, uiDataSize);
	uiRecordSize = ((uiDataSize + 2 + SPILL_STORE_RECORD - 1) / SPILL_STORE_RECORD) * SPILL_STORE_RECORD;
	if (uiRecordSize > MAX_DATA_SIZE)
	{
		uiRecordSize = MAX_DATA_SIZE;
	}
	memset(&mauiRecord[uiDataSize + 2], 0, uiRecordSize - (uiDataSize + 2));
	if (!mcTree.Put((uint8*)pvKey, uiKeySize, mauiRecord, uiRecordSize))
	{
		return false;
	}
	muiUncommitted += uiRecordSize;
	if (muiUncommitted > muiCacheSize)
	{
		return Commit();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes what the tree holds (the evicted and the removed) and commits the
// files' writes.
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeSpillStore::Commit(void)
{
	bool	bResult;

	muiUncommitted = 0;
	bResult = mcTree.Flush();
	bResult &= mcController.End();
	bResult &= mcController.Begin();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeSpillStore::Get(const void* pvKey, size uiKeySize, void* pvData, size uiMaxDataSize, size* puiDataSize)
{
	size	uiRecordSize;
	size	uiDataSize;

	if (!mcTree.Get((uint8*)pvKey, uiKeySize, mauiRecord, &uiRecordSize, MAX_DATA_SIZE) || (uiRecordSize < 2))
	{
		return false;
	}
	uiDataSize = (size)(mauiRecord[0] | (mauiRecord[1] << 8));
	if ((uiDataSize > uiMaxDataSize) || (uiDataSize + 2 > uiRecordSize))
	{
		return false;
	}
	memcpy(pvData, &mauiRecord[2], uiDataSize);
	*puiDataSize = uiDataSize;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIndexTreeSpillStore::Remove(const void* pvKey, size uiKeySize)
{
	return mcTree.Remove((uint8*)pvKey, uiKeySize);
}
