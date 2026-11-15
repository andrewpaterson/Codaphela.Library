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
#ifndef __SHAPE_INTERSECTION_H__
#define __SHAPE_INTERSECTION_H__
#include "ShapeGroup.h"


//Everything in all of its shapes: the furthest's distance at each point (max), and a bound, as outside near two surfaces
//it's further to where they meet than to either.  Its bounds are where every shape's overlap, empty if they don't.
class CShapeIntersection : public CShapeGroup
{
CONSTRUCTABLE(CShapeIntersection);
DESTRUCTABLE(CShapeIntersection);
public:
	void	GetBounds(SFloat32Bounds* psDest) override;

protected:
	bool	Prefers(float32 fDistance, float32 fChosen) override;
};


#endif // __SHAPE_INTERSECTION_H__

