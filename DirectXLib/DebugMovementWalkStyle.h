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
#ifndef __DEBUG_MOVEMENT_WALK_STYLE_H__
#define __DEBUG_MOVEMENT_WALK_STYLE_H__
#include <d3dx9.h>


#define BS_DOWN 1


class CDebugMovementWalkStyle
{
public:
	D3DMATRIX*	 mpsMatrix;

	void Init(D3DMATRIX* psMatrix);
	void Kill(void);
	void Update(float fx, float fy, char iStrafeLeft, char iStrafeRight, char iForward, char iBackward, char iStrafeUp, char iStrafeDown, char iAccelerate);
};


#endif // __DEBUG_MOVEMENT_WALK_STYLE_H__

