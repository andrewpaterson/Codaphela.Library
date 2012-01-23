/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __MARKUP_TEXT_PARSER_H__
#define __MARKUP_TEXT_PARSER_H__
#include "BaseLib/MarkupTag.h"


//Why doesn't this use the TypeParser?
class CMarkupTextParser
{
public:
	static CMarkupTag*	GetTag(CMarkupTag* pcParentTag, char* szTagName, BOOL bLogErrors = TRUE);
	static BOOL 		ReadFloat(CMarkupTag* pcFloatTag, float* pcFloat, BOOL bLogErrors = TRUE);
	static BOOL 		ReadInteger(CMarkupTag* pcIntTag, int* pcInt, BOOL bLogErrors = TRUE);
	static BOOL 		ReadBool(CMarkupTag* pcIntTag, BOOL* pcBool, BOOL bLogErrors = TRUE);

	static void 		LogError(CMarkupTag* pcTag, char* szError);
	static void 		LogErrorTagNotFound(CMarkupTag* pcParentTag, char* szExpectedTag);
	static void 		LogErrorTagWasEmpty(CMarkupTag* pcTag);

private:
	static void 		TagToError(CChars* psz, CMarkupTag* pcParentTag);
};


#endif // __MARKUP_TEXT_PARSER_H__

