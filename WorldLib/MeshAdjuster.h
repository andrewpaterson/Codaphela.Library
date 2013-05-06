/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela EditorLib

Codaphela EditorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela EditorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela EditorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __MESH_ADJUSTER_H__
#define __MESH_ADJUSTER_H__
#include "WorldLib/World.h"
#include "WorldLib/MeshConverter.h"  //For SFaceConversionData... move later.
#include "WorldLib/SelectionOverrides.h"
#include "StandardLib/StandardHeader.h"


#define	MAD_Edges		0x01
#define MAD_Vertices	0x02
#define	MAD_Faces		0x04
#define	MAD_Bounding	0x08
#define	MAD_Selection	0x10


class CWorldEditor;
class CMeshEditor;
class CMeshAdjuster : public CUnknown
{
BASE_FUNCTIONS(CMeshAdjuster)
protected:
	CWorldEditor*			mpcWorldEditor;
	CMeshEditor*			mpcMeshEditor;

	//These two should actually be contained in CMetaInstance..
	CMeshInstance*			mpcMainLinkInstance;
	CGraphicsInstance*		mpcMainGraphicsInstance;

	CGraphicsObject*		mpcSelectionObject;
	int						miMeshAdjusterDisplay;
	CSelectionColourArray	mcVertSelectionColours;
	CSelectionColourArray	mcFaceSelectionColours;
	CSelectionColourArray	mcEdgeSelectionColours;
	unsigned int			miObjectColour;
	unsigned int			miEdgeColour;
	int						miSelected;
	int						miColourPartSelected;
	int						miColourObjectEdgeNotSelected;
	int						miColourObjectVertNotSelected;
	int						miColourBlendPartSelected;
	int						miLightlessMaterialIndex;
	int						miZBufferlessMaterialIndex;
	int						miBlendMaterialIndex;
	int						miStateIndex;

public:
	void Init(CMeshEditor* pcMeshEditor, CGraphicsInstance* pcGraphicsInstance, CMeshInstance* pcMeshInstance, CWorldEditor* pcWorldEditor);
	void Kill(void);
	void SetupSelectionColours(void);

	void SetSelectionColours(void);
	void CreateObjectAsNecessary(void);
	void ReleaseObject(void);
	void Draw(void);

	void ClearVertSelection(void);
	void ClearEdgeSelection(void);
	void ClearFaceSelection(void);
};


#endif //__MESH_ADJUSTER_H__

