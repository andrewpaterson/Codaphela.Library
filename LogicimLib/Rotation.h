/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ROTATION_H__
#define __ROTATION_H__
#include "BaseLib/Float64Vec2.h"
#include "BaseLib/Int32Vec2.h"


//The way a component faces, as Logicim's Rotation.  Grid y increases downwards.  A component is drawn facing North
//(its output up, towards -y) and each other rotation turns it about its position.  Rotating right goes North, West,
//South, East.
enum ERotation
{
	ROT_Cannot,
	ROT_North,
	ROT_East,
	ROT_South,
	ROT_West,
};


ERotation	RotateRight(ERotation eRotation);
ERotation	RotateRight(ERotation eRotation, int iRightRotations);
ERotation	RotateLeft(ERotation eRotation);
ERotation	OppositeRotation(ERotation eRotation);
int			RotationToDegrees(ERotation eRotation);
bool		IsRotationEastWest(ERotation eRotation);
void		RotatePoint(ERotation eRotation, SFloat64Vec2* psPoint);
void		RotatePoint(ERotation eRotation, SInt32Vec2* psPoint);
const char*	RotationToString(ERotation eRotation);


#endif // __ROTATION_H__
