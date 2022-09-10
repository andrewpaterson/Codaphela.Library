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
#ifndef __NAMED_DEFINE_H__
#define __NAMED_DEFINE_H__
#include "BaseLib/MemoryStackExtended.h"
#include "BaseLib/ArrayTemplate.h"
#include "DefineMap.h"


#define NAMED_DEFINE_FLAGS_BRACKETED	DEFINE_FLAGS_BRACKETED
#define NAMED_DEFINE_FLAGS_UNDEFFED		0x40


class CNamedDefine
{
protected:
	CPPLine			mcReplacement;
	CArrayChars		maszArguments;
	int				miFlags;
	CChars			mszName;

public:
	void			Define(CExternalString* pcName, CDefine* pcSource, CPPTokenMemory* pcTokens);
	void			Undef(CExternalString* pcName);
	void			Kill(void);
	bool			IsBacketed(void);
	bool			IsUndeffed(void);
	bool			Equals(CNamedDefine* pcOther);
	char*			GetName(void);
	int				GetNameLength(void);
	CArrayChars*	GetArguments(void);
	CPPLine*		GetReplacement(void);
	void			Dump(void);
};


typedef CArrayTemplate<CNamedDefine>	__CArrayNamedDefines;


class CArrayNamedDefines : public __CArrayNamedDefines
{
public:
	bool Equals(CArrayNamedDefines* pcDefines);
	void Dump(void);
};


#endif // !__NAMED_DEFINE_H__

