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
#include "CTIdentifier.h"
#include "CTUserLiteral.h"
#include "CPPUnit.h"


//The names of functions and operators, as the unit keeps functions by them.  (The labels are CCPPMangler's.)


//////////////////////////////////////////////////////////////////////////
//
// A function name component's name: its identifier, or a negative number for an
// operator (GetOperatorName).  -1 if it is neither.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPUnit::GetFunctionName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;
	ECppTokens	eToken;
	bool		bArray;

	if (pcComponent && (pcComponent->meKind == CPPNC_Conversion))
	{
		return CPP_CONVERSION_NAME;
	}
	if (pcComponent && (pcComponent->meKind == CPPNC_Literal))
	{
		pcToken = mpcTokeniser->Get(pcComponent->miName);
		return CPP_LITERAL_NAME(pcToken->IsUserString() ? ((CCTUserLiteral*)pcToken)->miSuffix : ((CCTIdentifier*)pcToken)->miIdentifier);
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
// The identifier of a name component, or -1.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPUnit::GetComponentName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;

	if (!pcComponent || (pcComponent->miName < 0))
	{
		return -1;
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken->IsIdentifier())
	{
		return -1;
	}
	return ((CCTIdentifier*)pcToken)->miIdentifier;
}


//////////////////////////////////////////////////////////////////////////
//
// Operators are named by their token (CPP_OPERATOR_NAME), new[] and delete[]
// apart from new and delete.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPPUnit::GetOperatorName(ECppTokens eToken, bool bArray)
{
	return bArray ? CPP_ARRAY_OPERATOR_NAME(eToken) : CPP_OPERATOR_NAME(eToken);
}


