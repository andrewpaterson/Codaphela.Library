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
#ifndef __MESH_EDITOR_H__
#define __MESH_EDITOR_H__
#include "StandardLib/Unknown.h"
#include "MeshFaceReturn.h"
#include "MeshSelections.h"
#include "MeshEdgeVisibility.h"
#include "MeshPolygons.h"
#include "MeshModifierStack.h"


class CMesh;
class CMeshEditor : public CUnknown
{
CONSTRUCTABLE(CMeshEditor);
public:
	CMesh*					mpcMesh;
	CMeshModifierStack		mcModifiers;

	CMeshSelections			mcSelections;
	CMeshEdgeVisibility		mcEdgeVisibility;
	CMeshPolygons			mcPolygons;

	void 	Init(CMesh* pcMesh);
	void 	Kill(void);
	BOOL 	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);

	void	Touch(void);
	void	Collapse(void);

	void	RemoveModifiers(void);

	template<typename M>
	M*		AddModifier(void);

	int		NumCorners(void);
	int		NumFaces(void);
	int		NumEdges(void);
	int		NumVisibleEdges(char cEdge);
	int		NumPolygons(void);

	void	ReInitConnectivity(void);

	//These are the only methods that editors/modifers may use to alter mesh connectivity or positions.
	void 	AddCorner(float x, float y, float z);
	void	AddCorner(SFloat3* psPosition);

	CMFRet	AddFace(int iCorner1, int iCorner2, int iCorner3, int iName = 0, BOOL bEdge12 = TRUE, BOOL bEdge23 = TRUE, BOOL bEdge31 = TRUE);
	CMFRet	AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, BOOL bEdge12, BOOL bEdge23, BOOL bEdge31, BOOL bFlipFaces);
	CMFRet	AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iName = 0, BOOL bFlipFaces = FALSE);  //Corners clock-wise.
	CMFRet	AddPolygon(CArrayInt* paiCorners, int iName = 0, BOOL bFlipFaces = FALSE);  //Corners clock-wise.

	void	RemoveFace(int iFace);
	void	RemoveFaces(CArrayInt* paiFaces);

	void	RemoveCorner(int iCorner);

	void	MovePosition(int iPosition, SFloat3* psPosition);
	void	MovePositions(CArrayInt* paiPositions, SFloat3* psPosition);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<typename M>
M* CMeshEditor::AddModifier(void)
{
	return mcModifiers.AddModifier<M>();
}


#endif // !__MESH_EDITOR_H__

