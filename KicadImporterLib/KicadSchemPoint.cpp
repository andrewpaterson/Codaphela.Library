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
#include "KicadSchemPoint.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SKicadSchemPoint::Init(void)
{
	fX = 0.0;
	fY = 0.0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SKicadSchemPoint::Init(float64 fX, float64 fY)
{
	this->fX = fX;
	this->fY = fY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SKicadSchemPoint::Equals(SKicadSchemPoint* psOther)
{
	float64	fDX;
	float64	fDY;

	fDX = fX - psOther->fX;
	fDY = fY - psOther->fY;
	return (fDX < KICAD_SCHEM_POINT_TOLERANCE) && (fDX > -KICAD_SCHEM_POINT_TOLERANCE) &&
		(fDY < KICAD_SCHEM_POINT_TOLERANCE) && (fDY > -KICAD_SCHEM_POINT_TOLERANCE);
}

