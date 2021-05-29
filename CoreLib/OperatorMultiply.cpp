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
#include "OperatorMultiply.h"
#include "BaseLib/GeometricTypes.h"

void MultiplyInvalid(void* pvDest, void* pvLeft, void* pvRight) { gcUserError.Set("Multiply not defined for types"); }
#include "OperatorMultiply_Functions.inl"
void MultiplyFloat2Float2Float(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat2*)pvDest = *(SFloat2*)pvLeft * *(float*)pvRight; }
void MultiplyFloat3Float3Float(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat3*)pvDest = *(SFloat3*)pvLeft * *(float*)pvRight; }
void MultiplyFloat4Float4Float(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat4*)pvDest = *(SFloat4*)pvLeft * *(float*)pvRight; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorMultiply::Init(void)
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
				mapvMultiply[iDest][iLeft][iRight] = MultiplyInvalid;
			}
		}
	}
#include "OperatorMultiply_Mapping.inl"
	mapvMultiply[PT_M2float32][PT_M2float32][PT_float32] = MultiplyFloat2Float2Float;
	mapvMultiply[PT_M3float32][PT_M3float32][PT_float32] = MultiplyFloat3Float3Float;
	mapvMultiply[PT_M4float32][PT_M4float32][PT_float32] = MultiplyFloat4Float4Float;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorMultiply::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorMultiply::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eLeft, void* pvLeft, EPrimitiveType eRight, void* pvRight)
{
	if ((eLeft < PRIMTIVE_OPERATOR_END) && (eRight < PRIMTIVE_OPERATOR_END))
	{
		mapvMultiply[eDest][eLeft][eRight](pvDest, pvLeft, pvRight);
		return;
	}
	gcUserError.Set("Operator not defined for types (out of bounds).");
}

