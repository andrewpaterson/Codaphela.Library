/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __SPECIAL_OPERATOR_MAP_H__
#define __SPECIAL_OPERATOR_MAP_H__
#include "BaseLib/IndexPrimitiveTemplate.h"
#include "ASCIITree.h"
#include "SpecialOperator.h"


typedef CIndexPrimitiveTemplate<int64, CSpecialOperator>	CSpecialOperatorsIndex;


class CSpecialOperatorMap
{
protected:
	CASCIITree				mcNameToIDIndex;
	CSpecialOperatorsIndex	mcIDToSpecialOperatorIndex;

public:
	void 					Init(void);
	void 					Kill(void);
	CSpecialOperator*		AddSpecialOperator(char* szName, EPreprocessorSpecialOperator eType);
	void					RemoveSpecialOperator(CExternalString* pcName);
	void					RemoveSpecialOperator(char* szName);
	CSpecialOperator*		GetSpecialOperator(CExternalString* pcName, BOOL bExact);
	CSpecialOperator*		GetSpecialOperator(CChars* pszName);
	CSpecialOperator*		GetSpecialOperator(char* szName, BOOL bExact);
	void					Dump(void);
	CSpecialOperatorsIndex* GetIDToSpecialOperatorIndex(void);
};


#endif // !__SPECIAL_OPERATOR_MAP_H__

