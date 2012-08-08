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
#ifndef __NAMED_INDEXES_BLOCKS_H__
#define __NAMED_INDEXES_BLOCKS_H__
#include "BaseLib/ArrayTemplate.h"
#include "NamedIndexesBlock.h"


class CMemoryCache;
class CIndexedFiles;
class CNamedIndexes;
class CNamedIndexesBlocks
{
friend class CNamedIndexesBlocksLoader;
protected:
	CArrayNamedIndexesBlock		macBlocks;
	int							miBlockWidth;  //Same as miDataSize in CIndexedFile.  Sort of the maximum length of the string.
	int							miMinNameLength;
	int							miMaxNameLength;
	int							miBlockChunkSize;  //The number of strings per block.
	int							miFileNumber;
	CNamedIndexes*				mpcNamedIndexes;
	
public:
	void					Init(int iBlockSize, int iMinNameLength, int iMaxNameLength, int iBlockChunkSize, CNamedIndexes* pcNamedIndexes);
	void					Kill(void);
	BOOL					Load(int iFileNumber);
	BOOL					Save(void);

	BOOL					FitsLength(int iNameLength);

	BOOL					Add(OIndex oi, CChars* szName, BOOL bFailOnExisting);
	OIndex					GetIndex(CChars* szName);
	BOOL					Remove(CChars* szName);

	BOOL					Flush(void);
	BOOL					Cache(CNamedIndexesBlock* pcBlock);
	BOOL					AddNewBlock(int iBlockWidth, void* pvBlocks, filePos iBlockChunkSize, filePos iDataIndex);
	filePos					NumNames(void);
	void					GetPotentialContainingBlocks(CChars* szName, CArrayNamedIndexesBlockPtr* pcDest);
	void					SortBlockPtrsCachedFirst(CArrayNamedIndexesBlockPtr* pcDest);
	int						GetCacheDescriptorSize(void);
	int						GetDataSize(void);
	int						GetFileNumber(void);
	void					SetFileNumber(int iFileNumber);
	CNamedIndexesBlock*		GetBlock(void* pvCacheMem);
	CNamedIndexesBlock*		GetBlock(int iIndex);
	int						GetNumBlocks(void);
	int						GetMaxNameLength(void);
	
protected:
	int						FindLastCachedBlock(CArrayNamedIndexesBlockPtr* pcDest, int iEnd);
	int						FindFirstUncachedBlock(CArrayNamedIndexesBlockPtr* pcDest, int iStart);
};


typedef CArrayTemplate<CNamedIndexesBlocks> CArrayNamedIndexesBlocks;
 

#endif // __NAMED_INDEXES_BLOCKS_H__

