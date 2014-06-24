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
#ifndef __MAP_TYPE_UNKNOWN_H__
#define __MAP_TYPE_UNKNOWN_H__
#include "BaseLib/MapPtrTemplate.h"
#include "MapCommon.h"


template<class K>
class CMapTypeUnknown : public CMapCommon
{
BASE_FUNCTIONS(CMapTypeUnknown);
protected:
	CMapPtrTemplate<CUnknown*>	mcMap;

public:
	void Init(BOOL bKillElements = TRUE, BOOL bOverwriteExisting = TRUE, int iChunkSize = MAP_COMMOM_CHUNK_SIZE);
	void Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K>
void CMapTypeUnknown<K>::Init(BOOL bKillElements, BOOL bOverwriteExisting, int iChunkSize)
{
	mcMap.Init(iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class K>
void CMapTypeUnknown<K>::Kill(void)
{
	int				i;
	void*			pv;
	CUnknown**		ppcUnknown;

	if (miFlags & MAP_COMMOM_KILL_ELEMENT)
	{
		for (i = 0; i < mcMap.mcArray.NumElements(); i++)
		{
			pv = mcMap.mcArray.GetPtr(i);
			ppcUnknown = mcMap.GetDataForKey(pv);
			if (ppcUnknown)
			{
				(*ppcUnknown)->Kill();
			}
		}
	}

	mcMap.Kill();
	CMapCommon::Kill();
}


#endif // __MAP_TYPE_UNKNOWN_H__

