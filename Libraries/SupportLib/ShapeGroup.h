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
#ifndef __SHAPE_GROUP_H__
#define __SHAPE_GROUP_H__
#include "StandardLib/Array.h"
#include "Shape.h"


//Shapes of which one, chosen at each point, gives the distance and gradient there: the nearest for a union, the furthest for
//an intersection.  Where two are as near, the first.  The result is a bound, not exact.  With no shapes it's nothing, as far
//as can be from everywhere.
class CShapeGroup : public CShape
{
CONSTRUCTABLE(CShapeGroup);
DESTRUCTABLE(CShapeGroup);
public:
	CArray<CShape>	mapShapes;

	void			Init(void);
	void			Class(void) override;
	void			Free(void) override;

	bool			Add(Ptr<CShape> pShape);
	size			NumShapes(void);
	Ptr<CShape>		GetShape(size uiShape);

	float32			Distance(SFloat32Vec3* psPosition) override;
	void			Gradient(SFloat32Vec3* psDest, SFloat32Vec3* psPosition) override;
	bool			IsExact(void) override;

protected:
	virtual bool	Prefers(float32 fDistance, float32 fChosen) = 0;
	Ptr<CShape>		Choose(SFloat32Vec3* psPosition, float32* pfDistance);
};


#endif // __SHAPE_GROUP_H__

