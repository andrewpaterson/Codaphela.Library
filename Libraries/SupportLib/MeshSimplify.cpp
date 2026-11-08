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
#include <stdlib.h>
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/Numbers.h"
#include "StandardLib/Objects.h"
#include "MeshSimplify.h"


//A normal's axes are each split into this many divisions, and normals in the same ones merge.
#define MESH_SIMPLIFY_NORMAL_DIVISIONS	5

//How a cell is found in a 64 bit key: this many bits for each axis and the rest for its colour.
#define MESH_SIMPLIFY_AXIS_BITS			18
#define MESH_SIMPLIFY_MAX_AXIS_CELLS	((int32)1 << MESH_SIMPLIFY_AXIS_BITS)

//Colours that keep to themselves: as many as fit in a cell's key.  Any more share the last.
#define MESH_SIMPLIFY_MAX_COLOURS		1024


struct SMeshSimplifyCell
{
	float64	fX;
	float64	fY;
	float64	fZ;
	int		iCount;
	int		iOutputCorner;
};


struct SMeshSimplifyNormal
{
	float64	fX;
	float64	fY;
	float64	fZ;
	int		iOutputNormal;
};


//A face as it will be compared.  Its corners start from the smallest so it is the same whichever it started from.
struct SMeshSimplifyFace
{
	int		aiCell[NUM_FACE_CORNERS];
	uint32	uiColour;
	int		iFace;
};


typedef CArrayTemplate<SMeshSimplifyCell>	CArrayMeshSimplifyCell;
typedef CArrayTemplate<SMeshSimplifyNormal>	CArrayMeshSimplifyNormal;
typedef CArrayTemplate<SMeshSimplifyFace>	CArrayMeshSimplifyFace;
typedef CMapLongTemplate<int>				CMapMeshSimplifyKey;


//What's known about a mesh as it is simplified.
struct SMeshSimplify
{
	CMesh*						pcMesh;
	bool						bNormals;
	bool						bColours;

	SFloat32Vec3				sMin;
	float64						fCellSize;

	CMapMeshSimplifyKey			cColours;
	CMapMeshSimplifyKey			cCells;
	CMapMeshSimplifyKey			cNormals;

	CArrayMeshSimplifyCell		asCells;
	CArrayMeshSimplifyNormal	asNormals;
	CArrayInt					aiCornerCells;	//The cell of each of the mesh's corners, -1 if it isn't used.
	CArrayInt					aiFaceNormals;	//The merged normal of each corner of each face.
	CArrayInt					aiKeptFaces;	//Whether each face stays.
	int							iNumKept;
};


