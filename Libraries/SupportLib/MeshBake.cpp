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
#include <math.h>
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayUint32.h"
#include "StandardLib/Objects.h"
#include "ColourARGB32.h"
#include "Material.h"
#include "MeshBake.h"


//What a bake makes of a texel: its colour, and its normal as three bytes (x, y and z, each from -1 at 0 to 1 at 255) where
//a colour's red, green and blue are.
struct SMeshBakeTexel
{
	ARGB32	uiColour;
	ARGB32	uiNormal;
};


typedef CArrayLargeTemplate<SMeshBakeTexel>	CArrayMeshBakeTexel;


//From a texel to the four beside it.
static const int gaaiMeshBakeSteps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};


//////////////////////////////////////////////////////////////////////////
//
// A face's corners' positions and normals, and its UVs in texels.  False
// if it has no UVs, or no area in 3D or in texels.
//
// A point on the face is its corners weighted, and each corner's weight
// changes evenly across the texels (MeshBakeDraw's afWeights): by
// (y[j] - y[l]) / twice the area for a texel across and (x[l] - x[j]) for
// one down, j and l the corners after it.  So a texel across or down is
// the same step in the world everywhere on the face.
//
// A height's slope g on the face is in its plane, with g . across the
// height's rise a texel across and g . down its rise a texel down: two
// equations in how much of across and of down g is.  sAcrossRise and
// sDownRise are their answers for a rise of one across and of one down,
// so any slope is the two, each times its rise.
//
//////////////////////////////////////////////////////////////////////////
struct SMeshBakeFace
{
	SFloat32Vec3	asPositions[NUM_FACE_CORNERS];
	SFloat32Vec3	asNormals[NUM_FACE_CORNERS];
	float32			afX[NUM_FACE_CORNERS];
	float32			afY[NUM_FACE_CORNERS];
	float32			fTwiceArea;  //In texels, signed.
	float32			fWidth;  //Of a texel, in world units.
	SFloat32Vec3	sAcross;  //How far, in the world, a point on the face moves for a texel across,
	SFloat32Vec3	sDown;  //and for a texel down.
	SFloat32Vec3	sAcrossRise;  //The slope, in the face's plane, of a height that rises one unit a texel across and none down,
	SFloat32Vec3	sDownRise;  //and of one that rises one unit a texel down and none across.
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool MeshBakeFace(SMeshBakeFace* psDest, CMesh* pcMesh, CMeshUVLayer* pcLayer, int iFace, int iWidth, int iHeight)
{
	SMeshUVFace*		psUVFace;
	SMeshNormalFace*	psNormalFace;
	SFloat32Vec2*		psUV;
	SFloat32Vec3		sSide1;
	SFloat32Vec3		sSide2;
	SFloat32Vec3		sCross;
	int*				piCorners;
	float32				fArea3D;
	float32				fAcross;
	float32				fDown;
	float32				fBoth;
	float32				fDeterminant;
	int					iNormal;
	int					k;
	int					j;
	int					l;

	psUVFace = pcLayer->mcFaces.Get((lsize)iFace);
	piCorners = pcMesh->GetFaceCorners(iFace);
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		if ((psUVFace->aiUVs[k] < 0) || ((lsize)psUVFace->aiUVs[k] >= pcLayer->mcUVs.NumElements()))
		{
			return false;
		}
		psUV = pcLayer->mcUVs.Get((lsize)psUVFace->aiUVs[k]);
		psDest->afX[k] = psUV->x * (float32)iWidth;
		psDest->afY[k] = psUV->y * (float32)iHeight;
		psDest->asPositions[k] = *pcMesh->GetPosition(piCorners[k]);
	}

