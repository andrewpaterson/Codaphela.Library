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
#ifndef __NAMED_INDEXES_BLOCK_H__
#define __NAMED_INDEXES_BLOCK_H__
#include "BaseLib/AbstractFile.h"
#include "CoreLib/IndexedGeneral.h"
#include "NamedIndexedBlock.h"


class CNamedIndexesBlock
{
protected:
	int			miDataIndex;
	CChars		mszFirst;
	CChars		mszLast;

	void*		mpvCachePos;
	int			miBlockWidth;  //Max name length = miBlockWidth - sizeof(OIndex) - 1 (for trailing 0).
	int			miNumBlocks;
	int			miUsedBlocks;
	BOOL		mbDirty;

public:
	void					Init(int iBlockWidth, int iNumBlocks);
	void					Init(int iBlockWidth, void* pvBlocks, int iNumBlocks, int iDataIndex, void* pvCache);
	void					Kill(void);

	BOOL					CouldContain(CChars* szName);
	BOOL					IsCached(void);
	BOOL					IsNotFull(void);
	BOOL					IsFull(void);
	BOOL					IsInFile(void);
	BOOL					IsEmpty(void);
	BOOL					IsCache(void* pvCachePos);
	BOOL					IsDirty(void);

	int						GetUsedByteSize(void);
	int						GetAllocatedByteSize(void);

	BOOL					AddUnsafe(OIndex oi, CChars* szName);
	OIndex					GetIndex(CChars* szName);
	BOOL					SetCache(void* pvCache);
	BOOL					Remove(CChars* szName);
	void					Dirty(void);
	BOOL					Uncache(CIndexedFile* pcFile);
	BOOL					Cache(CIndexedFile* pcFile, void* pvCache);
	int						UsedNames(void);
	int						GetBlockWidth(void);
	int						GetNumBlocks(void);
	int						GetUsedBlocks(void);
	char*					GetFirst(void);
	char*					GetLast(void);
	void					Dump(void);
	void					Dump(CArrayBlock* pavFakeBlock);
	BOOL					Write(CIndexedFile* pcFile);

private:
	CNamedIndexedBlock*		GetUnsafe(int iIndex);
};


typedef CArrayTemplate<CNamedIndexesBlock>	CArrayNamedIndexesBlock;
typedef CArrayTemplate<CNamedIndexesBlock*>	CArrayNamedIndexesBlockPtr;


#endif // __NAMED_INDEXES_BLOCK_H__

