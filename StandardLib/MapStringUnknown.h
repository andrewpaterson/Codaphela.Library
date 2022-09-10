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
#ifndef __MAP_STRING_UNKNOWN_H__
#define __MAP_STRING_UNKNOWN_H__
#include "BaseLib/MapStringTemplate.h"
#include "MapCommon.h"


class CMapStringUnknownDataFree : public CDataFree
{
private:
	CMapStringUnknown*	mpcMap;

public:
	void Init(CMapStringUnknown* pcMap);
	void DataWillBeFreed(void* pvData);
};


class CMapStringUnknown : public CMapCommon
{
friend class CMapStringUnknownDataFree;
CONSTRUCTABLE(CMapStringUnknown);
private:
	CMapStringTemplate<CUnknown*>	mcMap;
	CMapStringUnknownDataFree		mcDataFree;

public:
	void		Init(bool bKillElements = true, bool bOverwriteExisting = true);
	void		Kill(void);
	bool		Save(CFileWriter* pcFile);
	bool		Load(CFileReader* pcFile);

	template<class M>
	M*			Put(char* szKey);
	bool		Put(char* szKey, CUnknown* pcValue);
	CUnknown*	Get(char* szKey);
	int			NumElements(void);

protected:
	void DataWillBeFreed(CUnknown* pcUnknown);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CMapStringUnknown::Put(char* szKey)
{
	M*		pv;
	bool	bResult;

	if (!StrEmpty(szKey))
	{
		pv = gcUnknowns.Add<M>();
		bResult = Put(szKey, pv);
		if (bResult)
		{
			return pv;
		}
		else
		{
			//Nasty, but if we allocated (and didn't initialise) the object then it must be freed but not killed.
			gcUnknowns.RemoveInKill(pv);
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}


#endif // !__MAP_STRING_UNKNOWN_H__

