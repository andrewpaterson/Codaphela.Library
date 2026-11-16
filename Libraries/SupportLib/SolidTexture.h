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


//What a solid texture gives at a point (ProceduralLib/TexturePlan.txt).  Floats, not clamped, so a chain of operators neither
//bands nor loses what goes out of range on the way.  A texture that is only a value (noise) gives it as a grey colour and as
//the height, so any texture can feed any operator.
struct SSolidSample
{
	SFloat32Vec4	sColour;  //Red, green, blue and alpha (x, y, z, w), 0 to 1.
	float32			fHeight;  //How far the surface is raised, in the texture's units; 0 where it has none.
	float32			fShine;   //How strong its highlight is, 0 to 1.
	float32			fGloss;   //How tight its highlight is, 0 (broad) to 1 (a point).
	float32			fGlow;    //How much of its colour it gives off as light, 0 to 1.

	void	Init(void);  //Opaque black, flat, dull and dark.
	void	Init(ARGB32 uiColour);
	ARGB32	Colour(void);  //Its colour clamped to 0 to 1 and rounded to 8 bits a channel.
};


//A sample at every point in space (a solid texture, Peachey and Perlin 1985), which a bake reads at a surface's points
//(UnwrapPlan.txt, Bake).  Its scale is in world units, so how a surface looks doesn't depend on how it's unwrapped or how many
//texels it has.  fWidth is how wide, in world units, the texel being baked is: a texture whose detail is finer than that gives
//its average over it rather than one point's sample, so a coarse mip level is the finer levels' blur, not their noise.
//
//Its placement is where it sits in the world: its origin, its axes (units at right angles, right handed) and how many world
//units one of its own is.  A point's sample is the texture's own (TextureSample) at ((point - origin) . axis) / scale for each
//axis, its normal turned the same way and the width divided by the scale.  It starts at the world's origin, along the world's
//axes, at scale 1.
class CSolidTexture : public CObject
{
CONSTRUCTABLE(CSolidTexture);
DESTRUCTABLE(CSolidTexture);
public:
	SFloat32Vec3	msOrigin;
	SFloat32Vec3	masAxes[3];
	float32			mfScale;

	void			Init(void);
	void			Class(void) override;
	void			Free(void) override;

	void			Place(SFloat32Vec3* psOrigin, SFloat32Vec3* psXAxis, SFloat32Vec3* psYAxis, float32 fScale);  //Z is X cross Y; Y is made at right angles to X, both units.
	void			Sample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample);  //At a point in the world.
	ARGB32			Colour(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth);  //Its sample's colour there.

protected:
	virtual void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) = 0;  //At a point in its own space; psSample is Init'd, so it sets only what it has.
};


#endif // __SOLID_TEXTURE_H__

