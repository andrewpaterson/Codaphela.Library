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
#include "CoreLib/DurableFile.h"
#include "NamedIndexesBlock.h"


class CNamedIndexesBlocks
{
protected:
	CArrayNamedIndexesBlock		macBlocks;
	int							miBlockWidth;  //same as miDataSize in CIndexedFile
	int							miMinNameLength;
	int							miMaxNameLength;
	int							miNewNumBlocks;
	CMemoryCache*				mpcCache;
	int							miFileNumber;

public:
	void	Init(int iBlockSize, int iMinNameLength, int iMaxNameLength, int iNewNumBlocks);
	void	Kill(void);
	void	Set(CMemoryCache* pcCache);

	BOOL	FitsLength(int iNameLength);

	BOOL	Add(OIndex oi, CChars* szName, BOOL bFailOnExisting);
	OIndex	GetIndex(CChars* szName);
	BOOL	Remove(CChars* szName);

	BOOL	Flush(void);
	BOOL	Cache(CNamedIndexesBlock* pcBlock);
};

typedef CArrayTemplate<CNamedIndexesBlocks> CArrayNamedIndexesBlocks;
 

#endif // __NAMED_INDEXES_BLOCKS_H__

