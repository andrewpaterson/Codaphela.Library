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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/ArrayInt.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FixedPoint.h"
#include "BaseLib/GeometricTypes.h"
#include "SupportLib/ColourARGB32.h"
#include "SupportLib/Mesh.h"
#include "RasterMesh.h"


//VRML's default material for a face with no colour.
#define RASTER_DEFAULT_COLOUR	0xFFCCCCCC

#define RASTER_MESH_FILE_MAGIC		0x48534D52  //RMSH.
#define RASTER_MESH_FILE_VERSION	1

//Simplify keeps apart vertices in a cell whose normals are in different ones of these many divisions of each axis, so
//a sharp edge stays sharp and a curve is only faceted a little.
#define RASTER_NORMAL_DIVISIONS		5

//Colours that keep to themselves when simplifying: as many as fit in a cell's key.  Any more share the last.
#define RASTER_MAX_CELL_COLOURS		1024


struct SRasterMeshFileHeader
{
	uint32		uiMagic;
	uint32		uiVersion;
	uint64		uiSource;
	uint32		uiNumVertices;
	uint32		uiNumOpaque;
	uint32		uiNumTransparent;
	uint32		uiPad;
	SInt32Vec3	sMin;
	SInt32Vec3	sMax;
};


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
	SRasterUnitVec3*	psNormal;
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
			iNormalX += psNormal->x.miRaw;
			iNormalY += psNormal->y.miRaw;
			iNormalZ += psNormal->z.miRaw;
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


//A table of unique keys, each with a value.  A key is never all ones, which means an empty slot.
struct SRasterKeyTable
{
	uint64*		puiKeys;
	int32*		piValues;
	uint64		uiMask;
};


