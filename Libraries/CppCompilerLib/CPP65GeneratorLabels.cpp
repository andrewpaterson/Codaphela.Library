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
#include "CTReservedWord.h"
#include "CPP65Generator.h"


//The generator's labels: mangled names of functions, classes and types.


//////////////////////////////////////////////////////////////////////////
//
// A function name component's name: its identifier, or a negative number for an
// operator (see GetOperatorLabel).  -1 if it is neither.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetFunctionName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;
	ECppTokens	eToken;
	bool		bArray;

	if (pcComponent && (pcComponent->meKind == CPPNC_Conversion))
	{
		return CPP65_CONVERSION_NAME;
	}
	if (!pcComponent || (pcComponent->meKind != CPPNC_Operator))
	{
		return GetComponentName(pcComponent);
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken || !pcToken->IsReservedWord())
	{
		return -1;
	}
	eToken = ((CCTReservedWord*)pcToken)->GetToken();
	bArray = (pcComponent->miLastToken > pcComponent->miName) && ((eToken == CT_new) || (eToken == CT_delete));
	return GetOperatorName(eToken, bArray);
}


//////////////////////////////////////////////////////////////////////////
//
// Operators are named by their token: -1 - token, and -1 - NUM_CPP_TOKENS - token
// for new[] and delete[].
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetOperatorName(ECppTokens eToken, bool bArray)
{
	return bArray ? (-1 - NUM_CPP_TOKENS - (int64)eToken) : (-1 - (int64)eToken);
}


