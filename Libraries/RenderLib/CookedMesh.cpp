/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "StandardLib/ClassDefines.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "CookedMesh.h"


//VRML's default material, for a face with no colour and no material.
#define COOKED_DEFAULT_COLOUR	0xFFCCCCCC


//What a face corner is as a vertex: its corner and the indices of what it has.  -1 is none.
struct SCookedCorner
{
	int32	iCorner;
	int32	iNormal;
	int32	iUV;
	int32	iColour;
};


//What decides a face's submesh.
struct SCookedFaceKind
{
	int32	iSlot;
	bool	bTransparent;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCookedMesh::Init(void)
{
	PreInit();

	masPositions.Init();
	masNormals.Init();
	masUVs.Init();
	mauiColours.Init();
	maiCorners.Init();
	masSkinVertices.Init();
	masSkinWeights.Init();
	muiNumMatrices = 0;
	mauiIndices.Init();
	masSubmeshes.Init();
	mapMaterials.Init();
	msMin.Zero();
	msMax.Zero();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCookedMesh::Free(void)
{
	masSubmeshes.Kill();
	mauiIndices.Kill();
	masSkinWeights.Kill();
	masSkinVertices.Kill();
	maiCorners.Kill();
	mauiColours.Kill();
	masUVs.Kill();
	masNormals.Kill();
	masPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCookedMesh::Class(void)
{
	U_Unknown(CArrayCookedVec3, masPositions);
	U_Unknown(CArrayCookedVec3, masNormals);
	U_Unknown(CArrayCookedVec2, masUVs);
	U_Unknown(CArrayCookedUint, mauiColours);
	U_Unknown(CArrayCookedInt, maiCorners);
	U_Unknown(CArrayCookedSkinVertex, masSkinVertices);
	U_Unknown(CArrayCookedSkinWeight, masSkinWeights);
	U_UInt32(muiNumMatrices);
	U_Unknown(CArrayCookedUint, mauiIndices);
	U_Unknown(CArrayCookedSubmesh, masSubmeshes);
	M_Embedded(mapMaterials);
	U_3Float32(msMin);
	U_3Float32(msMax);
}


//////////////////////////////////////////////////////////////////////////
//
// 0xAARRGGBB, each part rounded to a byte.
//
//////////////////////////////////////////////////////////////////////////
static uint32 CookedColourByte(float32 fPart)
{
	fPart = (fPart < 0.0f) ? 0.0f : ((fPart > 1.0f) ? 1.0f : fPart);
	return (uint32)floor(fPart * 255.0f + 0.5f);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint32 CookedMaterialColour(CMaterial* pcMaterial)
{
	CImageColourARGB*	pcColour;

	if (pcMaterial == NULL)
	{
		return COOKED_DEFAULT_COLOUR;
	}
	pcColour = &pcMaterial->msProperties.mcDiffuseColour;
	return (CookedColourByte(pcColour->mfAlpha) << 24) | (CookedColourByte(pcColour->mfRed) << 16) | (CookedColourByte(pcColour->mfGreen) << 8) | CookedColourByte(pcColour->mfBlue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint32 CookedVertexColour(ARGB32 uiColour)
{
	return ((uint32)Get8BitAlphaColour(uiColour) << 24) | ((uint32)Get8BitRedColour(uiColour) << 16) | ((uint32)Get8BitGreenColour(uiColour) << 8) | (uint32)Get8BitBlueColour(uiColour);
}


//////////////////////////////////////////////////////////////////////////
//
// A face corner's normal, UV and colour, -1 for each it hasn't.  A corner
// with no normal of its own has its face's.
//
//////////////////////////////////////////////////////////////////////////
static void GetCookedCorner(CMesh* pcMesh, int iFace, int k, bool bUVs, SCookedCorner* psCorner)
{
	SMeshNormalFace*	psNormalFace;

	psCorner->iCorner = pcMesh->GetFaceCorners(iFace)[k];
	psCorner->iNormal = -1;
	psCorner->iUV = -1;
	psCorner->iColour = -1;
	if (pcMesh->mcNormals.InUse())
	{
		psNormalFace = pcMesh->mcNormals.mcFaces.Get(iFace);
		psCorner->iNormal = psNormalFace->aiCornerNormals[k];
		if (psCorner->iNormal < 0)
		{
			psCorner->iNormal = psNormalFace->iFaceNormal;
		}
	}
	if (bUVs)
	{
		psCorner->iUV = pcMesh->mcUVs.mcLayers.Get(0)->mcFaces.Get(iFace)->aiUVs[k];
	}
	if (pcMesh->mcColours.InUse())
	{
		psCorner->iColour = pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours[k];
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A face's material slot, -1 for none, and whether it's transparent.
//
//////////////////////////////////////////////////////////////////////////
static void GetCookedFaceKind(CMesh* pcMesh, int iFace, SCookedFaceKind* psKind)
{
	CMaterial*	pcMaterial;
	int			iColour;

	psKind->iSlot = -1;
	psKind->bTransparent = false;
	if (pcMesh->mcMaterials.InUse())
	{
		psKind->iSlot = pcMesh->mcMaterials.mcFaces.GetValue((size)iFace);
		if ((psKind->iSlot >= 0) && ((size)psKind->iSlot < pcMesh->mcMaterials.mapMaterials.NumElements()))
		{
			pcMaterial = &pcMesh->mcMaterials.mapMaterials.Get((size)psKind->iSlot);
			psKind->bTransparent = (pcMaterial != NULL) && ((CookedMaterialColour(pcMaterial) >> 24) != 0xFF);
		}
		else
		{
			psKind->iSlot = -1;
		}
	}
	if (pcMesh->mcColours.InUse())
	{
		iColour = pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours[0];
		if ((iColour >= 0) && ((CookedVertexColour(pcMesh->mcColours.mcColours.GetValue((size)iColour)) >> 24) != 0xFF))
		{
			psKind->bTransparent = true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A position's weights, made to add up to one, most first, and none of
// nothing.
//
//////////////////////////////////////////////////////////////////////////
static void AddCookedSkinVertex(CCookedMesh* pcCooked, CMeshSkinVert* pcSkinVert)
{
	SCookedSkinVertex*	psSkin;
	SCookedSkinWeight*	psWeights;
	SCookedSkinWeight	sSwap;
	SSkinWeight*		psFrom;
	float64				fTotal;
	size				uiFirst;
	size				uiCount;
	size				i;
	size				j;

	uiFirst = pcCooked->masSkinWeights.NumElements();
	fTotal = 0.0;
	for (i = 0; i < pcSkinVert->mcWeights.NumElements(); i++)
	{
		psFrom = pcSkinVert->mcWeights.Get(i);
		if (psFrom->fWeight > 0.0f)
		{
			fTotal += psFrom->fWeight;
		}
	}
	for (i = 0; (fTotal > 0.0) && (i < pcSkinVert->mcWeights.NumElements()); i++)
	{
		psFrom = pcSkinVert->mcWeights.Get(i);
		if (psFrom->fWeight > 0.0f)
		{
			psWeights = pcCooked->masSkinWeights.Add();
			psWeights->uiMatrix = (uint32)psFrom->iMatrix;
			psWeights->fWeight = (float32)(psFrom->fWeight / fTotal);
		}
	}

	uiCount = pcCooked->masSkinWeights.NumElements() - uiFirst;
	psWeights = (uiCount > 0) ? pcCooked->masSkinWeights.Get(uiFirst) : NULL;
	for (i = 1; i < uiCount; i++)
	{
		for (j = i; (j > 0) && (psWeights[j].fWeight > psWeights[j - 1].fWeight); j--)
		{
			sSwap = psWeights[j];
			psWeights[j] = psWeights[j - 1];
			psWeights[j - 1] = sSwap;
		}
	}

	psSkin = pcCooked->masSkinVertices.Add();
	psSkin->uiFirst = (uint32)uiFirst;
	psSkin->uiCount = (uint32)uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A new vertex for a face corner.
//
//////////////////////////////////////////////////////////////////////////
static void AddCookedVertex(CCookedMesh* pcCooked, CMesh* pcMesh, SCookedCorner* psCorner, bool bUVs, bool bSkinned)
{
	SFloat32Vec3*	psNormal;
	SFloat32Vec2*	psUV;

	pcCooked->masPositions.Add(pcMesh->GetPosition(psCorner->iCorner));
	psNormal = pcCooked->masNormals.Add();
	psNormal->Zero();
	if (psCorner->iNormal >= 0)
	{
		*psNormal = *pcMesh->mcNormals.mcNormals.Get((size)psCorner->iNormal);
	}
	if (bUVs)
	{
		psUV = pcCooked->masUVs.Add();
		psUV->Init(0.0f, 0.0f);
		if (psCorner->iUV >= 0)
		{
			*psUV = *pcMesh->mcUVs.mcLayers.Get(0)->mcUVs.Get((size)psCorner->iUV);
		}
	}
	if (pcMesh->mcColours.InUse())
	{
		pcCooked->mauiColours.Add((psCorner->iColour >= 0) ? CookedVertexColour(pcMesh->mcColours.mcColours.GetValue((size)psCorner->iColour)) : COOKED_DEFAULT_COLOUR);
	}
	if (bSkinned)
	{
		AddCookedSkinVertex(pcCooked, pcMesh->mcSkin.mcSkinVerts.Get((size)psCorner->iCorner));
	}
	pcCooked->maiCorners.Add(psCorner->iCorner);
}


//////////////////////////////////////////////////////////////////////////
//
// The submesh a face's kind is, made the first time a face of it is found,
// so submeshes are in the order their first faces are.
//
//////////////////////////////////////////////////////////////////////////
static size FindCookedSubmesh(CCookedMesh* pcCooked, CMesh* pcMesh, SCookedFaceKind* psKind)
{
	SCookedSubmesh*	psSubmesh;
	CMaterial*		pcMaterial;
	size			i;

	for (i = 0; i < pcCooked->masSubmeshes.NumElements(); i++)
	{
		psSubmesh = pcCooked->masSubmeshes.Get(i);
		if ((psSubmesh->iSlot == psKind->iSlot) && (psSubmesh->bTransparent == psKind->bTransparent))
		{
			return i;
		}
	}
	pcMaterial = (psKind->iSlot >= 0) ? &pcMesh->mcMaterials.mapMaterials.Get((size)psKind->iSlot) : NULL;
	psSubmesh = pcCooked->masSubmeshes.Add();
	psSubmesh->iSlot = psKind->iSlot;
	psSubmesh->uiColour = CookedMaterialColour(pcMaterial);
	psSubmesh->uiFirstIndex = 0;
	psSubmesh->uiNumIndices = 0;
	psSubmesh->bTransparent = psKind->bTransparent;
	pcCooked->mapMaterials.Add((psKind->iSlot >= 0) ? pcMesh->mcMaterials.mapMaterials.Get((size)psKind->iSlot) : Ptr<CMaterial>());
	return pcCooked->masSubmeshes.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A vertex for each different corner of each face, and each face's
// submesh.  A corner's vertices are found by going through those made for
// it, as a corner has only a few.
//
//////////////////////////////////////////////////////////////////////////
void CCookedMesh::CookVertices(CMesh* pcMesh, CArrayCookedInt* paiFaceVertices, CArrayCookedInt* paiFaceSubmeshes)
{
	SCookedCorner					sCorner;
	SCookedCorner*					psMade;
	SCookedFaceKind					sKind;
	CArrayCookedInt					aiCornerFirst;
	CArrayCookedInt					aiVertexNext;
	CArrayTemplate<SCookedCorner>	asMade;
	int								i;
	int								k;
	int32							iVertex;
	bool							bUVs;
	bool							bSkinned;

	bUVs = pcMesh->mcUVs.InUse() && (pcMesh->mcUVs.mcLayers.NumElements() > 0);
	bSkinned = pcMesh->mcSkin.InUse() && (pcMesh->mcSkin.mcSkinVerts.NumElements() >= (size)pcMesh->NumCorners());
	if (bSkinned)
	{
		muiNumMatrices = (uint32)pcMesh->mcSkin.mcInverseSkinMatricies.NumElements();
	}

	aiCornerFirst.Init();
	aiVertexNext.Init();
	asMade.Init();
	aiCornerFirst.Resize(pcMesh->NumCorners());
	aiCornerFirst.SetArrayValues(-1);
	for (i = 0; i < pcMesh->NumFaces(); i++)
	{
		GetCookedFaceKind(pcMesh, i, &sKind);
		paiFaceSubmeshes->Add((int32)FindCookedSubmesh(this, pcMesh, &sKind));
		for (k = 0; k < 3; k++)
		{
			GetCookedCorner(pcMesh, i, k, bUVs, &sCorner);
			for (iVertex = aiCornerFirst.GetValue((size)sCorner.iCorner); iVertex >= 0; iVertex = aiVertexNext.GetValue((size)iVertex))
			{
				psMade = asMade.Get((size)iVertex);
				if ((psMade->iNormal == sCorner.iNormal) && (psMade->iUV == sCorner.iUV) && (psMade->iColour == sCorner.iColour))
				{
					break;
				}
			}
			if (iVertex < 0)
			{
				iVertex = (int32)asMade.NumElements();
				asMade.Add(&sCorner);
				aiVertexNext.Add(aiCornerFirst.GetValue((size)sCorner.iCorner));
				aiCornerFirst.SetValue((size)sCorner.iCorner, iVertex);
				AddCookedVertex(this, pcMesh, &sCorner, bUVs, bSkinned);
			}
			paiFaceVertices->Add(iVertex);
		}
	}
	asMade.Kill();
	aiVertexNext.Kill();
	aiCornerFirst.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each submesh's triangles, in the order their faces are.
//
//////////////////////////////////////////////////////////////////////////
void CCookedMesh::CookIndices(CArrayCookedInt* paiFaceVertices, CArrayCookedInt* paiFaceSubmeshes)
{
	SCookedSubmesh*	psSubmesh;
	size			i;
	size			j;
	int				k;

	for (j = 0; j < masSubmeshes.NumElements(); j++)
	{
		psSubmesh = masSubmeshes.Get(j);
		psSubmesh->uiFirstIndex = (uint32)mauiIndices.NumElements();
		for (i = 0; i < paiFaceSubmeshes->NumElements(); i++)
		{
			if (paiFaceSubmeshes->GetValue(i) == (int32)j)
			{
				for (k = 0; k < 3; k++)
				{
					mauiIndices.Add((uint32)paiFaceVertices->GetValue(i * 3 + k));
				}
			}
		}
		psSubmesh->uiNumIndices = (uint32)mauiIndices.NumElements() - psSubmesh->uiFirstIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCookedMesh::CookBounds(void)
{
	size	i;

	msMin.Zero();
	msMax.Zero();
	if (masPositions.NumElements() == 0)
	{
		return;
	}
	msMin = *masPositions.Get(0);
	msMax = msMin;
	for (i = 1; i < masPositions.NumElements(); i++)
	{
		Float32Vec3Minimize(&msMin, &msMin, masPositions.Get(i));
		Float32Vec3Maximize(&msMax, &msMax, masPositions.Get(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces what it had.
//
//////////////////////////////////////////////////////////////////////////
void CCookedMesh::Cook(CMesh* pcMesh)
{
	CArrayCookedInt	aiFaceVertices;
	CArrayCookedInt	aiFaceSubmeshes;

	masPositions.ReInit();
	masNormals.ReInit();
	masUVs.ReInit();
	mauiColours.ReInit();
	maiCorners.ReInit();
	masSkinVertices.ReInit();
	masSkinWeights.ReInit();
	mauiIndices.ReInit();
	masSubmeshes.ReInit();
	mapMaterials.RemoveAll();
	muiNumMatrices = 0;

	aiFaceVertices.Init();
	aiFaceSubmeshes.Init();
	CookVertices(pcMesh, &aiFaceVertices, &aiFaceSubmeshes);
	CookIndices(&aiFaceVertices, &aiFaceSubmeshes);
	CookBounds();
	aiFaceSubmeshes.Kill();
	aiFaceVertices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCookedMesh::NumVertices(void) { return masPositions.NumElements(); }
size CCookedMesh::NumTriangles(void) { return mauiIndices.NumElements() / 3; }
bool CCookedMesh::IsSkinned(void) { return masSkinVertices.NumElements() > 0; }
bool CCookedMesh::HasColours(void) { return mauiColours.NumElements() > 0; }
bool CCookedMesh::HasUVs(void) { return masUVs.NumElements() > 0; }

