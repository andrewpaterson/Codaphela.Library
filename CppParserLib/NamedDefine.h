/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __NAMED_DEFINE_H__
#define __NAMED_DEFINE_H__
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplate.h"
#include "DefineMap.h"


#define NAMED_DEFINE_FLAGS_BRACKETED	DEFINE_FLAGS_BRACKETED
#define NAMED_DEFINE_FLAGS_UNDEFFED		0x02

class CNamedDefine
{
public:
	CPPLine			mcReplacement;
	CArrayChars	mcArguments;
	int				miFlags;
	CChars			mszName;

	void Define(CExternalString* pcName, CDefine* pcSource, CMemoryStackExtended* mpcStack);
	void Undef(CExternalString* pcName);
	void Kill(void);
	BOOL IsBacketed(void);
	BOOL Equals(CNamedDefine* pcOther);
	void Dump(void);
};


typedef CArrayTemplate<CNamedDefine>	__CArrayNamedDefines;


class CArrayNamedDefines : public __CArrayNamedDefines
{
public:
	BOOL Equals(CArrayNamedDefines* pcDefines);
	void Dump(void);
};


#endif // !__NAMED_DEFINE_H__

