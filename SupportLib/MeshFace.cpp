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
#include "MeshFace.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshFace::Init(void)
{
	asEdge[0] = -1;
	asEdge[1] = -1;
	asEdge[2] = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshFace::Init(int iCorner1, int iCorner2, int iCorner3)
{
	asCorner[0] = iCorner1;
	asCorner[1] = iCorner2;
	asCorner[2] = iCorner3;
	asEdge[0] = -1;
	asEdge[1] = -1;
	asEdge[2] = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshFace::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteIntArray(asCorner, NUM_FACE_CORNERS));
	ReturnOnFalse(pcFile->WriteIntArray(asEdge, NUM_FACE_EDGES));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshFace::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadIntArray(asCorner, NUM_FACE_CORNERS));
	ReturnOnFalse(pcFile->ReadIntArray(asEdge, NUM_FACE_EDGES));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMeshFace::GetOffsetForCorner(int iCornerNum)
{
	int i;

	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		if (asCorner[i] == iCornerNum)
		{
			return i;
		}
	}
	return -1;
}


