/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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

	BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL			Add(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize);
	BOOL			Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize);

	BOOL			Set(OIndex oi, void* pvData);
	BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL			Set(char* szName, void* pvData);
	BOOL			Set(char* szName, void* pvData, unsigned int uiDataSize);
	BOOL			Set(CChars* szName, void* pvData);
	BOOL			Set(CChars* szName, void* pvData, unsigned int uiDataSize);

	BOOL			Put(OIndex oi, void* pvData, unsigned int uiDataSize);
	BOOL			Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize);
	BOOL			Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize);

	BOOL			Get(OIndex oi, void* pvData);
	BOOL			Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);
	BOOL			Get(char* szName, void* pvData);
	BOOL			Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);
	BOOL			Get(CChars* szName, void* pvData);
	BOOL			Get(CChars* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxDataSize);

	BOOL			GetName(OIndex oi, CChars* szName);
	BOOL			GetName(OIndex oi, char* szName, unsigned int* puiNameLength, unsigned int uiMaxNameLength);
	OIndex			GetIndex(char* szName);
	OIndex			GetIndex(CChars* szName);

	BOOL			Contains(OIndex oi);
	BOOL			Contains(char* szName);
	BOOL			Contains(CChars* szName);

	BOOL			Remove(OIndex oi);
	BOOL			Remove(char* szName);
	BOOL			Remove(CChars* szName);
	unsigned int	Size(OIndex oi);

	BOOL			Flush(void);
	BOOL			Flush(BOOL bClearCache);

	int64			NumIndices(void);
	int64			NumIndicesCached(void);
	int64			NumIndicesCached(int iSize);
	int64			NumNames(void);
	BOOL			IsCaching(void);

	BOOL			ValidateConfigInitialised(void);
	BOOL			ValidateConfigKilled(void);

private:
	BOOL			Add(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize);
	BOOL			Set(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize);
	BOOL			Put(OIndex oi, char* szName, int iNameLength, void* pvData, unsigned int uiDataSize);

	unsigned int	MinNameSize(unsigned int uiNameSize, unsigned int uiMaxNameSize);
	unsigned int	MinDataSize(unsigned int uiDataSize, unsigned int uiMaxDataSize);
};


#endif // __NAMED_INDEXED_DATA_H__

