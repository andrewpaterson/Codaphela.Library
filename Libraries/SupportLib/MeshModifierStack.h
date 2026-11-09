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
#ifndef __MESH_MODIFIER_STACK_H__
#define __MESH_MODIFIER_STACK_H__
#include <string.h>
#include "StandardLib/ArrayUnknown.h"
#include "MeshModifier.h"


//The stack holds a source mesh and GetOutput runs it up through the modifiers: Source | Modifier 0 | Modifier 1 | etc...
//The source isn't changed until Flatten makes the output the new source.  With no source it starts from an empty mesh.
//A CMeshEditor's stack has no source.  It applies its modifiers to the editor's mesh in place with ApplyAll.  If any of them
//makes a shape the mesh goes back first to what it was before they were first applied, so applying them again gives the
//same mesh, and a shape is added to a mesh that was collapsed.
class CMeshEditor;
class CMeshModifierStack : public CObject
{
CONSTRUCTABLE(CMeshModifierStack);
DESTRUCTABLE(CMeshModifierStack);
protected:
	CArrayUnknown	macEditors;  //Mesh | Editor 0 | Editor 1 | etc...
	int				miCurrent;
	Ptr<CMesh>		mpSource;
	Ptr<CMesh>		mpOutput;

public:
	void	Init(Ptr<CMesh> pSource = NULL);
	void 	Free(void) override;
	void	Class(void) override;

	bool	Load(CObjectReader* pcFile) override;
	bool	Save(CObjectWriter* pcFile) override;

	void	ReInit(void);

	void 	AddModifier(CMeshModifier* pcEditor);
	template<typename M>
	M*		AddModifier(void);
	template<typename M>
	M*		GetModifier(void);  //The first of its class, or NULL.
	int		NumEditors(void);

	void	ApplyAll(CMeshEditor* pcMeshEditor);
	bool	HasSourceModifier(void);

	Ptr<CMesh>	GetSource(void);
	Ptr<CMesh>	GetOutput(void);  //Kept until a modifier is added or Touch is called.
	void		Touch(void);  //Call this when a modifier's settings have changed.
	void		Flatten(void);  //The output becomes the new source and the modifiers are removed.

	void	ClearConnectivity(void);

	void	AddPosition(SFloat32Vec3* psPosition);

	void	AddFace(int iCorner1, int iCorner2, int iCorner3);

	void	RemoveFace(int iFace);
	void	RemoveFaces(CArrayInt* paiFaces);

	void	RemoveCorner(int iCorner);

	void	MovePosition(int iPosition, SFloat32Vec3* psDelta);
	void	MovePositions(CArrayInt* paiPositions, SFloat32Vec3* psDelta);

protected:
	void	TellOfFaces(CMeshModifier* pcModifier, CMesh* pcMesh);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<typename M>
M* CMeshModifierStack::AddModifier(void)
{
	M*	pv;

	if (mpcUnknownsThisIn != NULL)
	{
		pv = mpcUnknownsThisIn->Add<M>();
	}
	else
	{
		pv = UMalloc(M);
	}
	AddModifier(pv);
	return pv;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<typename M>
M* CMeshModifierStack::GetModifier(void)
{
	CUnknown*	pcModifier;
	size		i;

	for (i = 0; i < macEditors.NumElements(); i++)
	{
		pcModifier = macEditors.Get(i);
		if (strcmp(pcModifier->ClassName(), M::StaticClassName()) == 0)
		{
			return (M*)pcModifier;
		}
	}
	return NULL;
}


#endif // __MESH_MODIFIER_STACK_H__

