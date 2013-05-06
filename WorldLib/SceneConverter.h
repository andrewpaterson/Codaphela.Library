/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __SCENE_CONVERTER_H__
#define __SCENE_CONVERTER_H__
#include "MaterialConverter.h"
#include "MeshConverter.h"
#include "AnimationConverter.h"
#include "InstanceConverter.h"
#include "TextureConverter.h"
#include "ConversionMapper.h"
#include "World.h"


class CScene;
class CSceneConverter
{
BASE_FUNCTIONS(CSceneConverter);
protected:
	CMaterialConverter		mcMaterialConverter;
	CMeshConverter			mcMeshConverter;
	CAnimationConverter		mcAnimationConverter;
	CInstanceConverter		mcInstanceConverter;
	CTextureConverter		mcTextureConverter;
	CConversionMapper		mcMapper;
	CWorld*					mpcWorld;
	CScene*					mpcScene;

public:
	void				Init(CScene* pcScene, CWorld* pcWorld);
	void				Kill(void);

	BOOL				ConvertTexture(CGraphicsTexture** ppcGraphicsTexture, D3DFORMAT d3dFormat, CImage* pcImage, BOOL bDiscard, int iWidth, int iHeight, EPrimitiveTypes eType, EChannel eFirst, ...);
	BOOL				ConvertMeshToGraphicsObject(CGraphicsObject** pcGraphicsObject, CMeshObject** ppcMeshObject, CMesh* pcMesh, CArrayPointer* pcConnectionAndIndex);
	BOOL				ConvertMaterialToGraphicsMaterial(CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState, CMaterial* pcMaterial);
	BOOL				ConvertSequenceToMovement(CMovement** ppcMovement, CSequence* pcSequence, SMatrix* psMatrix);
	BOOL				ConvertInstanceToGraphicsInstance(CInstance* pcInstance);
	BOOL				ConvertMeshes(void);
	BOOL				ConvertScene(void);

	CWorld*				GetWorld(void);
	CScene*				GetScene(void);
	CConversionMapper*	GetMapper(void);
};


#endif // __SCENE_CONVERTER_H__

