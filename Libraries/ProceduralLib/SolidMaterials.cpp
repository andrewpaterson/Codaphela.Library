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
#include "SolidBricks.h"
#include "SolidCavity.h"
#include "SolidCells.h"
#include "SolidFlat.h"
#include "SolidGradient.h"
#include "SolidMask.h"
#include "SolidMerge.h"
#include "SolidNoise.h"
#include "SolidRamp.h"
#include "SolidSlope.h"
#include "SolidWarp.h"
#include "SolidMaterials.h"


//////////////////////////////////////////////////////////////////////////
//
// A key of a colour, a height, and a highlight's strength and tightness.
//
//////////////////////////////////////////////////////////////////////////
static void SolidMaterialKey(Ptr<CSolidRamp> pRamp, float32 fValue, uint8 uiRed, uint8 uiGreen, uint8 uiBlue, float32 fHeight, float32 fShine, float32 fGloss)
{
	SSolidSample	sSample;

	sSample.Init(Set32BitColour(uiRed, uiGreen, uiBlue));
	sSample.fHeight = fHeight;
	sSample.fShine = fShine;
	sSample.fGloss = fGloss;
	pRamp->AddKey(fValue, &sSample);
}


//////////////////////////////////////////////////////////////////////////
//
// A ramp of pInput that is black up to fBlack and white from fWhite: a
// mask's amount, none of its second input before the one and all of it
// after the other (either way round).
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CSolidRamp> SolidMaterialWhere(Ptr<CSolidTexture> pInput, float32 fBlack, float32 fWhite)
{
	Ptr<CSolidRamp>	pRamp;

	pRamp = OMalloc<CSolidRamp>(pInput);
	pRamp->AddKey(fBlack, Set32BitColour((uint8)0, (uint8)0, (uint8)0));
	pRamp->AddKey(fWhite, Set32BitColour((uint8)255, (uint8)255, (uint8)255));
	return pRamp;
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


//////////////////////////////////////////////////////////////////////////
//
// Blocks with mortar between: pBlocks (each block's colour, from its own
// number) times a fine noise's grit where pInside says the point is more
// than fBevel inside a block, pMortar where it's in the mortar, and a
// blend of them across the bevel, so a block's edges slope down to it.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CSolidTexture> SolidMaterialBlocks(Ptr<CSolidTexture> pBlocks, Ptr<CSolidTexture> pMortar, Ptr<CSolidTexture> pInside, float32 fBevel, uint32 uiSeed)
{
	Ptr<CSolidRamp>	pGrit;

	pGrit = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 4, uiSeed, 0.03f));
	SolidMaterialKey(pGrit, 0.3f, 206, 200, 196, 1.0f, 1.0f, 1.0f);
	SolidMaterialKey(pGrit, 0.7f, 255, 255, 255, 1.0f, 1.0f, 1.0f);
	return OMalloc<CSolidMask>(pMortar, OMalloc<CSolidMerge>(SM_Multiply, pBlocks, pGrit), SolidMaterialWhere(pInside, 0.0f, fBevel));
}


//////////////////////////////////////////////////////////////////////////
//
// A wall in running bond.  A brick's red is its own, from dark to orange;
// the mortar is a pale gritty grey, lower than the bricks' faces.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> MakeSolidBricks(uint32 uiSeed)
{
	Ptr<CSolidRamp>	pBricks;
	Ptr<CSolidRamp>	pMortar;
	SFloat32Vec3	sSize;

	sSize.Init(0.3f, 0.15f, 0.1f);
	pBricks = OMalloc<CSolidRamp>(OMalloc<CSolidBricks>(SB_Brick, &sSize, 0.016f, 0.5f, uiSeed));
	SolidMaterialKey(pBricks, 0.0f, 124, 50, 38, 0.008f, 0.10f, 0.2f);
	SolidMaterialKey(pBricks, 0.5f, 162, 68, 46, 0.008f, 0.10f, 0.2f);
	SolidMaterialKey(pBricks, 1.0f, 190, 98, 62, 0.008f, 0.10f, 0.2f);

	pMortar = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 3, uiSeed + 1, 0.02f));
	SolidMaterialKey(pMortar, 0.3f, 150, 146, 138, 0.000f, 0.0f, 0.0f);
	SolidMaterialKey(pMortar, 0.7f, 192, 188, 178, 0.002f, 0.0f, 0.0f);

	return SolidMaterialBlocks(pBricks, pMortar, OMalloc<CSolidBricks>(SB_Inside, &sSize, 0.016f, 0.5f, uiSeed), 0.008f, uiSeed + 2);
}


