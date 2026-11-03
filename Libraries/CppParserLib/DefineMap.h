/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __DEFINE_MAP_H__
#define __DEFINE_MAP_H__
#include "BaseLib/ExternalString.h"
#include "BaseLib/MemoryStackExtended.h"
#include "Define.h"


//The macros defined, by name: a hash table (open addressed, linear probing) of defines kept in chunks.  A define's ID
//is new each time a name is defined (an #undef'd and redefined macro is another).  An #undef'd define is killed; its
//memory is freed with the map.
class CDefineMap
{
protected:
	CDefine**				mapcTable;		//NULL for an empty slot.  Its length is a power of two (or 0).
	size					muiCapacity;
	size					muiDefines;
	int64					mlliNextID;
	CMemoryStackExtended	mcDefines;

public:
	void		Init(void);
	void		Kill(void);

	CDefine*	AddDefine(CExternalString* pcName);
	CDefine*	AddDefine(char* szName);
	CDefine*	Get(CExternalString* pcName, bool bExact);
	CDefine*	Get(char* szName, bool bExact);
	void		Remove(CExternalString* pcName);
	void		Remove(char* szName);
	size		NumDefines(void);
	void		Dump(void);

	static uint32	Hash(const char* szName, size uiLength);

protected:
	size		Find(const char* szName, size uiLength, uint32 uiHash);
	CDefine*	GetExact(const char* szName, size uiLength);
	void		Grow(void);
};


#endif // !__DEFINE_MAP_H__
