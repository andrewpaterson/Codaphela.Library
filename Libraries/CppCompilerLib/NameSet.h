/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __NAME_SET_H__
#define __NAME_SET_H__
#include "BaseLib/PrimitiveTypes.h"


struct SNameSetName
{
	const char*		szName;			//Not NULL terminated.
	uint32			uiLength;
};


//Names, by their text: a small open addressed hash set.  It doesn't own the text.
class CNameSet
{
protected:
	SNameSetName*	masNames;
	size			muiCapacity;	//A power of two.
	size			muiNames;

public:
	void			Init(void);
	void			Kill(void);
	bool			Contains(const char* szName, size uiLength);
	bool			Add(const char* szName, size uiLength);		//False if it was already in.
	size			NumNames(void);

	static uint64	Hash(const char* szName, size uiLength);

protected:
	void			Grow(void);
};


#endif // __NAME_SET_H__
