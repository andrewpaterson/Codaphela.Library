/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "SpecialOperatorMap.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the special operator called szName.  Returns NULL if it is already
// in the map.
//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CSpecialOperatorMap::AddSpecialOperator(char* szName, EPreprocessorSpecialOperator eType)
{
	CExternalString		cExternalString;
	CSpecialOperator*	pcSpecialOperator;
	int64				lliID;

	cExternalString.Init(szName, StrLen(szName));
	pcSpecialOperator = AddEntry(&cExternalString, &lliID);
	if (pcSpecialOperator)
	{
		pcSpecialOperator->Init(szName, eType, lliID);
	}
	return pcSpecialOperator;
}

