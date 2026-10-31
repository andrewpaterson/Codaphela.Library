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
#include "BaseLib/StringHelper.h"
#include "Rotation.h"


char* gaszRotationNames[] = { "Cannot", "North", "East", "South", "West" };


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERotation RotateRight(ERotation eRotation)
{
	switch (eRotation)
	{
	case ROT_North:
		return ROT_East;
	case ROT_East:
		return ROT_South;
	case ROT_South:
		return ROT_West;
	case ROT_West:
		return ROT_North;
	default:
		return ROT_Cannot;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Negative rotations rotate left.
//
//////////////////////////////////////////////////////////////////////////
ERotation RotateRight(ERotation eRotation, int iRightRotations)
{
	iRightRotations = iRightRotations % 4;
	if (iRightRotations < 0)
	{
		iRightRotations += 4;
	}
	while (iRightRotations > 0)
	{
		eRotation = RotateRight(eRotation);
		iRightRotations--;
	}
	return eRotation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERotation RotateLeft(ERotation eRotation)
{
	return RotateRight(eRotation, 3);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERotation OppositeRotation(ERotation eRotation)
{
	return RotateRight(eRotation, 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Counterclockwise on the screen from East.
//
//////////////////////////////////////////////////////////////////////////
int RotationToDegrees(ERotation eRotation)
{
	switch (eRotation)
	{
	case ROT_North:
		return 90;
	case ROT_West:
		return 180;
	case ROT_South:
		return 270;
	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsRotationEastWest(ERotation eRotation)
{
	return (eRotation == ROT_East) || (eRotation == ROT_West);
}


//////////////////////////////////////////////////////////////////////////
//
// Turns a point drawn facing East to face eRotation.
//
//////////////////////////////////////////////////////////////////////////
void RotatePoint(ERotation eRotation, SFloat64Vec2* psPoint)
{
	float64		x;
	float64		y;

	x = psPoint->x;
	y = psPoint->y;
	switch (eRotation)
	{
	case ROT_North:
		psPoint->Init(y, -x);
		break;
	case ROT_South:
		psPoint->Init(-y, x);
		break;
	case ROT_West:
		psPoint->Init(-x, -y);
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void RotatePoint(ERotation eRotation, SInt32Vec2* psPoint)
{
	int32	x;
	int32	y;

	x = psPoint->x;
	y = psPoint->y;
	switch (eRotation)
	{
	case ROT_North:
		psPoint->Init(y, -x);
		break;
	case ROT_South:
		psPoint->Init(-y, x);
		break;
	case ROT_West:
		psPoint->Init(-x, -y);
		break;
	default:
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Mirrors a point drawn facing East top to bottom, if bMirrored, and then
// turns it to face eRotation.
//
//////////////////////////////////////////////////////////////////////////
void TransformPoint(ERotation eRotation, bool bMirrored, SFloat64Vec2* psPoint)
{
	if (bMirrored)
	{
		psPoint->y = -psPoint->y;
	}
	RotatePoint(eRotation, psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TransformPoint(ERotation eRotation, bool bMirrored, SInt32Vec2* psPoint)
{
	if (bMirrored)
	{
		psPoint->y = -psPoint->y;
	}
	RotatePoint(eRotation, psPoint);
}


//////////////////////////////////////////////////////////////////////////
//
// The way a component faces once it's mirrored top to bottom on the
// screen: North and South swap, East and West stay.
//
//////////////////////////////////////////////////////////////////////////
ERotation MirrorRotation(ERotation eRotation)
{
	switch (eRotation)
	{
	case ROT_North:
		return ROT_South;
	case ROT_South:
		return ROT_North;
	default:
		return eRotation;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Mirrors a component lying as *peRotation and *pbMirrored top to bottom
// on the screen, if bMirror, and then turns it right iRightRotations
// times, as an edit that has been flipped and turned moves it.  A
// component that can't turn can't be mirrored either.
//
//////////////////////////////////////////////////////////////////////////
void TransformOrientation(ERotation* peRotation, bool* pbMirrored, bool bMirror, int iRightRotations)
{
	if (*peRotation == ROT_Cannot)
	{
		return;
	}
	if (bMirror)
	{
		*peRotation = MirrorRotation(*peRotation);
		*pbMirrored = !*pbMirrored;
	}
	*peRotation = RotateRight(*peRotation, iRightRotations);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* RotationToString(ERotation eRotation)
{
	if ((eRotation < ROT_Cannot) || (eRotation > ROT_West))
	{
		return "?";
	}
	return gaszRotationNames[eRotation];
}


//////////////////////////////////////////////////////////////////////////
//
// The rotation RotationToString names; ROT_Cannot for anything else.
//
//////////////////////////////////////////////////////////////////////////
ERotation StringToRotation(const char* szRotation)
{
	int		i;

	if (szRotation == NULL)
	{
		return ROT_Cannot;
	}
	for (i = ROT_North; i <= ROT_West; i++)
	{
		if (StringCompare(gaszRotationNames[i], szRotation) == 0)
		{
			return (ERotation)i;
		}
	}
	return ROT_Cannot;
}

