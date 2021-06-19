/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __MESH_EDGE_H__
#define __MESH_EDGE_H__
#include "BaseLib/ChunkFile.h"


#define NUM_EDGE_CORNERS	2
#define MAX_EDGE_FACES		2


class CMeshEdge
{
public:
	int		aiCorner[NUM_EDGE_CORNERS];		//Corners on this edge.
	int		aiFace[MAX_EDGE_FACES];			//Faces on this edge.  A Maximum of two is assumed.  Unused indices are set to -1.

	void	Init(void);
	BOOL 	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
	BOOL	AddFace(int iFaceNum);
};


#endif // !__MESH_EDGE_H__

