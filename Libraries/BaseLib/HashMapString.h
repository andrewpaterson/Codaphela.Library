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

** ------------------------------------------------------------------------ **/
#ifndef __HASH_MAP_STRING_H__
#define __HASH_MAP_STRING_H__
#include "HashMapTemplate.h"
#include "HashStringBase.h"


//A map from text to V by a hash of the text (HASH: SHashOfText, or SHashOfTextIgnoreCase or SHashOfFileName), which
//keeps its own copy of each key (see CHashStringBase).  Otherwise as CHashMapTemplate: the order added, pointers to
//values lasting until the next Put or Remove.
template<class V, class SIZE = SArrayNaturalSize, class HASH = SHashOfText>
class CHashMapString : public CHashStringBase<CHashMapTemplate<SHashText, V, HASH, SIZE>, SIZE>
{
public:
	typedef CHashStringBase<CHashMapTemplate<SHashText, V, HASH, SIZE>, SIZE>	CBase;
	typedef typename SIZE::Count												Count;

	V*		Get(const char* szKey);
	V*		Get(const char* szKey, size uiLength);
	V*		Put(const char* szKey);
	V*		FindOrAdd(const char* szKey, size uiLength, bool* pbAdded = NULL);
	bool	Put(const char* szKey, const V* pValue);
	V*		GetValue(Count uiIndex);

	void	SetDataFreeCallback(CDataFree* pcDataFree);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE, class HASH>
V* CHashMapString<V, SIZE, HASH>::Get(const char* szKey)
{
	return Get(szKey, (size)strlen(szKey));
}


//////////////////////////////////////////////////////////////////////////
//
// szKey needn't be NULL terminated.
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE, class HASH>
V* CHashMapString<V, SIZE, HASH>::Get(const char* szKey, size uiLength)
{
	SHashText	sKey;

	sKey.Init(szKey, uiLength);
	return CBase::mcMap.Get(&sKey);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE, class HASH>
V* CHashMapString<V, SIZE, HASH>::Put(const char* szKey)
{
	return FindOrAdd(szKey, (size)strlen(szKey), NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The value keyed szKey (needn't be NULL terminated), added (zeroed, the
// text copied) if there isn't one.  NULL if there is no memory for it.
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE, class HASH>
V* CHashMapString<V, SIZE, HASH>::FindOrAdd(const char* szKey, size uiLength, bool* pbAdded)
{
	Count	uiIndex;

	uiIndex = CBase::FindOrAddIndex(szKey, uiLength, pbAdded);
	return (uiIndex == SIZE::NotFound) ? NULL : CBase::mcMap.GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// szKey's value is a copy of *pValue, whether it had one or not.
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE, class HASH>
bool CHashMapString<V, SIZE, HASH>::Put(const char* szKey, const V* pValue)
{
	V*	pDest;

	pDest = Put(szKey);
	if (pDest == NULL)
	{
		return false;
	}
	memcpy((void*)pDest, (const void*)pValue, sizeof(V));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE, class HASH>
V* CHashMapString<V, SIZE, HASH>::GetValue(Count uiIndex)
{
	return CBase::mcMap.GetValue(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class V, class SIZE, class HASH>
void CHashMapString<V, SIZE, HASH>::SetDataFreeCallback(CDataFree* pcDataFree)
{
	CBase::mcMap.SetDataFreeCallback(pcDataFree);
}


#endif // __HASH_MAP_STRING_H__
