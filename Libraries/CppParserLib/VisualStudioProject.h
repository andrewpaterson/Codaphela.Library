/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __VISUAL_STUDIO_PROJECT_H__
#define __VISUAL_STUDIO_PROJECT_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"


//The C++ sources of a Visual Studio project (its <ClCompile Include="..."/> entries).  Their names are the files' on
//disk: Visual Studio's need not match their case.
class CVisualStudioProject
{
protected:
	CChars		mszDirectory;
	CArrayChars	maszSources;

public:
	void		Init(void);
	void		Kill(void);

	bool		Read(const char* szProjectFile);

	char*		GetDirectory(void);
	size		NumSources(void);
	char*		GetSource(size uiIndex);

protected:
	void		MatchCase(CArrayChars* paszFiles, CChars* pszSource);
};


#endif // __VISUAL_STUDIO_PROJECT_H__

