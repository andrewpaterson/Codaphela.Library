/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela OldDragonLib

Codaphela OldDragonLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela OldDragonLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela OldDragonLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "OldDragonScene.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonMesh::Init(void)
{
	mszName[0] = 0;
	miID = OLD_DRAGON_NO_INDEX;
	macPositions.Init();
	macFaces.Init();
	macNormals.Init();
	macUVs.Init();
	macSkinVerts.Init();
	macSkinWeights.Init();
	macInverseSkinMatrices.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonMesh::Kill(void)
{
	macInverseSkinMatrices.Kill();
	macSkinWeights.Kill();
	macSkinVerts.Kill();
	macUVs.Kill();
	macNormals.Kill();
	macFaces.Kill();
	macPositions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonImage::Init(void)
{
	mszName[0] = 0;
	miID = OLD_DRAGON_NO_INDEX;
	miWidth = 0;
	miHeight = 0;
	miChannels = 0;
	mpuiPixels = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonImage::Kill(void)
{
	free(mpuiPixels);
	mpuiPixels = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COldDragonImage::SetPixels(const uint8* puiPixels, size uiSize)
{
	free(mpuiPixels);
	mpuiPixels = (uint8*)malloc(uiSize);
	if (mpuiPixels == NULL)
	{
		return false;
	}
	memcpy(mpuiPixels, puiPixels, uiSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonSequence::Init(void)
{
	mszName[0] = 0;
	miID = OLD_DRAGON_NO_INDEX;
	miConnection = OLD_DRAGON_NO_INDEX;
	macKeyFrames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonSequence::Kill(void)
{
	macKeyFrames.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonInstance::Init(void)
{
	mszName[0] = 0;
	miID = OLD_DRAGON_NO_INDEX;
	miType = OLD_DRAGON_NO_INDEX;
	miObject = OLD_DRAGON_NO_INDEX;
	maiConnections.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonInstance::Kill(void)
{
	maiConnections.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonScene::Init(void)
{
	meVersion = ODV_Unknown;
	mbDamaged = false;
	muiSkippedChunks = 0;
	macMeshes.Init();
	macImages.Init();
	macMaterials.Init();
	macLights.Init();
	macCameras.Init();
	macConnections.Init();
	macSequences.Init();
	macInstances.Init();
	macParticles.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COldDragonScene::Kill(void)
{
	size	i;

	macParticles.Kill();

	for (i = 0; i < macInstances.NumElements(); i++)
	{
		macInstances.Get(i)->Kill();
	}
	macInstances.Kill();

	for (i = 0; i < macSequences.NumElements(); i++)
	{
		macSequences.Get(i)->Kill();
	}
	macSequences.Kill();

	macConnections.Kill();
	macCameras.Kill();
	macLights.Kill();
	macMaterials.Kill();

	for (i = 0; i < macImages.NumElements(); i++)
	{
		macImages.Get(i)->Kill();
	}
	macImages.Kill();

	for (i = 0; i < macMeshes.NumElements(); i++)
	{
		macMeshes.Get(i)->Kill();
	}
	macMeshes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COldDragonMesh* COldDragonScene::AddMesh(void)
{
	COldDragonMesh*	pcMesh;

	pcMesh = macMeshes.Add();
	pcMesh->Init();
	return pcMesh;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COldDragonImage* COldDragonScene::AddImage(void)
{
	COldDragonImage*	pcImage;

	pcImage = macImages.Add();
	pcImage->Init();
	return pcImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SOldDragonMaterial* COldDragonScene::AddMaterial(void)
{
	SOldDragonMaterial*	psMaterial;
	int32				i;

	psMaterial = macMaterials.Add();
	memset(psMaterial, 0, sizeof(SOldDragonMaterial));
	psMaterial->iID = OLD_DRAGON_NO_INDEX;
	for (i = 0; i < OLD_DRAGON_MAX_TEXTURES; i++)
	{
		psMaterial->aiTextures[i] = OLD_DRAGON_NO_INDEX;
	}
	return psMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SOldDragonLight* COldDragonScene::AddLight(void)
{
	SOldDragonLight*	psLight;

	psLight = macLights.Add();
	memset(psLight, 0, sizeof(SOldDragonLight));
	psLight->iID = OLD_DRAGON_NO_INDEX;
	return psLight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SOldDragonCamera* COldDragonScene::AddCamera(void)
{
	SOldDragonCamera*	psCamera;

	psCamera = macCameras.Add();
	memset(psCamera, 0, sizeof(SOldDragonCamera));
	psCamera->iID = OLD_DRAGON_NO_INDEX;
	return psCamera;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SOldDragonConnection* COldDragonScene::AddConnection(void)
{
	SOldDragonConnection*	psConnection;

	psConnection = macConnections.Add();
	memset(psConnection, 0, sizeof(SOldDragonConnection));
	psConnection->iID = OLD_DRAGON_NO_INDEX;
	return psConnection;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COldDragonSequence* COldDragonScene::AddSequence(void)
{
	COldDragonSequence*	pcSequence;

	pcSequence = macSequences.Add();
	pcSequence->Init();
	return pcSequence;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
COldDragonInstance* COldDragonScene::AddInstance(void)
{
	COldDragonInstance*	pcInstance;

	pcInstance = macInstances.Add();
	pcInstance->Init();
	return pcInstance;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IndexInRange(int32 iIndex, size uiNum, bool bMayBeNone)
{
	if (iIndex == OLD_DRAGON_NO_INDEX)
	{
		return bMayBeNone;
	}
	return (iIndex >= 0) && ((size)iIndex < uiNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool ValidateOldDragonMesh(COldDragonMesh* pcMesh)
{
	size					i;
	int32					j;
	SOldDragonFace*			psFace;
	SOldDragonSkinVert*		psSkinVert;
	SOldDragonSkinWeight*	psSkinWeight;

	if ((pcMesh->macSkinVerts.NumElements() != 0) && (pcMesh->macSkinVerts.NumElements() != pcMesh->macPositions.NumElements()))
	{
		return false;
	}
	for (i = 0; i < pcMesh->macSkinVerts.NumElements(); i++)
	{
		psSkinVert = pcMesh->macSkinVerts.Get(i);
		if ((psSkinVert->iFirstWeight < 0) || (psSkinVert->iNumWeights < 0) || ((size)psSkinVert->iFirstWeight + (size)psSkinVert->iNumWeights > pcMesh->macSkinWeights.NumElements()))
		{
			return false;
		}
	}
	for (i = 0; i < pcMesh->macSkinWeights.NumElements(); i++)
	{
		psSkinWeight = pcMesh->macSkinWeights.Get(i);
		if (!IndexInRange(psSkinWeight->iMatrix, pcMesh->macInverseSkinMatrices.NumElements(), false))
		{
			return false;
		}
	}

	for (i = 0; i < pcMesh->macFaces.NumElements(); i++)
	{
		psFace = pcMesh->macFaces.Get(i);
		for (j = 0; j < 3; j++)
		{
			if (!IndexInRange(psFace->aiPositions[j], pcMesh->macPositions.NumElements(), false))
			{
				return false;
			}
			if (!IndexInRange(psFace->aiNormals[j], pcMesh->macNormals.NumElements(), true))
			{
				return false;
			}
			if (!IndexInRange(psFace->aiUVs[j], pcMesh->macUVs.NumElements(), true))
			{
				return false;
			}
		}
	}
	return true;
}

