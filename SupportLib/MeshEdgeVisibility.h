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
#ifndef __MESH_EDGE_VISIBILITY_H__
#define __MESH_EDGE_VISIBILITY_H__
#include "BaseLib/ArrayChar.h"
#include "MeshDetail.h"
#include "MeshFace.h"


class CMeshPolygons;
class CMeshConnectivity;
class CMeshEdgeVisibility : public CUnknown
{
CONSTRUCTABLE(CMeshEdgeVisibility);
public:
	//0 if not visible, > 0 if visible.  Char value is supposed to represent visible style.  Larger numbers are more visible.
	CArrayChar	mcEdges;  //mcEdges.NumElements == mpcMesh->mcEdges.NumElements

	void 	Init(void);
	void	ReInit(void);
	void 	Kill(void);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile);

	void	AddFace(CMeshFace* pcFace, int iNewEdges, char cEdge12 = 1, char cEdge23 = 1, char cEdge31 = 1);
	char	GetEdge(int iEdge);
	int		NumVisibleEdges(char cEdge);

	void	GenerateEdgeSelectionFromPolygons(CMeshConnectivity* pcConn, CMeshPolygons* pcPolygons);
};


#endif // __MESH_EDGE_VISIBILITY_H__

