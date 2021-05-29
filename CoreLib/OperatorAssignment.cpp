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
#include "OperatorAssignment.h"
#include "BaseLib/GeometricTypes.h"

void AssignmentInvalid(void* pvDest, void* pvLeft) { gcUserError.Set("Assignment not defined for types"); }
#include "OperatorAssignment_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorAssignment::Init(void)
{
	int		iDest;
	int		iLeft;

	for (iDest = 0; iDest < PRIMTIVE_OPERATOR_END; iDest++)
	{
		for (iLeft = 0; iLeft < PRIMTIVE_OPERATOR_END; iLeft++)
		{
			mapvAssignment[iDest][iLeft] = AssignmentInvalid;
		}
	}
#include "OperatorAssignment_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorAssignment::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorAssignment::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eLeft, void* pvLeft)
{
	if ((eLeft < PRIMTIVE_OPERATOR_END) && (eLeft < PRIMTIVE_OPERATOR_END))
	{
		mapvAssignment[eDest][eLeft](pvDest, pvLeft);
		return;
	}
	gcUserError.Set("Operator not defined for types (out of bounds).");
}

