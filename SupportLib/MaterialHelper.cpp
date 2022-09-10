/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MeshLib

Codaphela MeshLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MeshLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MeshLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "MaterialHelper.h"
#include "BumpMapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialHelper::Init(CMaterial* pcMaterial)
{
	mpcMaterial = pcMaterial;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialHelper::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialHelper::Touch(void)
{
	//CImage*		pcImage;

	//if (mpcMaterial->msSpecular.iImageID != -1)
	//{
	//	pcImage = pcImageTracker->GetWithID(mpcMaterial->msSpecular.iImageID);
	//	if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
	//	{
	//		pcImage->RenameChannel(IMAGE_DIFFUSE_GREY, IMAGE_SPECULAR);
	//	}
	//}

	//if (mpcMaterial->msIllumination.iImageID != -1)
	//{
	//	pcImage = pcImageTracker->GetWithID(mpcMaterial->msIllumination.iImageID);
	//	if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
	//	{
	//		pcImage->RenameChannel(IMAGE_DIFFUSE_GREY, IMAGE_ILLUMINATION);
	//	}
	//	else if (pcImage->HasChannels(IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_ZERO))
	//	{
	//		pcImage->RenameChannel(IMAGE_DIFFUSE_RED, IMAGE_ILLUMINATION_RED);
	//		pcImage->RenameChannel(IMAGE_DIFFUSE_GREEN, IMAGE_ILLUMINATION_GREEN);
	//		pcImage->RenameChannel(IMAGE_DIFFUSE_BLUE, IMAGE_ILLUMINATION_BLUE);
	//	}
	//}

	//if (mpcMaterial->msOpacity.iImageID != -1)
	//{
	//	pcImage = pcImageTracker->GetWithID(mpcMaterial->msOpacity.iImageID);
	//	if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
	//	{
	//		pcImage->RenameChannel(IMAGE_DIFFUSE_GREY, IMAGE_OPACITY);
	//	}
	//}

	//if (mpcMaterial->msBump.iImageID != -1)
	//{
	//	pcImage = pcImageTracker->GetWithID(mpcMaterial->msBump.iImageID);
	//	if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
	//	{
	//		ConvertHeightMapTo(true, false, pcImage, pcImage, IMAGE_DIFFUSE_GREY);
	//	}
	//}
}

