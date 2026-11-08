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
#include "StandardLib/Objects.h"
#include "MeshFixReversed.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshFixReversed::Init(void)
{
	muiNumFixed = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshFixReversed::Modify(Ptr<CMesh> pMesh)
{
	muiNumFixed = 0;
	if (!pMesh->mcNormals.InUse() || !HasReversed(&pMesh) || !CanRebuild(&pMesh))
	{
		return pMesh;
	}
	return Fix(&pMesh);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMeshFixReversed::NumFixed(void)
{
	return muiNumFixed;
}


//////////////////////////////////////////////////////////////////////////
//
// Calculated in 64 bits from the positions as a small face's normal is lost
// in a stored unit one.
//
//////////////////////////////////////////////////////////////////////////
bool CMeshFixReversed::IsReversed(CMesh* pcMesh, int iFace)
{
	SMeshNormalFace*	psNormalFace;
	SFloat32Vec3*		psNormal;
	SFloat32Vec3*		psA;
	SFloat32Vec3*		psB;
	SFloat32Vec3*		psC;
	int*				piCorners;
	float64				fNormalX;
	float64				fNormalY;
	float64				fNormalZ;
	float64				fBX;
	float64				fBY;
	float64				fBZ;
	float64				fCX;
	float64				fCY;
	float64				fCZ;
	bool				bCornerNormals;
	int					i;
	int					iNormal;

	psNormalFace = pcMesh->mcNormals.mcFaces.Get(iFace);
	bCornerNormals = false;
	fNormalX = 0.0;
	fNormalY = 0.0;
	fNormalZ = 0.0;
	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		iNormal = psNormalFace->aiCornerNormals[i];
		if (iNormal < 0)
		{
			iNormal = psNormalFace->iFaceNormal;
		}
		else
		{
			bCornerNormals = true;
		}
		psNormal = pcMesh->mcNormals.mcNormals.Get(iNormal);
		fNormalX += psNormal->x;
		fNormalY += psNormal->y;
		fNormalZ += psNormal->z;
	}
	if (!bCornerNormals)
	{
		return false;
	}

	piCorners = pcMesh->GetFaceCorners(iFace);
	psA = pcMesh->GetPosition(piCorners[0]);
	psB = pcMesh->GetPosition(piCorners[1]);
	psC = pcMesh->GetPosition(piCorners[2]);
	fBX = (float64)psB->x - psA->x;
	fBY = (float64)psB->y - psA->y;
	fBZ = (float64)psB->z - psA->z;
	fCX = (float64)psC->x - psA->x;
	fCY = (float64)psC->y - psA->y;
	fCZ = (float64)psC->z - psA->z;
	return (fBY * fCZ - fBZ * fCY) * fNormalX + (fBZ * fCX - fBX * fCZ) * fNormalY + (fBX * fCY - fBY * fCX) * fNormalZ < 0.0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshFixReversed::HasReversed(CMesh* pcMesh)
{
	int	i;
	int	iNumFaces;

	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		if (IsReversed(pcMesh, i))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A new mesh the same as pcMesh but for its reversed faces.  Its faces make
// the same flat normals, so its corner normals have the same indices.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshFixReversed::Fix(CMesh* pcMesh)
{
	Ptr<CMesh>			pOutput;
	SMeshNormalFace*	psNormalFace;
	SMeshNormalFace*	psOutputNormalFace;
	SFloat32Vec3*		psNormal;
	int*				piCorners;
	int*				piColours;
	int					iNumFaces;
	int					iNumNormals;
	int					iNumCorners;
	int					i;
	int					iSecond;
	int					iThird;
	bool				bColours;

	bColours = pcMesh->mcColours.InUse();

	pOutput = OMalloc<CMesh>();
	pOutput->mcNormals.mbInUse = true;
	pOutput->mcColours.mbInUse = bColours;
	for (i = 0; i < (int)pcMesh->mcColours.mcColours.NumElements(); i++)
	{
		pOutput->mcColours.mcColours.Add(pcMesh->mcColours.mcColours.GetValue(i));
	}

	iNumCorners = pcMesh->NumCorners();
	for (i = 0; i < iNumCorners; i++)
	{
		pOutput->AddCorner(pcMesh->GetPosition(i));
	}

	iNumFaces = pcMesh->NumFaces();
	for (i = 0; i < iNumFaces; i++)
	{
		iSecond = 1;
		iThird = 2;
		if (IsReversed(pcMesh, i))
		{
			iSecond = 2;
			iThird = 1;
			muiNumFixed++;
		}

		piCorners = pcMesh->GetFaceCorners(i);
		pOutput->AddFace(piCorners[0], piCorners[iSecond], piCorners[iThird]);

		psNormalFace = pcMesh->mcNormals.mcFaces.Get(i);
		psOutputNormalFace = pOutput->mcNormals.mcFaces.Get(i);
		psOutputNormalFace->aiCornerNormals[0] = psNormalFace->aiCornerNormals[0];
		psOutputNormalFace->aiCornerNormals[1] = psNormalFace->aiCornerNormals[iSecond];
		psOutputNormalFace->aiCornerNormals[2] = psNormalFace->aiCornerNormals[iThird];

		if (bColours)
		{
			piColours = pcMesh->mcColours.mcFaces.Get(i)->aiCornerColours;
			pOutput->mcColours.mcFaces.Get(i)->aiCornerColours[0] = piColours[0];
			pOutput->mcColours.mcFaces.Get(i)->aiCornerColours[1] = piColours[iSecond];
			pOutput->mcColours.mcFaces.Get(i)->aiCornerColours[2] = piColours[iThird];
		}
	}

	iNumNormals = (int)pcMesh->mcNormals.mcNormals.NumElements();
	for (i = iNumFaces; i < iNumNormals; i++)
	{
		psNormal = pcMesh->mcNormals.mcNormals.Get(i);
		pOutput->AddNormal(psNormal->x, psNormal->y, psNormal->z);
	}
	return pOutput;
}
