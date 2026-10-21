/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __NAMED_INDEXED_DATA_H__
#define __NAMED_INDEXED_DATA_H__
#include "IndexedData.h"
#include "NamedIndexes.h"


class CNamedIndexedData
{
protected:
	CIndexedData	mcData;
	CNamedIndexes	mcNames;

public:
	void 			Init(CDurableFileController* pcController, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig);
	void 			Kill(void);

	bool			Add(OIndex oi, void* pvData, size uiDataSize);
	bool			Add(OIndex oi, const char* szName, void* pvData, size uiDataSize);
	bool			Add(OIndex oi, CChars* szName, void* pvData, size uiDataSize);

	bool			Set(OIndex oi, void* pvData);
	bool			Set(OIndex oi, void* pvData, size uiDataSize);
	bool			Set(const char* szName, void* pvData);
	bool			Set(const char* szName, void* pvData, size uiDataSize);
	bool			Set(CChars* szName, void* pvData);
	bool			Set(CChars* szName, void* pvData, size uiDataSize);

	bool			Put(OIndex oi, void* pvData, size uiDataSize);
	bool			Put(OIndex oi, const char* szName, void* pvData, size uiDataSize);
	bool			Put(OIndex oi, CChars* szName, void* pvData, size uiDataSize);

	bool			Get(OIndex oi, void* pvData);
	bool			Get(OIndex oi, size* puiDataSize, void* pvData, size uiMaxDataSize);
	bool			GetWithName(OIndex oi, CChars* pszName, size* puiDataSize, void* pvData, size uiMaxDataSize);
	bool			Get(const char* szName, void* pvData);
	bool			Get(const char* szName, size* puiDataSize, void* pvData, size uiMaxDataSize);
	bool			Get(CChars* szName, void* pvData);
	bool			Get(CChars* szName, size* puiDataSize, void* pvData, size uiMaxDataSize);

	bool			GetName(OIndex oi, CChars* szName);
	bool			GetName(OIndex oi, char* szName, size* puiNameLength, size uiMaxNameLength);
	OIndex			GetIndex(const char* szName);
	OIndex			GetIndex(CChars* szName);

	bool			Contains(OIndex oi);
	bool			Contains(const char* szName);
	bool			Contains(CChars* szName);

	bool			Remove(OIndex oi);
	bool			Remove(const char* szName);
	bool			Remove(CChars* szName);
	size	Size(OIndex oi);

	bool			Flush(void);
	bool			Flush(bool bClearCache);

	OIndex 			StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	OIndex			IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size* piDataSize, size iMaxDataSize);
	bool			StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool			NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);

	int64			NumIndices(void);
	int64			NumIndicesCached(void);
	int64			NumIndicesCached(size iSize);
	int64			NumDataCached(void);
	int64			NumDataCached(size iSize);
	int64			NumNames(void);
	int64			NumNamesCached(void);
	bool			IsCaching(void);
	bool			IsWriteThrough(void);

	bool			ValidateIdentifiers(void);
	bool			ValidateNameToOidToName(void);
	bool			ValidateOidToNameToOid(void);

	bool			ValidateConfigInitialised(void);
	bool			ValidateConfigKilled(void);

	bool			ValidateCache(void);

	size			GetIndiciesSystemMemorySize(void);
	size			GetDataSystemMemorySize(void);
	size			GetNamesSystemMemorySize(void);

	void			Dump(void);

private:
	bool			Add(OIndex oi, const char* szName, int iNameLength, void* pvData, size uiDataSize);
	bool			Set(OIndex oi, const char* szName, int iNameLength, void* pvData, size uiDataSize);
	bool			Put(OIndex oi, const char* szName, int iNameLength, void* pvData, size uiDataSize);

	size	MinNameSize(size uiNameSize, size uiMaxNameSize);
};


#endif // __NAMED_INDEXED_DATA_H__

