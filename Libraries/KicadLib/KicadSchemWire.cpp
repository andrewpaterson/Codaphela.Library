/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadLib

Codaphela KicadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "KicadSchemWire.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemWire::Init(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, bool bBus)
{
	msStart = *psStart;
	msEnd = *psEnd;
	mbBus = bBus;
	mszUuid.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemWire::Kill(void)
{
	mszUuid.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemWire::SetUuid(char* szUuid)
{
	mszUuid.Set(szUuid);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CKicadSchemWire::GetStart(void)
{
	return &msStart;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat64Vec2* CKicadSchemWire::GetEnd(void)
{
	return &msEnd;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemWire::IsBus(void)
{
	return mbBus;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CKicadSchemWire::GetUuid(void)
{
	return mszUuid.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemWire::IsEnd(SFloat64Vec2* psPoint)
{
	return msStart.CloselyEqual(psPoint, KICAD_SCHEM_POINT_TOLERANCE) || msEnd.CloselyEqual(psPoint, KICAD_SCHEM_POINT_TOLERANCE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemWire::Contains(SFloat64Vec2* psPoint)
{
	float64	fDX;
	float64	fDY;
	float64	fPX;
	float64	fPY;
	float64	fCross;
	float64	fDot;
	float64	fLengthSquared;

	//True if the point is on the segment, ends included.
	fDX = msEnd.x - msStart.x;
	fDY = msEnd.y - msStart.y;
	fPX = psPoint->x - msStart.x;
	fPY = psPoint->y - msStart.y;
	fLengthSquared = fDX * fDX + fDY * fDY;
	if (fLengthSquared == 0.0)
	{
		return msStart.CloselyEqual(psPoint, KICAD_SCHEM_POINT_TOLERANCE);
	}

	fCross = fDX * fPY - fDY * fPX;
	if ((fCross * fCross) > (KICAD_SCHEM_POINT_TOLERANCE * KICAD_SCHEM_POINT_TOLERANCE * fLengthSquared))
	{
		return false;
	}

	fDot = fDX * fPX + fDY * fPY;
	return (fDot >= -KICAD_SCHEM_POINT_TOLERANCE) && (fDot <= fLengthSquared + KICAD_SCHEM_POINT_TOLERANCE);
}

