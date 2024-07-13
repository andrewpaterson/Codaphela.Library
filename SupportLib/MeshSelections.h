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
#ifndef __MESH_SELECTIONS_H__
#define __MESH_SELECTIONS_H__
#include "BaseLib/ArrayBit.h"
#include "MeshDetail.h"


class CMeshSelections : public CObject
{
CONSTRUCTABLE(CMeshSelections);
DESTRUCTABLE(CMeshSelections);
public:
	CArrayBit	mcVerts;  //1 if selected.  mcVerts.NumElements == mpcMesh->mcPositions.NumElements
	CArrayBit	mcEdges;  //mcEdges.NumElements == mpcMesh->mcEdges.NumElements
	CArrayBit	mcFaces;  //mcFaces.NumElements == mpcMesh->mcFaces.NumElements

	void 	Init(void);
	void 	Free(void);
	void	Class(void);

	void 	Clear(void);

	bool	Load(CObjectReader* pcFile);
	bool	Save(CObjectWriter* pcFile);

	int		GetNumberOfSelectedFaces(bool bSelected);
	void	SetFaceSelectionTo(bool bSelected);
	void	ClearFaceSelection(void);
	int		GetNumberOfSelectedEdges(bool bSelected);
	void	SetEdgeSelectionTo(bool bSelected);
	void	ClearEdgeSelection(void);
	int		GetNumberOfSelectedCorners(bool bSelected);
	void	SetCornerSelectionTo(bool bSelected);
	void	ClearCornerSelection(void);

	void	AddCorner(void);
	void	AddFace(int iNewEdges);
};


#endif // __MESH_SELECTIONS_H__

