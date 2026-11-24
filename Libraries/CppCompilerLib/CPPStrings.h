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
#ifndef __C_P_P_STRINGS_H__
#define __C_P_P_STRINGS_H__
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/PrimitiveTypes.h"


class CCPPUnit;
class CCPPSyntax;
class CCTString;
struct SCPP65Type;


//Decodes the unit's string literals (CCPPUnit::GetStrings): their bytes in their encoding's elements (narrow, UTF-8,
//UTF-16, UTF-32, wide), raw strings, adjacent literals joined, and the names MSVC's __FUNCTION__, __FUNCSIG__ and
//__FUNCDNAME__ stand for, which are the function's being worked out (the unit's context) and its label.
class CCPPStrings
{
protected:
	CCPPUnit*				mpcUnit;

public:
	void					Init(CCPPUnit* pcUnit);
	void					Kill(void);

	bool					DecodeString(CCPPSyntax* pcString, CArrayUint8* pauiBytes);
	int						StringElementBytes(CCPPSyntax* pcString);
	SCPP65Type*				StringArrayType(CCPPSyntax* pcString);
	SCPP65Type*				StringElementType(CCPPSyntax* pcString);
	uint32					StringElement(CArrayUint8* pauiBytes, int iElementBytes, size uiElement);
	void					DecodeRawStringToken(CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes);
	bool					DecodeStringToken(CCPPSyntax* pcString, CCTString* pcToken, int iElementBytes, CArrayUint8* pauiBytes);

	bool					FunctionNameText(int64 lliName, CChars* pszText);
	bool					IsFunctionName(CCPPSyntaxIdExpression* pcId);
};


#endif // __C_P_P_STRINGS_H__

