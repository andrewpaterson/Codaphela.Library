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

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#ifndef __SOLID_TEXTURE_H__
#define __SOLID_TEXTURE_H__
#include "BaseLib/GeometricTypes.h"
#include "StandardLib/Object.h"
#include "ColourARGB32.h"


//A colour at every point in space (a solid texture, Peachey and Perlin 1985), which a bake reads at a surface's points
//(UnwrapPlan.txt, Bake).  Its scale is in world units, so how a surface looks doesn't depend on how it's unwrapped or how many
//texels it has.  fWidth is how wide, in world units, the texel being baked is: a texture whose detail is finer than that gives
//its average over it rather than one point's colour, so a coarse mip level is the finer levels' blur, not their noise.
class CSolidTexture : public CObject
{
CONSTRUCTABLE(CSolidTexture);
DESTRUCTABLE(CSolidTexture);
public:
	void			Init(void);
	void			Class(void) override;
	void			Free(void) override;

	virtual ARGB32	Colour(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth) = 0;
};


#endif // __SOLID_TEXTURE_H__

