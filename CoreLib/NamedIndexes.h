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
#include "CoreLib/IndexTreeEvicting.h"
#include "CoreLib/IndexTreeEvictionStrategyRandom.h"
#include "CoreLib/EvictionCallbackWrapper.h"
#include "CoreLib/IndexConfig.h"


class CNamedIndexes : public CEvictionCallback
{
protected:
	CIndexTreeEvicting						mcIndexTree;
	CIndexTreeEvictionStrategyRandom		mcEvictionStrategy;
	EIndexWriteThrough						meWriteThrough;
	
public:
	void			Init(CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse);
	void			Init(CDurableFileController* pcDurableFileController, BOOL bDirtyTesting, size_t uiCutoff, EIndexWriteThrough eWriteThrough, EIndexKeyReverse eKeyReverse, CEvictionCallback* pcEvictionCallback);
	BOOL			Kill(void);

	BOOL			Add(OIndex oi, char* szName, BOOL bFailOnExisting = TRUE);
	BOOL			Add(OIndex oi, CChars* szName, BOOL bFailOnExisting = TRUE);
	OIndex			GetIndex(char* szName);
	OIndex			GetIndex(CChars* szName);
	BOOL			Remove(char* szName);
	BOOL			Remove(CChars* szName);
	filePos			NumElements(void);
	BOOL			Flush(void);

	BOOL			NodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);
};


#endif // __NAMED_INDEXES_H__

