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
#include "NamedIndexedDataConfig.h"
#include "NamedIndexes.h"


class CNamedIndexedData
{
protected:
	CIndexedData	mcData;
	CNamedIndexes	mcNames;

public:
	void 			Init(CNamedIndexedDataConfig* pcConfig);
	void 			Kill(void);

	BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	BOOL			Set(OIndex oi, void* pvData, unsigned int uiTimeStamp);
	BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	BOOL			Put(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			Put(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			Put(OIndex oi, char* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	unsigned int	Size(OIndex oi);

	BOOL			Get(OIndex oi, void* pvData);
	BOOL			Get(OIndex oi, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize);
	BOOL			Get(char* szName, void* pvData);
	BOOL			Get(char* szName, unsigned int* puiDataSize, void* pvData, unsigned int uiMaxSize);

	OIndex			GetIndex(char* szName);

	BOOL			Contains(OIndex oi);
	BOOL			Contains(char* szName);

	BOOL			Remove(CChars* szName);
	BOOL			Remove(OIndex oi);

	BOOL			Flush(BOOL bClearCache);

	int64			NumIndices(void);
	int64			NumIndicesCached(void);
	int64			NumIndicesCached(int iSize);
	int64			NumNames(void);
	BOOL			IsCaching(void);
};


#endif // __NAMED_INDEXED_DATA_H__

