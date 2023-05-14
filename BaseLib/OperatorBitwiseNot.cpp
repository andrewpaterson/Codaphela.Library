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
#include "OperatorBitwiseNot.h"
#include "GeometricTypes.h"

void BitwiseNotInvalid(void* pvDest, void* pvRight) { gcUserError.Set("Not not defined for types"); }
#include "OperatorBitwiseNot_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorBitwiseNot::Init(void)
{
	int		iDest;
	int		iRight;

	for (iDest = 0; iDest < PRIMTIVE_OPERATOR_END; iDest++)
	{
		for (iRight = 0; iRight < PRIMTIVE_OPERATOR_END; iRight++)
		{
			mapvBitwiseNot[iDest][iRight] = BitwiseNotInvalid;
		}
	}
#include "OperatorBitwiseNot_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorBitwiseNot::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorBitwiseNot::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eRight, void* pvRight)
{
	if ((eRight < PRIMTIVE_OPERATOR_END) && (eDest < PRIMTIVE_OPERATOR_END))
	{
		mapvBitwiseNot[eDest][eRight](pvDest, pvRight);
		return;
	}
	gcUserError.Set("Operator not defined for types (out of bounds).");
}

