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


//The way a component faces on the screen: North is up, East is right, South is down and West is left.  Grid y
//increases downwards.  A component is drawn facing East, as it's normally read (its output to the right, towards +x,
//and -y up), and each other rotation turns it about its position to face that way.  A component with one port faces
//the way that port points out of it, so an upright ground faces North and an upright positive supply faces South.
//Rotating right (clockwise) goes North, East, South, West.
//
//A component can also be mirrored: it is drawn facing East and mirrored top to bottom (y becomes -y) before it is
//turned, so its output still points the way it faces.  Turning and mirroring together give all eight ways a component
//can lie; mirroring left to right is mirroring top to bottom and turning it half way round.  TransformPoint does both.
//
//Logicim's Rotation drew components facing North and had East and West the other way round: Logicim's West is East
//here, its East is West, and North and South are the same.  Its rotateRight goes North, West, South, East by its
//names, which is the same clockwise turn as RotateRight here.  Anything ported from Logicim that rotates or places
//components (rotate commands, the editor, file import) must be translated to these rotations to behave as it did in
//Logicim; Logicim's creation rotation, West, is East here.
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
void		TransformPoint(ERotation eRotation, bool bMirrored, SFloat64Vec2* psPoint);
void		TransformPoint(ERotation eRotation, bool bMirrored, SInt32Vec2* psPoint);
ERotation	MirrorRotation(ERotation eRotation);
void		TransformOrientation(ERotation* peRotation, bool* pbMirrored, bool bMirror, int iRightRotations);
const char*	RotationToString(ERotation eRotation);
ERotation	StringToRotation(const char* szRotation);


#endif // __ROTATION_H__
