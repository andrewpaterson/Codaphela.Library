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
#include "OperatorBitwiseNot.h"
#include "BaseLib/GeometricTypes.h"

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
void COperatorBitwiseNot::Do(EPrimitiveTypes eDest, void* pvDest, EPrimitiveTypes eRight, void* pvRight)
{
	mapvBitwiseNot[eDest][eRight](pvDest, pvRight);
}
