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
#include <math.h>
#include "FloatHelper.h"
#include "Noise.h"


//Averages over all space, measured (TestNoise checks them): what a noise is where it's too fine to see.
#define NOISE_MEAN_ABSOLUTE		(0.2204f)
#define NOISE_MEAN_RIDGED		(0.6322f)
#define NOISE_MEAN_NEAREST		(0.5201f)
#define NOISE_MEAN_SECOND		(0.7167f)

//The width, in lattice cubes, over which nothing of a noise is left but its mean.
#define NOISE_WIDEST_KEPT		(2.5f)

//Further, squared, than any point a cellular search finds.
#define NOISE_FURTHEST			(1e30f)

//Where each octave's lattice is moved to, times its number, so the octaves' zeros don't all fall on the first's lattice points.
#define NOISE_OCTAVE_OFFSET_X	(19.19f)
#define NOISE_OCTAVE_OFFSET_Y	(7.71f)
#define NOISE_OCTAVE_OFFSET_Z	(13.37f)


//Perlin's twelve gradients, to the middles of a cube's edges, and four of them again so a hash's low four bits choose one.
static const int8 gaaiNoiseGradients[16][3] =
{
	{ 1,  1,  0}, {-1,  1,  0}, { 1, -1,  0}, {-1, -1,  0},
	{ 1,  0,  1}, {-1,  0,  1}, { 1,  0, -1}, {-1,  0, -1},
	{ 0,  1,  1}, { 0, -1,  1}, { 0,  1, -1}, { 0, -1, -1},
	{ 1,  1,  0}, {-1,  1,  0}, { 0, -1,  1}, { 0, -1, -1},
};


//////////////////////////////////////////////////////////////////////////
//
// Each coordinate times a large odd number, xored, then MurmurHash3's
// finaliser, which changes about half the bits for any bit changed.
//
//////////////////////////////////////////////////////////////////////////
uint32 NoiseHash(int32 x, int32 y, int32 z, uint32 uiSeed)
{
	uint32	uiHash;

	uiHash = uiSeed ^ ((uint32)x * 0x8DA6B343u) ^ ((uint32)y * 0xD8163841u) ^ ((uint32)z * 0xCB1AB31Fu);
	uiHash ^= uiHash >> 16;
	uiHash *= 0x85EBCA6Bu;
	uiHash ^= uiHash >> 13;
	uiHash *= 0xC2B2AE35u;
	uiHash ^= uiHash >> 16;
	return uiHash;
}


//////////////////////////////////////////////////////////////////////////
//
// Fitted to gradient noise averaged over squares: the best multiple of
// the noise at a square's middle for its average is 0.87 at half a unit
// wide, 0.58 at one, 0.15 at two and 0.03 at four.  This is within 0.07
// of those.
//
//////////////////////////////////////////////////////////////////////////
float32 NoiseKept(float32 fWidth)
{
	float32	f;

	f = fWidth / NOISE_WIDEST_KEPT;
	if (f <= 0.0f)
	{
		return 1.0f;
	}
	if (f >= 1.0f)
	{
		return 0.0f;
	}
	return 1.0f - (f * f * (3.0f - (2.0f * f)));
}


