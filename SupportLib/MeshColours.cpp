/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "StandardLib/ClassDefines.h"
#include "StandardLib/ObjectReader.h"
#include "StandardLib/ObjectWriter.h"
#include "MeshDefines.h"
#include "MeshColours.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::Init(void)
{
	PreInit();

	CMeshDetail::Init();
	mcColours.Init();
	mcFaces.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::Clear(void)
{
	mcColours.ReInit();
	mcFaces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::Free(void)
{
	mcFaces.Kill();
	mcColours.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMeshColours::Class(void)
{
	CMeshDetail::Class();

	U_Unknown(CArrayInt, mcColours);
	U_Unknown(CArrayMeshFaceColours, mcFaces);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshColours::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(LoadMeshDetail(pcFile));

	ReturnOnFalse(mcFaces.Read(pcFile));
	ReturnOnFalse(mcColours.Read(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CMeshColours::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(SaveMeshDetail(pcFile));
	ReturnOnFalse(mcFaces.Write(pcFile));
	ReturnOnFalse(mcColours.Write(pcFile));
	return true;
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
	size				i;
	size				iFaceIndex;
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

