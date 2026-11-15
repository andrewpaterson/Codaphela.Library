/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __REVOLVED_SHAPE_H__
#define __REVOLVED_SHAPE_H__
#include "BaseLib/Float32Vec2.h"
#include "Shape.h"


//A shape turned around an axis from msStart along msAxis (a unit), as a mesh editor's revolved shapes are.  A point is turned
//into the plane of its profile: how far it is from the axis, and how far along the axis from msStart.  The profile's distance
//there is the shape's, exactly, as long as the profile doesn't cross the axis; its gradient there turns back into the
//shape's.
class CRevolvedShape : public CShape
{
CONSTRUCTABLE(CRevolvedShape);
DESTRUCTABLE(CRevolvedShape);
public:
	SFloat32Vec3	msStart;
	SFloat32Vec3	msAxis;
	float32			mfLength;

	void			Init(SFloat32Vec3* psAxis, SFloat32Vec3* psStart, float32 fLength);  //psAxis needn't be a unit.
	void			Class(void) override;
	void			Free(void) override;

	float32			Distance(SFloat32Vec3* psPosition) override;
	void			Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;

	//The distance from the profile of a point fRadial from the axis and fAlong it, and the gradient there (x out from the axis,
	//y along it).
	virtual float32	ProfileDistance(float32 fRadial, float32 fAlong, SFloat32Vec2* psGradient) = 0;

protected:
	void			ToProfile(SFloat32Vec3* psPosition, float32* pfRadial, float32* pfAlong, SFloat32Vec3* psOutward);
	void			AddDisc(SFloat32Bounds* psBounds, float32 fAlong, float32 fRadius);
	static float32	BoxProfileDistance(float32 x, float32 y, float32 fHalfWidth, float32 fHalfHeight, SFloat32Vec2* psGradient);
	static float32	PointProfileDistance(float32 x, float32 y, SFloat32Vec2* psGradient);
};


#endif // __REVOLVED_SHAPE_H__

