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
#include "ConversionMapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageGraphicsTexture::Init(CGraphicsTexture* pcGraphicsTexture, D3DFORMAT d3dFormat)
{
	miMinusOne = -1;
	miInitialised = 1;
	u.mcOne.pcGraphicsTexture = pcGraphicsTexture;
	u.mcOne.d3dFormat = d3dFormat;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageGraphicsTexture::Kill(void)
{
	if (miMinusOne != -1)
	{
		u.mcArray.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CGraphicsTexture* CImageGraphicsTexture::GetGraphicsTexture(D3DFORMAT d3dFormat)
{
	int								i;
	SImageGraphicsTextureFormat*	psImageTexture;

	if (miMinusOne == -1)
	{
		if (u.mcOne.d3dFormat == d3dFormat)
		{
			return u.mcOne.pcGraphicsTexture;
		}
	}
	else
	{
		for (i = 0; i < u.mcArray.NumElements(); i++)
		{
			psImageTexture = u.mcArray.Get(i);
			if (psImageTexture->d3dFormat == d3dFormat)
			{
				return psImageTexture->pcGraphicsTexture;
			}
		}
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageGraphicsTexture::AddGraphicsTexture(CGraphicsTexture* pcGraphicsTexture, D3DFORMAT d3dFormat)
{
	CGraphicsTexture*				pcExisting;
	SImageGraphicsTextureFormat*	psImageTexture;
	CGraphicsTexture*				pcFirstGraphicsTexture;
	D3DFORMAT						d3dFirstFormat;

	pcExisting = GetGraphicsTexture(d3dFormat);
	if (!pcExisting)
	{
		if (miMinusOne == -1)
		{
			miMinusOne = 1;
			pcFirstGraphicsTexture = u.mcOne.pcGraphicsTexture;
			d3dFirstFormat = u.mcOne.d3dFormat;
			u.mcArray.Init();
			psImageTexture = u.mcArray.Add();
			psImageTexture->d3dFormat = d3dFirstFormat;
			psImageTexture->pcGraphicsTexture = pcFirstGraphicsTexture;
		}
		psImageTexture = u.mcArray.Add();
		psImageTexture->d3dFormat = d3dFormat;
		psImageTexture->pcGraphicsTexture = pcGraphicsTexture;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConversionMapper::Init(void)
{
	macInstances.Init(128);
	macMeshes.Init(128);
	macMaterials.Init(128);
	macTextures.Init(128);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConversionMapper::Kill(void)
{
	int						i;
	CImageGraphicsTexture*	pcImageTexture;

	macInstances.Kill();
	macMeshes.Kill();
	macMaterials.Kill();

	for (i = 0; i < macTextures.NumElements(); i++)
	{
		pcImageTexture = macTextures.Get(i);
		pcImageTexture->Kill();
	}
	macTextures.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConversionMapper::AddGraphicsObject(int iMeshID, CGraphicsObject* pcGraphicsObject, CMeshObject* pcMeshObject)
{
	SMeshIDToGraphicsObjectAndMeshObject*		psMeshObject;

	macMeshes.GrowToAtLeastNumElements(iMeshID+1, TRUE);

	psMeshObject = macMeshes.Get(iMeshID);
	psMeshObject->iMeshID = iMeshID;
	psMeshObject->pcGraphicsObject = pcGraphicsObject;
	psMeshObject->pcMeshObject = pcMeshObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConversionMapper::AddGraphicsMaterial(int iMaterialID, CGraphicsMaterial* pcGraphicsMaterial, CGraphicsState* pcGraphicsState)
{
	SMaterialIDToGraphicsMaterialAndGraphicsState*		psMaterialThing;

	if ((pcGraphicsMaterial == NULL) || (pcGraphicsState == NULL))
	{
		return;
	}

	macMaterials.GrowToAtLeastNumElements(iMaterialID+1, TRUE, -1);

	psMaterialThing = macMaterials.Get(iMaterialID);
	psMaterialThing->iMaterialID = iMaterialID;
	psMaterialThing->pcGraphicsState = pcGraphicsState;
	psMaterialThing->pcGraphicsMaterial= pcGraphicsMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConversionMapper::AddGraphicsInstance(int iSpecimenID, CGraphicsInstance* pcGraphicsInstance, CMeshInstance* pcMeshInstance)
{
	SSpecimenIDToGraphicsInstanceAndMeshInstance*		psInstanceInstance;

	macInstances.GrowToAtLeastNumElements(iSpecimenID+1, TRUE, -1);

	psInstanceInstance = macInstances.Get(iSpecimenID);
	psInstanceInstance->iSpecimenID = iSpecimenID;
	psInstanceInstance->pcGraphicsInstance = pcGraphicsInstance;
	psInstanceInstance->pcMeshInstance = pcMeshInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConversionMapper::AddImage(int iImageIndex, D3DFORMAT d3dFormat, CGraphicsTexture* pcGraphicsTexture)
{
	CImageGraphicsTexture*		pcImageTexture;

	if (iImageIndex == -1)
	{
		return FALSE;
	}

	pcImageTexture = macTextures.GrowToAtLeastNumElements(iImageIndex+1, TRUE, -1);
	if (pcImageTexture->miInitialised == 1)
	{
		pcImageTexture->AddGraphicsTexture(pcGraphicsTexture, d3dFormat);
	}
	else
	{
		pcImageTexture->Init(pcGraphicsTexture, d3dFormat);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConversionMapper::GetMaterial(int iMaterialIndex, CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState)
{
	SMaterialIDToGraphicsMaterialAndGraphicsState*	psMaterial;

	psMaterial = macMaterials.SafeGet(iMaterialIndex);
	if ((psMaterial) && (psMaterial->iMaterialID != -1))
	{
		*ppcGraphicsMaterial = psMaterial->pcGraphicsMaterial;
		*ppcGraphicsState = psMaterial->pcGraphicsState;
		return TRUE;
	}
	else
	{
		*ppcGraphicsMaterial = NULL;
		*ppcGraphicsState = NULL;
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConversionMapper::GetMesh(int iMeshIndex, CGraphicsObject** ppcGraphicsObject, CMeshObject** ppcMeshObject)
{
	SMeshIDToGraphicsObjectAndMeshObject*		psMesh;

	psMesh = macMeshes.SafeGet(iMeshIndex);
	if (psMesh)
	{
		*ppcGraphicsObject = psMesh->pcGraphicsObject;
		*ppcMeshObject = psMesh->pcMeshObject;
		return TRUE;
	}
	else
	{
		*ppcGraphicsObject = NULL;
		*ppcMeshObject = NULL;
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConversionMapper::GetImage(int iImageIndex, D3DFORMAT d3dFormat, CGraphicsTexture** ppcGraphicsTexture)
{
	CImageGraphicsTexture*		pcImageTexture;
	CGraphicsTexture*			pcTexture;

	pcImageTexture = macTextures.SafeGet(iImageIndex);
	if (pcImageTexture)
	{
		pcTexture = pcImageTexture->GetGraphicsTexture(d3dFormat);
		if (pcTexture)
		{
			*ppcGraphicsTexture = pcTexture;
			return TRUE;
		}
	}
	*ppcGraphicsTexture = NULL;
	return FALSE;
}

