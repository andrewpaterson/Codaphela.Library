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
#include "BaseLib/ArrayInt.h"
#include "BaseLib/FixedPoint.h"
#include "BaseLib/GeometricTypes.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "RasterLib/RasterMesh.h"
#include "RasterMeshBuild.h"


//VRML's default material for a face with no colour.
#define RASTER_DEFAULT_COLOUR	0xFFCCCCCC


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static uint32 RasterColour(CMesh* pcMesh, int iFace)
{
	int		iColour;
	ARGB32	uiColour;

	if (!pcMesh->mcColours.InUse())
	{
		return RASTER_DEFAULT_COLOUR;
	}

	iColour = pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours[0];
	if (iColour < 0)
	{
		return RASTER_DEFAULT_COLOUR;
	}

	uiColour = pcMesh->mcColours.mcColours.GetValue(iColour);
	return ((uint32)Get8BitAlphaColour(uiColour) << 24) | ((uint32)Get8BitRedColour(uiColour) << 16) | ((uint32)Get8BitGreenColour(uiColour) << 8) | (uint32)Get8BitBlueColour(uiColour);
}


//////////////////////////////////////////////////////////////////////////
//
// Empties pcRaster.  False if pcMesh has nothing to build.
//
//////////////////////////////////////////////////////////////////////////
static bool FindRasterBounds(CRasterMesh* pcRaster, CMesh* pcMesh, SFloat32Vec3* psMin, SFloat32Vec3* psMax)
{
	SFloat32Vec3*	psPosition;
	int				iNumCorners;
	int				i;

	pcRaster->Clear();

	iNumCorners = pcMesh->NumCorners();
	if ((iNumCorners == 0) || (pcMesh->NumFaces() == 0))
	{
		return false;
	}

	*psMin = *pcMesh->GetPosition(0);
	*psMax = *psMin;
	for (i = 1; i < iNumCorners; i++)
	{
		psPosition = pcMesh->GetPosition(i);
		Float32Vec3Minimize(psMin, psMin, psPosition);
		Float32Vec3Maximize(psMax, psMax, psPosition);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Corners are shared between faces if they use the same normal.  Only each corner's latest vertex is checked: a corner
// whose faces alternate normals gets extra vertices but they look the same.
//
//////////////////////////////////////////////////////////////////////////
static void BuildRasterMeshFrom(CRasterMesh* pcRaster, CMesh* pcMesh, SFloat32Vec3* psCentre, int32 iScale)
{
	SFloat32Vec3*		psPosition;
	SFloat32Vec3*		psNormal;
	SFloat32Vec3		sOffset;
	CArrayInt			aiCornerVertex;
	CArrayInt			aiVertexNormal;
	int*				piCorners;
	SMeshNormalFace*	psNormalFace;
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	int					iNumCorners;
	int					iNumFaces;
	int					i;
	int					k;
	int					iCorner;
	int					iNormal;
	int					iVertex;
	int					aiVertex[3];
	uint32				uiColour;
	bool				bNormals;

	iNumCorners = pcMesh->NumCorners();
	iNumFaces = pcMesh->NumFaces();
	bNormals = pcMesh->mcNormals.InUse();

	aiCornerVertex.Init();
	aiVertexNormal.Init();
	aiCornerVertex.Resize(iNumCorners);
	aiCornerVertex.SetArrayValues(-1);

	for (i = 0; i < iNumFaces; i++)
	{
		piCorners = pcMesh->GetFaceCorners(i);
		psNormalFace = bNormals ? pcMesh->mcNormals.mcFaces.Get(i) : NULL;

		for (k = 0; k < 3; k++)
		{
			iCorner = piCorners[k];

			iNormal = -1;
			if (bNormals)
			{
				iNormal = psNormalFace->aiCornerNormals[k];
				if (iNormal < 0)
				{
					iNormal = psNormalFace->iFaceNormal;
				}
			}

			iVertex = aiCornerVertex.GetValue(iCorner);
			if ((iVertex < 0) || (aiVertexNormal.GetValue(iVertex) != iNormal))
			{
				iVertex = (int)pcRaster->masVertices.NumElements();
				aiCornerVertex.SetValue(iCorner, iVertex);
				aiVertexNormal.Add(iNormal);

				psPosition = pcMesh->GetPosition(iCorner);
				sOffset.Init(psPosition->x - psCentre->x, psPosition->y - psCentre->y, psPosition->z - psCentre->z);

				psVertex = pcRaster->masVertices.Add();
				memset(psVertex, 0, sizeof(SRasterVertex));  //Its padding is saved with it, so WriteFile writes the same bytes each time.
				psVertex->sPosition.Init(FloatToIntScaled(sOffset.x, iScale), FloatToIntScaled(sOffset.y, iScale), FloatToIntScaled(sOffset.z, iScale));
				psVertex->sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0));
				if (iNormal >= 0)
				{
					psNormal = pcMesh->mcNormals.mcNormals.Get(iNormal);
					psVertex->sNormal.Init(SRasterUnit::FromFloat(psNormal->x), SRasterUnit::FromFloat(psNormal->y), SRasterUnit::FromFloat(psNormal->z));
				}
			}
			aiVertex[k] = iVertex;
		}

		uiColour = RasterColour(pcMesh, i);
		psTriangle = ((uiColour >> 24) == 0xFF) ? pcRaster->masOpaque.Add() : pcRaster->masTransparent.Add();
		psTriangle->aiVertex[0] = aiVertex[0];
		psTriangle->aiVertex[1] = aiVertex[1];
		psTriangle->aiVertex[2] = aiVertex[2];
		psTriangle->uiColour = uiColour;
	}

	aiVertexNormal.Kill();
	aiCornerVertex.Kill();

	pcRaster->RemoveDegenerate();
	pcRaster->UpdateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// Centred on the origin, and scaled so that its largest half width is
// RASTER_MESH_HALF_EXTENT.
//
//////////////////////////////////////////////////////////////////////////
void BuildRasterMesh(CRasterMesh* pcRaster, CMesh* pcMesh)
{
	SFloat32Vec3	sMin;
	SFloat32Vec3	sMax;
	SFloat32Vec3	sCentre;
	float32			fHalfExtent;
	int32			iScale;

	if (!FindRasterBounds(pcRaster, pcMesh, &sMin, &sMax))
	{
		return;
	}
	sCentre.Init((sMin.x + sMax.x) * 0.5f, (sMin.y + sMax.y) * 0.5f, (sMin.z + sMax.z) * 0.5f);

	fHalfExtent = (sMax.x - sMin.x) * 0.5f;
	if ((sMax.y - sMin.y) * 0.5f > fHalfExtent)
	{
		fHalfExtent = (sMax.y - sMin.y) * 0.5f;
	}
	if ((sMax.z - sMin.z) * 0.5f > fHalfExtent)
	{
		fHalfExtent = (sMax.z - sMin.z) * 0.5f;
	}
	iScale = (fHalfExtent > 0.0f) ? (int32)floor((float64)RASTER_MESH_HALF_EXTENT / (float64)fHalfExtent) : 1;
	BuildRasterMeshFrom(pcRaster, pcMesh, &sCentre, iScale);
}


//////////////////////////////////////////////////////////////////////////
//
// Where it is, with iScale units to each of the mesh's.
//
//////////////////////////////////////////////////////////////////////////
void BuildRasterMesh(CRasterMesh* pcRaster, CMesh* pcMesh, int32 iScale)
{
	SFloat32Vec3	sMin;
	SFloat32Vec3	sMax;
	SFloat32Vec3	sCentre;

	if (FindRasterBounds(pcRaster, pcMesh, &sMin, &sMax))
	{
		sCentre.Init(0.0f, 0.0f, 0.0f);
		BuildRasterMeshFrom(pcRaster, pcMesh, &sCentre, iScale);
	}
}

