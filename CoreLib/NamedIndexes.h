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
#include "DurableFileController.h"
#include "IndexedFiles.h"
#include "IndexTreeEvicting.h"
#include "IndexTreeEvictionStrategyRandom.h"
#include "EvictionCallbackWrapper.h"
#include "IndexedDataConfig.h"


class CNamedIndexes : public CIndexTreeEvictionCallback, public CIndexTreeFileCallback
{
protected:
	CIndexTreeEvicting						mcIndexTree;
	CIndexTreeEvictionStrategyRandom		mcEvictionStrategy;
	EIndexWriteThrough						meWriteThrough;
	
public:
	void			Init(CDurableFileController* pcDurableFileController, size_t uiCutoff, EIndexWriteThrough eWriteThrough);
	void			Init(CDurableFileController* pcDurableFileController, size_t uiCutoff, EIndexWriteThrough eWriteThrough, CIndexTreeEvictionCallback* pcEvictionCallback);
	BOOL			Kill(void);

	BOOL			Put(char* szName, OIndex oi, BOOL bFailOnExisting = TRUE);
	BOOL			Put(CChars* szName, OIndex oi, BOOL bFailOnExisting = TRUE);
	OIndex			Get(char* szName);
	OIndex			Get(CChars* szName);
	BOOL			Remove(char* szName);
	BOOL			Remove(CChars* szName);
	filePos			NumElements(void);
	BOOL			Flush(void);

	BOOL			IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	unsigned short	IndexTreeDataSize(unsigned short uiSourceSize);
	BOOL			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, unsigned short uiSourceDataSize);
	BOOL			IndexTreeReadData(void* pvDest, void* pvDataBuffer, unsigned short uiDestDataSize, int iFileDataSize);

	void			Dump(void);
};


#endif // __NAMED_INDEXES_H__

