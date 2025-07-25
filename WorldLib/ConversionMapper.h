/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SceneLib

Codaphela SceneLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SceneLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SceneLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __CONVERSION_MAPPER_H__
#define __CONVERSION_MAPPER_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/IndexedGeneral.h"
#include "DirectXLib/Direct3DGlobal.h"
#include "WorldLib/GraphicsInstance.h"
#include "WorldLib/GraphicsTexture.h"
#include "WorldLib/MeshInstance.h"


struct SMaterialIDToGraphicsMaterialAndGraphicsState
{
	OIndex			iMaterialID;
	CGraphicsMaterial*	pcGraphicsMaterial;
	CGraphicsState*		pcGraphicsState;
};


struct SMeshIDToGraphicsObjectAndMeshObject
{
	OIndex				iMeshID;
	CGraphicsObject*	pcGraphicsObject;
	CMeshObject*		pcMeshObject;
};


struct SImageGraphicsTextureFormat
{
	CGraphicsTexture*	pcGraphicsTexture;
	D3DFORMAT			d3dFormat;
};
typedef CArrayTemplateMinimal<SImageGraphicsTextureFormat> CArrayImageGraphicsTextureFormat;


class CImageGraphicsTexture
{
public:
	int		miMinusOne;  //If this is -1 then the array is not in use.
	int		miInitialised;
	union
	{
		CArrayImageGraphicsTextureFormat	mcArray;
		SImageGraphicsTextureFormat			mcOne;
	} u;

	void 				Init(CGraphicsTexture* pcGraphicsTexture, D3DFORMAT d3dFormat);
	void 				Kill(void);
	CGraphicsTexture*	GetGraphicsTexture(D3DFORMAT d3dFormat);
	void				AddGraphicsTexture(CGraphicsTexture* pcGraphicsTexture, D3DFORMAT d3dFormat);
};


struct SSpecimenIDToGraphicsInstanceAndMeshInstance
{
	OIndex				iSpecimenID;
	CGraphicsInstance*	pcGraphicsInstance;
	CMeshInstance*		pcMeshInstance;
};


typedef CArrayTemplate<SMaterialIDToGraphicsMaterialAndGraphicsState>	CArrayMaterialIDToGraphicsMaterialAndGraphicsState;
typedef CArrayTemplate<SMeshIDToGraphicsObjectAndMeshObject>			CArrayMeshIDToGraphicsObjectAndMeshObject;
typedef CArrayTemplate<SSpecimenIDToGraphicsInstanceAndMeshInstance>	CArraySpecimenIDToGraphicsInstanceAndMeshInstance;
typedef CArrayTemplate<CImageGraphicsTexture>							CArrayImageTexture;


class CConversionMapper
{
public:
	CArraySpecimenIDToGraphicsInstanceAndMeshInstance	macInstances;	//Instance -> GraphicsInstance & MeshInstance
	CArrayMeshIDToGraphicsObjectAndMeshObject			macMeshes;		//Mesh -> GraphicsObject & MeshObject
	CArrayMaterialIDToGraphicsMaterialAndGraphicsState	macMaterials;	//Material -> GraphicsMaterial & GraphicsState
	CArrayImageTexture									macTextures;	//Image -> GraphicsTexture[]

	void Init(void);
	void Kill(void);

	void AddGraphicsObject(OIndex iMeshID, CGraphicsObject* pcGraphicsObject, CMeshObject* pcMeshObject);
	void AddGraphicsMaterial(OIndex iMaterialID, CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState);
	void AddGraphicsInstance(OIndex iSpecimenID, CGraphicsInstance* pcGraphicsInstance, CMeshInstance* pcMeshInstance);
	bool AddImage(OIndex iImageIndex, D3DFORMAT d3dFormat, CGraphicsTexture* pcGraphicsTexture);

	bool GetMaterial(OIndex iMaterialID, CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState);
	bool GetMesh(OIndex iMeshID, CGraphicsObject** ppcGraphicsObject, CMeshObject** ppcMeshObject);
	bool GetImage(OIndex iImageIndex, D3DFORMAT d3dFormat, CGraphicsTexture** ppcGraphicsTexture);
};


#endif // __CONVERSION_MAPPER_H__

