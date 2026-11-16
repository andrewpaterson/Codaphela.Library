/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __NOISE_H__
#define __NOISE_H__
#include "Float32Vec3.h"


//Noise for procedural textures and shapes (ProceduralLib/TexturePlan.txt, Noise): functions of a point in space and a seed,
//the same on every machine, with nothing to set up.  A unit of space is one lattice cube: features are about a unit across.
//
//Each takes the width of what is looking (a texel, in the noise's units) where it matters, and gives its average over that
//width rather than its value at the point, so something sampled coarsely is the blur of the same thing sampled finely, not
//its flicker.


//A lattice point and a seed to 32 well mixed bits.
uint32	NoiseHash(int32 x, int32 y, int32 z, uint32 uiSeed);

//How much of a noise's difference from its mean is left when it's averaged over fWidth: all of it at a point, none from two
//and a half units, smoothly between.
float32	NoiseKept(float32 fWidth);

//Perlin's improved noise (2002): -1 to 1, 0 at every lattice point, smooth.  psGradient, if given, is its slope along each axis.
float32	GradientNoise(SFloat32Vec3* psPosition, uint32 uiSeed, SFloat32Vec3* psGradient = NULL);


enum EFractalNoise
{
	FN_Sum,			//Octaves added (fractional Brownian motion): clouds, stone.  -1 to 1.
	FN_Turbulence,	//Their absolute values added: creases along each octave's zeros, as marble's veins and fire.  0 to 1.
	FN_Ridged,		//One less their absolute values, squared: sharp ridges along the zeros.  0 to 1.
};

//uiOctaves of gradient noise, each twice as fine and half as strong as the one before, over the sum of their strengths.  An
//octave too fine for fWidth is its average (NoiseKept), so finer octaves cost nothing.  psGradient, if given, is its slope.
float32	FractalNoise(EFractalNoise eKind, SFloat32Vec3* psPosition, uint32 uiSeed, size uiOctaves, float32 fWidth, SFloat32Vec3* psGradient = NULL);


//Worley's cellular noise (1996): one point somewhere in each lattice cube, and how far the nearest of them are.
struct SCellularNoise
{
	float32	fNearest;	//To the nearest point (F1): 0 at it, rising to the cell's edge.  Pebbles, scales.
	float32	fSecond;	//To the second nearest (F2).  fSecond - fNearest is 0 along the cells' edges: cracks, mortar.
	float32	fCell;		//The nearest point's own number, 0 to 1: the same all over its cell.
};

//Each of the three is its average over all space as fWidth nears a unit (NoiseKept).
void	CellularNoise(SCellularNoise* psNoise, SFloat32Vec3* psPosition, uint32 uiSeed, float32 fWidth);


#endif // __NOISE_H__