//////////////////////////////////////////////////////////////////////////
//
// The value of iKey, which is iNew if it wasn't in the map.
//
//////////////////////////////////////////////////////////////////////////
static int MeshSimplifyFind(CMapMeshSimplifyKey* pcMap, int64 iKey, int iNew)
{
	int*	piValue;

	piValue = pcMap->Get(iKey);
	if (piValue != NULL)
	{
		return *piValue;
	}
	piValue = pcMap->Put(iKey);
	*piValue = iNew;
	return iNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshSimplifyInit(SMeshSimplify* ps, CMesh* pcMesh, float32 fCellSize)
{
	SFloat32Vec3*	psPosition;
	SFloat32Vec3	sMax;
	float32			fExtent;
	int				i;
	int				iNumCorners;

	ps->pcMesh = pcMesh;
	ps->bNormals = pcMesh->mcNormals.InUse();
	ps->bColours = pcMesh->mcColours.InUse();

	ps->cColours.Init(false);
	ps->cCells.Init(false);
	ps->cNormals.Init(false);
	ps->asCells.Init();
	ps->asNormals.Init();
	ps->aiCornerCells.Init();
	ps->aiFaceNormals.Init();
	ps->aiKeptFaces.Init();
	ps->iNumKept = 0;

	iNumCorners = pcMesh->NumCorners();
	ps->aiCornerCells.Resize(iNumCorners);
	ps->aiCornerCells.SetArrayValues(-1);

	ps->sMin = *pcMesh->GetPosition(0);
	sMax = ps->sMin;
	for (i = 1; i < iNumCorners; i++)
	{
		psPosition = pcMesh->GetPosition(i);
		ps->sMin.x = (psPosition->x < ps->sMin.x) ? psPosition->x : ps->sMin.x;
		ps->sMin.y = (psPosition->y < ps->sMin.y) ? psPosition->y : ps->sMin.y;
		ps->sMin.z = (psPosition->z < ps->sMin.z) ? psPosition->z : ps->sMin.z;
		sMax.x = (psPosition->x > sMax.x) ? psPosition->x : sMax.x;
		sMax.y = (psPosition->y > sMax.y) ? psPosition->y : sMax.y;
		sMax.z = (psPosition->z > sMax.z) ? psPosition->z : sMax.z;
	}

	//A mesh too big for its cells to fit in a key gets bigger ones, rather than far apart corners merging.
	fExtent = sMax.x - ps->sMin.x;
	fExtent = (sMax.y - ps->sMin.y > fExtent) ? sMax.y - ps->sMin.y : fExtent;
	fExtent = (sMax.z - ps->sMin.z > fExtent) ? sMax.z - ps->sMin.z : fExtent;
	ps->fCellSize = fCellSize;
	if ((float64)fExtent / ps->fCellSize >= MESH_SIMPLIFY_MAX_AXIS_CELLS)
	{
		ps->fCellSize = (float64)fExtent / (MESH_SIMPLIFY_MAX_AXIS_CELLS - 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshSimplifyKill(SMeshSimplify* ps)
{
	ps->aiKeptFaces.Kill();
	ps->aiFaceNormals.Kill();
	ps->aiCornerCells.Kill();
	ps->asNormals.Kill();
	ps->asCells.Kill();
	ps->cNormals.Kill();
	ps->cCells.Kill();
	ps->cColours.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A face with no colour isn't the colour of any value of uiColour.
//
//////////////////////////////////////////////////////////////////////////
static int64 MeshSimplifyFaceColour(SMeshSimplify* ps, int iFace)
{
	int	iColour;

	if (!ps->bColours)
	{
		return -1;
	}
	iColour = ps->pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours[0];
	if (iColour < 0)
	{
		return -1;
	}
	return (int64)(uint32)ps->pcMesh->mcColours.mcColours.GetValue(iColour);
}


//////////////////////////////////////////////////////////////////////////
//
// The cell of a corner, which takes the colour of the first face to use it:
// a shape's faces have one colour, and don't share corners with another's.
//
//////////////////////////////////////////////////////////////////////////
static int MeshSimplifyCornerCell(SMeshSimplify* ps, int iCorner, int iFace)
{
	SFloat32Vec3*		psPosition;
	SMeshSimplifyCell*	psCell;
	int*				piCell;
	int					iColour;
	int					iCell;
	int					iNumColours;
	int					iNumCells;
	uint64				uiKey;

	piCell = ps->aiCornerCells.Get(iCorner);
	if (*piCell >= 0)
	{
		return *piCell;
	}

	iNumColours = (int)ps->cColours.NumElements();
	iColour = (iNumColours < MESH_SIMPLIFY_MAX_COLOURS - 1) ? iNumColours : MESH_SIMPLIFY_MAX_COLOURS - 1;
	iColour = MeshSimplifyFind(&ps->cColours, MeshSimplifyFaceColour(ps, iFace), iColour);

	psPosition = ps->pcMesh->GetPosition(iCorner);
	uiKey = (uint64)(((float64)psPosition->x - ps->sMin.x) / ps->fCellSize);
	uiKey |= (uint64)(((float64)psPosition->y - ps->sMin.y) / ps->fCellSize) << MESH_SIMPLIFY_AXIS_BITS;
	uiKey |= (uint64)(((float64)psPosition->z - ps->sMin.z) / ps->fCellSize) << (MESH_SIMPLIFY_AXIS_BITS * 2);
	uiKey |= (uint64)iColour << (MESH_SIMPLIFY_AXIS_BITS * 3);

	iNumCells = (int)ps->asCells.NumElements();
	iCell = MeshSimplifyFind(&ps->cCells, (int64)uiKey, iNumCells);
	if (iCell == iNumCells)
	{
		psCell = ps->asCells.Add();
		psCell->fX = 0.0;
		psCell->fY = 0.0;
		psCell->fZ = 0.0;
		psCell->iCount = 0;
		psCell->iOutputCorner = -1;
	}

	psCell = ps->asCells.Get(iCell);
	psCell->fX += psPosition->x;
	psCell->fY += psPosition->y;
	psCell->fZ += psPosition->z;
	psCell->iCount++;

	*piCell = iCell;
	return iCell;
}


//////////////////////////////////////////////////////////////////////////
//
// Which division a part of a unit normal is in.
//
//////////////////////////////////////////////////////////////////////////
static uint64 MeshSimplifyNormalDivision(float32 fPart)
{
	int	iDivision;

	iDivision = (int)floorf((fPart + 1.0f) * MESH_SIMPLIFY_NORMAL_DIVISIONS * 0.5f);
	if (iDivision < 0)
	{
		return 0;
	}
	return (iDivision >= MESH_SIMPLIFY_NORMAL_DIVISIONS) ? MESH_SIMPLIFY_NORMAL_DIVISIONS - 1 : (uint64)iDivision;
}


//////////////////////////////////////////////////////////////////////////
//
// The merged normal of one corner of a face.
//
//////////////////////////////////////////////////////////////////////////
static int MeshSimplifyCornerNormal(SMeshSimplify* ps, int iCell, int iFace, int iFaceCorner)
{
	SMeshNormalFace*		psNormalFace;
	SFloat32Vec3*			psNormal;
	SMeshSimplifyNormal*	psMerged;
	int						iNormal;
	int						iNumNormals;
	int						iMerged;
	uint64					uiKey;

	psNormalFace = ps->pcMesh->mcNormals.mcFaces.Get(iFace);
	iNormal = psNormalFace->aiCornerNormals[iFaceCorner];
	if (iNormal < 0)
	{
		iNormal = psNormalFace->iFaceNormal;
	}
	psNormal = ps->pcMesh->mcNormals.mcNormals.Get(iNormal);

	uiKey = (uint64)(uint32)iCell << 12;
	uiKey |= MeshSimplifyNormalDivision(psNormal->x);
	uiKey |= MeshSimplifyNormalDivision(psNormal->y) << 4;
	uiKey |= MeshSimplifyNormalDivision(psNormal->z) << 8;

	iNumNormals = (int)ps->asNormals.NumElements();
	iMerged = MeshSimplifyFind(&ps->cNormals, (int64)uiKey, iNumNormals);
	if (iMerged == iNumNormals)
	{
		psMerged = ps->asNormals.Add();
		psMerged->fX = 0.0;
		psMerged->fY = 0.0;
		psMerged->fZ = 0.0;
		psMerged->iOutputNormal = -1;
	}

	psMerged = ps->asNormals.Get(iMerged);
	psMerged->fX += psNormal->x;
	psMerged->fY += psNormal->y;
	psMerged->fZ += psNormal->z;
	return iMerged;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshSimplifyMerge(SMeshSimplify* ps)
{
	int*	piCorners;
	int		iNumFaces;
	int		iFace;
	int		i;
	int		iCell;

	iNumFaces = ps->pcMesh->NumFaces();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = ps->pcMesh->GetFaceCorners(iFace);
		for (i = 0; i < NUM_FACE_CORNERS; i++)
		{
			iCell = MeshSimplifyCornerCell(ps, piCorners[i], iFace);
			if (ps->bNormals)
			{
				ps->aiFaceNormals.Add(MeshSimplifyCornerNormal(ps, iCell, iFace, i));
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshSimplifyCellPosition(SMeshSimplifyCell* psCell, float64* pfX, float64* pfY, float64* pfZ)
{
	*pfX = psCell->fX / psCell->iCount;
	*pfY = psCell->fY / psCell->iCount;
	*pfZ = psCell->fZ / psCell->iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool MeshSimplifyHasArea(SMeshSimplify* ps, int* piCells)
{
	float64	fAX;
	float64	fAY;
	float64	fAZ;
	float64	fBX;
	float64	fBY;
	float64	fBZ;
	float64	fCX;
	float64	fCY;
	float64	fCZ;

	if ((piCells[0] == piCells[1]) || (piCells[1] == piCells[2]) || (piCells[2] == piCells[0]))
	{
		return false;
	}

	MeshSimplifyCellPosition(ps->asCells.Get(piCells[0]), &fAX, &fAY, &fAZ);
	MeshSimplifyCellPosition(ps->asCells.Get(piCells[1]), &fBX, &fBY, &fBZ);
	MeshSimplifyCellPosition(ps->asCells.Get(piCells[2]), &fCX, &fCY, &fCZ);
	fBX -= fAX;
	fBY -= fAY;
	fBZ -= fAZ;
	fCX -= fAX;
	fCY -= fAY;
	fCZ -= fAZ;
	return (fBY * fCZ != fBZ * fCY) || (fBZ * fCX != fBX * fCZ) || (fBX * fCY != fBY * fCX);
}


//////////////////////////////////////////////////////////////////////////
//
// Faces the same come together, the first of them first.
//
//////////////////////////////////////////////////////////////////////////
static int MeshSimplifyCompareFaces(const void* pv1, const void* pv2)
{
	SMeshSimplifyFace*	ps1;
	SMeshSimplifyFace*	ps2;
	int					i;

	ps1 = (SMeshSimplifyFace*)pv1;
	ps2 = (SMeshSimplifyFace*)pv2;
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		if (ps1->aiCell[i] != ps2->aiCell[i])
		{
			return (ps1->aiCell[i] < ps2->aiCell[i]) ? -1 : 1;
		}
	}
	if (ps1->uiColour != ps2->uiColour)
	{
		return (ps1->uiColour < ps2->uiColour) ? -1 : 1;
	}
	if (ps1->iFace != ps2->iFace)
	{
		return (ps1->iFace < ps2->iFace) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Faces with no area go, and so do repeats of a face, whichever corner they
// start from.  One wound the other way faces the other way, so it stays.
//
//////////////////////////////////////////////////////////////////////////
static void MeshSimplifyChooseFaces(SMeshSimplify* ps)
{
	CArrayMeshSimplifyFace	asFaces;
	SMeshSimplifyFace*		psFace;
	SMeshSimplifyFace*		psLast;
	int*					piCorners;
	int						aiCells[NUM_FACE_CORNERS];
	int						iNumFaces;
	int						iFace;
	int						iStart;
	int						i;
	size					uiNumElements;
	size					u;

	asFaces.Init();
	iNumFaces = ps->pcMesh->NumFaces();
	ps->aiKeptFaces.Resize(iNumFaces);
	ps->aiKeptFaces.SetArrayValues(0);

	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = ps->pcMesh->GetFaceCorners(iFace);
		for (i = 0; i < NUM_FACE_CORNERS; i++)
		{
			aiCells[i] = ps->aiCornerCells.GetValue(piCorners[i]);
		}
		if (MeshSimplifyHasArea(ps, aiCells))
		{
			iStart = 0;
			iStart = (aiCells[1] < aiCells[iStart]) ? 1 : iStart;
			iStart = (aiCells[2] < aiCells[iStart]) ? 2 : iStart;

			psFace = asFaces.Add();
			for (i = 0; i < NUM_FACE_CORNERS; i++)
			{
				psFace->aiCell[i] = aiCells[(iStart + i) % NUM_FACE_CORNERS];
			}
			psFace->uiColour = (uint32)MeshSimplifyFaceColour(ps, iFace);
			psFace->iFace = iFace;
		}
	}

	uiNumElements = asFaces.NumElements();
	if (uiNumElements > 0)
	{
		qsort(asFaces.GetData(), uiNumElements, sizeof(SMeshSimplifyFace), MeshSimplifyCompareFaces);
	}

	psLast = NULL;
	for (u = 0; u < uiNumElements; u++)
	{
		psFace = asFaces.Get(u);
		if ((psLast == NULL) || (psFace->uiColour != psLast->uiColour) || (memcmp(psFace->aiCell, psLast->aiCell, sizeof(psFace->aiCell)) != 0))
		{
			ps->aiKeptFaces.SetValue(psFace->iFace, 1);
			ps->iNumKept++;
		}
		psLast = psFace;
	}

	asFaces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A new mesh of the faces that stay, and only the corners and normals they
// use.  Flat normals come first, so the corner normals are numbered after
// the faces that stay.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> MeshSimplifyBuild(SMeshSimplify* ps)
{
	Ptr<CMesh>				pOutput;
	CMesh*					pcMesh;
	SMeshSimplifyCell*		psCell;
	SMeshSimplifyNormal*	psNormal;
	SMeshNormalFace*		psNormalFace;
	CArrayInt				aiOutputNormals;
	float64					fX;
	float64					fY;
	float64					fZ;
	float64					fLength;
	int*					piCorners;
	int*					piColours;
	int						aiCorners[NUM_FACE_CORNERS];
	int						iNumFaces;
	int						iOutputFace;
	int						iFace;
	int						i;
	int						iMerged;
	size					u;

	pcMesh = ps->pcMesh;
	pOutput = OMalloc<CMesh>();
	pOutput->mcNormals.mbInUse = ps->bNormals;
	pOutput->mcColours.mbInUse = ps->bColours;
	for (u = 0; u < pcMesh->mcColours.mcColours.NumElements(); u++)
	{
		pOutput->mcColours.mcColours.Add(pcMesh->mcColours.mcColours.GetValue(u));
	}

	aiOutputNormals.Init();
	iNumFaces = pcMesh->NumFaces();
	iOutputFace = 0;
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		if (ps->aiKeptFaces.GetValue(iFace) != 0)
		{
			piCorners = pcMesh->GetFaceCorners(iFace);
			for (i = 0; i < NUM_FACE_CORNERS; i++)
			{
				psCell = ps->asCells.Get(ps->aiCornerCells.GetValue(piCorners[i]));
				if (psCell->iOutputCorner < 0)
				{
					MeshSimplifyCellPosition(psCell, &fX, &fY, &fZ);
					psCell->iOutputCorner = pOutput->NumCorners();
					pOutput->AddCorner((float32)fX, (float32)fY, (float32)fZ);
				}
				aiCorners[i] = psCell->iOutputCorner;
			}
			pOutput->AddFace(aiCorners[0], aiCorners[1], aiCorners[2]);
			pOutput->SetFaceName(iOutputFace, pcMesh->GetFaceName(iFace));

			if (ps->bNormals)
			{
				psNormalFace = pOutput->mcNormals.mcFaces.Get(iOutputFace);
				for (i = 0; i < NUM_FACE_CORNERS; i++)
				{
					iMerged = ps->aiFaceNormals.GetValue(iFace * NUM_FACE_CORNERS + i);
					psNormal = ps->asNormals.Get(iMerged);
					if (psNormal->iOutputNormal < 0)
					{
						psNormal->iOutputNormal = ps->iNumKept + (int)aiOutputNormals.NumElements();
						aiOutputNormals.Add(iMerged);
					}
					psNormalFace->aiCornerNormals[i] = psNormal->iOutputNormal;
				}
			}

			if (ps->bColours)
			{
				piColours = pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours;
				for (i = 0; i < NUM_FACE_CORNERS; i++)
				{
					pOutput->mcColours.mcFaces.Get(iOutputFace)->aiCornerColours[i] = piColours[i];
				}
			}
			iOutputFace++;
		}
	}

	for (u = 0; u < aiOutputNormals.NumElements(); u++)
	{
		psNormal = ps->asNormals.Get(aiOutputNormals.GetValue(u));
		fLength = sqrt(psNormal->fX * psNormal->fX + psNormal->fY * psNormal->fY + psNormal->fZ * psNormal->fZ);
		if (fLength > 0.0)
		{
			pOutput->AddNormal((float32)(psNormal->fX / fLength), (float32)(psNormal->fY / fLength), (float32)(psNormal->fZ / fLength));
		}
		else
		{
			pOutput->AddNormal(0.0f, 0.0f, 0.0f);
		}
	}
	aiOutputNormals.Kill();
	return pOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshSimplify::Init(float32 fCellSize)
{
	mfCellSize = fCellSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshSimplify::Modify(Ptr<CMesh> pMesh)
{
	SMeshSimplify	sSimplify;
	Ptr<CMesh>		pOutput;

	if ((pMesh->NumFaces() == 0) || (mfCellSize <= 0.0f) || !CanRebuild(&pMesh))
	{
		return pMesh;
	}

	MeshSimplifyInit(&sSimplify, &pMesh, mfCellSize);
	MeshSimplifyMerge(&sSimplify);
	MeshSimplifyChooseFaces(&sSimplify);
	pOutput = MeshSimplifyBuild(&sSimplify);
	MeshSimplifyKill(&sSimplify);
	return pOutput;
}
