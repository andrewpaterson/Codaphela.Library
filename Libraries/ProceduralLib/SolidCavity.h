/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ProceduralLib

Codaphela ProceduralLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ProceduralLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ProceduralLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __SOLID_CAVITY_H__
#define __SOLID_CAVITY_H__
#include "StandardLib/Pointer.h"
#include "SupportLib/Shape.h"
#include "SupportLib/SolidTexture.h"


//Where a shape's surface is hollowed and where it's edged, as a value, from the shape's own distances (TexturePlan.txt,
//Operators): a half on a flat or evenly curved surface, towards 0 in a groove or an inside corner, towards 1 on an outside
//edge or corner.  Through a ramp it's dirt gathered in the hollows and wear along the edges, which a texture of points alone
//can't know.
//
//mfReach out from the surface along its normal, the shape's surface is mfReach away if nothing else is near; in a hollow
//another part of it is nearer.  mfReach in, it's mfReach deep; under an edge another face is nearer.  How much nearer each
//is, as a share of mfReach, is how hollowed and how edged the surface is there, and the value is a half, less half the one,
//plus half the other.  So mfReach is how wide a hollow or an edge shows.
//
//The shape is in the space the texture is placed in (the world's, for a texture nothing else holds), not the texture's own.
class CSolidCavity : public CSolidTexture
{
CONSTRUCTABLE(CSolidCavity);
DESTRUCTABLE(CSolidCavity);
public:
	Ptr<CShape>	mpShape;
	float32		mfReach;

	void	Init(Ptr<CShape> pShape, float32 fReach);
	void	Class(void) override;

protected:
	void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) override;
};


#endif // __SOLID_CAVITY_H__

