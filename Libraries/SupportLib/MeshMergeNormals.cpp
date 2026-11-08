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
#include "BaseLib/ArrayTemplate.h"
#include "StandardLib/Objects.h"
#include "MeshMergeNormals.h"


//So that normals exactly the same are still the same after rounding, with a maximum angle of none.
#define MESH_MERGE_NORMALS_ROUNDING	0.000000001


//One normal at one corner, and the normals that are merging into it.
struct SMeshMergedNormal
{
	float64	fFirstX;  //The first of them as a unit normal.  It doesn't change, so the others can't creep away from it.
	float64	fFirstY;
	float64	fFirstZ;
	float64	fSumX;
	float64	fSumY;
	float64	fSumZ;
	int		iFirstNormal;
};


typedef CArrayTemplate<SMeshMergedNormal>	CArrayMeshMergedNormal;


//What's known about a mesh as its normals merge.  A face corner is the face's index * NUM_FACE_CORNERS + which corner of it.
struct SMeshMergeNormals
{
	CMesh*					pcMesh;
	float64					fMinCosine;

	CArrayInt				aiFirstFaceCorner;	//Of each corner, a list through aiNextFaceCorner, -1 at its end.
	CArrayInt				aiNextFaceCorner;
	CArrayMeshMergedNormal	asMerged;
	CArrayInt				aiFaceCornerMerged;	//The merged normal of each face corner, -1 if it has no normal of its own.
};


