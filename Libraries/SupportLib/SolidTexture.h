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


//How many numbers a sample is: its colour's four, then its height, shine, gloss and glow.
#define SOLID_SAMPLE_CHANNELS	8


//What a solid texture gives at a point (ProceduralLib/TexturePlan.txt).  Floats, not clamped, so a chain of operators neither
//bands nor loses what goes out of range on the way.  Operators that treat every channel alike take it as its numbers in a row
//(Channels).
//
//A texture that is only a number at each point (a noise, a gradient) gives it as a grey and as the height (InitValue).  The
//one number an operator reads from a sample (Value: a mask's amount, a ramp's place) is its colour's brightness, so anything
//seen as light and dark can be used as one.
struct SSolidSample
{
	SFloat32Vec4	sColour;  //Red, green, blue and alpha (x, y, z, w), 0 to 1.
	float32			fHeight;  //How far the surface is raised, in the texture's units; 0 where it has none.
	float32			fShine;   //How strong its highlight is, 0 to 1.
	float32			fGloss;   //How tight its highlight is, 0 (broad) to 1 (a point).
	float32			fGlow;    //How much of its colour it gives off as light, 0 to 1.

	void		Init(void);  //Opaque black, flat, dull and dark.
	void		Init(ARGB32 uiColour);
	void		InitValue(float32 fValue);  //An opaque grey of that brightness, and that height.

	ARGB32		Colour(void);  //Its colour clamped to 0 to 1 and rounded to 8 bits a channel.
	float32		Value(void);  //Its colour's brightness: the mean of its red, green and blue.
	float32*	Channels(void);  //Its SOLID_SAMPLE_CHANNELS numbers.
	void		Blend(SSolidSample* psOther, float32 fAmount);  //Towards psOther: none of it at 0, all of it at 1.
};


//A sample at every point in space (a solid texture, Peachey and Perlin 1985), which a bake reads at a surface's points
//(UnwrapPlan.txt, Bake).  Its scale is in world units, so how a surface looks doesn't depend on how it's unwrapped or how many
//texels it has.  fWidth is how wide, in world units, the texel being baked is: a texture whose detail is finer than that gives
//its average over it rather than one point's sample, so a coarse mip level is the finer levels' blur, not their noise.
//
//Its placement is where it sits in the world: its origin, its axes (units at right angles, right handed) and how many world
//units one of its own is.  A point's sample is the texture's own (TextureSample) at ((point - origin) . axis) / scale for each
//axis (ToOwn), its normal turned the same way and the width divided by the scale; the sample's height, which the texture gives
//in its own units, is multiplied by the scale, so a texture placed half the size is raised half as far and slopes as it did.
//It starts at the world's origin, along the world's axes, at scale 1.
//Design page: Libraries/ProceduralLib/Docs/libraries/procedurallib/textures.md
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
	virtual void	Changed(void);  //Its fields were set from outside, by an editor say: what Init and Place make of them is made again.  Overrides call it.
	void			ToOwn(SFloat32Vec3* psOwnPosition, SFloat32Vec3* psOwnNormal, SFloat32Vec3* psPosition, SFloat32Vec3* psNormal);  //A point and a normal in the world, in its own space.
	void			FromOwn(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, SFloat32Vec3* psOwnPosition, SFloat32Vec3* psOwnNormal);  //And back.
	void			Sample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample);  //At a point in the world.
	ARGB32			Colour(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth);  //Its sample's colour there.

protected:
	virtual void	TextureSample(SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, float32 fWidth, SSolidSample* psSample) = 0;  //At a point in its own space; psSample is Init'd, so it sets only what it has.
};


#endif // __SOLID_TEXTURE_H__

