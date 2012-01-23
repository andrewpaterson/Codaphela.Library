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
#include "Scene.h"
#include "MaterialHelper.h"
#include "ImageRGBToGrey.h"
#include "ImageResampler.h"


int giSceneID = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScene::Init(void)
{
	mcMeshTracker.Init();
	mcLightTracker.Init();
	mcCameraTracker.Init();
	mcImageTracker.Init();
	mcMaterialTracker.Init();
	mcConnectionTracker.Init();
	mcSequenceTracker.Init();
	mcInstanceTracker.Init();
	iUniqueID = giSceneID;
	giSceneID++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CScene::Kill(void)
{
	mcInstanceTracker.Kill();
	mcSequenceTracker.Kill();
	mcMaterialTracker.Kill();
	mcCameraTracker.Kill();
	mcLightTracker.Kill();
	mcMeshTracker.Kill();
	mcImageTracker.Kill();
	mcConnectionTracker.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CScene::Load(CFileReader* pcChunkFile)
{
	mcMeshTracker.Load(pcChunkFile);
	mcLightTracker.Load(pcChunkFile);
	mcCameraTracker.Load(pcChunkFile);
	mcMaterialTracker.Load(pcChunkFile);
	mcImageTracker.Load(pcChunkFile);
	mcConnectionTracker.Load(pcChunkFile);
	mcSequenceTracker.Load(pcChunkFile);
	mcInstanceTracker.Load(pcChunkFile);

	return Touch();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CScene::Touch(void)
{
	int					i;
	CMesh*				pcMesh;
	CImage*				pcImage;
	CImageRGBToGrey		cImageRGBToGrey;
	CMaterial*			pcMaterial;
	CMaterialHelper		cMaterialHelper;
	BOOL				bResult;
	CImageResampler		cImageResampler;
	
	//First do the images... this is important.
	for (i = 0; i < mcImageTracker.Num(); i++)
	{
		pcImage = mcImageTracker.Get(i);
		
		cImageRGBToGrey.Init(RGBTGS_OnlyIfChannelsSame);
		cImageRGBToGrey.Modify(pcImage);
		cImageRGBToGrey.Kill();

		cImageResampler.Init(IR_NearestNeighbour, RS_Auto);
		cImageResampler.Modify(pcImage);
		cImageResampler.Kill();
	}

	//Then do the materials.
	for (i = 0; i < mcMaterialTracker.Num(); i++)
	{
		pcMaterial = mcMaterialTracker.Get(i);

		cMaterialHelper.Init(pcMaterial);
		cMaterialHelper.Touch(&mcImageTracker);
		cMaterialHelper.Kill();
	}

	bResult = TRUE;
	for (i = 0; i < mcMeshTracker.Num(); i++)
	{
		pcMesh = mcMeshTracker.Get(i);

		bResult &= pcMesh->Touch();
	}
	return bResult;
}
