/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "MeshConnectivity.h"
#include "MeshSmoothGroups.h"
#include "MeshPositions.h"
#include "Mesh.h"
#include "MeshNormals.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshNormals::Init(void)
{
	CMeshDetail::Init();
	mcNormals.Init(MESH_CORNERS_CHUNK_SIZE);  //Give or take.
	mcFaces.Init(MESH_FACES_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshNormals::ReInit(int iCornerChunkSize, int iFaceChunkSize)
{
	mcNormals.ReInit(iCornerChunkSize);
	mcFaces.ReInit(iFaceChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshNormals::Kill(void)
{
	mcFaces.Kill();
	mcNormals.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshNormals::Load(CFileReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	ReturnOnFalse(pcFile->ReadArrayTemplate(&mcNormals));
	ReturnOnFalse(pcFile->ReadArrayTemplate(&mcFaces));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshNormals::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	ReturnOnFalse(pcFile->WriteArrayTemplate(&mcNormals));
	ReturnOnFalse(pcFile->WriteArrayTemplate(&mcFaces));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshNormals::GenerateFlatFaceNormals(CMeshPositions* pcPositions, CMeshConnectivity* pcConn)
{
	int					iFaceNum;
	SFloat3				sVec1;
	SFloat3				sVec2;
	SFloat3				sNormal;
	int					iNumFaces;

	if (mbInUse)
	{
		iNumFaces = pcConn->mcFaces.NumElements();

		mcFaces.Kill();
		mcFaces.Init(pcConn->mcFaces.ChunkSize());

		mcNormals.Kill();
		mcNormals.Init(pcConn->mcCorners.ChunkSize());

		for (iFaceNum = 0; iFaceNum < iNumFaces; iFaceNum++)
		{
			AddFace(iFaceNum, pcPositions, pcConn);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshNormals::AddFace(int iFaceNum, CMeshPositions* pcPositions, CMeshConnectivity* pcConn)
{
	CMeshFace*			psFace;
	CMeshCorner*		psCornerA;
	CMeshCorner*		psCornerB;
	CMeshCorner*		psCornerC;
	SFloat3*			psPosA;
	SFloat3*			psPosB;
	SFloat3*			psPosC;
	SFloat3				sVec1;
	SFloat3				sVec2;
	SFloat3				sNormal;
	SMeshNormalFace*	psNormalFace;
	SFloat3*			psNormal;

	if (mbInUse)
	{
		psFace = pcConn->GetFace(iFaceNum);
		psNormalFace = mcFaces.InsertAt(iFaceNum);
		psNormal = mcNormals.InsertAt(iFaceNum);

		psCornerA = pcConn->GetCorner(psFace->asCorner[0]);
		psCornerB = pcConn->GetCorner(psFace->asCorner[1]);
		psCornerC = pcConn->GetCorner(psFace->asCorner[2]);

		psPosA = pcPositions->mcPositions.Get(psFace->asCorner[0]);
		psPosB = pcPositions->mcPositions.Get(psFace->asCorner[1]);
		psPosC = pcPositions->mcPositions.Get(psFace->asCorner[2]);

		sVec1 = *psPosA - *psPosB;
		sVec2 = *psPosC - *psPosB;
		Float3Cross(psNormal, &sVec2, &sVec1);
		psNormal->Normalize();

		psNormalFace->aiCornerNormals[0] = -1;
		psNormalFace->aiCornerNormals[1] = -1;
		psNormalFace->aiCornerNormals[2] = -1;
		psNormalFace->iFaceNormal = iFaceNum;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshNormals::IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces)
{
	int					i;
	int					iFaceIndex;
	SMeshNormalFace*	psFaceNormal;

	//This assumes something else has check that paiSourceFaces actually have corner normals.
	for (i = 0; i < paiSourceFaces->NumElements(); i++)
	{
		iFaceIndex = paiSourceFaces->GetValue(i);
		psFaceNormal = mcFaces.Get(iFaceIndex);

		*paiDest = psFaceNormal->aiCornerNormals[0];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		*paiDest = psFaceNormal->aiCornerNormals[1];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		*paiDest = psFaceNormal->aiCornerNormals[2];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CMeshNormals::GetFaceNormal(int iFace)
{
	return mcNormals.Get(iFace);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SFloat3* CMeshNormals::GetCornerNormal(int iCorner)
{
	return mcNormals.Get(iCorner);
}

