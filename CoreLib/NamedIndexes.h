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
#include "BaseLib/LifeCycle.h"
#include "DurableFileController.h"
#include "IndexedFiles.h"
#include "IndexTreeEvicting.h"
#include "IndexTreeEvictionStrategyRandom.h"
#include "NamedIndexesConfig.h"


class CNamedIndexes : public CIndexTreeEvictionCallback, public CIndexTreeFileDataCallback
{
protected:
	CIndexTreeEvicting			mcIndexTree;
	CLife<CNamedIndexesConfig>	mcConfig;
	CNamedIndexesConfig*		mpcConfig;
	
public:
	void			Init(CDurableFileController* pcController, CLifeInit<CNamedIndexesConfig> pcConfig);
	BOOL			Kill(void);

	BOOL			Add(char* szName, OIndex oi);
	BOOL			Add(CChars* szName, OIndex oi);
	BOOL			Add(char* szName, int iNameLength, OIndex oi);

	BOOL			Set(char* szName, OIndex oi);
	BOOL			Set(CChars* szName, OIndex oi);

	BOOL			Put(char* szName, OIndex oi);
	BOOL			Put(CChars* szName, OIndex oi);

	OIndex			Get(char* szName);
	OIndex			Get(CChars* szName);

	BOOL			Remove(char* szName);
	BOOL			Remove(CChars* szName);

	int64			NumElements(void);
	int64			NumElementsCached(void);
	BOOL			Flush(void);

	BOOL			StartIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	BOOL			Iterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);

	BOOL			IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	uint16			IndexTreeDataSize(uint16 uiSourceSize);
	BOOL			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize);
	BOOL			IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize);

	size_t			GetSystemMemorySize(void);
	size_t			GetUserMemorySize(void);
	unsigned char	GetRootFlags(void);


	void			DumpIndex(void);
	BOOL			ValidateIndex(void);
	BOOL			ValidateConfigInitialised(void);
	BOOL			ValidateConfigKilled(void);
};


#endif // !__NAMED_INDEXES_H__