//////////////////////////////////////////////////////////////////////////
//
// Perlin's quintic, 6t^5 - 15t^4 + 10t^3: 0 to 1 with no slope or
// curvature at either end, so the noise's second derivative is continuous
// across lattice planes.
//
//////////////////////////////////////////////////////////////////////////
static float32 NoiseFade(float32 t)
{
	return t * t * t * ((t * ((t * 6.0f) - 15.0f)) + 10.0f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float32 NoiseFadeSlope(float32 t)
{
	return 30.0f * t * t * (t - 1.0f) * (t - 1.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// Eight values at a cube's corners (corner i is at x = i & 1,
// y = (i >> 1) & 1, z = i >> 2) blended to a point inside it.
//
//////////////////////////////////////////////////////////////////////////
static float32 NoiseBlend(float32* pafCorners, float32 u, float32 v, float32 w)
{
	float32	fNear;
	float32	fFar;

	fNear = InterpolateFloat(InterpolateFloat(pafCorners[0], pafCorners[1], u), InterpolateFloat(pafCorners[2], pafCorners[3], u), v);
	fFar = InterpolateFloat(InterpolateFloat(pafCorners[4], pafCorners[5], u), InterpolateFloat(pafCorners[6], pafCorners[7], u), v);
	return InterpolateFloat(fNear, fFar, w);
}


//////////////////////////////////////////////////////////////////////////
//
// Each corner of the point's lattice cube has a gradient; its value at
// the point is the gradient dot the point's offset from it, and the noise
// is the corners' values blended by the faded position in the cube.
//
// Its slope along x is the corners' gradients' x blended the same way,
// and how the blend itself changes: the fade's slope times the blend at
// the cube's far x face less that at its near one.  So with y and z.
//
//////////////////////////////////////////////////////////////////////////
float32 GradientNoise(SFloat32Vec3* psPosition, uint32 uiSeed, SFloat32Vec3* psGradient)
{
	SFloat32Vec3	sFloor;
	SFloat32Vec3	sIn;
	const int8*		paiGradient;
	float32			afValues[8];
	float32			aafGradients[3][8];
	float32			u;
	float32			v;
	float32			w;
	int32			x;
	int32			y;
	int32			z;
	int32			i;

	sFloor.Init(floorf(psPosition->x), floorf(psPosition->y), floorf(psPosition->z));
	sIn = *psPosition - sFloor;
	x = (int32)sFloor.x;
	y = (int32)sFloor.y;
	z = (int32)sFloor.z;
	for (i = 0; i < 8; i++)
	{
		paiGradient = gaaiNoiseGradients[NoiseHash(x + (i & 1), y + ((i >> 1) & 1), z + (i >> 2), uiSeed) & 15];
		aafGradients[0][i] = (float32)paiGradient[0];
		aafGradients[1][i] = (float32)paiGradient[1];
		aafGradients[2][i] = (float32)paiGradient[2];
		afValues[i] = (aafGradients[0][i] * (sIn.x - (float32)(i & 1))) + (aafGradients[1][i] * (sIn.y - (float32)((i >> 1) & 1))) + (aafGradients[2][i] * (sIn.z - (float32)(i >> 2)));
	}

	u = NoiseFade(sIn.x);
	v = NoiseFade(sIn.y);
	w = NoiseFade(sIn.z);
	if (psGradient)
	{
		psGradient->x = NoiseBlend(aafGradients[0], u, v, w) + (NoiseFadeSlope(sIn.x) * (NoiseBlend(afValues, 1.0f, v, w) - NoiseBlend(afValues, 0.0f, v, w)));
		psGradient->y = NoiseBlend(aafGradients[1], u, v, w) + (NoiseFadeSlope(sIn.y) * (NoiseBlend(afValues, u, 1.0f, w) - NoiseBlend(afValues, u, 0.0f, w)));
		psGradient->z = NoiseBlend(aafGradients[2], u, v, w) + (NoiseFadeSlope(sIn.z) * (NoiseBlend(afValues, u, v, 1.0f) - NoiseBlend(afValues, u, v, 0.0f)));
	}
	return NoiseBlend(afValues, u, v, w);
}


//////////////////////////////////////////////////////////////////////////
//
// What an octave's noise n becomes, and (in pfSlope) how fast that changes
// with n, for the sum's gradient.
//
//////////////////////////////////////////////////////////////////////////
static float32 FractalNoiseShape(EFractalNoise eKind, float32 fNoise, float32* pfSlope)
{
	float32	fSign;
	float32	fRidge;

	if (eKind == FN_Sum)
	{
		*pfSlope = 1.0f;
		return fNoise;
	}

	fSign = (fNoise < 0.0f) ? -1.0f : 1.0f;
	if (eKind == FN_Turbulence)
	{
		*pfSlope = fSign;
		return fNoise * fSign;
	}

	fRidge = 1.0f - (fNoise * fSign);
	*pfSlope = -2.0f * fRidge * fSign;
	return fRidge * fRidge;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static float32 FractalNoiseMean(EFractalNoise eKind)
{
	if (eKind == FN_Turbulence)
	{
		return NOISE_MEAN_ABSOLUTE;
	}
	if (eKind == FN_Ridged)
	{
		return NOISE_MEAN_RIDGED;
	}
	return 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
// Octave i is the noise at the point times 2^i (its lattice moved, so the
// octaves' lattices don't coincide), with its own seed, 1 / 2^i as strong.
// Averaged over the width it is its mean and what is kept of its
// difference from it: an octave of which nothing is kept isn't worked out,
// and nor are those after it, which are finer still.
//
//////////////////////////////////////////////////////////////////////////
float32 FractalNoise(EFractalNoise eKind, SFloat32Vec3* psPosition, uint32 uiSeed, size uiOctaves, float32 fWidth, SFloat32Vec3* psGradient)
{
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sOctaveGradient;
	SFloat32Vec3	sGradient;
	float32			fMean;
	float32			fFrequency;
	float32			fStrength;
	float32			fStrengths;
	float32			fKept;
	float32			fSlope;
	float32			fSum;
	size			i;

	fMean = FractalNoiseMean(eKind);
	fFrequency = 1.0f;
	fStrength = 1.0f;
	fStrengths = 0.0f;
	fSum = 0.0f;
	sGradient.Init(0, 0, 0);
	for (i = 0; i < uiOctaves; i++)
	{
		fKept = NoiseKept(fWidth * fFrequency);
		if (fKept > 0.0f)
		{
			sPosition.Init((psPosition->x * fFrequency) + (NOISE_OCTAVE_OFFSET_X * (float32)i), (psPosition->y * fFrequency) + (NOISE_OCTAVE_OFFSET_Y * (float32)i), (psPosition->z * fFrequency) + (NOISE_OCTAVE_OFFSET_Z * (float32)i));
			fSum += fStrength * (fMean + (fKept * (FractalNoiseShape(eKind, GradientNoise(&sPosition, uiSeed + (uint32)i, &sOctaveGradient), &fSlope) - fMean)));
			sGradient += sOctaveGradient * (fStrength * fKept * fSlope * fFrequency);
		}
		else
		{
			fSum += fStrength * fMean;
		}
		fStrengths += fStrength;
		fFrequency *= 2.0f;
		fStrength *= 0.5f;
	}

	if (fStrengths == 0.0f)
	{
		fStrengths = 1.0f;
	}
	if (psGradient)
	{
		*psGradient = sGradient / fStrengths;
	}
	return fSum / fStrengths;
}


//////////////////////////////////////////////////////////////////////////
//
// The nearest two points to the position among those of the lattice cubes
// within iReach cubes of its own.  A cube's point is its hash's three ten
// bit numbers, as fractions of the cube.
//
//////////////////////////////////////////////////////////////////////////
static void CellularNoiseSearch(SCellularNoise* psNoise, int32* paiNearest, SFloat32Vec3* psPosition, int32* paiCube, int32 iReach, uint32 uiSeed)
{
	SFloat32Vec3	sOffset;
	float32			fSquared;
	uint32			uiHash;
	int32			x;
	int32			y;
	int32			z;

	psNoise->fNearest = NOISE_FURTHEST;
	psNoise->fSecond = NOISE_FURTHEST;
	paiNearest[0] = paiCube[0];
	paiNearest[1] = paiCube[1];
	paiNearest[2] = paiCube[2];
	for (z = paiCube[2] - iReach; z <= paiCube[2] + iReach; z++)
	{
		for (y = paiCube[1] - iReach; y <= paiCube[1] + iReach; y++)
		{
			for (x = paiCube[0] - iReach; x <= paiCube[0] + iReach; x++)
			{
				uiHash = NoiseHash(x, y, z, uiSeed);
				sOffset.Init((float32)x + ((float32)(uiHash & 0x3FF) / 1024.0f) - psPosition->x, (float32)y + ((float32)((uiHash >> 10) & 0x3FF) / 1024.0f) - psPosition->y, (float32)z + ((float32)((uiHash >> 20) & 0x3FF) / 1024.0f) - psPosition->z);
				fSquared = Float32Vec3Dot(&sOffset, &sOffset);
				if (fSquared < psNoise->fNearest)
				{
					psNoise->fSecond = psNoise->fNearest;
					psNoise->fNearest = fSquared;
					paiNearest[0] = x;
					paiNearest[1] = y;
					paiNearest[2] = z;
				}
				else if (fSquared < psNoise->fSecond)
				{
					psNoise->fSecond = fSquared;
				}
			}
		}
	}
	psNoise->fNearest = sqrtf(psNoise->fNearest);
	psNoise->fSecond = sqrtf(psNoise->fSecond);
}


//////////////////////////////////////////////////////////////////////////
//
// The cubes round the position's own are searched, and that is the answer
// if the second nearest point is no further than the nearest face of the
// block searched, as no point outside it can then be nearer.  Otherwise
// (a few times in a hundred) the block a cube bigger each way is searched.
//
//////////////////////////////////////////////////////////////////////////
void CellularNoise(SCellularNoise* psNoise, SFloat32Vec3* psPosition, uint32 uiSeed, float32 fWidth)
{
	SFloat32Vec3	sFloor;
	SFloat32Vec3	sIn;
	float32			fEdge;
	float32			fKept;
	int32			aiCube[3];
	int32			aiNearest[3];
	int32			iReach;

	sFloor.Init(floorf(psPosition->x), floorf(psPosition->y), floorf(psPosition->z));
	sIn = *psPosition - sFloor;
	fEdge = minf(minf(minf(sIn.x, 1.0f - sIn.x), minf(sIn.y, 1.0f - sIn.y)), minf(sIn.z, 1.0f - sIn.z));
	aiCube[0] = (int32)sFloor.x;
	aiCube[1] = (int32)sFloor.y;
	aiCube[2] = (int32)sFloor.z;
	iReach = 0;
	do
	{
		iReach++;
		CellularNoiseSearch(psNoise, aiNearest, psPosition, aiCube, iReach, uiSeed);
	}
	while (psNoise->fSecond > ((float32)iReach + fEdge));

	psNoise->fCell = (float32)(NoiseHash(aiNearest[0], aiNearest[1], aiNearest[2], ~uiSeed) >> 8) / 16777216.0f;

	fKept = NoiseKept(fWidth);
	psNoise->fNearest = InterpolateFloat(NOISE_MEAN_NEAREST, psNoise->fNearest, fKept);
	psNoise->fSecond = InterpolateFloat(NOISE_MEAN_SECOND, psNoise->fSecond, fKept);
	psNoise->fCell = InterpolateFloat(0.5f, psNoise->fCell, fKept);
}

