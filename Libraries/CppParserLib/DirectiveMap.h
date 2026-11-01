/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2021 Andrew Paterson

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
#ifndef __DIRECTIVE_MAP_H__
#define __DIRECTIVE_MAP_H__
#include "NameMap.h"
#include "PreprocessorDirective.h"


class CDirectiveMap : public CNameMap<CPreprocessorDirective>
{
public:
	CPreprocessorDirective*		AddDirective(CExternalString* pcName, EPreprocessorDirective eDirective);
	CPreprocessorDirective*		AddDirective(char* szName, EPreprocessorDirective eDirective);
	CPreprocessorDirective*		GetDirective(char* szName, bool bExact);
	void						RemoveDirective(char* szName);
};


#endif // !__DIRECTIVE_MAP_H__

