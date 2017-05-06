/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "GeometricTypes.h"
#include "Numbers.h"
#include "FloatHelper.h"
#include "PointerRemapper.h"
#include "PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* GetPosition(SFloat3* psPoints, int iStride, int iPoint)
{
	return (SFloat3*)RemapSinglePointer(psPoints, iStride*iPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* GetNormal(SFloat3* psNormals, int iStride, int iNormal)
{
	return (SFloat3*)RemapSinglePointer(psNormals, iStride*iNormal);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetIndex(SFloat3* psPoints, int iStride, SFloat3* psPosition)
{
	int		iIndex;
	int		iModulus;

	iIndex = ((int)(size_t)psPosition - (int)(size_t)psPoints);
	iModulus = iIndex % iStride;
	iIndex = iIndex / iStride;

	return iIndex;
}

