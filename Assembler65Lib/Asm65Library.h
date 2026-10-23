/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela Assembler65Lib

Codaphela Assembler65Lib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela Assembler65Lib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela Assembler65Lib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ASM65_LIBRARY_H__
#define __ASM65_LIBRARY_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/AbstractFile.h"
#include "BaseLib/Chars.h"
#include "Asm65Object.h"


//The library file: a PackFile holding "Library" (name and object count) and each
//object's ChunkFile as "Objects/0", "Objects/1" ...
#define ASM65_LIBRARY_MAGIC		"Asm65Library"
#define ASM65_LIBRARY_VERSION	1


//Objects the linker includes only when they define a name that is still needed.
//The library owns its objects.
class CAsm65Library
{
protected:
	CChars							mszName;
	CArrayTemplatePtr<CAsm65Object>	mapcObjects;

public:
	void			Init(const char* szName);
	void			Kill(void);

	char*			GetName(void);
	CAsm65Object*	AddObject(const char* szName);
	size			NumObjects(void);
	CAsm65Object*	GetObject(size uiIndex);

	bool			Write(CAbstractFile* pcFile);
	bool			Read(CAbstractFile* pcFile);
};


#endif // __ASM65_LIBRARY_H__
