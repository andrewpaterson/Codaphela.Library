/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __MARKUP_TEXT_PARSER_H__
#define __MARKUP_TEXT_PARSER_H__
#include "MarkupTag.h"


//Why doesn't this use the TypeParser?
class CMarkupTextParser
{
public:
	static CMarkupTag* GetTag(CMarkupTag* pcParentTag, char* szTagName, bool bLogErrors = true);
	static bool 		ReadFloat(CMarkupTag* pcFloatTag, float* pcFloat, bool bLogErrors = true);
	static bool 		ReadInteger(CMarkupTag* pcIntTag, int* pcInt, bool bLogErrors = true);
	static bool 		ReadBool(CMarkupTag* pcIntTag, bool* pcBool, bool bLogErrors = true);

	static void 		LogError(CMarkupTag* pcTag, char* szError);
	static void 		LogErrorTagNotFound(CMarkupTag* pcParentTag, char* szExpectedTag);
	static void 		LogErrorTagWasEmpty(CMarkupTag* pcTag);

private:
	static void 		TagToError(CChars* psz, CMarkupTag* pcParentTag);
};


#endif // !__MARKUP_TEXT_PARSER_H__

