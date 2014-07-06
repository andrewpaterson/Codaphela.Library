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
#ifndef __MESH_CORNER_H__
#define __MESH_CORNER_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/ArraySimpleInt.h"


class CMeshCorner
{
public:
	CArraySimpleInt		aiFace;		//Faces on this corner.
	CArraySimpleInt		aiEdge;		//Edges on this corner.

	void 	Init(void);
	void 	Kill(void);
	BOOL	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
};


#endif // __MESH_CORNER_H__

