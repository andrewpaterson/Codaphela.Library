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
#include "MeshFace.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshFace::Init(void)
{
	maiEdge[0] = -1;
	maiEdge[1] = -1;
	maiEdge[2] = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshFace::Init(int iCorner1, int iCorner2, int iCorner3)
{
	maiCorner[0] = iCorner1;
	maiCorner[1] = iCorner2;
	maiCorner[2] = iCorner3;
	maiEdge[0] = -1;
	maiEdge[1] = -1;
	maiEdge[2] = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshFace::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteIntArray(maiCorner, NUM_FACE_CORNERS));
	ReturnOnFalse(pcFile->WriteIntArray(maiEdge, NUM_FACE_EDGES));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CMeshFace::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadIntArray(maiCorner, NUM_FACE_CORNERS));
	ReturnOnFalse(pcFile->ReadIntArray(maiEdge, NUM_FACE_EDGES));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMeshFace::GetOffsetForCorner(int iCornerNum)
{
	size	i;

	for (i = 0; i < NUM_FACE_CORNERS; i++)
	{
		if (maiCorner[i] == iCornerNum)
		{
			return (int)i;
		}
	}
	return -1;
}


