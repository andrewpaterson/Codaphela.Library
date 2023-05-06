/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "OperatorLessThanOrEqual.h"
#include "GeometricTypes.h"

void LessThanOrEqualInvalid(bool* pvDest, void* pvLeft, void* pvRight) { gcUserError.Set("LessThanOrEqual not defined for types"); }
#include "OperatorLessThanOrEqual_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorLessThanOrEqual::Init(void)
{
	int		iLeft;
	int		iRight;

	for (iLeft = 0; iLeft < PRIMTIVE_OPERATOR_END; iLeft++)
	{
		for (iRight = 0; iRight < PRIMTIVE_OPERATOR_END; iRight++)
		{
			mapvLessThanOrEqual[iLeft][iRight] = LessThanOrEqualInvalid;
		}
	}
#include "OperatorLessThanOrEqual_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorLessThanOrEqual::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorLessThanOrEqual::Do(bool* pvDest, EPrimitiveType eLeft, void* pvLeft, EPrimitiveType eRight, void* pvRight)
{
	if ((eLeft < PRIMTIVE_OPERATOR_END) && (eRight < PRIMTIVE_OPERATOR_END))
	{
		mapvLessThanOrEqual[eLeft][eRight](pvDest, pvLeft, pvRight);
		return;
	}
	gcUserError.Set("Operator not defined for types (out of bounds).");
}

