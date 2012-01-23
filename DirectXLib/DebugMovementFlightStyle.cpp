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
#include <d3dx9.h>
#include "DebugMovementFlightStyle.h"
#include "BaseLib/GeometricTypes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMovementFlightStyle::Init(SFloat4x4* psMatrix)
{
	mpsMatrix = psMatrix;
	fx = 0; 
	fy = 0; 
	iStrafeLeft = 0; 
	iStrafeRight = 0; 
	iForward = 0; 
	iBackward = 0; 
	iRollLeft = 0; 
	iRollRight = 0; 
	iStrafeUp = 0; 
	iStrafeDown = 0; 
	iAccelerate = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMovementFlightStyle::Kill(void)
{
	mpsMatrix = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMovementFlightStyle::Update(void)
{
	float		acc;
	SFloat4x4	r, s;
	float		s1;
	float		s2;

	//Get acceleration.
	acc = 0.1f;
	if (iAccelerate & BS_DOWN)
		acc = 0.5;
	acc *= 10.0f;

	s1 = 1.0f;
	s2 = 0.8f;

	Float4x4Inverse(mpsMatrix, NULL, mpsMatrix);

	//Get forward and backwards.
	Float4x4Identity(&r);
	if (iForward & BS_DOWN)
		Float4x4Translation(&r, 0, 0, s1*acc);
	if (iBackward & BS_DOWN)
		Float4x4Translation(&r, 0, 0, -s1*acc);
	Float4x4Multiply(mpsMatrix, mpsMatrix, &r);

	//Get strafe up and down.
	Float4x4Identity(&r);
	if (iStrafeUp & BS_DOWN)
		Float4x4Translation(&r, 0, -s2*acc, 0);
	if (iStrafeDown & BS_DOWN)
		Float4x4Translation(&r, 0, s2*acc,0);
	Float4x4Multiply(mpsMatrix, mpsMatrix, &r);

	//Get strafe left and right.
	Float4x4Identity(&r);
	if (iStrafeLeft & BS_DOWN)
		Float4x4Translation(&r, s2*acc, 0, 0);
	if (iStrafeRight & BS_DOWN)
		Float4x4Translation(&r, -s2*acc, 0, 0);
	Float4x4Multiply(mpsMatrix, mpsMatrix, &r);

	//Get roll left and right.
	Float4x4Identity(&r);
	if (iRollLeft & BS_DOWN)
		Float4x4RotationZ(&r, -0.005f*acc);
	if (iRollRight & BS_DOWN)
		Float4x4RotationZ(&r, 0.005f*acc);

	Float4x4Multiply(mpsMatrix, mpsMatrix, &r);
	Float4x4RotationY(&s, fx/200);
	Float4x4Multiply(mpsMatrix, mpsMatrix, &s);
	Float4x4RotationX(&s, -fy/200.0f);
	Float4x4Multiply(mpsMatrix, mpsMatrix, &s);

	Float4x4Inverse(mpsMatrix, NULL, mpsMatrix);
}

