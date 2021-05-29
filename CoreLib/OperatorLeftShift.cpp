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
#include "OperatorLeftShift.h"
#include "BaseLib/GeometricTypes.h"

void LeftShiftInvalid(void* pvDest, void* pvLeft, void* pvRight) { gcUserError.Set("LeftShift not defined for types"); }
#include "OperatorLeftShift_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorLeftShift::Init(void)
{
	int		iDest;
	int		iLeft;
	int		iRight;

	for (iDest = 0; iDest < PRIMTIVE_OPERATOR_END; iDest++)
	{
		for (iLeft = 0; iLeft < PRIMTIVE_OPERATOR_END; iLeft++)
		{
			for (iRight = 0; iRight < PRIMTIVE_OPERATOR_END; iRight++)
			{
				mapvLeftShift[iDest][iLeft][iRight] = LeftShiftInvalid;
			}
		}
	}
#include "OperatorLeftShift_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorLeftShift::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorLeftShift::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eLeft, void* pvLeft, EPrimitiveType eRight, void* pvRight)
{
	if ((eLeft < PRIMTIVE_OPERATOR_END) && (eRight < PRIMTIVE_OPERATOR_END) && (eDest < PRIMTIVE_OPERATOR_END))
	{
		mapvLeftShift[eDest][eLeft][eRight](pvDest, pvLeft, pvRight);
		return;
	}
	gcUserError.Set("Operator not defined for types (out of bounds).");
}

