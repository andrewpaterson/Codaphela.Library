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
#ifndef __PP_SPECIAL_OPERATOR_H__
#define __PP_SPECIAL_OPERATOR_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ExternalString.h"


enum EPreprocessorSpecialOperator
{
	PPSO_Defined,
	PPSO_HasAttribute,
	PPSO_HasCppAttribute,
	PPSO_HasBuiltIn,
	PPSO_HasInclude,
	PPSO_HasIncludeNext,
	PPSO_HasFeature
};


class CSpecialOperator
{
public:
	CChars							mszName;
	EPreprocessorSpecialOperator	meType;
	int64							mlliID;

	void 							Init(char* pcName, EPreprocessorSpecialOperator eType, int64 lliID);
	void 							Kill(void);
	bool 							Equals(CSpecialOperator* pcOther);
	char*							GetName(void);
	EPreprocessorSpecialOperator	GetType(void);
	void							Dump(void);
};


#endif // !__PP_SPECIAL_OPERATOR_H__

