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
#include "BaseObject.h"


class CNamedIndexedData
{
protected:
	CIndexedData	mcData;
	CNamedIndexes	mcNames;

public:
	void 			Init(CIndexedConfig* pcConfig);
	void 			Kill(void);

	BOOL			Add(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			Add(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	BOOL			Set(OIndex oi, void* pvData, unsigned int uiTimeStamp);
	BOOL			Set(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	BOOL			SetOrAdd(OIndex oi, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);
	BOOL			SetOrAdd(OIndex oi, CChars* szName, void* pvData, unsigned int uiDataSize, unsigned int uiTimeStamp);

	unsigned int	Size(OIndex oi);
	unsigned int	Flags(OIndex oi);
	OIndex			GetIndex(CChars* szName);
	BOOL			Get(OIndex oi, void* pvData);

	BOOL			Remove(CChars* szName);
	BOOL			Remove(OIndex oi);

	BOOL			Flush(void);
	void			DurableBegin(void);
	void			DurableEnd(void);
};


#endif // __NAMED_INDEXED_DATA_H__