//////////////////////////////////////////////////////////////////////////
//
// With room for uiMaxKeys keys.
//
//////////////////////////////////////////////////////////////////////////
static void RasterKeyTableInit(SRasterKeyTable* psTable, size uiMaxKeys)
{
	uint64	uiSize;

	uiSize = 16;
	while (uiSize < (uint64)uiMaxKeys * 2)
	{
		uiSize *= 2;
	}
	psTable->puiKeys = (uint64*)malloc((size)uiSize * sizeof(uint64));
	psTable->piValues = (int32*)malloc((size)uiSize * sizeof(int32));
	psTable->uiMask = uiSize - 1;
	memset(psTable->puiKeys, 0xFF, (size)uiSize * sizeof(uint64));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void RasterKeyTableKill(SRasterKeyTable* psTable)
{
	free(psTable->piValues);
	free(psTable->puiKeys);
}


//////////////////////////////////////////////////////////////////////////
//
// The value of uiKey, which is iNew if it wasn't in the table.
//
//////////////////////////////////////////////////////////////////////////
static int32 RasterKeyTableFind(SRasterKeyTable* psTable, uint64 uiKey, int32 iNew)
{
	uint64	uiSlot;

	//A multiplicative hash, from the top bits, which the low bits of every key reach.
	uiSlot = ((uiKey * 0x9E3779B97F4A7C15ULL) >> 20) & psTable->uiMask;
	while (psTable->puiKeys[uiSlot] != 0xFFFFFFFFFFFFFFFFULL)
	{
		if (psTable->puiKeys[uiSlot] == uiKey)
		{
			return psTable->piValues[uiSlot];
		}
		uiSlot = (uiSlot + 1) & psTable->uiMask;
	}
	psTable->puiKeys[uiSlot] = uiKey;
	psTable->piValues[uiSlot] = iNew;
	return iNew;
}


//////////////////////////////////////////////////////////////////////////
//
// Which of RASTER_NORMAL_DIVISIONS a part of a normal is in.
//
//////////////////////////////////////////////////////////////////////////
static uint64 RasterNormalDivision(SRasterUnit sPart)
{
	int32	iDivision;

	iDivision = (((int32)sPart.miRaw + (1 << SRasterUnit::Bits)) * RASTER_NORMAL_DIVISIONS) >> (SRasterUnit::Bits + 1);
	if (iDivision < 0)
	{
		return 0;
	}
	return (iDivision >= RASTER_NORMAL_DIVISIONS) ? RASTER_NORMAL_DIVISIONS - 1 : (uint64)iDivision;
}


//////////////////////////////////////////////////////////////////////////
//
// Vertex clustering: space is cut into cells iCellSize wide and every vertex
// of a colour in a cell moves to the same place, the middle of them, so that
// the mesh has no new gaps however the triangles fall.  Triangles that are
// left with no area, or the same as another, go.  It doesn't care if the mesh
// is closed or even tidy, and takes one pass.  The shape only changes by up to
// a cell, so a cell should be about what a pixel will be.
//
// Colours keep to themselves as what's exported from a board has its copper,
// solder mask and silk screen a hair apart.  Merged together they would be in
// one plane and fight to be drawn.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::Simplify(int32 iCellSize)
{
	if (masVertices.NumElements() == 0)
	{
		return;
	}
	MergeCells(iCellSize);
	RemoveDegenerate();
	RemoveRepeatedTriangles(&masOpaque);
	RemoveRepeatedTriangles(&masTransparent);
	RemoveUnusedVertices();
	UpdateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// A vertex stays apart from one in its cell whose normal is too different,
// but at the same place.  Each merged vertex's normal is those of the
// vertices in it, added and made a unit.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::MergeCells(int32 iCellSize)
{
	SRasterKeyTable		sColours;
	SRasterKeyTable		sCells;
	SRasterKeyTable		sVertices;
	SRasterVertex*		psVertex;
	SRasterVertex*		psMerged;
	SRasterTriangle*	psTriangle;
	CArrayRasterTriangle*	pasTriangles;
	int64*				piCellSums;
	int32*				piVertexMap;
	int32*				piVertexCell;
	int32*				piVertexColour;
	int32*				piNormalSums;
	uint64				uiKey;
	size				uiNumVertices;
	size				uiNumTriangles;
	size				i;
	int32				iNumColours;
	int32				iColour;
	int32				iNumCells;
	int32				iNumMerged;
	int32				iCell;
	int32				iMerged;
	float64				fLength;
	int					k;
	int					iCorner;

	uiNumVertices = masVertices.NumElements();
	psVertex = masVertices.GetData();
	UpdateBounds();

	//Each vertex's colour is that of the first triangle using it: a shape's triangles have one colour, and don't share
	//vertices with another's.
	RasterKeyTableInit(&sColours, RASTER_MAX_CELL_COLOURS);
	piVertexColour = (int32*)calloc(uiNumVertices, sizeof(int32));
	iNumColours = 1;
	for (k = 0; k < 2; k++)
	{
		pasTriangles = (k == 0) ? &masOpaque : &masTransparent;
		uiNumTriangles = pasTriangles->NumElements();
		psTriangle = pasTriangles->GetData();
		for (i = 0; i < uiNumTriangles; i++)
		{
			iColour = RasterKeyTableFind(&sColours, psTriangle[i].uiColour, iNumColours);
			if ((iColour == iNumColours) && (iNumColours < RASTER_MAX_CELL_COLOURS - 1))
			{
				iNumColours++;
			}
			for (iCorner = 0; iCorner < 3; iCorner++)
			{
				if (piVertexColour[psTriangle[i].aiVertex[iCorner]] == 0)
				{
					piVertexColour[psTriangle[i].aiVertex[iCorner]] = iColour;
				}
			}
		}
	}

	RasterKeyTableInit(&sCells, uiNumVertices);
	RasterKeyTableInit(&sVertices, uiNumVertices);
	piCellSums = (int64*)calloc(uiNumVertices * 4, sizeof(int64));  //x, y, z and how many, by cell.
	piVertexMap = (int32*)malloc(uiNumVertices * sizeof(int32));
	piVertexCell = (int32*)malloc(uiNumVertices * sizeof(int32));
	piNormalSums = (int32*)calloc(uiNumVertices * 3, sizeof(int32));
	psMerged = (SRasterVertex*)malloc(uiNumVertices * sizeof(SRasterVertex));

	iNumCells = 0;
	iNumMerged = 0;
	for (i = 0; i < uiNumVertices; i++)
	{
		//18 bits for each axis and the rest for the colour.
		uiKey = (uint64)(((int64)psVertex[i].sPosition.x - msMin.x) / iCellSize);
		uiKey |= (uint64)(((int64)psVertex[i].sPosition.y - msMin.y) / iCellSize) << 18;
		uiKey |= (uint64)(((int64)psVertex[i].sPosition.z - msMin.z) / iCellSize) << 36;
		uiKey |= (uint64)piVertexColour[i] << 54;
		iCell = RasterKeyTableFind(&sCells, uiKey, iNumCells);
		if (iCell == iNumCells)
		{
			iNumCells++;
		}
		piCellSums[iCell * 4] += psVertex[i].sPosition.x;
		piCellSums[iCell * 4 + 1] += psVertex[i].sPosition.y;
		piCellSums[iCell * 4 + 2] += psVertex[i].sPosition.z;
		piCellSums[iCell * 4 + 3]++;

		uiKey = (uint64)(uint32)iCell << 16;
		uiKey |= RasterNormalDivision(psVertex[i].sNormal.x);
		uiKey |= RasterNormalDivision(psVertex[i].sNormal.y) << 4;
		uiKey |= RasterNormalDivision(psVertex[i].sNormal.z) << 8;
		iMerged = RasterKeyTableFind(&sVertices, uiKey, iNumMerged);
		if (iMerged == iNumMerged)
		{
			piVertexCell[iNumMerged] = iCell;
			iNumMerged++;
		}
		piNormalSums[iMerged * 3] += psVertex[i].sNormal.x.miRaw;
		piNormalSums[iMerged * 3 + 1] += psVertex[i].sNormal.y.miRaw;
		piNormalSums[iMerged * 3 + 2] += psVertex[i].sNormal.z.miRaw;
		piVertexMap[i] = iMerged;
	}

	for (i = 0; i < (size)iNumMerged; i++)
	{
		iCell = piVertexCell[i];
		psMerged[i].sPosition.Init((int32)(piCellSums[iCell * 4] / piCellSums[iCell * 4 + 3]), (int32)(piCellSums[iCell * 4 + 1] / piCellSums[iCell * 4 + 3]), (int32)(piCellSums[iCell * 4 + 2] / piCellSums[iCell * 4 + 3]));
		fLength = sqrt((float64)piNormalSums[i * 3] * piNormalSums[i * 3] + (float64)piNormalSums[i * 3 + 1] * piNormalSums[i * 3 + 1] + (float64)piNormalSums[i * 3 + 2] * piNormalSums[i * 3 + 2]);
		if (fLength > 0.0)
		{
			psMerged[i].sNormal.Init(SRasterUnit::FromRaw((int16)((float64)piNormalSums[i * 3] * (1 << SRasterUnit::Bits) / fLength)), SRasterUnit::FromRaw((int16)((float64)piNormalSums[i * 3 + 1] * (1 << SRasterUnit::Bits) / fLength)), SRasterUnit::FromRaw((int16)((float64)piNormalSums[i * 3 + 2] * (1 << SRasterUnit::Bits) / fLength)));
		}
		else
		{
			psMerged[i].sNormal.Init(SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0), SRasterUnit::FromRaw(0));
		}
	}

	masVertices.ReInit();
	masVertices.Resize(iNumMerged);
	memcpy(masVertices.GetData(), psMerged, (size)iNumMerged * sizeof(SRasterVertex));

	for (k = 0; k < 2; k++)
	{
		pasTriangles = (k == 0) ? &masOpaque : &masTransparent;
		uiNumTriangles = pasTriangles->NumElements();
		psTriangle = pasTriangles->GetData();
		for (i = 0; i < uiNumTriangles; i++)
		{
			psTriangle[i].aiVertex[0] = piVertexMap[psTriangle[i].aiVertex[0]];
			psTriangle[i].aiVertex[1] = piVertexMap[psTriangle[i].aiVertex[1]];
			psTriangle[i].aiVertex[2] = piVertexMap[psTriangle[i].aiVertex[2]];
		}
	}

	free(psMerged);
	free(piNormalSums);
	free(piVertexCell);
	free(piVertexMap);
	free(piCellSums);
	RasterKeyTableKill(&sVertices);
	RasterKeyTableKill(&sCells);
	free(piVertexColour);
	RasterKeyTableKill(&sColours);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int __cdecl RasterCompareTriangles(const void* pv1, const void* pv2)
{
	SRasterTriangle*	ps1;
	SRasterTriangle*	ps2;
	int					i;

	ps1 = (SRasterTriangle*)pv1;
	ps2 = (SRasterTriangle*)pv2;
	for (i = 0; i < 3; i++)
	{
		if (ps1->aiVertex[i] != ps2->aiVertex[i])
		{
			return (ps1->aiVertex[i] < ps2->aiVertex[i]) ? -1 : 1;
		}
	}
	if (ps1->uiColour != ps2->uiColour)
	{
		return (ps1->uiColour < ps2->uiColour) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Two the same, whichever corner they start from.  One wound the other way
// faces the other way, so it is a different triangle.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::RemoveRepeatedTriangles(CArrayRasterTriangle* pasTriangles)
{
	SRasterTriangle*	psTriangle;
	size				uiNumTriangles;
	size				uiKept;
	size				i;
	int32				iFirst;
	int32				iSecond;
	int32				iThird;

	uiNumTriangles = pasTriangles->NumElements();
	if (uiNumTriangles < 2)
	{
		return;
	}
	psTriangle = pasTriangles->GetData();
	for (i = 0; i < uiNumTriangles; i++)
	{
		iFirst = psTriangle[i].aiVertex[0];
		iSecond = psTriangle[i].aiVertex[1];
		iThird = psTriangle[i].aiVertex[2];
		if ((iSecond < iFirst) && (iSecond < iThird))
		{
			psTriangle[i].aiVertex[0] = iSecond;
			psTriangle[i].aiVertex[1] = iThird;
			psTriangle[i].aiVertex[2] = iFirst;
		}
		else if ((iThird < iFirst) && (iThird < iSecond))
		{
			psTriangle[i].aiVertex[0] = iThird;
			psTriangle[i].aiVertex[1] = iFirst;
			psTriangle[i].aiVertex[2] = iSecond;
		}
	}
	qsort(psTriangle, uiNumTriangles, sizeof(SRasterTriangle), RasterCompareTriangles);

	uiKept = 1;
	for (i = 1; i < uiNumTriangles; i++)
	{
		if (RasterCompareTriangles(&psTriangle[i], &psTriangle[uiKept - 1]) != 0)
		{
			psTriangle[uiKept] = psTriangle[i];
			uiKept++;
		}
	}
	if (uiKept < uiNumTriangles)
	{
		pasTriangles->RemoveRange(uiKept, uiNumTriangles);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Vertices no triangle uses go, and the rest keep their order.
//
//////////////////////////////////////////////////////////////////////////
void CRasterMesh::RemoveUnusedVertices(void)
{
	SRasterVertex*		psVertex;
	SRasterTriangle*	psTriangle;
	CArrayRasterTriangle*	pasTriangles;
	int32*				piMap;
	size				uiNumVertices;
	size				uiNumTriangles;
	size				uiKept;
	size				i;
	int					k;
	int					iCorner;

	uiNumVertices = masVertices.NumElements();
	piMap = (int32*)malloc(uiNumVertices * sizeof(int32));
	memset(piMap, 0xFF, uiNumVertices * sizeof(int32));
	for (k = 0; k < 2; k++)
	{
		pasTriangles = (k == 0) ? &masOpaque : &masTransparent;
		uiNumTriangles = pasTriangles->NumElements();
		psTriangle = pasTriangles->GetData();
		for (i = 0; i < uiNumTriangles; i++)
		{
			for (iCorner = 0; iCorner < 3; iCorner++)
			{
				piMap[psTriangle[i].aiVertex[iCorner]] = 0;
			}
		}
	}

	psVertex = masVertices.GetData();
	uiKept = 0;
	for (i = 0; i < uiNumVertices; i++)
	{
		if (piMap[i] == 0)
		{
			piMap[i] = (int32)uiKept;
			psVertex[uiKept] = psVertex[i];
			uiKept++;
		}
	}
	if (uiKept < uiNumVertices)
	{
		masVertices.RemoveRange(uiKept, uiNumVertices);
	}

	for (k = 0; k < 2; k++)
	{
		pasTriangles = (k == 0) ? &masOpaque : &masTransparent;
		uiNumTriangles = pasTriangles->NumElements();
		psTriangle = pasTriangles->GetData();
		for (i = 0; i < uiNumTriangles; i++)
		{
			for (iCorner = 0; iCorner < 3; iCorner++)
			{
				psTriangle[i].aiVertex[iCorner] = piMap[psTriangle[i].aiVertex[iCorner]];
			}
		}
	}
	free(piMap);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::Save(const char* szFilename, uint64 uiSource)
{
	CDiskFile				cFile;
	SRasterMeshFileHeader	sHeader;
	bool					bResult;

	memset(&sHeader, 0, sizeof(sHeader));
	sHeader.uiMagic = RASTER_MESH_FILE_MAGIC;
	sHeader.uiVersion = RASTER_MESH_FILE_VERSION;
	sHeader.uiSource = uiSource;
	sHeader.uiNumVertices = (uint32)masVertices.NumElements();
	sHeader.uiNumOpaque = (uint32)masOpaque.NumElements();
	sHeader.uiNumTransparent = (uint32)masTransparent.NumElements();
	sHeader.sMin = msMin;
	sHeader.sMax = msMax;

	cFile.Init(szFilename);
	if (!cFile.Open(EFM_Write_Replace))
	{
		cFile.Kill();
		return false;
	}
	bResult = cFile.Write(&sHeader, sizeof(sHeader), 1) == 1;
	bResult = bResult && ((sHeader.uiNumVertices == 0) || (cFile.Write(masVertices.GetData(), sizeof(SRasterVertex), sHeader.uiNumVertices) == sHeader.uiNumVertices));
	bResult = bResult && ((sHeader.uiNumOpaque == 0) || (cFile.Write(masOpaque.GetData(), sizeof(SRasterTriangle), sHeader.uiNumOpaque) == sHeader.uiNumOpaque));
	bResult = bResult && ((sHeader.uiNumTransparent == 0) || (cFile.Write(masTransparent.GetData(), sizeof(SRasterTriangle), sHeader.uiNumTransparent) == sHeader.uiNumTransparent));
	cFile.Close();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// False, leaving the mesh as it was, if the file isn't there, isn't this
// version or wasn't built from uiSource.
//
//////////////////////////////////////////////////////////////////////////
bool CRasterMesh::Load(const char* szFilename, uint64 uiSource)
{
	CDiskFile				cFile;
	SRasterMeshFileHeader	sHeader;
	bool					bResult;

	cFile.Init(szFilename);
	if (!cFile.Open(EFM_Read))
	{
		cFile.Kill();
		return false;
	}
	bResult = cFile.Read(&sHeader, sizeof(sHeader), 1) == 1;
	bResult = bResult && (sHeader.uiMagic == RASTER_MESH_FILE_MAGIC) && (sHeader.uiVersion == RASTER_MESH_FILE_VERSION) && (sHeader.uiSource == uiSource);
	if (bResult)
	{
		masVertices.ReInit();
		masOpaque.ReInit();
		masTransparent.ReInit();
		masVertices.Resize(sHeader.uiNumVertices);
		masOpaque.Resize(sHeader.uiNumOpaque);
		masTransparent.Resize(sHeader.uiNumTransparent);
		bResult = (sHeader.uiNumVertices == 0) || (cFile.Read(masVertices.GetData(), sizeof(SRasterVertex), sHeader.uiNumVertices) == sHeader.uiNumVertices);
		bResult = bResult && ((sHeader.uiNumOpaque == 0) || (cFile.Read(masOpaque.GetData(), sizeof(SRasterTriangle), sHeader.uiNumOpaque) == sHeader.uiNumOpaque));
		bResult = bResult && ((sHeader.uiNumTransparent == 0) || (cFile.Read(masTransparent.GetData(), sizeof(SRasterTriangle), sHeader.uiNumTransparent) == sHeader.uiNumTransparent));
		msMin = sHeader.sMin;
		msMax = sHeader.sMax;
	}
	cFile.Close();
	cFile.Kill();
	return bResult;
}
