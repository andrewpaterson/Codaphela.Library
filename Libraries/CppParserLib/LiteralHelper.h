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
#ifndef __LITERAL_HELPER_H__
#define __LITERAL_HELPER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CTString.h"


//What ParseIntegerLiteral found.
enum EIntegerLiteralResult
{
	ILR_Integer,
	ILR_Floating,		//A floating literal: szNumber holds it without digit separators.
	ILR_InvalidDigit,
	ILR_TooLarge,
	ILR_InvalidSuffix,
	ILR_NoDigits,		//0x or 0b with no digits after it.
};


//An integer literal's value and suffix.  [lex.icon]
struct SIntegerLiteral
{
	char	szNumber[256];	//The literal without digit separators.
	uint64	ulliValue;
	int		iBase;
	bool	bUnsigned;		//u or U.
	int		iLongs;			//0, 1 (l) or 2 (ll).
	bool	bInt64;			//Microsoft's i64.
};


//What ParseCharacterLiteral found.
enum ECharacterLiteralResult
{
	CLR_Character,
	CLR_Empty,
	CLR_InvalidEscape,
	CLR_Unterminated,	//No closing quote before the end of the line.
};


//A character literal's encoding and value.  [lex.ccon]
struct SCharacterLiteral
{
	ECTEncoding	eEncoding;
	uint32		uiValue;	//One character's, or several packed in (as MSVC does).
	int			iCount;		//How many characters.
	size		uiLength;	//The literal's length, prefix and quotes included.
};


EIntegerLiteralResult	ParseIntegerLiteral(const char* szText, size uiLength, SIntegerLiteral* psLiteral);
ECharacterLiteralResult	ParseCharacterLiteral(const char* szText, size uiLength, SCharacterLiteral* psLiteral);
int64					CharacterLiteralValue(SCharacterLiteral* psLiteral);
bool					DecodeEscape(const char** psz, const char* szEnd, uint32* puiValue);


#endif // !__LITERAL_HELPER_H__

