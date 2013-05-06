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
#include "OperatorEqual.h"
#include "BaseLib/GeometricTypes.h"

void EqualInvalid(BOOL* pvDest, void* pvLeft, void* pvRight) { gcUserError.Set("Equal not defined for types"); }
#include "OperatorEqualNumber_Functions.inl"
#include "OperatorEqualBool_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorEqual::Init(void)
{
	int		iLeft;
	int		iRight;

	for (iLeft = 0; iLeft < PRIMTIVE_OPERATOR_END; iLeft++)
	{
		for (iRight = 0; iRight < PRIMTIVE_OPERATOR_END; iRight++)
		{
			mapvEqual[iLeft][iRight] = EqualInvalid;
		}
	}
#include "OperatorEqualNumber_Mapping.inl"
#include "OperatorEqualBool_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorEqual::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorEqual::Do(BOOL* pvDest, EPrimitiveTypes eLeft, void* pvLeft, EPrimitiveTypes eRight, void* pvRight)
{
	mapvEqual[eLeft][eRight](pvDest, pvLeft, pvRight);
}
