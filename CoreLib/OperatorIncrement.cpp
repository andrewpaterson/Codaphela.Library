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
#include "OperatorIncrement.h"
#include "BaseLib/GeometricTypes.h"

void IncrementInvalid(void* pvDest) { gcUserError.Set("Increment not defined for type"); }
#include "OperatorIncrement_Functions.inl"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorIncrement::Init(void)
{
	int		iDest;

	for (iDest = 0; iDest < PRIMTIVE_OPERATOR_END; iDest++)
	{
		mapvIncrement[iDest] = IncrementInvalid;
	}
#include "OperatorIncrement_Mapping.inl"
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorIncrement::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorIncrement::Do(EPrimitiveTypes eDest, void* pvDest)
{
	mapvIncrement[eDest](pvDest);
}
