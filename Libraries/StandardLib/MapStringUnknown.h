/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __MAP_STRING_UNKNOWN_H__
#define __MAP_STRING_UNKNOWN_H__
#include "BaseLib/MapStringTemplate.h"
#include "MapCommonUnknown.h"
#include "Unknowns.h"


class CMapStringUnknown : public CMapCommonUnknown
{
CONSTRUCTABLE(CMapStringUnknown)
private:
	CMapStringTemplate<CUnknown*>	mcMap;

public:
	void		Init(bool bKillElements = true, bool bOverwriteExisting = true);
	void		Kill(void) override;

	bool		Save(CFileWriter* pcFile) override;
	bool		Load(CFileReader* pcFile) override;

	template<class M>
	M*			Put(const char* szKey);  //Note this allocates M and returns a pointer to it.  Not a pointer to the pointer in the map.
	bool		Put(const char* szKey, CUnknown* pcValue);
	CUnknown*	Get(const char* szKey);

	size		NumElements(void) override;
	size		GetSortedSize(void) override;
	size		GetHoldingSize(void) override;

	bool		StartIteration(SMapIterator* psIterator, char** ppvKey, CUnknown** ppvData);
	bool		Iterate(SMapIterator* psIterator, char** ppvKey, CUnknown** ppvData);

	void		Pack(void);

protected:
	void		FreeData(void* pvData) override;

	bool		WriteData(CFileWriter* pcFile, void* pvData) override;
	bool		ReadData(CFileReader* pcFile, void* pvData) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CMapStringUnknown::Put(const char* szKey)
{
	M*	pv;

	if (StrEmpty(szKey))
	{
		return NULL;
	}
	pv = gcUnknowns.Add<M>();
	return gcUnknowns.KeptOrFreed(pv, Put(szKey, pv));
}


#endif // __MAP_STRING_UNKNOWN_H__

