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
#include "BaseLib/Float32Vec3.h"
#include "MeshWeldCorners.h"


//A corner of a face as it will be compared.
struct SMeshWeldFaceCorner
{
	float32	fX;
	float32	fY;
	float32	fZ;
	int64	iColour;  //-1 if it has none.
	int		iFaceCorner;
};


typedef CArrayTemplate<SMeshWeldFaceCorner>	CArrayMeshWeldFaceCorner;


//////////////////////////////////////////////////////////////////////////
//
// Corners that weld come together, the first of them first.
//
//////////////////////////////////////////////////////////////////////////
static int MeshWeldCompare(SMeshWeldFaceCorner* ps1, SMeshWeldFaceCorner* ps2)
{
	if (ps1->fX != ps2->fX)
	{
		return (ps1->fX < ps2->fX) ? -1 : 1;
	}
	if (ps1->fY != ps2->fY)
	{
		return (ps1->fY < ps2->fY) ? -1 : 1;
	}
	if (ps1->fZ != ps2->fZ)
	{
		return (ps1->fZ < ps2->fZ) ? -1 : 1;
	}
	if (ps1->iColour != ps2->iColour)
	{
		return (ps1->iColour < ps2->iColour) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int __cdecl MeshWeldCompareFaceCorners(const void* pv1, const void* pv2)
{
	SMeshWeldFaceCorner*	ps1;
	SMeshWeldFaceCorner*	ps2;
	int						iResult;

	ps1 = (SMeshWeldFaceCorner*)pv1;
	ps2 = (SMeshWeldFaceCorner*)pv2;
	iResult = MeshWeldCompare(ps1, ps2);
	if (iResult != 0)
	{
		return iResult;
	}
	return (ps1->iFaceCorner < ps2->iFaceCorner) ? -1 : ((ps1->iFaceCorner > ps2->iFaceCorner) ? 1 : 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int64 MeshWeldColour(CMesh* pcMesh, int iFace, int iCorner)
{
	int	iColour;

	if (!pcMesh->mcColours.InUse())
	{
		return -1;
	}
	iColour = pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours[iCorner];
	if (iColour < 0)
	{
		return -1;
	}
	return (int64)(uint32)pcMesh->mcColours.mcColours.GetValue(iColour);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static void MeshWeldSortFaceCorners(CMesh* pcMesh, CArrayMeshWeldFaceCorner* pasFaceCorners)
{
	SMeshWeldFaceCorner*	psFaceCorner;
	SFloat32Vec3*			psPosition;
	int*					piCorners;
	int						iNumFaces;
	int						iFace;
	int						i;

	iNumFaces = pcMesh->NumFaces();
	pasFaceCorners->Resize(iNumFaces * NUM_FACE_CORNERS);
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pcMesh->GetFaceCorners(iFace);
		for (i = 0; i < NUM_FACE_CORNERS; i++)
		{
			psPosition = pcMesh->GetPosition(piCorners[i]);
			psFaceCorner = pasFaceCorners->Get(iFace * NUM_FACE_CORNERS + i);
			psFaceCorner->fX = psPosition->x;
			psFaceCorner->fY = psPosition->y;
			psFaceCorner->fZ = psPosition->z;
			psFaceCorner->iColour = MeshWeldColour(pcMesh, iFace, i);
			psFaceCorner->iFaceCorner = iFace * NUM_FACE_CORNERS + i;
		}
	}
	qsort(pasFaceCorners->GetData(), pasFaceCorners->NumElements(), sizeof(SMeshWeldFaceCorner), MeshWeldCompareFaceCorners);
}


//////////////////////////////////////////////////////////////////////////
//
// The corners and the corner of each face.  Returns whether that's any
// different from pcMesh: if not, each corner was already one welded corner.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshWeldNumberCorners(CMesh* pcMesh, CArrayMeshWeldFaceCorner* pasFaceCorners, CArrayFloat32Vec3* pasCorners, CArrayInt* paiFaceCorners)
{
	SMeshWeldFaceCorner*	psFaceCorner;
	SMeshWeldFaceCorner*	psLast;
	SFloat32Vec3*			psCorner;
	CArrayInt				aiWeldedOfCorner;
	int*					piWelded;
	int						iSourceCorner;
	int						iWeldedSource;
	int						iWelded;
	size					u;
	size					uiNumElements;
	bool					bChanged;

	aiWeldedOfCorner.Init();
	aiWeldedOfCorner.Resize(pcMesh->NumCorners());
	aiWeldedOfCorner.SetArrayValues(-1);

	uiNumElements = pasFaceCorners->NumElements();
	paiFaceCorners->Resize(uiNumElements);
	psLast = NULL;
	iWelded = -1;
	iWeldedSource = -1;
	bChanged = false;
	for (u = 0; u < uiNumElements; u++)
	{
		psFaceCorner = pasFaceCorners->Get(u);
		iSourceCorner = pcMesh->GetFaceCorners(psFaceCorner->iFaceCorner / NUM_FACE_CORNERS)[psFaceCorner->iFaceCorner % NUM_FACE_CORNERS];
		if ((psLast == NULL) || (MeshWeldCompare(psLast, psFaceCorner) != 0))
		{
			iWelded = (int)pasCorners->NumElements();
			iWeldedSource = iSourceCorner;
			psCorner = pasCorners->Add();
			psCorner->x = psFaceCorner->fX;
			psCorner->y = psFaceCorner->fY;
			psCorner->z = psFaceCorner->fZ;
		}
		else if (iWeldedSource != iSourceCorner)
		{
			bChanged = true;  //Two corners became one.
		}

		piWelded = aiWeldedOfCorner.Get(iSourceCorner);
		if ((*piWelded >= 0) && (*piWelded != iWelded))
		{
			bChanged = true;  //One corner became two.
		}
		*piWelded = iWelded;

		paiFaceCorners->SetValue(psFaceCorner->iFaceCorner, iWelded);
		psLast = psFaceCorner;
	}

	aiWeldedOfCorner.Kill();
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A face with two corners that are now the same goes.  Returns whether any did.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshWeldRemoveDegenerate(CArrayInt* paiFaceCorners)
{
	int*	piCorners;
	size	u;
	size	uiNumElements;
	bool	bRemoved;

	bRemoved = false;
	uiNumElements = paiFaceCorners->NumElements();
	for (u = 0; u < uiNumElements; u += NUM_FACE_CORNERS)
	{
		piCorners = paiFaceCorners->Get(u);
		if ((piCorners[0] == piCorners[1]) || (piCorners[1] == piCorners[2]) || (piCorners[2] == piCorners[0]))
		{
			piCorners[0] = -1;
			bRemoved = true;
		}
	}
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshWeldCorners::Init(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshWeldCorners::Modify(Ptr<CMesh> pMesh)
{
	CArrayMeshWeldFaceCorner	asFaceCorners;
	CArrayFloat32Vec3			asCorners;
	CArrayInt					aiFaceCorners;
	Ptr<CMesh>					pOutput;
	bool						bChanged;

	if ((pMesh->NumFaces() == 0) || !CanRebuild(&pMesh))
	{
		return pMesh;
	}

	asFaceCorners.Init();
	asCorners.Init();
	aiFaceCorners.Init();

	MeshWeldSortFaceCorners(&pMesh, &asFaceCorners);
	bChanged = MeshWeldNumberCorners(&pMesh, &asFaceCorners, &asCorners, &aiFaceCorners);
	bChanged |= MeshWeldRemoveDegenerate(&aiFaceCorners);

	pOutput = pMesh;
	if (bChanged)
	{
		pOutput = BuildFromFaces(&pMesh, asCorners.GetData(), (int)asCorners.NumElements(), aiFaceCorners.GetData());
	}

	aiFaceCorners.Kill();
	asCorners.Kill();
	asFaceCorners.Kill();
	return pOutput;
}
