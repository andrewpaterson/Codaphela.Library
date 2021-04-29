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
#include "OperatorRightShift.h"
#include "BaseLib/GeometricTypes.h"

void RightShiftInvalid(void* pvDest, void* pvLeft, void* pvRight) { gcUserError.Set("RightShift not defined for types"); }
#include "OperatorRightShift_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorRightShift::Init(void)
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
				mapvRightShift[iDest][iLeft][iRight] = RightShiftInvalid;
			}
		}
	}
#include "OperatorRightShift_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorRightShift::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorRightShift::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eLeft, void* pvLeft, EPrimitiveType eRight, void* pvRight)
{
	mapvRightShift[eDest][eLeft][eRight](pvDest, pvLeft, pvRight);
}
