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
#ifndef __MAP_UNKNOWN_UNKNOWN_H__
#define __MAP_UNKNOWN_UNKNOWN_H__
#include "BaseLib/MapPtrPtr.h"
#include "MapCommonUnknown.h"
#include "Unknowns.h"


class CMapUnknownUnknown : public CMapCommonUnknown
{
CONSTRUCTABLE(CMapUnknownUnknown)
private:
	CMapPtrPtr	mcMap;

public:	
	void		_Init(void);  //Valid but empty.  Allocates nothing.  Safe to walk before Init.
	void		Init(bool bKillElements = true, bool bOverwriteExisting = true);
	void		Kill(void) override;
	void		ReInit(void);

	bool		Save(CFileWriter* pcFile) override;
	bool		Load(CFileReader* pcFile) override;

	bool		WriteMapUnknownHeader(CFileWriter* pcFileWriter);
	bool		ReadMapUnknownHeader(CFileReader* pcFileReader);

	template<class M, class N>
	N*			Put(M* pcKey);  //Note this allocates N and returns a pointer to it.  Not a pointer to the pointer in the map.
	bool		Put(CUnknown* pcKey, CUnknown* pcValue);

	template<class M, class N>
	N*			Get(M* pcKey);
	CUnknown*	Get(CUnknown* pcKey);
	SMNode*		GetNode(CUnknown* pcKey);
	bool		Contains(CUnknown* pcKey);

	void		SetKey(SMapIterator* psIter, CUnknown* pcNewKey);
	void		SetValue(SMapIterator* psIter, CUnknown* pcNewValue);

	size		NumElements(void) override;
	size		GetSortedSize(void) override;
	size		GetHoldingSize(void) override;
	size		NonNullElements(void);

	bool		Remove(CUnknown* pcKey);

	bool		StartIteration(SMapIterator* psIterator, CUnknown** ppvKey, CUnknown** ppvValue);
	bool		Iterate(SMapIterator* psIterator, CUnknown** ppvKey, CUnknown** ppvValue);

	void		Pack(void);
	void		Print(CChars* psz) override;
	void		Dump(void);
	size		Sort(void);
	bool		IsSorted(void);
	CMapPtrPtr* GetPointerMap(void);

protected:
	void		FreeData(void* pvData) override;
	bool		WriteData(CFileWriter* pcFile, void* pvData) override;
	bool		ReadData(CFileReader* pcFile, void* pvData) override;
	void		ReplaceValue(CUnknown** ppcValue, CUnknown* pcKey, CUnknown* pcValue);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
N* CMapUnknownUnknown::Put(M* pcKey)
{
	N*	pv;

	if (pcKey == NULL)
	{
		return NULL;
	}
	pv = gcUnknowns.Add<N>();
	return gcUnknowns.KeptOrFreed(pv, Put(pcKey, pv));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class N>
N* CMapUnknownUnknown::Get(M* pcKey)
{
	return (N*)Get((CUnknown*)pcKey);
}


#endif // __MAP_UNKNOWN_UNKNOWN_H__

