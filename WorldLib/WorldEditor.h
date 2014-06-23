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
#ifndef __WORLD_EDITOR_H__
#define __WORLD_EDITOR_H__
#include "StandardLib/SetType.h"
#include "WorldLib/MeshSelectionConverter.h"
#include "WorldLib/MatrixSelectionConverter.h"
#include "MeshAdjuster.h"


typedef CSetType<CMeshAdjuster>			CMeshAdjusterSet;


class CSceneConverter;
class CWorldEditor
{
public:
	CMeshSelectionConverter				mcMeshSelectionConverter;
	CTransformNodeSelectionConverter	mcMatrixSelectionConverter;
	CMeshAdjusterSet					macMeshAdjusters;
	CSelectionColourArray				mcWhiteZeroColours;
	CGraphicsMaterial*					mpcZBufferlessMaterial;
	CGraphicsMaterial*					mpCLightInstancelessMaterial;
	CGraphicsMaterial*					mpcBlendMaterial;
	float								mfPointSize;
	int									miMeshID;
	CSceneConverter*					mpcSceneConverter;

	void 			Init(CSceneConverter* pcSceneConverter);
	void 			Kill(void);
	void 			Draw(void);
	void			SetupSelectionColours(void);
	void			SetupMaterials(void);
	BOOL 			AddScene(void);
	CMeshAdjuster*	AddMeshAdjuster(CMeshEditor* pcMeshEditor, CGraphicsInstance* pcGraphicsInstance, CMeshInstance* pcMeshInstance);
	void			ClearAllSelections(void);
	void			SetPointSize(float fPointSize);
};


#endif //__WORLD_EDITOR_H__

