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
#ifndef __MESH_EDITOR_H__
#define __MESH_EDITOR_H__
#include "StandardLib/Unknown.h"
#include "MeshFaceReturn.h"
#include "MeshSelections.h"
#include "MeshEdgeVisibility.h"
#include "MeshPolygons.h"
#include "MeshModifierStack.h"
#include "Mesh.h"

class CMesh;
//The mesh and its modifiers.  GetOutput is the mesh with its modifiers applied, which is the mesh itself if there are none.  It's
//kept until the mesh is changed through the editor, a modifier is added, or mcModifiers.Touch says a modifier's settings changed.
//What's changed in the mesh directly, not through the editor's methods, isn't seen.
//
//The structure counter goes up when corners or faces are added or removed, and the shape counter when positions move, so that
//whatever uses the output can tell what it has to redo.  ClearOutput lets the output go once something has made what it needs from
//it, as with modifiers it's a second whole mesh.
//
//Until Apply goes (ScenePlan step 2), Touch still changes the mesh in place, so a shape it adds is in the mesh and GetOutput would add
//it again.  Use one or the other.
class CMeshEditor : public CObject
{
CONSTRUCTABLE(CMeshEditor);
DESTRUCTABLE(CMeshEditor);
public:
	CChars					mszName;  //What it's called in a file, and many can have the same one, which an object's name can't.
	Ptr<CMesh>				mpMesh;
	CMeshModifierStack		mcModifiers;

	CMeshSelections			mcSelections;
	CMeshEdgeVisibility		mcEdgeVisibility;
	CMeshPolygons			mcPolygons;

protected:
	//The mesh as it was before its modifiers made any shapes, which they add to each time they're applied.
	bool					mbBaseKept;
	Ptr<CMesh>				mpBase;
	CArrayChar				macBaseEdges;  //Whether each edge of each of its faces is visible.

	size					muiStructureChanges;
	size					muiShapeChanges;
	size					muiOutputStructure;  //What the counters were when the output was made.
	size					muiOutputShape;

public:
	void	Init(Ptr<CMesh> pMesh);
	void 	Free(void) override;
	void	Class(void) override;

	bool	Load(CObjectReader* pcFile) override;
	bool	Save(CObjectWriter* pcFile) override;

	void		Touch(void);
	void		Collapse(void);

	Ptr<CMesh>	GetOutput(void);
	void		ClearOutput(void);
	size		NumStructureChanges(void);
	size		NumShapeChanges(void);

	void	RemoveModifiers(void);

	template<typename M>
	M*		AddModifier(void);

	int		NumCorners(void);
	int		NumFaces(void);
	int		NumEdges(void);
	int		NumVisibleEdges(char cEdge);
	int		NumPolygons(void);

	void	ClearConnectivity(void);
	void	RestoreBase(void);  //For CMeshModifierStack: the mesh goes back to what its shapes are added to.

	//These are the only methods that editors/modifers may use to alter mesh connectivity or positions.
	void 	AddCorner(float32 x, float32 y, float32 z);
	void	AddCorner(SFloat32Vec3* psPosition);

	CMFRet	AddFace(int iCorner1, int iCorner2, int iCorner3, int iName = 0, bool bEdge12 = true, bool bEdge23 = true, bool bEdge31 = true);
	CMFRet	AddFace(int iCorner1, int iCorner2, int iCorner3, int iName, bool bEdge12, bool bEdge23, bool bEdge31, bool bFlipFaces);
	CMFRet	AddQuad(int iCorner1, int iCorner2, int iCorner3, int iCorner4, int iName = 0, bool bFlipFaces = false);  //Corners clock-wise.
	CMFRet	AddPolygon(CArrayInt* paiCorners, int iName = 0, bool bFlipFaces = false);  //Corners clock-wise.

	void	RemoveFace(int iFace);
	void	RemoveFaces(CArrayInt* paiFaces);

	void	RemoveCorner(int iCorner);

	void	MovePosition(int iPosition, SFloat32Vec3* psPosition);
	void	MovePositions(CArrayInt* paiPositions, SFloat32Vec3* psPosition);

protected:
	void	ClearMesh(void);
	void	ChangedStructure(void);
	void	ChangedShape(void);
	void	KeepBase(void);
	void	ForgetBase(void);
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


#endif // __MESH_EDITOR_H__

