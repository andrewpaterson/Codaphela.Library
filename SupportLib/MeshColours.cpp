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
#include "MeshDefines.h"
#include "MeshColours.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::Init(void)
{
	CMeshDetail::Init();
	mcColours.Init(MESH_CORNERS_CHUNK_SIZE);
	mcFaces.Init(MESH_FACES_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::ReInit(int iCornerChunkSize, int iFaceChunkSize)
{
	mcColours.ReInit(iCornerChunkSize);
	mcFaces.ReInit(iFaceChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::Kill(void)
{
	mcFaces.Kill();
	mcColours.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshColours::Load(CFileReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));
	ReturnOnFalse(pcFile->ReadArrayTemplate(&mcFaces));
	ReturnOnFalse(pcFile->ReadArrayInt(&mcColours));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshColours::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	ReturnOnFalse(pcFile->WriteArrayTemplate(&mcFaces));
	ReturnOnFalse(pcFile->WriteArrayInt(&mcColours));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::AddFace(void)
{
	SMeshFaceColour*	psMeshFaceColour;

	if (mbInUse)
	{
		psMeshFaceColour = mcFaces.Add();
		psMeshFaceColour->aiCornerColours[0] = -1;
		psMeshFaceColour->aiCornerColours[1] = -1;
		psMeshFaceColour->aiCornerColours[2] = -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::IndexFaceCorners(int* paiDest, int iStride, CArrayInt* paiSourceFaces)
{
	int					i;
	int					iFaceIndex;
	SMeshFaceColour*	psFaceColour;

	//This assumes something else has check that paiSourceFaces actually have corner normals.
	for (i = 0; i < paiSourceFaces->NumElements(); i++)
	{
		iFaceIndex = paiSourceFaces->GetValue(i);
		psFaceColour = mcFaces.Get(iFaceIndex);

		*paiDest = psFaceColour->aiCornerColours[0];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		*paiDest = psFaceColour->aiCornerColours[1];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);

		*paiDest = psFaceColour->aiCornerColours[2];
		paiDest = (int*)RemapSinglePointer(paiDest, iStride);
	}
}

