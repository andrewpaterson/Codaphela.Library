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
#include "OperatorMinus.h"
#include "GeometricTypes.h"

void MinusInvalid(void* pvDest, void* pvLeft, void* pvRight) { gcUserError.Set("Minus not defined for types"); }
#include "OperatorMinus_Functions.inl"
void MinusFloat2Float2Float2(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat2*)pvDest = *(SFloat2*)pvLeft - *(SFloat2*)pvRight; }
void MinusFloat3Float3Float3(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat3*)pvDest = *(SFloat3*)pvLeft - *(SFloat3*)pvRight; }
void MinusFloat4Float4Float4(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat4*)pvDest = *(SFloat4*)pvLeft - *(SFloat4*)pvRight; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorMinus::Init(void)
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
				mapvMinus[iDest][iLeft][iRight] = MinusInvalid;
			}
		}
	}
#include "OperatorMinus_Mapping.inl"
	mapvMinus[PT_M2float32][PT_M2float32][PT_M2float32] = MinusFloat2Float2Float2;
	mapvMinus[PT_M3float32][PT_M3float32][PT_M3float32] = MinusFloat3Float3Float3;
	mapvMinus[PT_M4float32][PT_M4float32][PT_M4float32] = MinusFloat4Float4Float4;
	//mapvMinus[PT_M2int32][PT_M2int32][PT_M2int32] = MinusInt2Int2Int2;
	//mapvMinus[PT_M3int32][PT_M3int32][PT_M3int32] = MinusInt3Int3Int3;
	//mapvMinus[PT_M4int32][PT_M4int32][PT_M4int32] = MinusInt4Int4Int4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorMinus::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorMinus::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eLeft, void* pvLeft, EPrimitiveType eRight, void* pvRight)
{
	if ((eLeft < PRIMTIVE_OPERATOR_END) && (eRight < PRIMTIVE_OPERATOR_END))
	{
		mapvMinus[eDest][eLeft][eRight](pvDest, pvLeft, pvRight);
		return;
	}
	gcUserError.Set("Operator not defined for types (out of bounds).");
}

