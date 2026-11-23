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
#ifndef __HASH_MAP_PRIMITIVE_TEMPLATE_H__
#define __HASH_MAP_PRIMITIVE_TEMPLATE_H__
#include "HashMapTemplate.h"


//CHashMapTemplate with its key given by value: an integer, an enum or a pointer (as CMapPrimitiveTemplate is to
//CMapTemplateTemplate).
template<class K, class V, class HASH = SHashOf<K>, class SIZE = SArrayNaturalSize>
class CHashMapPrimitiveTemplate : public CHashMapTemplate<K, V, HASH, SIZE>
{
public:
	typedef CHashMapTemplate<K, V, HASH, SIZE>	CMap;
	typedef typename SIZE::Count				Count;

	V*		Get(K tKey)								{ return CMap::Get(&tKey); }
	V*		Put(K tKey)								{ return CMap::Put(&tKey); }
	bool	Put(K tKey, const V* pValue)			{ return CMap::Put(&tKey, pValue); }
	V*		FindOrAdd(K tKey, bool* pbAdded)		{ return CMap::FindOrAdd(&tKey, pbAdded); }
	bool	HasKey(K tKey)							{ return CMap::HasKey(&tKey); }
	bool	Remove(K tKey)							{ return CMap::Remove(&tKey); }
	Count	Find(K tKey)							{ return CMap::Find(&tKey); }
	Count	FindOrAddIndex(K tKey, bool* pbAdded)	{ return CMap::FindOrAddIndex(&tKey, pbAdded); }
};


//The drop in for CMapLongTemplate<V>: int64 keys.
template<class V>
class CHashMapLongTemplate : public CHashMapPrimitiveTemplate<int64, V> {};


#endif // __HASH_MAP_PRIMITIVE_TEMPLATE_H__

