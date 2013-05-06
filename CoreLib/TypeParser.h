/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __TYPE_PARSER_H__
#define __TYPE_PARSER_H__
#include "BaseLib/DataTypes.h"
#include "BaseLib/GeometricTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Number.h"
#include "BaseLib/Date.h"
#include "TextParser.h"


class CTypeParser
{
public:
	CTextParser*	mpcParser;
	BOOL			mbFreeParser;

	BOOL 		Init(char* szText, int iTextLen);
	BOOL 		Init(char* szText);
	BOOL 		Init(CChars* szText);
	void 		AllocateParser(void);
	void 		Init(CTextParser* pcParser);
	void 		Kill(void);

	TRISTATE	GetSmallIntegerValue(long long int* plliValue, long long int lliMax, long long int lliMin);
	TRISTATE	GetCharAndFloat(char c, float* pf);
	TRISTATE	GetCharAndInt(char c, int* pi);

	TRISTATE	ParseInt(int* pDest);  	
	TRISTATE	ParseChar(char* pDest);
	TRISTATE	ParseShort(short* pDest);
	TRISTATE	ParseFloat(float* pDest);
	TRISTATE	ParseDouble(double* pDest);
	TRISTATE	ParseUInt(unsigned int* pDest);
	TRISTATE	ParseUShort(unsigned short* pDest);
	TRISTATE	ParseUChar(unsigned char* pDest);
	TRISTATE	ParseLong(long long int* pDest);
	TRISTATE	ParseULong(unsigned long long int* pDest);
	TRISTATE	ParseFloat2(SFloat2* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseFloat3(SFloat3* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseFloat4(SFloat4* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseInt2(SInt2* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseInt3(SInt3* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseInt4(SInt4* pDest, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
	TRISTATE	ParseBool(BOOL* pDest);

	TRISTATE	ParseString(CChars* pDest);  
	TRISTATE	ParseNumber(CNumber* pDest);	
	TRISTATE	ParseDate(CDate* pDest);

	TRISTATE	Parse(EPrimitiveTypes eType, void* pvDest, int iDestLength, char cOpen = '\0', char cSplit = ',', char cClose = '\0');
};


#endif // __TYPE_PARSER_H__

