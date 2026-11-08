/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RasterLib

Codaphela RasterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RasterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RasterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <math.h>
#include "BaseLib/ArrayInt.h"
#include "BaseLib/FixedPoint.h"
#include "BaseLib/GeometricTypes.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "RasterMesh.h"


//VRML's default material for a face with no colour.
#define RASTER_DEFAULT_COLOUR	0xFFCCCCCC


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Init(void)
{
	masVertices.Init();
	masOpaque.Init();
	masTransparent.Init();
	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Kill(void)
{
	masTransparent.Kill();
	masOpaque.Kill();
	masVertices.Kill();
}


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
// Centred on the origin, and scaled so that its largest half width is
// RASTER_MESH_HALF_EXTENT.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Build(CMesh* pcMesh)
{
	SFloat32Vec3	sMin;
	SFloat32Vec3	sMax;
	SFloat32Vec3	sCentre;
	float32			fHalfExtent;
	int32			iScale;

	if (!FindBounds(pcMesh, &sMin, &sMax))
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
	BuildFrom(pcMesh, &sCentre, iScale);
}


//////////////////////////////////////////////////////////////////////////
//
// Where it is, with iScale units to each of the mesh's.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Build(CMesh* pcMesh, int32 iScale)
{
	SFloat32Vec3	sMin;
	SFloat32Vec3	sMax;
	SFloat32Vec3	sCentre;

	if (FindBounds(pcMesh, &sMin, &sMax))
	{
		sCentre.Init(0.0f, 0.0f, 0.0f);
		BuildFrom(pcMesh, &sCentre, iScale);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Empties this mesh.  False if pcMesh has nothing to build.
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::FindBounds(CMesh* pcMesh, SFloat32Vec3* psMin, SFloat32Vec3* psMax)
{
	SFloat32Vec3*	psPosition;
	int				iNumCorners;
	int				i;

	masVertices.ReInit();
	masOpaque.ReInit();
	masTransparent.ReInit();
	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);

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
void CRasterMesh::BuildFrom(CMesh* pcMesh, SFloat32Vec3* psCentre, int32 iScale)
{
	SFloat32Vec3*		psPosition;
	SFloat32Vec3*		psNormal;
	SFloat32Vec3		sOffset;
	CArrayInt			aiCornerVertex;
	CArrayInt			aiVertexNormal;
	CMeshFace*			pcFace;
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
		pcFace = pcMesh->GetFace(i);
		psNormalFace = bNormals ? pcMesh->mcNormals.mcFaces.Get(i) : NULL;

		for (k = 0; k < 3; k++)
		{
			iCorner = pcFace->maiCorner[k];

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
				iVertex = (int)masVertices.NumElements();
				aiCornerVertex.SetValue(iCorner, iVertex);
				aiVertexNormal.Add(iNormal);

				psPosition = pcMesh->GetPosition(iCorner);
				sOffset.Init(psPosition->x - psCentre->x, psPosition->y - psCentre->y, psPosition->z - psCentre->z);

				psVertex = masVertices.Add();
				psVertex->sPosition.Init(FloatToIntScaled(sOffset.x, iScale), FloatToIntScaled(sOffset.y, iScale), FloatToIntScaled(sOffset.z, iScale));
				psVertex->sNormal.Init(0, 0, 0);
				if (iNormal >= 0)
				{
					psNormal = pcMesh->mcNormals.mcNormals.Get(iNormal);
					psVertex->sNormal.Init((int16)FixedFromFloat(psNormal->x, RASTER_NORMAL_BITS), (int16)FixedFromFloat(psNormal->y, RASTER_NORMAL_BITS), (int16)FixedFromFloat(psNormal->z, RASTER_NORMAL_BITS));
				}
			}
			aiVertex[k] = iVertex;
		}

		uiColour = RasterColour(pcMesh, i);
		psTriangle = ((uiColour >> 24) == 0xFF) ? masOpaque.Add() : masTransparent.Add();
		psTriangle->aiVertex[0] = aiVertex[0];
		psTriangle->aiVertex[1] = aiVertex[1];
		psTriangle->aiVertex[2] = aiVertex[2];
		psTriangle->uiColour = uiColour;
	}

	aiVertexNormal.Kill();
	aiCornerVertex.Kill();

	UpdateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::UpdateBounds(void)
{
	size	i;

	msMin.Init(0, 0, 0);
	msMax.Init(0, 0, 0);
	if (masVertices.NumElements() == 0)
	{
		return;
	}
	msMin = masVertices.Get(0)->sPosition;
	msMax = msMin;
	for (i = 1; i < masVertices.NumElements(); i++)
	{
		Int32Vec3Minimize(&msMin, &msMin, &masVertices.Get(i)->sPosition);
		Int32Vec3Maximize(&msMax, &msMax, &masVertices.Get(i)->sPosition);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CRasterMesh::NumTriangles(void)
{
	return masOpaque.NumElements() + masTransparent.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// The triangle's normal, not made a unit, by the right hand rule.
//
//////////////////////////////////////////////////////////////////////////
static void RasterTriangleNormal(CRasterMesh* pcMesh, SRasterTriangle* psTriangle, int64* piX, int64* piY, int64* piZ)
{
	SInt32Vec3*	psA;
	SInt32Vec3*	psB;
	SInt32Vec3*	psC;
	int64		iBX;
	int64		iBY;
	int64		iBZ;
	int64		iCX;
	int64		iCY;
	int64		iCZ;

	psA = &pcMesh->masVertices.Get(psTriangle->aiVertex[0])->sPosition;
	psB = &pcMesh->masVertices.Get(psTriangle->aiVertex[1])->sPosition;
	psC = &pcMesh->masVertices.Get(psTriangle->aiVertex[2])->sPosition;
	iBX = (int64)psB->x - psA->x;
	iBY = (int64)psB->y - psA->y;
	iBZ = (int64)psB->z - psA->z;
	iCX = (int64)psC->x - psA->x;
	iCY = (int64)psC->y - psA->y;
	iCZ = (int64)psC->z - psA->z;
	*piX = iBY * iCZ - iBZ * iCY;
	*piY = iBZ * iCX - iBX * iCZ;
	*piZ = iBX * iCY - iBY * iCX;
}


//////////////////////////////////////////////////////////////////////////
//
// Keeps the triangles with some area.  Returns how many went.
//
//////////////////////////////////////////////////////////////////////////
static size RasterRemoveDegenerate(CRasterMesh* pcMesh, CArrayRasterTriangle* pasTriangles)
{
	SRasterTriangle*	psTriangle;
	size				uiNumTriangles;
	size				uiKept;
	size				i;
	int64				iX;
	int64				iY;
	int64				iZ;

	uiNumTriangles = pasTriangles->NumElements();
	psTriangle = pasTriangles->GetData();
	uiKept = 0;
	for (i = 0; i < uiNumTriangles; i++)
	{
		RasterTriangleNormal(pcMesh, &psTriangle[i], &iX, &iY, &iZ);
		if ((iX != 0) || (iY != 0) || (iZ != 0))
		{
			psTriangle[uiKept] = psTriangle[i];
			uiKept++;
		}
	}
	if (uiKept < uiNumTriangles)
	{
		pasTriangles->RemoveRange(uiKept, uiNumTriangles);
	}
	return uiNumTriangles - uiKept;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns how many triangles went.
//
//////////////////////////////////////////////////////////////////////////
size CRasterMesh::RemoveDegenerate(void)
{
	size	uiRemoved;

	uiRemoved = RasterRemoveDegenerate(this, &masOpaque);
	uiRemoved += RasterRemoveDegenerate(this, &masTransparent);
	return uiRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// A triangle is reversed if its corners' normals, added up, point away from
// the way it winds.  One whose corners have no normals can't be.  The sizes
// here would overflow 64 bits, so it is done in floats: a mesh is only built
// once.
//
//////////////////////////////////////////////////////////////////////////
static size RasterFixReversed(CRasterMesh* pcMesh, CArrayRasterTriangle* pasTriangles)
{
	SRasterTriangle*	psTriangle;
	SInt16Vec3*			psNormal;
	size				uiNumTriangles;
	size				uiFixed;
	size				i;
	int64				iX;
	int64				iY;
	int64				iZ;
	int32				iNormalX;
	int32				iNormalY;
	int32				iNormalZ;
	int32				iSwap;
	int					k;

	uiNumTriangles = pasTriangles->NumElements();
	psTriangle = pasTriangles->GetData();
	uiFixed = 0;
	for (i = 0; i < uiNumTriangles; i++)
	{
		iNormalX = 0;
		iNormalY = 0;
		iNormalZ = 0;
		for (k = 0; k < 3; k++)
		{
			psNormal = &pcMesh->masVertices.Get(psTriangle[i].aiVertex[k])->sNormal;
			iNormalX += psNormal->x;
			iNormalY += psNormal->y;
			iNormalZ += psNormal->z;
		}

		RasterTriangleNormal(pcMesh, &psTriangle[i], &iX, &iY, &iZ);
		if ((float64)iX * iNormalX + (float64)iY * iNormalY + (float64)iZ * iNormalZ < 0.0)
		{
			iSwap = psTriangle[i].aiVertex[1];
			psTriangle[i].aiVertex[1] = psTriangle[i].aiVertex[2];
			psTriangle[i].aiVertex[2] = iSwap;
			uiFixed++;
		}
	}
	return uiFixed;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns how many triangles were turned round.
//
//////////////////////////////////////////////////////////////////////////
size CRasterMesh::FixReversed(void)
{
	size	uiFixed;

	uiFixed = RasterFixReversed(this, &masOpaque);
	uiFixed += RasterFixReversed(this, &masTransparent);
	return uiFixed;
}
