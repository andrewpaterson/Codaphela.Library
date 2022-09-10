/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __NAMED_INDEXED_DATA_H__
#define __NAMED_INDEXED_DATA_H__
#include "CoreLib/IndexedData.h"
#include "NamedIndexes.h"


class CNamedIndexedData
{
protected:
	CIndexedData	mcData;
	CNamedIndexes	mcNames;

public:
	void 			Init(CDurableFileController* pcController, CLifeInit<CIndexedDataConfig> cIndexConfig, CLifeInit<CNamedIndexesConfig> cNamedConfig);
	void 			Kill(void);

	bool			Add(OIndex oi, void* pvData, unsigned int uiDataSize);
	bool			Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize);
	bool			Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize);

	bool			Set(OIndex oi, void* pvData);
	bool			Set(OIndex oi, void* pvData, unsigned int uiDataSize);
	bool			Set(char* szName, void* pvData);
	bool			Set(char* szName, void* pvData, unsigned int uiDataSize);
	bool			Set(CChars* szName, void* pvData);
	bool			Set(CChars* szName, void* pvData, unsigned int uiDataSize);

	bool			Put(OIndex oi, void* pvData, unsigned int uiDataSize);
	bool			Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize);
	bool			Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize);

	bool			Get(OIndex oi, void* pvData);
	bool			Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);
	bool			Get(char* szName, void* pvData);
	bool			Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);
	bool			Get(CChars* szName, void* pvData);
	bool			Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);

	bool			GetName(OIndex oi, CChars* szName);
	bool			GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength);
	OIndex			GetIndex(char* szName);
	OIndex			GetIndex(CChars* szName);

	bool			Contains(OIndex oi);
	bool			Contains(char* szName);
	bool			Contains(CChars* szName);

	bool			Remove(OIndex oi);
	bool			Remove(char* szName);
	bool			Remove(CChars* szName);
	unsigned int	Size(OIndex oi);

	bool			Flush(void);
	bool			Flush(bool bClearCache);

	OIndex 			StartIndexIteration(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize);
	OIndex			IndexIterate(SIndexTreeFileIterator* psIterator, void* pvData, size_t* piDataSize, size_t iMaxDataSize);
	bool			StartNameIteration(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);
	bool			NameIterate(SIndexTreeFileIterator* psIterator, char* szKey, OIndex* poi);

	int64			NumIndices(void);
	int64			NumIndicesCached(void);
	int64			NumIndicesCached(size_t iSize);
	int64			NumDataCached(void);
	int64			NumDataCached(size_t iSize);
	int64			NumNames(void);
	int64			NumNamesCached(void);
	bool			IsCaching(void);
	bool			IsWriteThrough(void);

	bool			ValidateIdentifiers(void);
	bool			ValidateNameToOidToName(void);
	bool			ValidateOidToNameToOid(void);

	bool			ValidateConfigInitialised(void);
	bool			ValidateConfigKilled(void);

	size_t			GetIndiciesSystemMemorySize(void);
	size_t			GetDataSystemMemorySize(void);
	size_t			GetNamesSystemMemorySize(void);

	void			Dump(void);

private:
	bool			Add(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize);
	bool			Set(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize);
	bool			Put(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize);

	unsigned int	MinNameSize(unsigned int uiNameSize, unsigned int uiMaxNameSize);
};


#endif // !__NAMED_INDEXED_DATA_H__

