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
#include "StandardLib/Objects.h"
#include "SolidAdjust.h"
#include "SolidCells.h"
#include "SolidFlat.h"
#include "SolidGradient.h"
#include "SolidMask.h"
#include "SolidMerge.h"
#include "SolidNoise.h"
#include "SolidRamp.h"
#include "SolidWarp.h"
#include "SolidMaterials.h"


//////////////////////////////////////////////////////////////////////////
//
// A sample of a colour, a height, and a highlight's strength and
// tightness.
//
//////////////////////////////////////////////////////////////////////////
static void SolidMaterialSample(SSolidSample* psSample, uint8 uiRed, uint8 uiGreen, uint8 uiBlue, float32 fHeight, float32 fShine, float32 fGloss)
{
	psSample->Init(Set32BitColour(uiRed, uiGreen, uiBlue));
	psSample->fHeight = fHeight;
	psSample->fShine = fShine;
	psSample->fGloss = fGloss;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void SolidMaterialKey(Ptr<CSolidRamp> pRamp, float32 fValue, uint8 uiRed, uint8 uiGreen, uint8 uiBlue, float32 fHeight, float32 fShine, float32 fGloss)
{
	SSolidSample	sSample;

	SolidMaterialSample(&sSample, uiRed, uiGreen, uiBlue, fHeight, fShine, fGloss);
	pRamp->AddKey(fValue, &sSample);
}


//////////////////////////////////////////////////////////////////////////
//
// A noise whose first octave's lumps are fScale across.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CSolidNoise> SolidMaterialNoise(EFractalNoise eKind, size uiOctaves, uint32 uiSeed, float32 fScale)
{
	Ptr<CSolidNoise>	pNoise;

	pNoise = OMalloc<CSolidNoise>(eKind, uiOctaves, uiSeed);
	pNoise->mfScale = fScale;
	return pNoise;
}


//////////////////////////////////////////////////////////////////////////
//
// Perlin's marble: bands (a wave along x, three to the unit) read where a
// noise has pushed the point about, so they fold into veins, then through
// a ramp that is pale for most of a band and dark only at its crest.  It's
// polished: flat, with a strong tight highlight.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> MakeSolidMarble(uint32 uiSeed)
{
	Ptr<CSolidGradient>	pBands;
	Ptr<CSolidRamp>		pRamp;

	pBands = OMalloc<CSolidGradient>(SG_Plane, SR_Wave);
	pBands->mfScale = 0.7f;
	pRamp = OMalloc<CSolidRamp>(OMalloc<CSolidWarp>(pBands, SolidMaterialNoise(FN_Sum, 5, uiSeed, 0.8f), 0.9f));
	SolidMaterialKey(pRamp, 0.00f, 244, 240, 232, 0.0f, 0.8f, 0.8f);
	SolidMaterialKey(pRamp, 0.55f, 232, 226, 216, 0.0f, 0.8f, 0.8f);
	SolidMaterialKey(pRamp, 0.80f, 176, 172, 176, 0.0f, 0.8f, 0.8f);
	SolidMaterialKey(pRamp, 0.93f, 96, 98, 112, 0.0f, 0.8f, 0.8f);
	SolidMaterialKey(pRamp, 1.00f, 52, 56, 72, 0.0f, 0.8f, 0.8f);
	return pRamp;
}


//////////////////////////////////////////////////////////////////////////
//
// Rings round the z axis, eight to the unit, each a saw: pale early wood
// darkening to a thin line of late wood, then pale again.  A broad noise
// wobbles them, as a tree's aren't circles, and a fine one multiplies in
// streaks of grain.  Late wood stands a little proud and shines less.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> MakeSolidWood(uint32 uiSeed)
{
	Ptr<CSolidGradient>	pRings;
	Ptr<CSolidRamp>		pRamp;
	Ptr<CSolidRamp>		pGrain;

	pRings = OMalloc<CSolidGradient>(SG_Axis, SR_Saw);
	pRings->mfScale = 0.125f;
	pRamp = OMalloc<CSolidRamp>(OMalloc<CSolidWarp>(pRings, SolidMaterialNoise(FN_Sum, 3, uiSeed, 1.2f), 0.22f));
	SolidMaterialKey(pRamp, 0.00f, 206, 158, 98, 0.00f, 0.35f, 0.4f);
	SolidMaterialKey(pRamp, 0.60f, 188, 136, 78, 0.00f, 0.35f, 0.4f);
	SolidMaterialKey(pRamp, 0.90f, 138, 90, 46, 0.01f, 0.20f, 0.3f);
	SolidMaterialKey(pRamp, 1.00f, 112, 70, 34, 0.01f, 0.20f, 0.3f);

	pGrain = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 3, uiSeed + 1, 0.04f));
	SolidMaterialKey(pGrain, 0.30f, 214, 204, 196, 1.0f, 1.0f, 1.0f);
	SolidMaterialKey(pGrain, 0.60f, 255, 255, 255, 1.0f, 1.0f, 1.0f);
	return OMalloc<CSolidMerge>(SM_Multiply, pRamp, pGrain);
}


