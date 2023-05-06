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
#include "OperatorDecrement.h"
#include "GeometricTypes.h"

void DecrementInvalid(void* pvDest) { gcUserError.Set("Decrement not defined for type"); }
#include "OperatorDecrement_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorDecrement::Init(void)
{
	int		iDest;

	for (iDest = 0; iDest < PRIMTIVE_OPERATOR_END; iDest++)
	{
		mapvDecrement[iDest] = DecrementInvalid;
	}
#include "OperatorDecrement_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorDecrement::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorDecrement::Do(EPrimitiveType eDest, void* pvDest)
{
	if (eDest < PRIMTIVE_OPERATOR_END)
	{
		mapvDecrement[eDest](pvDest);
		return;
	}
	gcUserError.Set("Operator not defined for types (out of bounds).");
}

