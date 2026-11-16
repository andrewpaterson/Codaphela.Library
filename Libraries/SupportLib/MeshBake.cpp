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


//////////////////////////////////////////////////////////////////////////
//
// A face's corners' positions and normals, and its UVs in texels.  False
// if it has no UVs, or no area in 3D or in texels.
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
	int					iNormal;
	int					k;

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
// The texels whose centres a face covers (its barycentric weights all at
// least nothing, either way round), each its texture's colour there.
//
//////////////////////////////////////////////////////////////////////////
static void MeshBakeDraw(SMeshBakeFace* psFace, CSolidTexture* pcTexture, ARGB32 uiTint, int iWidth, int iHeight, CArrayLargeUint32* pauiColours, CArrayLargeInt* paiCovered)
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
			pauiColours->SetValue(uiTexel, Modulate32BitColours(pcTexture->Colour(&sPosition, &sNormal, psFace->fWidth), uiTint));
			paiCovered->SetValue(uiTexel, 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Breadth first out from every covered texel at once, so each other
// texel takes the colour of one of the nearest (by steps across and down).
//
//////////////////////////////////////////////////////////////////////////
static void MeshBakeDilate(int iWidth, int iHeight, CArrayLargeUint32* pauiColours, CArrayLargeInt* paiCovered)
{
	CArrayLargeInt	aiQueue;
	lsize			uiNumTexels;
	lsize			uiTexel;
	lsize			uiNext;
	lsize			uiHead;
	int				x;
	int				y;
	int				k;
	static const int	aaiSteps[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

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
			if ((x + aaiSteps[k][0] < 0) || (x + aaiSteps[k][0] >= iWidth) || (y + aaiSteps[k][1] < 0) || (y + aaiSteps[k][1] >= iHeight))
			{
				continue;
			}
			uiNext = uiTexel + (lsize)aaiSteps[k][0] + ((lsize)aaiSteps[k][1] * (lsize)iWidth);  //Unsigned, so a step back wraps round to it.
			if (paiCovered->GetValue(uiNext) == 0)
			{
				paiCovered->SetValue(uiNext, 1);
				pauiColours->SetValue(uiNext, pauiColours->GetValue(uiTexel));
				aiQueue.Add((int)uiNext);
			}
		}
	}
	aiQueue.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> BakeMeshTexture(CMesh* pcMesh, int iLayer, CSolidTexture* pcTexture, int iWidth, int iHeight, ARGB32* pauiTints)
{
	Ptr<CImage>			pImage;
	CMeshUVLayer*		pcLayer;
	SMeshBakeFace		sFace;
	CArrayLargeUint32	auiColours;
	CArrayLargeInt		aiCovered;
	uint8*				puiData;
	ARGB32				uiColour;
	lsize				uiNumTexels;
	lsize				uiTexel;
	int					iFace;

	if (!pcMesh->mcUVs.InUse() || (iLayer < 0) || ((size)iLayer >= pcMesh->mcUVs.mcLayers.NumElements()) || (iWidth <= 0) || (iHeight <= 0))
	{
		return NULL;
	}
	pcLayer = pcMesh->mcUVs.mcLayers.Get((size)iLayer);

	uiNumTexels = (lsize)iWidth * (lsize)iHeight;
	auiColours.Init();
	auiColours.Resize(uiNumTexels);
	auiColours.SetArrayValues(Set32BitColour((uint8)255, (uint8)255, (uint8)255));
	aiCovered.Init();
	aiCovered.Resize(uiNumTexels);
	aiCovered.SetArrayValues(0);

	for (iFace = 0; iFace < pcMesh->NumFaces(); iFace++)
	{
		if (MeshBakeFace(&sFace, pcMesh, pcLayer, iFace, iWidth, iHeight))
		{
			MeshBakeDraw(&sFace, pcTexture, (pauiTints != NULL) ? pauiTints[iFace] : Set32BitColour((uint8)255, (uint8)255, (uint8)255), iWidth, iHeight, &auiColours, &aiCovered);
		}
	}
	MeshBakeDilate(iWidth, iHeight, &auiColours, &aiCovered);

	pImage = OMalloc<CImage>((uint32)iWidth, (uint32)iHeight, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	puiData = (uint8*)pImage->GetData();
	for (uiTexel = 0; uiTexel < uiNumTexels; uiTexel++)
	{
		uiColour = auiColours.GetValue(uiTexel);
		puiData[(uiTexel * 3)] = Get8BitRedColour(uiColour);
		puiData[(uiTexel * 3) + 1] = Get8BitGreenColour(uiColour);
		puiData[(uiTexel * 3) + 2] = Get8BitBlueColour(uiColour);
	}

	aiCovered.Kill();
	auiColours.Kill();
	return pImage;
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
// Level 0 at the layout's size, then each level half the one before
// (rounded up) to 1 by 1, every one baked from the texture.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshBake::Modify(Ptr<CMesh> pMesh)
{
	Ptr<CMesh>			pOutput;
	Ptr<CMaterial>		pMaterial;
	CMaterialImage*		pcDiffuse;
	CMeshUVLayer*		pcLayer;
	CArrayLargeUint32	auiTints;
	int					iWidth;
	int					iHeight;

	if (mpTexture.IsNull() || (pMesh->NumFaces() == 0) || !pMesh->mcUVs.InUse() || (pMesh->mcUVs.mcLayers.NumElements() == 0))
	{
		return pMesh;
	}
	pcLayer = pMesh->mcUVs.mcLayers.Get(0);
	iWidth = pcLayer->miLayoutWidth;
	iHeight = pcLayer->miLayoutHeight;
	auiTints.Init();
	MeshBakeTints(&auiTints, &pMesh);
	mpImage = BakeMeshTexture(&pMesh, 0, &mpTexture, iWidth, iHeight, auiTints.GetData());
	if (mpImage.IsNull())
	{
		auiTints.Kill();
		return pMesh;
	}

	pMaterial = OMalloc<CMaterial>();
	pMaterial->msProperties.mcDiffuseColour.Init(1.0f, 1.0f, 1.0f, 1.0f);
	pcDiffuse = pMaterial->GetImage(MI_Diffuse);
	pcDiffuse->mpImage = mpImage;
	while ((iWidth > 1) || (iHeight > 1))
	{
		iWidth = (iWidth + 1) / 2;
		iHeight = (iHeight + 1) / 2;
		pcDiffuse->mapLevels.Add(BakeMeshTexture(&pMesh, 0, &mpTexture, iWidth, iHeight, auiTints.GetData()));
	}
	auiTints.Kill();

	pOutput = OMalloc<CMesh>();
	pOutput->CopyFrom(&pMesh);
	pOutput->mcMaterials.mbInUse = true;
	pOutput->mcMaterials.mapMaterials.RemoveAll();
	pOutput->mcMaterials.mapMaterials.Add(pMaterial);
	pOutput->mcMaterials.mcFaces.Resize((lsize)pOutput->NumFaces());
	pOutput->mcMaterials.mcFaces.SetArrayValues(0);
	return pOutput;
}

