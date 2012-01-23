/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __POSITION_HELPER_H__
#define __POSITION_HELPER_H__
#include "Position.h"


void PositionHelperMakeJump(CPosition* pcPosition, D3DXVECTOR3* psStart, float fHeight);
void PositionHelperMakeStationary(CPosition* pcPosition, D3DXVECTOR3* psStart);
void PositionHelperMakeQuarterCircle(CPosition* pcPosition, D3DXVECTOR3* psCenter, float fRadius, int iQuadrant);
void PositionHelperMakeStraightLine(CPosition* pcPosition, D3DXVECTOR3* psStart, D3DXVECTOR3* psEnd);
void PositionHelperMakeHalfCos(CPosition* pcPosition, D3DXVECTOR3* psStart, D3DXVECTOR3* psEnd, float fAmplitude);
void PositionHelperMakeBezierSpline(CPosition* pcPosition, D3DXVECTOR3* psStartPosition, D3DXVECTOR3* psStartControl, D3DXVECTOR3* psEndPosition, D3DXVECTOR3* psEndControl);


#endif //__POSITION_HELPER_H__

