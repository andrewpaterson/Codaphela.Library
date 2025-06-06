/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __TYPE_PARSER_H__
#define __TYPE_PARSER_H__
#include "DataTypes.h"
#include "Int2.h"
#include "Int3.h"
#include "Int4.h"
#include "GeometricTypes.h"
#include "Chars.h"
#include "Number.h"
#include "Date.h"
#include "TextParser.h"


class CTypeParser
{
public:
	CTextParser*	mpcParser;
	bool			mbFreeParser;

	bool 		Init(char* szText, size iTextLen);
	bool 		Init(char* szText);
	bool 		Init(CChars* szText);
	void 		AllocateParser(void);
	void 		Init(CTextParser* pcParser);
	void 		Kill(void);

	TRISTATE	GetSmallIntegerValue(int64* plliValue, int64 lliMax, int64 lliMin);
	TRISTATE	GetCharAndFloat(char c, float* pf);
	TRISTATE	GetCharAndInt(char c, int32* pi);

	TRISTATE	ParseInt(int32* pDest);  	
	TRISTATE	ParseChar(char* pDest);
	TRISTATE	ParseShort(int16* pDest);
	TRISTATE	ParseFloat(float* pDest);
	TRISTATE	ParseDouble(double* pDest);
	TRISTATE	ParseUInt(uint32* pDest);
	TRISTATE	ParseUShort(uint16* pDest);
	TRISTATE	ParseUChar(uint8* pDest);
	TRISTATE	ParseLong(int64* pDest);
	TRISTATE	ParseULong(uint64* pDest);
	TRISTATE	ParseFloat2(SFloat2* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseFloat3(SFloat3* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseFloat4(SFloat4* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseInt2(SInt2* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseInt3(SInt3* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseInt4(SInt4* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseBool(bool* pDest);

	TRISTATE	ParseString(CChars* pDest);  
	TRISTATE	ParseNumber(CNumber* pDest);	
	TRISTATE	ParseDate(CDate* pDest);

	TRISTATE	Parse(EPrimitiveType eType, void* pvDest, size iDestLength, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
};


#endif // __TYPE_PARSER_H__

