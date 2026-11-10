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
#include "BaseLib/ArrayInt.h"
#include "MeshRemoveDegenerate.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshRemoveDegenerate::Init(void)
{
	muiNumRemoved = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CMesh> CMeshRemoveDegenerate::Modify(Ptr<CMesh> pMesh)
{
	CArrayInt	aiFaceCorners;
	Ptr<CMesh>	pOutput;
	int*		piCorners;
	int*		piFaceCorners;
	int			iNumFaces;
	int			iFace;
	int			i;

	muiNumRemoved = 0;
	iNumFaces = pMesh->NumFaces();
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		if (IsDegenerate(&pMesh, iFace))
		{
			muiNumRemoved++;
		}
	}
	if ((muiNumRemoved == 0) || !CanRebuild(&pMesh))
	{
		muiNumRemoved = 0;
		return pMesh;
	}

	aiFaceCorners.Init();
	aiFaceCorners.Resize(iNumFaces * NUM_FACE_CORNERS);
	for (iFace = 0; iFace < iNumFaces; iFace++)
	{
		piCorners = pMesh->GetFaceCorners(iFace);
		piFaceCorners = aiFaceCorners.Get(iFace * NUM_FACE_CORNERS);
		for (i = 0; i < NUM_FACE_CORNERS; i++)
		{
			piFaceCorners[i] = piCorners[i];
		}
		if (IsDegenerate(&pMesh, iFace))
		{
			piFaceCorners[0] = -1;
		}
	}
	//A mesh's positions are one array, which a face being removed means isn't empty.
	pOutput = BuildFromFaces(&pMesh, pMesh->GetPosition(0), pMesh->NumCorners(), aiFaceCorners.GetData());
	aiFaceCorners.Kill();
	return pOutput;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CMeshRemoveDegenerate::NumRemoved(void)
{
	return muiNumRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
// Worked out in 64 bits from the positions, in which the differences of
// two positions are exact, so that only a face that really is a line has
// no area.
//
//////////////////////////////////////////////////////////////////////////
bool CMeshRemoveDegenerate::IsDegenerate(CMesh* pcMesh, int iFace)
{
	SFloat32Vec3*	psA;
	SFloat32Vec3*	psB;
	SFloat32Vec3*	psC;
	int*			piCorners;
	float64			fBX;
	float64			fBY;
	float64			fBZ;
	float64			fCX;
	float64			fCY;
	float64			fCZ;

	piCorners = pcMesh->GetFaceCorners(iFace);
	if ((piCorners[0] == piCorners[1]) || (piCorners[1] == piCorners[2]) || (piCorners[2] == piCorners[0]))
	{
		return true;
	}

	psA = pcMesh->GetPosition(piCorners[0]);
	psB = pcMesh->GetPosition(piCorners[1]);
	psC = pcMesh->GetPosition(piCorners[2]);
	fBX = (float64)psB->x - psA->x;
	fBY = (float64)psB->y - psA->y;
	fBZ = (float64)psB->z - psA->z;
	fCX = (float64)psC->x - psA->x;
	fCY = (float64)psC->y - psA->y;
	fCZ = (float64)psC->z - psA->z;
	return ((fBY * fCZ - fBZ * fCY) == 0.0) && ((fBZ * fCX - fBX * fCZ) == 0.0) && ((fBX * fCY - fBY * fCX) == 0.0);
}