//////////////////////////////////////////////////////////////////////////
//
// Two rocks, each a noise through its own ramp of greys and browns, one
// or the other by cell (cells a third of a unit across, their edges a
// little soft), so it's in patches; then cracks, dark and sunk, where a
// second set of cells' edges are.  It's rough: its height follows the
// noise and it has almost no highlight.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> MakeSolidStone(uint32 uiSeed)
{
	Ptr<CSolidRamp>		pPale;
	Ptr<CSolidRamp>		pDark;
	Ptr<CSolidRamp>		pPatches;
	Ptr<CSolidRamp>		pCracks;
	Ptr<CSolidCells>	pCells;

	pPale = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 6, uiSeed, 0.35f));
	SolidMaterialKey(pPale, 0.25f, 118, 112, 104, 0.00f, 0.05f, 0.1f);
	SolidMaterialKey(pPale, 0.75f, 196, 188, 172, 0.04f, 0.10f, 0.1f);

	pDark = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 6, uiSeed + 1, 0.2f));
	SolidMaterialKey(pDark, 0.25f, 72, 66, 62, 0.00f, 0.05f, 0.1f);
	SolidMaterialKey(pDark, 0.75f, 142, 126, 108, 0.03f, 0.10f, 0.1f);

	pCells = OMalloc<CSolidCells>(SC_Cell, uiSeed + 2);
	pCells->mfScale = 0.33f;
	pPatches = OMalloc<CSolidRamp>(pCells);
	pPatches->AddKey(0.55f, Set32BitColour((uint8)0, (uint8)0, (uint8)0));
	pPatches->AddKey(0.65f, Set32BitColour((uint8)255, (uint8)255, (uint8)255));

	pCells = OMalloc<CSolidCells>(SC_Edge, uiSeed + 3);
	pCells->mfScale = 0.6f;
	pCracks = OMalloc<CSolidRamp>(OMalloc<CSolidWarp>(pCells, SolidMaterialNoise(FN_Sum, 3, uiSeed + 4, 0.15f), 0.05f));
	SolidMaterialKey(pCracks, 0.000f, 70, 64, 60, 0.0f, 1.0f, 1.0f);
	SolidMaterialKey(pCracks, 0.045f, 255, 255, 255, 1.0f, 1.0f, 1.0f);

	return OMalloc<CSolidMerge>(SM_Multiply, OMalloc<CSolidMask>(pPale, pDark, pPatches), pCracks);
}


//////////////////////////////////////////////////////////////////////////
//
// Steel (even, bright, a strong tight highlight) and rust (a fine noise
// from dark brown to orange, raised, with no highlight), the rust where a
// ridged noise is high, so it spreads in branching patches with a narrow
// edge.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> MakeSolidRust(uint32 uiSeed)
{
	Ptr<CSolidRamp>	pSteel;
	Ptr<CSolidRamp>	pRust;
	Ptr<CSolidRamp>	pWhere;

	pSteel = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 2, uiSeed, 0.5f));
	SolidMaterialKey(pSteel, 0.3f, 138, 144, 152, 0.0f, 0.9f, 0.7f);
	SolidMaterialKey(pSteel, 0.7f, 170, 176, 184, 0.0f, 0.9f, 0.7f);

	pRust = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 5, uiSeed + 1, 0.12f));
	SolidMaterialKey(pRust, 0.25f, 74, 34, 20, 0.02f, 0.0f, 0.0f);
	SolidMaterialKey(pRust, 0.55f, 150, 72, 30, 0.04f, 0.0f, 0.0f);
	SolidMaterialKey(pRust, 0.80f, 204, 120, 52, 0.05f, 0.0f, 0.0f);

	pWhere = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Ridged, 5, uiSeed + 2, 0.9f));
	pWhere->AddKey(0.62f, Set32BitColour((uint8)0, (uint8)0, (uint8)0));
	pWhere->AddKey(0.70f, Set32BitColour((uint8)255, (uint8)255, (uint8)255));

	return OMalloc<CSolidMask>(pSteel, pRust, pWhere);
}

