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
#ifndef __MAP_COMMON_UNKNOWN_H__
#define __MAP_COMMON_UNKNOWN_H__
#include "Unknown.h"


#define MAP_COMMOM_KILL_ELEMENT	0x0001


class CMapCommonUnknown;
class CMapCommonUnknownDataFree : public CDataFree
{
private:
	CMapCommonUnknown*	mpcMap;

public:
	void Init(CMapCommonUnknown* pcMap);
	void DataWillBeFreed(void* pvData);
};


class CMapCommonUnknown : public CUnknown
{
CONSTRUCTABLE(CMapCommonUnknown);
protected:
	uint16						miFlags;
	CMapCommonUnknownDataFree	mcDataFree;

public:
			void 		Init(bool bKillElements = true, bool bOverwriteExisting = true);
			void 		Kill(void);

	virtual size		NumElements(void) =0;
	virtual size		GetSortedSize(void) =0;
	virtual size		GetHoldingSize(void) =0;

			bool 		IsKillElements(void);
			bool 		IsEmpty(void);
			bool 		IsNotEmpty(void);

	virtual void		DataWillBeFreed(SMNode* psNode) =0;
};


#endif // __MAP_COMMON_UNKNOWN_H__

