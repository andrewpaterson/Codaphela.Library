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
#ifndef __NAMED_INDEXES_H__
#define __NAMED_INDEXES_H__
#include "BaseLib/Define.h"
#include "BaseLib/MemoryCache.h"
#include "CoreLib/DurableFileController.h"
#include "CoreLib/IndexedFiles.h"
#include "NamedIndexesBlocks.h"


class CNamedIndexes
{
protected:
	CArrayNamedIndexesBlocks	macBlocks;
	CMemoryCache				mcCache;
	CIndexedFiles				mcFiles;

public:
	void	Init(CDurableFileController* pcController, int iCacheSize, int iNewNumBlocks);
	void	Kill(void);

	BOOL	Add(OIndex oi, char* szName, BOOL bFailOnExisting = TRUE);
	BOOL	Add(OIndex oi, CChars* szName, BOOL bFailOnExisting = TRUE);
	OIndex	GetIndex(char* szName);
	OIndex	GetIndex(CChars* szName);
	BOOL	Remove(CChars* szName);

	BOOL	Flush(void);

protected:
	CNamedIndexesBlocks*	GetBlock(int iNameLength);
	CNamedIndexesBlocks*	AddBlock(int iBlockSize, int iMinNameLength, int iMaxNameLength, int iNewNumBlocks);
};


#endif // __NAMED_INDEXES_H__

