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
#include "DebugMovementWalkStyle.h"
#include "BaseLib/GeometricTypes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMovementWalkStyle::Init(D3DMATRIX* psMatrix)
{
	mpsMatrix = psMatrix;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMovementWalkStyle::Kill(void)
{
	mpsMatrix = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDebugMovementWalkStyle::Update(float fx, float fy, char iStrafeLeft, char iStrafeRight, char iForward, char iBackward, char iStrafeUp, char iStrafeDown, char iAccelerate)
{
	float		acc;
	SFloat4x4	r, s;
	float		s1;
	float		s2;
	SFloat3		at;
	SFloat3		atNew;
	SFloat3		left;
	SFloat3		leftNew;

	//Get acceleration.
	acc = 0.1f;
	if (iAccelerate & BS_DOWN)
		acc = 0.5;
	acc *= 10.0f;

	s1 = 1.0f;
	s2 = 0.8f;

	//Get forward and backwards.
	at.Copy(&((SFloat4x4*)mpsMatrix)->x);
	at.y = 0;
	at.Normalize();
	Float4x4Identity((SFloat4x4*)&r);
	if (iForward & BS_DOWN)
		atNew = at * s1*acc;
	else if (iBackward & BS_DOWN)
		atNew = at * -s1*acc;
	else
		atNew.Init(0,0,0);
	Float4x4Translation((SFloat4x4*)&r, atNew.z, atNew.y, atNew.x);
	Float4x4Multiply((SFloat4x4*)mpsMatrix, (SFloat4x4*)&r, (SFloat4x4*)mpsMatrix);

	//Get strafe left and right.
	left.Copy(&((SFloat4x4*)mpsMatrix)->z);
	left.y = 0;
	left.Normalize();
	Float4x4Identity((SFloat4x4*)&r);
	if (iStrafeLeft & BS_DOWN)
		leftNew = left * s2*acc;
	else if (iStrafeRight & BS_DOWN)
		leftNew = left * -s2*acc;
	else
		leftNew.Init(0,0,0);
	Float4x4Translation((SFloat4x4*)&r, leftNew.z, leftNew.y, leftNew.x);
	Float4x4Multiply((SFloat4x4*)mpsMatrix, (SFloat4x4*)&r, (SFloat4x4*)mpsMatrix);

	Float4x4RotationY((SFloat4x4*)&s, fx/200);
	Float4x4Multiply((SFloat4x4*)mpsMatrix, (SFloat4x4*)mpsMatrix, (SFloat4x4*)&s);

	//Get strafe up and down.
	Float4x4Identity((SFloat4x4*)&r);
	if (iStrafeUp & BS_DOWN)
		Float4x4Translation((SFloat4x4*)&r, 0, -s2*acc, 0);
	if (iStrafeDown & BS_DOWN)
		Float4x4Translation((SFloat4x4*)&r, 0, s2*acc,0);
	Float4x4Multiply((SFloat4x4*)mpsMatrix, (SFloat4x4*)&r, (SFloat4x4*)mpsMatrix);
}

