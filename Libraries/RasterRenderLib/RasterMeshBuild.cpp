/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterRenderLib

Codaphela RasterRenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterRenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterRenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include <string.h>
#include "BaseLib/FixedPoint.h"
#include "StandardLib/Objects.h"
#include "SupportLib/Mesh.h"
#include "RenderLib/CookedMesh.h"
#include "RasterLib/RasterMesh.h"
#include "RasterMeshBuild.h"


//////////////////////////////////////////////////////////////////////////
//
// A raster vertex's weights, from its cooked vertex's.  Each is its share
// of RASTER_SKIN_ONE, rounded, and what the rounding leaves over goes to the
// largest, so they add up to exactly that.  They're most first, and one
// that rounds to nothing is left out.
//
//////////////////////////////////////////////////////////////////////////
static void AddRasterSkinVertex(CRasterMesh* pcRaster, CCookedMesh* pcCooked, size uiVertex)
{
	SCookedSkinVertex*	psFrom;
	SCookedSkinWeight*	psFromWeight;
	SRasterSkinVertex*	psSkin;
	SRasterSkinWeight*	psWeights;
	SRasterSkinWeight	sSwap;
	int32				iTotal;
	int32				iWeight;
	size				uiFirst;
	size				uiCount;
	size				uiLargest;
	size				i;
	size				j;

	psFrom = pcCooked->masSkinVertices.Get(uiVertex);
	uiFirst = pcRaster->masSkinWeights.NumElements();
	iTotal = 0;
	for (i = 0; i < psFrom->uiCount; i++)
	{
		psFromWeight = pcCooked->masSkinWeights.Get(psFrom->uiFirst + i);
		iWeight = (int32)floor(psFromWeight->fWeight * RASTER_SKIN_ONE + 0.5);
		if (iWeight > 0)
		{
			psWeights = pcRaster->masSkinWeights.Add();
			psWeights->uiMatrix = (uint16)psFromWeight->uiMatrix;
			psWeights->uiWeight = (uint16)iWeight;
			iTotal += iWeight;
		}
	}

	uiCount = pcRaster->masSkinWeights.NumElements() - uiFirst;
	if (uiCount > 0)
	{
		psWeights = pcRaster->masSkinWeights.Get(uiFirst);
		uiLargest = 0;
		for (i = 1; i < uiCount; i++)
		{
			uiLargest = (psWeights[i].uiWeight > psWeights[uiLargest].uiWeight) ? i : uiLargest;
		}
		psWeights[uiLargest].uiWeight = (uint16)(psWeights[uiLargest].uiWeight + RASTER_SKIN_ONE - iTotal);
		for (i = 1; i < uiCount; i++)
		{
			for (j = i; (j > 0) && (psWeights[j].uiWeight > psWeights[j - 1].uiWeight); j--)
			{
				sSwap = psWeights[j];
				psWeights[j] = psWeights[j - 1];
				psWeights[j - 1] = sSwap;
			}
		}
	}

	psSkin = pcRaster->masSkinVertices.Add();
	psSkin->uiFirst = (uint32)uiFirst;
	psSkin->uiCount = (uint32)uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle's colour is its first corner's if the mesh has vertex colours,
// and its submesh's if not.  It's transparent if its colour is.
//
//////////////////////////////////////////////////////////////////////////
static void AddRasterTriangles(CRasterMesh* pcRaster, CCookedMesh* pcCooked, SCookedSubmesh* psSubmesh)
{
	SRasterTriangle*	psTriangle;
	uint32*				puiIndices;
	uint32				uiColour;
	uint32				i;
	int					k;

	puiIndices = pcCooked->mauiIndices.GetData() + psSubmesh->uiFirstIndex;
	for (i = 0; i < psSubmesh->uiNumIndices; i += 3)
	{
		uiColour = pcCooked->HasColours() ? pcCooked->mauiColours.GetValue(puiIndices[i]) : psSubmesh->uiColour;
		psTriangle = ((uiColour >> 24) == 0xFF) ? pcRaster->masOpaque.Add() : pcRaster->masTransparent.Add();
		for (k = 0; k < 3; k++)
		{
			psTriangle->aiVertex[k] = (int32)puiIndices[i + k];
		}
		psTriangle->uiColour = uiColour;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The cooked mesh's vertices, moved by -psCentre and with iScale units to
// each of the mesh's, and its triangles.
//
//////////////////////////////////////////////////////////////////////////
static void BuildRasterMeshFrom(CRasterMesh* pcRaster, CCookedMesh* pcCooked, SFloat32Vec3* psCentre, int32 iScale)
{
	SFloat32Vec3*	psPosition;
	SFloat32Vec3*	psNormal;
	SRasterVertex*	psVertex;
	size			i;

	for (i = 0; i < pcCooked->NumVertices(); i++)
	{
		psPosition = pcCooked->masPositions.Get(i);
		psNormal = pcCooked->masNormals.Get(i);
		psVertex = pcRaster->masVertices.Add();
		memset(psVertex, 0, sizeof(SRasterVertex));  //Its padding is saved with it, so WriteFile writes the same bytes each time.
		psVertex->sPosition.Init(FloatToIntScaled(psPosition->x - psCentre->x, iScale), FloatToIntScaled(psPosition->y - psCentre->y, iScale), FloatToIntScaled(psPosition->z - psCentre->z, iScale));
		psVertex->sNormal.Init(SRasterUnit::FromFloat(psNormal->x), SRasterUnit::FromFloat(psNormal->y), SRasterUnit::FromFloat(psNormal->z));
		if (pcCooked->IsSkinned())
		{
			AddRasterSkinVertex(pcRaster, pcCooked, i);
		}
	}
	pcRaster->muiNumMatrices = pcCooked->IsSkinned() ? pcCooked->muiNumMatrices : 0;

	for (i = 0; i < pcCooked->masSubmeshes.NumElements(); i++)
	{
		AddRasterTriangles(pcRaster, pcCooked, pcCooked->masSubmeshes.Get(i));
	}

	pcRaster->RemoveDegenerate();
	pcRaster->UpdateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// Centred on the origin, and scaled so that its largest half width is
// RASTER_MESH_HALF_EXTENT.
//
//////////////////////////////////////////////////////////////////////////
void BuildRasterMesh(CRasterMesh* pcRaster, CCookedMesh* pcCooked)
{
	SFloat32Vec3	sCentre;
	float32			fHalfExtent;
	int32			iScale;

	pcRaster->Clear();
	if (pcCooked->NumTriangles() == 0)
	{
		return;
	}
	sCentre.Init((pcCooked->msMin.x + pcCooked->msMax.x) * 0.5f, (pcCooked->msMin.y + pcCooked->msMax.y) * 0.5f, (pcCooked->msMin.z + pcCooked->msMax.z) * 0.5f);
	fHalfExtent = (pcCooked->msMax.x - pcCooked->msMin.x) * 0.5f;
	if ((pcCooked->msMax.y - pcCooked->msMin.y) * 0.5f > fHalfExtent)
	{
		fHalfExtent = (pcCooked->msMax.y - pcCooked->msMin.y) * 0.5f;
	}
	if ((pcCooked->msMax.z - pcCooked->msMin.z) * 0.5f > fHalfExtent)
	{
		fHalfExtent = (pcCooked->msMax.z - pcCooked->msMin.z) * 0.5f;
	}
	iScale = (fHalfExtent > 0.0f) ? (int32)floor((float64)RASTER_MESH_HALF_EXTENT / (float64)fHalfExtent) : 1;
	BuildRasterMeshFrom(pcRaster, pcCooked, &sCentre, iScale);
}


//////////////////////////////////////////////////////////////////////////
//
// Where it is, with iScale units to each of the mesh's.
//
//////////////////////////////////////////////////////////////////////////
void BuildRasterMesh(CRasterMesh* pcRaster, CCookedMesh* pcCooked, int32 iScale)
{
	SFloat32Vec3	sCentre;

	pcRaster->Clear();
	if (pcCooked->NumTriangles() > 0)
	{
		sCentre.Zero();
		BuildRasterMeshFrom(pcRaster, pcCooked, &sCentre, iScale);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BuildRasterMesh(CRasterMesh* pcRaster, CMesh* pcMesh)
{
	Ptr<CCookedMesh>	pCooked;

	pCooked = OMalloc<CCookedMesh>();
	pCooked->Cook(pcMesh);
	BuildRasterMesh(pcRaster, &pCooked);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void BuildRasterMesh(CRasterMesh* pcRaster, CMesh* pcMesh, int32 iScale)
{
	Ptr<CCookedMesh>	pCooked;

	pCooked = OMalloc<CCookedMesh>();
	pCooked->Cook(pcMesh);
	BuildRasterMesh(pcRaster, &pCooked, iScale);
}