	sSide1 = psDest->asPositions[1] - psDest->asPositions[0];
	sSide2 = psDest->asPositions[2] - psDest->asPositions[0];
	Float32Vec3Cross(&sCross, &sSide1, &sSide2);
	fArea3D = (float32)sCross.Magnitude();
	psDest->fTwiceArea = ((psDest->afX[1] - psDest->afX[0]) * (psDest->afY[2] - psDest->afY[0])) - ((psDest->afX[2] - psDest->afX[0]) * (psDest->afY[1] - psDest->afY[0]));
	if ((fArea3D <= 0.0f) || (psDest->fTwiceArea == 0.0f))
	{
		return false;
	}
	psDest->fWidth = sqrtf(fArea3D / fabsf(psDest->fTwiceArea));
	psDest->sAcross.Init(0, 0, 0);
	psDest->sDown.Init(0, 0, 0);
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		j = (k + 1) % NUM_FACE_CORNERS;
		l = (k + 2) % NUM_FACE_CORNERS;
		psDest->sAcross += psDest->asPositions[k] * ((psDest->afY[j] - psDest->afY[l]) / psDest->fTwiceArea);
		psDest->sDown += psDest->asPositions[k] * ((psDest->afX[l] - psDest->afX[j]) / psDest->fTwiceArea);
	}
	fAcross = Float32Vec3Dot(&psDest->sAcross, &psDest->sAcross);
	fDown = Float32Vec3Dot(&psDest->sDown, &psDest->sDown);
	fBoth = Float32Vec3Dot(&psDest->sAcross, &psDest->sDown);
	fDeterminant = (fAcross * fDown) - (fBoth * fBoth);
	psDest->sAcrossRise = ((psDest->sAcross * fDown) - (psDest->sDown * fBoth)) / fDeterminant;
	psDest->sDownRise = ((psDest->sDown * fAcross) - (psDest->sAcross * fBoth)) / fDeterminant;

	sCross /= fArea3D;
	for (k = 0; k < NUM_FACE_CORNERS; k++)
	{
		psDest->asNormals[k] = sCross;
		if (pcMesh->mcNormals.InUse())
		{
			psNormalFace = pcMesh->mcNormals.mcFaces.Get(iFace);
			iNormal = psNormalFace->aiCornerNormals[k];
			psDest->asNormals[k] = *pcMesh->mcNormals.mcNormals.Get((lsize)((iNormal >= 0) ? iNormal : iFace));
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SMeshBakeSettings::Init(void)
{
	iSamples = 1;
	fRelief = 0.0f;
	sLight.Init(0, 0, 1);
}


//////////////////////////////////////////////////////////////////////////
//
// A number from -1 to 1 as a byte, 0 to 255.
//
//////////////////////////////////////////////////////////////////////////
static uint8 MeshBakeNormalByte(float32 fValue)
{
	return (uint8)((((fValue < -1.0f) ? -1.0f : ((fValue > 1.0f) ? 1.0f : fValue)) * 127.5f) + 128.0f);
}


//////////////////////////////////////////////////////////////////////////
//
// A texel: its colour and its normal.
//
// Its sample is the mean of the texture's at iSamples by iSamples points
// spread evenly across it, each as wide as its share of the texel.  A
// texture that can't average itself over a texel (an edge a ramp or a mask
// makes) is averaged here.  A mean is kept as each sample moves it its
// share of the way: the first all the way, the second half...
//
// With bTilt the samples' heights give the height's rise a texel across
// and down (the best fit through them: the points are spread evenly about
// the centre, so each rise is the heights times how far across, or down,
// over those distances squared), the rises the height's slope on the face,
// and the normal less the slope, made a unit, is the normal of the surface
// raised by the height.  That takes two samples each way at least.
//
// Relief shades the colour by how much more of the light the tilted
// normal faces than the smooth one, a glowing texel that much less.
//
//////////////////////////////////////////////////////////////////////////
static void MeshBakeTexel(SMeshBakeTexel* psTexel, SMeshBakeFace* psFace, CSolidTexture* pcTexture, SFloat32Vec3* psPosition, SFloat32Vec3* psNormal, SMeshBakeSettings* psSettings, bool bTilt, ARGB32 uiTint)
{
	SSolidSample	sMean;
	SSolidSample	sSample;
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sTilted;
	float32			fAcross;
	float32			fDown;
	float32			fRiseAcross;
	float32			fRiseDown;
	float32			fSquares;
	float32			fShade;
	float32			fGlow;
	int				iSamples;
	int				x;
	int				y;

	iSamples = (bTilt && (psSettings->iSamples < 2)) ? 2 : psSettings->iSamples;
	sMean.Init();
	fRiseAcross = 0.0f;
	fRiseDown = 0.0f;
	fSquares = 0.0f;
	for (y = 0; y < iSamples; y++)
	{
		fDown = (((float32)y + 0.5f) / (float32)iSamples) - 0.5f;
		for (x = 0; x < iSamples; x++)
		{
			fAcross = (((float32)x + 0.5f) / (float32)iSamples) - 0.5f;
			sPosition = *psPosition + (psFace->sAcross * fAcross) + (psFace->sDown * fDown);
			pcTexture->Sample(&sPosition, psNormal, psFace->fWidth / (float32)iSamples, &sSample);
			sMean.Blend(&sSample, 1.0f / (float32)((y * iSamples) + x + 1));
			fRiseAcross += sSample.fHeight * fAcross;
			fRiseDown += sSample.fHeight * fDown;
			fSquares += fAcross * fAcross;
		}
	}

	sTilted = *psNormal;
	if (bTilt)
	{
		sTilted -= (psFace->sAcrossRise * (fRiseAcross / fSquares)) + (psFace->sDownRise * (fRiseDown / fSquares));
		sTilted.Normalize();
		fGlow = (sMean.fGlow < 0.0f) ? 0.0f : ((sMean.fGlow > 1.0f) ? 1.0f : sMean.fGlow);
		fShade = psSettings->fRelief * (Float32Vec3Dot(&sTilted, &psSettings->sLight) - Float32Vec3Dot(psNormal, &psSettings->sLight)) * (1.0f - fGlow);
		fShade = (fShade < -1.0f) ? 0.0f : (1.0f + fShade);
		sMean.sColour.x *= fShade;
		sMean.sColour.y *= fShade;
		sMean.sColour.z *= fShade;
	}
	psTexel->uiColour = Modulate32BitColours(sMean.Colour(), uiTint);
	psTexel->uiNormal = Set32BitColour(MeshBakeNormalByte(sTilted.x), MeshBakeNormalByte(sTilted.y), MeshBakeNormalByte(sTilted.z));
}


//////////////////////////////////////////////////////////////////////////
//
// The texels whose centres a face covers (its barycentric weights all at
// least nothing, either way round), each made from the texture there.
//
//////////////////////////////////////////////////////////////////////////
static void MeshBakeDraw(SMeshBakeFace* psFace, CSolidTexture* pcTexture, SMeshBakeSettings* psSettings, bool bTilt, ARGB32 uiTint, int iWidth, int iHeight, CArrayMeshBakeTexel* pasTexels, CArrayLargeInt* paiCovered)
{
	SFloat32Vec3	sPosition;
	SFloat32Vec3	sNormal;
	float32			fMinX;
	float32			fMaxX;
	float32			fMinY;
	float32			fMaxY;
	float32			fCentreX;
	float32			fCentreY;
	float32			afWeights[NUM_FACE_CORNERS];
	float32			fLength;
	int				iX0;
	int				iX1;
	int				iY0;
	int				iY1;
	int				x;
	int				y;
	int				k;
	int				j;
	int				l;
	bool			bInside;
	lsize			uiTexel;

	fMinX = psFace->afX[0];
	fMaxX = psFace->afX[0];
	fMinY = psFace->afY[0];
	fMaxY = psFace->afY[0];
	for (k = 1; k < NUM_FACE_CORNERS; k++)
	{
		fMinX = (psFace->afX[k] < fMinX) ? psFace->afX[k] : fMinX;
		fMaxX = (psFace->afX[k] > fMaxX) ? psFace->afX[k] : fMaxX;
		fMinY = (psFace->afY[k] < fMinY) ? psFace->afY[k] : fMinY;
		fMaxY = (psFace->afY[k] > fMaxY) ? psFace->afY[k] : fMaxY;
	}
	iX0 = (int)floorf(fMinX - 0.5f);
	iX1 = (int)ceilf(fMaxX - 0.5f);
	iY0 = (int)floorf(fMinY - 0.5f);
	iY1 = (int)ceilf(fMaxY - 0.5f);
	iX0 = (iX0 < 0) ? 0 : iX0;
	iY0 = (iY0 < 0) ? 0 : iY0;
	iX1 = (iX1 >= iWidth) ? (iWidth - 1) : iX1;
	iY1 = (iY1 >= iHeight) ? (iHeight - 1) : iY1;

	for (y = iY0; y <= iY1; y++)
	{
		fCentreY = (float32)y + 0.5f;
		for (x = iX0; x <= iX1; x++)
		{
			fCentreX = (float32)x + 0.5f;
			bInside = true;
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				j = (k + 1) % NUM_FACE_CORNERS;
				l = (k + 2) % NUM_FACE_CORNERS;
				afWeights[k] = (((psFace->afX[j] - fCentreX) * (psFace->afY[l] - fCentreY)) - ((psFace->afX[l] - fCentreX) * (psFace->afY[j] - fCentreY))) / psFace->fTwiceArea;
				if (afWeights[k] < -1e-6f)
				{
					bInside = false;
				}
			}
			if (!bInside)
			{
				continue;
			}

			sPosition = (psFace->asPositions[0] * afWeights[0]) + (psFace->asPositions[1] * afWeights[1]) + (psFace->asPositions[2] * afWeights[2]);
			sNormal = (psFace->asNormals[0] * afWeights[0]) + (psFace->asNormals[1] * afWeights[1]) + (psFace->asNormals[2] * afWeights[2]);
			fLength = (float32)sNormal.Magnitude();
			if (fLength > 0.0f)
			{
				sNormal /= fLength;
			}
			uiTexel = ((lsize)y * (lsize)iWidth) + (lsize)x;
			MeshBakeTexel(pasTexels->Get(uiTexel), psFace, pcTexture, &sPosition, &sNormal, psSettings, bTilt, uiTint);
			paiCovered->SetValue(uiTexel, 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Breadth first out from every covered texel at once, so each other
// texel is one of the nearest's (by steps across and down).
//
//////////////////////////////////////////////////////////////////////////
static void MeshBakeDilate(int iWidth, int iHeight, CArrayMeshBakeTexel* pasTexels, CArrayLargeInt* paiCovered)
{
	CArrayLargeInt	aiQueue;
	lsize			uiNumTexels;
	lsize			uiTexel;
	lsize			uiNext;
	lsize			uiHead;
	int				x;
	int				y;
	int				k;

	aiQueue.Init();
	uiNumTexels = (lsize)iWidth * (lsize)iHeight;
	for (uiTexel = 0; uiTexel < uiNumTexels; uiTexel++)
	{
		if (paiCovered->GetValue(uiTexel) != 0)
		{
			aiQueue.Add((int)uiTexel);
		}
	}
	for (uiHead = 0; uiHead < aiQueue.NumElements(); uiHead++)
	{
		uiTexel = (lsize)aiQueue.GetValue(uiHead);
		x = (int)(uiTexel % (lsize)iWidth);
		y = (int)(uiTexel / (lsize)iWidth);
		for (k = 0; k < 4; k++)
		{
			if ((x + gaaiMeshBakeSteps[k][0] < 0) || (x + gaaiMeshBakeSteps[k][0] >= iWidth) || (y + gaaiMeshBakeSteps[k][1] < 0) || (y + gaaiMeshBakeSteps[k][1] >= iHeight))
			{
				continue;
			}
			uiNext = uiTexel + (lsize)gaaiMeshBakeSteps[k][0] + ((lsize)gaaiMeshBakeSteps[k][1] * (lsize)iWidth);  //Unsigned, so a step back wraps round to it.
			if (paiCovered->GetValue(uiNext) == 0)
			{
				paiCovered->SetValue(uiNext, 1);
				*pasTexels->Get(uiNext) = *pasTexels->Get(uiTexel);
				aiQueue.Add((int)uiNext);
			}
		}
	}
	aiQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The texels' colours as an image of red, green and blue, or their
// normals as one of x, y and z.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CImage> MeshBakeImage(CArrayMeshBakeTexel* pasTexels, int iWidth, int iHeight, bool bNormals)
{
	Ptr<CImage>	pImage;
	uint8*		puiData;
	ARGB32		uiTexel;
	lsize		i;

	if (bNormals)
	{
		pImage = OMalloc<CImage>((uint32)iWidth, (uint32)iHeight, PT_uint8, IMAGE_NORMAL_X, IMAGE_NORMAL_Y, IMAGE_NORMAL_Z, CHANNEL_STOP);
	}
	else
	{
		pImage = OMalloc<CImage>((uint32)iWidth, (uint32)iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	}
	puiData = (uint8*)pImage->GetData();
	for (i = 0; i < pasTexels->NumElements(); i++)
	{
		uiTexel = bNormals ? pasTexels->Get(i)->uiNormal : pasTexels->Get(i)->uiColour;
		puiData[(i * 3)] = Get8BitRedColour(uiTexel);
		puiData[(i * 3) + 1] = Get8BitGreenColour(uiTexel);
		puiData[(i * 3) + 2] = Get8BitBlueColour(uiTexel);
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
// A mesh with no face in the layer is white, and flat along z.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CImage> MeshBake(CMesh* pcMesh, int iLayer, CSolidTexture* pcTexture, int iWidth, int iHeight, ARGB32* pauiTints, SMeshBakeSettings* psSettings, bool bNormals, Ptr<CImage>& pNormals)
{
	SMeshBakeSettings	sOneSample;
	SMeshBakeTexel		sNothing;
	Ptr<CImage>			pImage;
	CMeshUVLayer*		pcLayer;
	SMeshBakeFace		sFace;
	CArrayMeshBakeTexel	asTexels;
	CArrayLargeInt		aiCovered;
	lsize				uiNumTexels;
	lsize				uiTexel;
	int					iFace;
	bool				bTilt;

	if (!pcMesh->mcUVs.InUse() || (iLayer < 0) || ((size)iLayer >= pcMesh->mcUVs.mcLayers.NumElements()) || (iWidth <= 0) || (iHeight <= 0))
	{
		return NULL;
	}
	pcLayer = pcMesh->mcUVs.mcLayers.Get((size)iLayer);
	if (psSettings == NULL)
	{
		sOneSample.Init();
		psSettings = &sOneSample;
	}
	bTilt = bNormals || (psSettings->fRelief != 0.0f);

	uiNumTexels = (lsize)iWidth * (lsize)iHeight;
	sNothing.uiColour = Set32BitColour((uint8)255, (uint8)255, (uint8)255);
	sNothing.uiNormal = Set32BitColour(MeshBakeNormalByte(0.0f), MeshBakeNormalByte(0.0f), MeshBakeNormalByte(1.0f));
	asTexels.Init();
	asTexels.Resize(uiNumTexels);
	for (uiTexel = 0; uiTexel < uiNumTexels; uiTexel++)
	{
		*asTexels.Get(uiTexel) = sNothing;
	}
	aiCovered.Init();
	aiCovered.Resize(uiNumTexels);
	aiCovered.SetArrayValues(0);

	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		if (MeshBakeFace(&sFace, pcMesh, pcLayer, iFace, iWidth, iHeight))
		{
			MeshBakeDraw(&sFace, pcTexture, psSettings, bTilt, (pauiTints != NULL) ? pauiTints[iFace] : sNothing.uiColour, iWidth, iHeight, &asTexels, &aiCovered);
		}
	}
	MeshBakeDilate(iWidth, iHeight, &asTexels, &aiCovered);

	pImage = MeshBakeImage(&asTexels, iWidth, iHeight, false);
	if (bNormals)
	{
		pNormals = MeshBakeImage(&asTexels, iWidth, iHeight, true);
	}

	aiCovered.Kill();
	asTexels.Kill();
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> BakeMeshTexture(CMesh* pcMesh, int iLayer, CSolidTexture* pcTexture, int iWidth, int iHeight, ARGB32* pauiTints, SMeshBakeSettings* psSettings)
{
	Ptr<CImage>	pNoNormals;

	return MeshBake(pcMesh, iLayer, pcTexture, iWidth, iHeight, pauiTints, psSettings, false, pNoNormals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> BakeMeshTexture(CMesh* pcMesh, int iLayer, CSolidTexture* pcTexture, int iWidth, int iHeight, ARGB32* pauiTints, SMeshBakeSettings* psSettings, Ptr<CImage>& pNormals)
{
	return MeshBake(pcMesh, iLayer, pcTexture, iWidth, iHeight, pauiTints, psSettings, true, pNormals);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBake::Init(Ptr<CSolidTexture> pTexture)
{
	PreInit();
	CMeshModifier::Init();
	mpTexture = pTexture;
	miSamples = MESH_BAKE_SAMPLES;
	mfRelief = 1.0f;
	msLight.Init(MESH_BAKE_LIGHT_X, MESH_BAKE_LIGHT_Y, MESH_BAKE_LIGHT_Z);
	msLight.Normalize();
	mbNormals = false;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshBake::Class(void)
{
	CMeshModifier::Class();
	M_Pointer(mpTexture);
	M_Pointer(mpImage);
	M_Pointer(mpNormals);
	U_SInt(miSamples);
	U_Float32(mfRelief);
	U_3Float32(msLight);
	U_Bool(mbNormals);
}


//////////////////////////////////////////////////////////////////////////
//
// Each face's material's diffuse colour, white for a face without one.
//
//////////////////////////////////////////////////////////////////////////
static void MeshBakeTints(CArrayLargeUint32* pauiDest, CMesh* pcMesh)
{
	Ptr<CMaterial>		pMaterial;
	CImageColourARGB*	pcColour;
	ARGB32				uiTint;
	int					iMaterial;
	int					iFace;

	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		uiTint = Set32BitColour((uint8)255, (uint8)255, (uint8)255);
		if (pcMesh->mcMaterials.InUse() && ((lsize)iFace < pcMesh->mcMaterials.mcFaces.NumElements()))
		{
			iMaterial = pcMesh->mcMaterials.mcFaces.GetValue((lsize)iFace);
			if ((iMaterial >= 0) && ((size)iMaterial < pcMesh->mcMaterials.mapMaterials.NumElements()))
			{
				pMaterial = pcMesh->mcMaterials.mapMaterials.Get((size)iMaterial);
				if (pMaterial.IsNotNull())
				{
					pcColour = &pMaterial->msProperties.mcDiffuseColour;
					uiTint = Set32BitColour(pcColour->mfRed, pcColour->mfGreen, pcColour->mfBlue);
				}
			}
		}
		pauiDest->Add(uiTint);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A material's image's next level: the image itself, then each of its
// smaller levels.
//
//////////////////////////////////////////////////////////////////////////
static void MeshBakeAddLevel(CMaterialImage* pcImage, Ptr<CImage> pLevel)
{
	if (pcImage->mpImage.IsNull())
	{
		pcImage->mpImage = pLevel;
	}
	else
	{
		pcImage->mapLevels.Add(pLevel);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Level 0 at the layout's size, then each level half the one before
// (rounded up) to 1 by 1, every one baked from the texture.  Level 0 is
// one sample a texel; each level after has twice as many each way, up to
// miSamples, so a texel twice as wide is still made of samples as fine.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshBake::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>			pOutput;
	Ptr<CMaterial>		pMaterial;
	Ptr<CImage>			pNormals;
	CMeshUVLayer*		pcLayer;
	CArrayLargeUint32	auiTints;
	SMeshBakeSettings	sSettings;
	int					iWidth;
	int					iHeight;
	bool				bLast;

	if (mpTexture.IsNull() || (pMesh->NumFaces() == 0) || !pMesh->mcUVs.InUse() || (pMesh->mcUVs.mcLayers.NumElements() == 0))
	{
		return pMesh;
	}
	pcLayer = pMesh->mcUVs.mcLayers.Get(0);
	iWidth = pcLayer->miLayoutWidth;
	iHeight = pcLayer->miLayoutHeight;
	if ((iWidth <= 0) || (iHeight <= 0))
	{
		return pMesh;
	}

	auiTints.Init();
	MeshBakeTints(&auiTints, &pMesh);
	pMaterial = OMalloc<CMaterial>();
	pMaterial->msProperties.mcDiffuseColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	sSettings.iSamples = 1;
	sSettings.fRelief = mfRelief;
	sSettings.sLight = msLight;
	do
	{
		if (mbNormals)
		{
			MeshBakeAddLevel(pMaterial->GetImage(MI_Diffuse), BakeMeshTexture(&pMesh, 0, &mpTexture, iWidth, iHeight, auiTints.GetData(), &sSettings, pNormals));
			MeshBakeAddLevel(pMaterial->GetImage(MI_Bump), pNormals);
		}
		else
		{
			MeshBakeAddLevel(pMaterial->GetImage(MI_Diffuse), BakeMeshTexture(&pMesh, 0, &mpTexture, iWidth, iHeight, auiTints.GetData(), &sSettings));
		}

		bLast = (iWidth == 1) && (iHeight == 1);
		iWidth = (iWidth + 1) / 2;
		iHeight = (iHeight + 1) / 2;
		if ((sSettings.iSamples * 2) <= miSamples)
		{
			sSettings.iSamples *= 2;
		}
	}
	while (!bLast);
	auiTints.Kill();
	mpImage = pMaterial->GetImage(MI_Diffuse)->mpImage;
	mpNormals = pMaterial->GetImage(MI_Bump)->mpImage;

	pOutput = OMalloc<CMesh>();
	pOutput->CopyFrom(&pMesh);
	pOutput->mcMaterials.mbInUse = true;
	pOutput->mcMaterials.mapMaterials.RemoveAll();
	pOutput->mcMaterials.mapMaterials.Add(pMaterial);
	pOutput->mcMaterials.mcFaces.Resize((lsize)pOutput->NumFaces());
	pOutput->mcMaterials.mcFaces.SetArrayValues(0);
	return pOutput;
}

