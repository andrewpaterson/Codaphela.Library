/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "KicadSchemTransform.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemTransform::Init(float64 fAngle, EKicadSchemMirror eMirror)
{
	int32	iQuarterTurns;
	int32	i;

	//Unrotated: the library's Y axis is flipped onto the sheet's.
	miX1 = 1;
	miY1 = 0;
	miX2 = 0;
	miY2 = -1;

	if ((fAngle > -0.5) && (fAngle < 0.5))
	{
		iQuarterTurns = 0;
	}
	else if ((fAngle > 89.5) && (fAngle < 90.5))
	{
		iQuarterTurns = 1;
	}
	else if ((fAngle > 179.5) && (fAngle < 180.5))
	{
		iQuarterTurns = 2;
	}
	else if ((fAngle > 269.5) && (fAngle < 270.5))
	{
		iQuarterTurns = 3;
	}
	else
	{
		return false;
	}

	for (i = 0; i < iQuarterTurns; i++)
	{
		Multiply(0, 1, -1, 0);
	}

	if (eMirror == KSM_X)
	{
		Multiply(1, 0, 0, -1);
	}
	else if (eMirror == KSM_Y)
	{
		Multiply(-1, 0, 0, 1);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemTransform::Multiply(int32 iX1, int32 iY1, int32 iX2, int32 iY2)
{
	int32	iNewX1;
	int32	iNewY1;
	int32	iNewX2;
	int32	iNewY2;

	//The same product as SCH_SYMBOL::SetOrientation.
	iNewX1 = miX1 * iX1 + miX2 * iY1;
	iNewY1 = miY1 * iX1 + miY2 * iY1;
	iNewX2 = miX1 * iX2 + miX2 * iY2;
	iNewY2 = miY1 * iX2 + miY2 * iY2;

	miX1 = iNewX1;
	miY1 = iNewY1;
	miX2 = iNewX2;
	miY2 = iNewY2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemTransform::Transform(SKicadSchemPoint* psDest, SKicadSchemPoint* psLibrary)
{
	float64	fX;
	float64	fY;

	fX = miX1 * psLibrary->fX + miY1 * psLibrary->fY;
	fY = miX2 * psLibrary->fX + miY2 * psLibrary->fY;
	psDest->Init(fX, fY);
}

