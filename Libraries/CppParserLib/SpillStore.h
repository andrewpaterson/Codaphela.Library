/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SPILL_STORE_H__
#define __SPILL_STORE_H__
#include "BaseLib/MapBlock.h"
#include "BaseLib/Chars.h"
#include "BaseLib/DurableFileController.h"
#include "BaseLib/IndexTreeEvicting.h"
#include "BaseLib/IndexTreeEvictionStrategyDataOrderer.h"
#include "BaseLib/AccessDataOrderer.h"


#define SPILL_STORE_MAX_DATA	0xFFFF		//The most bytes one key holds (a size on the W65C816).
#define SPILL_STORE_RECORD		1024		//CIndexTreeSpillStore's records are multiples of this.
#define SPILL_STORE_MIN_CACHE	(4 * MAX_DATA_SIZE)	//CIndexTreeSpillStore's smallest cache (it must hold a few records).


//Where the compiler puts what it has made but doesn't need again for a while (each
//function's assembly until the unit is assembled): blobs of at most
//MaxDataSize() bytes by key (at most SPILL_STORE_MAX_DATA).  Bigger things are
//stored in pieces.
class CSpillStore
{
public:
	virtual void	Kill(void) =0;

	virtual size	MaxDataSize(void) =0;
	virtual bool	Put(const void* pvKey, size uiKeySize, const void* pvData, size uiDataSize) =0;
	virtual bool	Get(const void* pvKey, size uiKeySize, void* pvData, size uiMaxDataSize, size* puiDataSize) =0;
	virtual bool	Remove(const void* pvKey, size uiKeySize) =0;
};


//Everything kept in memory.
class CMemorySpillStore : public CSpillStore
{
protected:
	CMapBlock	mcMap;

public:
	void	Init(void);
	void	Kill(void) override;

	size	MaxDataSize(void) override;
	bool	Put(const void* pvKey, size uiKeySize, const void* pvData, size uiDataSize) override;
	bool	Get(const void* pvKey, size uiKeySize, void* pvData, size uiMaxDataSize, size* puiDataSize) override;
	bool	Remove(const void* pvKey, size uiKeySize) override;
};


//An evicting index tree in szDirectory: up to uiCacheSize bytes (at least
//SPILL_STORE_MIN_CACHE) stay in memory and the least recently used are written out.
//The directory is made by Init and removed by Kill.  The tree keeps a file for each
//size of data, so each blob is stored with its length first, padded to a multiple of
//SPILL_STORE_RECORD bytes: a key holds at most the tree's MAX_DATA_SIZE less the
//length.  The files' writes are kept (as a durable file's log) until committed, so
//they are committed whenever more than the cache size has been put since the last
//time.  (On the W65C816 the tree is to be backed by a block device.)
class CIndexTreeSpillStore : public CSpillStore
{
protected:
	CChars									mszDirectory;
	CDurableFileController					mcController;
	CIndexTreeEvicting						mcTree;
	CIndexTreeEvictionStrategyDataOrderer	mcStrategy;
	CAccessDataOrderer						mcOrderer;
	uint8									mauiRecord[MAX_DATA_SIZE];
	lsize									muiCacheSize;
	lsize									muiUncommitted;	//Bytes put since the files' writes were last committed.

public:
	bool	Init(const char* szDirectory, lsize uiCacheSize);
	void	Kill(void) override;

	size	MaxDataSize(void) override;
	bool	Put(const void* pvKey, size uiKeySize, const void* pvData, size uiDataSize) override;
	bool	Get(const void* pvKey, size uiKeySize, void* pvData, size uiMaxDataSize, size* puiDataSize) override;
	bool	Remove(const void* pvKey, size uiKeySize) override;

protected:
	bool	Commit(void);
};


#endif // __SPILL_STORE_H__
