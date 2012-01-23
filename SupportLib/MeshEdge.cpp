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
#include "MeshEdge.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMeshEdge::Init(void)
{
	aiFace[0] = -1;
	aiFace[1] = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshEdge::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteIntArray(aiCorner, NUM_EDGE_CORNERS));
	ReturnOnFalse(pcFile->WriteIntArray(aiFace, MAX_EDGE_FACES));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshEdge::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadIntArray(aiCorner, NUM_EDGE_CORNERS));
	ReturnOnFalse(pcFile->ReadIntArray(aiFace, MAX_EDGE_FACES));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMeshEdge::AddFace(int iFaceNum)
{
	if (aiFace[0] == -1)
	{
		aiFace[0] = iFaceNum;
		return TRUE;
	}
	else if (aiFace[1] == -1)
	{
		aiFace[1] = iFaceNum;
		return TRUE;
	}
	return FALSE;
}

