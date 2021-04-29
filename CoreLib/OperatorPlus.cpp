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
#include "OperatorPlus.h"
#include "BaseLib/GeometricTypes.h"

void PlusInvalid(void* pvDest, void* pvLeft, void* pvRight) { gcUserError.Set("Plus not defined for types"); }
#include "OperatorPlus_Functions.inl"
void PlusFloat2Float2Float2(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat2*)pvDest = *(SFloat2*)pvLeft + *(SFloat2*)pvRight; }
void PlusFloat3Float3Float3(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat3*)pvDest = *(SFloat3*)pvLeft + *(SFloat3*)pvRight; }
void PlusFloat4Float4Float4(void* pvDest, void* pvLeft, void* pvRight) { *(SFloat4*)pvDest = *(SFloat4*)pvLeft + *(SFloat4*)pvRight; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorPlus::Init(void)
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
				mapvPlus[iDest][iLeft][iRight] = PlusInvalid;
			}
		}
	}
	#include "OperatorPlus_Mapping.inl"
	mapvPlus[PT_float2][PT_float2][PT_float2] = PlusFloat2Float2Float2;
	mapvPlus[PT_float3][PT_float3][PT_float3] = PlusFloat3Float3Float3;
	mapvPlus[PT_float4][PT_float4][PT_float4] = PlusFloat4Float4Float4;
	//mapvPlus[PT_int2][PT_int2][PT_int2] = PlusInt2Int2Int2;
	//mapvPlus[PT_int3][PT_int3][PT_int3] = PlusInt3Int3Int3;
	//mapvPlus[PT_int4][PT_int4][PT_int4] = PlusInt4Int4Int4;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorPlus::Kill(void)
{
	//Does Nothing...
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COperatorPlus::Do(EPrimitiveType eDest, void* pvDest, EPrimitiveType eLeft, void* pvLeft, EPrimitiveType eRight, void* pvRight)
{
	mapvPlus[eDest][eLeft][eRight](pvDest, pvLeft, pvRight);
}
