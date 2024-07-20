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
#ifndef __TYPE_NAMES_H__
#define __TYPE_NAMES_H__
#include "MapStringInt.h"


struct STypeName
{
	EPrimitiveType		eType;
	size					iByteSize;
	size					iBitSize;
	char*				szPrettyName;
	char*				szCppName;
	char*				szPrimitiveName;
};


typedef CArrayTemplate<STypeName> CArrayTypeNames;


class CTypeNames
{
private:
	CArrayTypeNames		masTypeNames;
	CMapStringInt		mmsziPrettyNames;
	CMapStringInt		mmsziCppNames;

public:
	void 				Init(void);
	void 				Kill(void);

	const char* 		GetPrettyName(EPrimitiveType eType);
	const char* 		GetCPPName(EPrimitiveType eType);
	const char* 		GetPrimitiveName(EPrimitiveType eType);
	EPrimitiveType		GetTypeFromPrettyName(const char* szPrettyName);
	EPrimitiveType		GetTypeFromCPPName(const char* szPrettyName);
	size				GetByteSize(EPrimitiveType eType);
	size				GetBitSize(EPrimitiveType eType);

private:
	void				AddType(EPrimitiveType eType, size iSize, const char* szPrettyName, const char* szCppName, const char* szPrimitiveName);
};


extern CTypeNames	gcTypeNames;
extern bool			gbTypeNames;


void TypesInit(void);
void TypesKill(void);
bool TypesValidate(void);


void PrintPrimitiveType(EPrimitiveType eType, CChars* psz);
char* PrimitiveTypeToString(EPrimitiveType eType);


#endif // __TYPE_NAMES_H__

