/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "Define.h"
#include "MemoryCache.h"
#include "LifeCycle.h"
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
	bool			Kill(void);

	bool			Add(char* szName, OIndex oi);
	bool			Add(CChars* szName, OIndex oi);
	bool			Add(char* szName, int iNameLength, OIndex oi);

	bool			Set(char* szName, OIndex oi);
	bool			Set(CChars* szName, OIndex oi);

	bool			Put(char* szName, OIndex oi);
	bool			Put(CChars* szName, OIndex oi);

	OIndex			Get(char* szName);
	OIndex			Get(CChars* szName);

	bool			Remove(char* szName);
	bool			Remove(CChars* szName);

	int64			NumElements(void);
	int64			NumElementsCached(void);
	bool			Flush(void);

	bool			StartIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool			Iterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);

	bool			IndexTreeNodeEvicted(void* pvKey, int iKeySize, void* pvData, int iDataSize);

	uint16			IndexTreeDataSize(uint16 uiSourceSize);
	bool			IndexTreeWriteData(void* pvDataBuffer, void* pvSource, int iFileDataSize, uint16 uiSourceDataSize);
	bool			IndexTreeReadData(void* pvDest, void* pvDataBuffer, uint16 uiDestDataSize, int iFileDataSize);

	size_t			GetSystemMemorySize(void);
	size_t			GetUserMemorySize(void);
	unsigned char	GetRootFlags(void);


	void			DumpIndex(void);
	bool			ValidateIndex(void);
	bool			ValidateConfigInitialised(void);
	bool			ValidateConfigKilled(void);
};


#endif // !__NAMED_INDEXES_H__