//////////////////////////////////////////////////////////////////////////
//
// Each corner's list has its faces in order, as the lists are made backwards.
//
//////////////////////////////////////////////////////////////////////////
static void MeshMergeNormalsMakeLists(SMeshMergeNormals* ps)
{
	int*	piCorners;
	int*	piFirst;
	int		iFace;
	int		i;
	int		iFaceCorner;

	ps->aiFirstFaceCorner.Resize(ps->pcMesh->NumCorners());
	ps->aiFirstFaceCorner.SetArrayValues(-1);
	ps->aiNextFaceCorner.Resize(ps->pcMesh->NumFaces() * NUM_FACE_CORNERS);
	ps->aiFaceCornerMerged.Resize(ps->pcMesh->NumFaces() * NUM_FACE_CORNERS);
	ps->aiFaceCornerMerged.SetArrayValues(-1);

	for (iFace = ps->pcMesh->NumFaces() - 1; iFace >= 0; iFace--)
	{
		piCorners = ps->pcMesh->GetFaceCorners(iFace);
		for (i = 0; i < NUM_FACE_CORNERS; i++)
		{
			iFaceCorner = iFace * NUM_FACE_CORNERS + i;
			piFirst = ps->aiFirstFaceCorner.Get(piCorners[i]);
			ps->aiNextFaceCorner.SetValue(iFaceCorner, *piFirst);
			*piFirst = iFaceCorner;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Which of this corner's merged normals, from iFirstMerged on, iNormal belongs
// in.  Returns -1 if it's none of them.
//
//////////////////////////////////////////////////////////////////////////
static int MeshMergeNormalsFind(SMeshMergeNormals* ps, int iFirstMerged, int iNormal, float64 fX, float64 fY, float64 fZ)
{
	SMeshMergedNormal*	psMerged;
	int					iMerged;
	int					iNumMerged;

	iNumMerged = (int)ps->asMerged.NumElements();
	for (iMerged = iFirstMerged; iMerged < iNumMerged; iMerged++)
	{
		psMerged = ps->asMerged.Get(iMerged);
		if ((psMerged->iFirstNormal == iNormal) || (psMerged->fFirstX * fX + psMerged->fFirstY * fY + psMerged->fFirstZ * fZ >= ps->fMinCosine))
		{
			return iMerged;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns whether any normal merged with a different one.
//
//////////////////////////////////////////////////////////////////////////
static bool MeshMergeNormalsMergeCorner(SMeshMergeNormals* ps, int iCorner)
{
	SMeshNormalFace*	psNormalFace;
	SFloat32Vec3*		psNormal;
	SMeshMergedNormal*	psMerged;
	float64				fX;
	float64				fY;
	float64				fZ;
	float64				fLength;
	int					iFaceCorner;
	int					iFirstMerged;
	int					iMerged;
	int					iNormal;
	bool				bChanged;

	bChanged = false;
	iFirstMerged = (int)ps->asMerged.NumElements();
	iFaceCorner = ps->aiFirstFaceCorner.GetValue(iCorner);
	while (iFaceCorner >= 0)
	{
		psNormalFace = ps->pcMesh->mcNormals.mcFaces.Get(iFaceCorner / NUM_FACE_CORNERS);
		iNormal = psNormalFace->aiCornerNormals[iFaceCorner % NUM_FACE_CORNERS];
		if (iNormal >= 0)
		{
			psNormal = ps->pcMesh->mcNormals.mcNormals.Get(iNormal);
			fX = psNormal->x;
			fY = psNormal->y;
			fZ = psNormal->z;
			fLength = sqrt(fX * fX + fY * fY + fZ * fZ);
			if (fLength > 0.0)
			{
				fX /= fLength;
				fY /= fLength;
				fZ /= fLength;
			}

			iMerged = MeshMergeNormalsFind(ps, iFirstMerged, iNormal, fX, fY, fZ);
			if (iMerged < 0)
			{
				iMerged = (int)ps->asMerged.NumElements();
				psMerged = ps->asMerged.Add();
				psMerged->fFirstX = fX;
				psMerged->fFirstY = fY;
				psMerged->fFirstZ = fZ;
				psMerged->fSumX = 0.0;
				psMerged->fSumY = 0.0;
				psMerged->fSumZ = 0.0;
				psMerged->iFirstNormal = iNormal;
			}
			psMerged = ps->asMerged.Get(iMerged);
			bChanged |= (psMerged->iFirstNormal != iNormal);
			psMerged->fSumX += fX;
			psMerged->fSumY += fY;
			psMerged->fSumZ += fZ;
			ps->aiFaceCornerMerged.SetValue(iFaceCorner, iMerged);
		}
		iFaceCorner = ps->aiNextFaceCorner.GetValue(iFaceCorner);
	}
	return bChanged;
}


//////////////////////////////////////////////////////////////////////////
//
// A new mesh the same as pcMesh but for its normals.  Its flat normals come
// first, so the merged normals are numbered after its faces.
//
//////////////////////////////////////////////////////////////////////////
static Ptr<CMesh> MeshMergeNormalsBuild(SMeshMergeNormals* ps)
{
	Ptr<CMesh>			pOutput;
	CMesh*				pcMesh;
	SMeshNormalFace*	psNormalFace;
	SMeshMergedNormal*	psMerged;
	int*				piCorners;
	int*				piColours;
	float64				fLength;
	int					iNumFaces;
	int					iFace;
	int					iMerged;
	int					i;
	int					k;
	size				u;

	pcMesh = ps->pcMesh;
	pOutput = OMalloc<CMesh>();
	pOutput->mcNormals.mbInUse = true;
	pOutput->mcColours.mbInUse = pcMesh->mcColours.InUse();
	for (u = 0; u < pcMesh->mcColours.mcColours.NumElements(); u++)
	{
		pOutput->mcColours.mcColours.Add(pcMesh->mcColours.mcColours.GetValue(u));
	}
	for (i = 0; i < pcMesh->NumCorners(); i++)
	{
		pOutput->AddCorner(pcMesh->GetPosition(i));
	}

	iNumFaces = pcMesh->NumFaces();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pcMesh->GetFaceCorners(iFace);
		pOutput->AddFace(piCorners[0], piCorners[1], piCorners[2]);
		pOutput->SetFaceName(iFace, pcMesh->GetFaceName(iFace));

		psNormalFace = pOutput->mcNormals.mcFaces.Get(iFace);
		for (k = 0; k < NUM_FACE_CORNERS; k++)
		{
			iMerged = ps->aiFaceCornerMerged.GetValue(iFace * NUM_FACE_CORNERS + k);
			psNormalFace->aiCornerNormals[k] = (iMerged >= 0) ? iNumFaces + iMerged : -1;
		}

		if (pcMesh->mcColours.InUse())
		{
			piColours = pcMesh->mcColours.mcFaces.Get(iFace)->aiCornerColours;
			for (k = 0; k < NUM_FACE_CORNERS; k++)
			{
				pOutput->mcColours.mcFaces.Get(iFace)->aiCornerColours[k] = piColours[k];
			}
		}
	}

	for (u = 0; u < ps->asMerged.NumElements(); u++)
	{
		psMerged = ps->asMerged.Get(u);
		fLength = sqrt(psMerged->fSumX * psMerged->fSumX + psMerged->fSumY * psMerged->fSumY + psMerged->fSumZ * psMerged->fSumZ);
		if (fLength > 0.0)
		{
			pOutput->AddNormal((float32)(psMerged->fSumX / fLength), (float32)(psMerged->fSumY / fLength), (float32)(psMerged->fSumZ / fLength));
		}
		else
		{
			pOutput->AddNormal(0.0f, 0.0f, 0.0f);
		}
	}
	return pOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshMergeNormals::Init(float32 fMaxAngle)
{
	mfMaxAngle = fMaxAngle;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshMergeNormals::Modify(Ptr<CMesh> pMesh)
{
	SMeshMergeNormals	sMerge;
	Ptr<CMesh>			pOutput;
	int					i;
	int					iNumCorners;
	bool				bChanged;

	if ((pMesh->NumFaces() == 0) || !pMesh->mcNormals.InUse() || (mfMaxAngle < 0.0f) || !CanRebuild(&pMesh))
	{
		return pMesh;
	}

	sMerge.pcMesh = &pMesh;
	sMerge.fMinCosine = cos((float64)mfMaxAngle) - MESH_MERGE_NORMALS_ROUNDING;
	sMerge.aiFirstFaceCorner.Init();
	sMerge.aiNextFaceCorner.Init();
	sMerge.asMerged.Init();
	sMerge.aiFaceCornerMerged.Init();

	MeshMergeNormalsMakeLists(&sMerge);
	bChanged = false;
	iNumCorners = pMesh->NumCorners();
	for (i = 0; i < iNumCorners; i++)
	{
		bChanged |= MeshMergeNormalsMergeCorner(&sMerge, i);
	}

	pOutput = pMesh;
	if (bChanged)
	{
		pOutput = MeshMergeNormalsBuild(&sMerge);
	}

	sMerge.aiFaceCornerMerged.Kill();
	sMerge.asMerged.Kill();
	sMerge.aiNextFaceCorner.Kill();
	sMerge.aiFirstFaceCorner.Kill();
	return pOutput;
}