//////////////////////////////////////////////////////////////////////////
//
// Tiles in a grid: cubes a quarter of a unit each way, so every face of a
// thing square to them is tiled.  Half are cream and half terracotta, by
// their own numbers (two keys at a half are the edge between), each a
// little lighter or darker; they're glazed, and the grout is dark.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> MakeSolidTiles(uint32 uiSeed)
{
	Ptr<CSolidRamp>	pTiles;
	Ptr<CSolidRamp>	pGrout;
	SFloat32Vec3	sSize;

	sSize.Init(0.25f, 0.25f, 0.25f);
	pTiles = OMalloc<CSolidRamp>(OMalloc<CSolidBricks>(SB_Brick, &sSize, 0.012f, 0.0f, uiSeed));
	SolidMaterialKey(pTiles, 0.0f, 214, 200, 172, 0.004f, 0.8f, 0.7f);
	SolidMaterialKey(pTiles, 0.5f, 238, 228, 204, 0.004f, 0.8f, 0.7f);
	SolidMaterialKey(pTiles, 0.5f, 156, 82, 54, 0.004f, 0.8f, 0.7f);
	SolidMaterialKey(pTiles, 1.0f, 192, 108, 70, 0.004f, 0.8f, 0.7f);

	pGrout = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 3, uiSeed + 1, 0.02f));
	SolidMaterialKey(pGrout, 0.3f, 58, 56, 54, 0.0f, 0.0f, 0.0f);
	SolidMaterialKey(pGrout, 0.7f, 88, 84, 80, 0.0f, 0.0f, 0.0f);

	return SolidMaterialBlocks(pTiles, pGrout, OMalloc<CSolidBricks>(SB_Inside, &sSize, 0.012f, 0.0f, uiSeed), 0.004f, uiSeed + 2);
}


//////////////////////////////////////////////////////////////////////////
//
// How hollowed or edged the shape is (a CSolidCavity reaching 0.15) with
// a noise added, so that what follows it has a ragged edge; then, over
// the material: itself paler and greyer where that's well past a half
// (worn edges), a dark earth where it's well under (dirt in the hollows),
// and a green moss where the surface faces up and a broad noise is high
// (in patches).
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSolidTexture> MakeSolidWeathered(Ptr<CSolidTexture> pMaterial, Ptr<CShape> pShape, uint32 uiSeed)
{
	Ptr<CSolidTexture>	pCavity;
	Ptr<CSolidTexture>	pWeathered;
	Ptr<CSolidAdjust>	pWorn;
	Ptr<CSolidRamp>		pRagged;
	Ptr<CSolidRamp>		pDirt;
	Ptr<CSolidRamp>		pMoss;
	SSolidSample		sSample;

	pRagged = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 4, uiSeed, 0.12f));
	sSample.InitValue(-0.12f);
	pRagged->AddKey(0.3f, &sSample);
	sSample.InitValue(0.12f);
	pRagged->AddKey(0.7f, &sSample);
	pCavity = OMalloc<CSolidMerge>(SM_Add, OMalloc<CSolidCavity>(pShape, 0.15f), pRagged);

	pWorn = OMalloc<CSolidAdjust>(pMaterial);
	pWorn->mfSaturation = 0.5f;
	pWorn->mfBrightness = 0.22f;
	pWeathered = OMalloc<CSolidMask>(pMaterial, pWorn, SolidMaterialWhere(pCavity, 0.62f, 0.8f));

	pDirt = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 4, uiSeed + 1, 0.08f));
	SolidMaterialKey(pDirt, 0.3f, 40, 33, 26, 0.004f, 0.0f, 0.0f);
	SolidMaterialKey(pDirt, 0.7f, 76, 62, 46, 0.008f, 0.0f, 0.0f);
	pWeathered = OMalloc<CSolidMask>(pWeathered, pDirt, SolidMaterialWhere(pCavity, 0.42f, 0.22f));

	pMoss = OMalloc<CSolidRamp>(SolidMaterialNoise(FN_Sum, 5, uiSeed + 2, 0.06f));
	SolidMaterialKey(pMoss, 0.3f, 44, 72, 30, 0.006f, 0.0f, 0.0f);
	SolidMaterialKey(pMoss, 0.7f, 104, 136, 58, 0.016f, 0.0f, 0.0f);
	return OMalloc<CSolidMask>(pWeathered, pMoss, OMalloc<CSolidMerge>(SM_Multiply, SolidMaterialWhere(OMalloc<CSolidSlope>(), 0.72f, 0.9f), SolidMaterialWhere(SolidMaterialNoise(FN_Sum, 4, uiSeed + 3, 0.5f), 0.46f, 0.56f)));
}
