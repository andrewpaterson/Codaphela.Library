/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela DirectXLib

Codaphela DirectXLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela DirectXLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela DirectXLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX and associated libraries are Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __DEBUG_MOVEMENT_FLIGHT_STYLE_H__
#define __DEBUG_MOVEMENT_FLIGHT_STYLE_H__
#include "BaseLib/GeometricTypes.h"


#define BS_DOWN 1


class CDebugMovementFlightStyle
{
public:
	SFloat4x4*	mpsMatrix;
	float 		fx; 
	float 		fy; 
	int 		iStrafeLeft; 
	int 		iStrafeRight; 
	int 		iForward; 
	int 		iBackward; 
	int 		iRollLeft; 
	int 		iRollRight; 
	int 		iStrafeUp; 
	int 		iStrafeDown; 
	int 		iAccelerate;

	void Init(SFloat4x4* psMatrix);
	void Kill(void);
	void Update(void);
};


#endif // __DEBUG_MOVEMENT_FLIGHT_STYLE_H__

