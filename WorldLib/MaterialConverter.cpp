/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BaseLib/PointerFunctions.h"
#include "CoreLib/Operators.h"
#include "SupportLib/BumpMapper.h"
#include "SupportLib/ImageAccessorCreator.h"
#include "SupportLib/ImageAccessor.h"
#include "SupportLib/Image.h"
#include "SupportLib/ImageWriter.h"
#include "SceneConverter.h"
#include "MaterialConverter.h"


#define TRANSPARENCY	5
#define TRANSLUCENCY	250


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialConverter::Init(CSceneConverter* pcSceneConverter)
{
	mpcSceneConverter = pcSceneConverter;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialConverter::Kill(void)
{
	mpcSceneConverter = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMaterialConverter::Convert(CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState, CMaterial* pcMaterial)
{
	BOOL	bResult;

	mpcMaterial = pcMaterial;

	if (pcMaterial)
	{
		if (mpcSceneConverter->GetMapper()->GetMaterial(pcMaterial->GetID(), ppcGraphicsMaterial, ppcGraphicsState))
		{
			return TRUE;
		}

		bResult = CompleteConversion(ppcGraphicsMaterial, ppcGraphicsState);
		return bResult;
	}
	else
	{
		*ppcGraphicsMaterial = mpcSceneConverter->GetWorld()->GetGreyGraphicsMaterial();
		*ppcGraphicsState = mpcSceneConverter->GetWorld()->GetGreyGraphicsState();
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EImageOpacity CMaterialConverter::UpdateOpacity(EImageOpacity eOldOpacity, CImage* pcImage)
{
	EImageOpacity	eOpacity;

	eOpacity = GetOpacity(pcImage);
	if (eOpacity > eOldOpacity)
	{
		return eOpacity;
	}
	return eOldOpacity;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CMaterialConverter::CompleteConversion(CGraphicsMaterial** ppcGraphicsMaterial, CGraphicsState** ppcGraphicsState)
{
	CGraphicsState*			pcGraphicsState;
	CGraphicsMaterial*		pcGraphicsMaterial;
	CGraphicsTexture*		pcGraphicsTextureDiffuse;
	CGraphicsTexture*		pcGraphicsTextureDiffuseLevel;
	CGraphicsTexture*		pcGraphicsTextureBaseMask;
	CGraphicsTexture*		pcGraphicsTextureSpecular;
	CGraphicsTexture*		pcGraphicsTextureIllumination;
	CGraphicsTexture*		pcGraphicsTextureOpacity;
	CGraphicsTexture*		pcGraphicsTextureNormal;
	CGraphicsTexture*		pcGraphicsTextureBump;
	CGraphicsTexture*		pcGraphicsTextureDetail;
	CGraphicsTexture*		pcGraphicsTextureDecal;
	CGraphicsTexture*		pcGraphicsTextureDecalMask;
	CGraphicsTexture*		pcGraphicsTextureReflection;
	CImage*					pcImage;
	CImage					cExport;
	EImageOpacity			eOpacity;
	BOOL					bResult;

	pcGraphicsState = mpcSceneConverter->GetWorld()->CreateGraphicsState();
	pcGraphicsMaterial = mpcSceneConverter->GetWorld()->CreateGraphicsMaterial();

	pcGraphicsTextureDiffuse = NULL;
	pcGraphicsTextureDiffuseLevel = NULL;
	pcGraphicsTextureBaseMask = NULL;
	pcGraphicsTextureSpecular = NULL;
	pcGraphicsTextureIllumination = NULL;
	pcGraphicsTextureOpacity = NULL;
	pcGraphicsTextureNormal = NULL;
	pcGraphicsTextureBump = NULL;
	pcGraphicsTextureDetail = NULL;
	pcGraphicsTextureDecal = NULL;
	pcGraphicsTextureDecalMask = NULL;
	pcGraphicsTextureReflection = NULL;

	eOpacity = IO_Opaque;

	bResult = TRUE;

	// **** Diffuse (and Opacity) ****
	if (mpcMaterial->msDiffuse.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msDiffuse.iImageID);
		char* szName = pcImage->ClassName();
		if (pcImage->HasChannels(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_ZERO))
		{
			eOpacity = UpdateOpacity(eOpacity, pcImage);
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDiffuse, D3DFMT_A8R8G8B8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, IMAGE_OPACITY, CHANNEL_ZERO);
		}
		else if (pcImage->HasChannels(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_ZERO))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDiffuse, D3DFMT_X8R8G8B8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL(IP_Diffuse, CT_Ignored), CHANNEL_ZERO);
		}
		else if (pcImage->HasChannels(IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, CHANNEL_ZERO))
		{
			eOpacity = UpdateOpacity(eOpacity, pcImage);
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDiffuse, D3DFMT_A8L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_GREY, IMAGE_OPACITY, CHANNEL_ZERO);
		}
		else if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDiffuse, D3DFMT_L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_GREY, CHANNEL_ZERO);
		}
	}

	// **** Specular ****
	if (mpcMaterial->msSpecular.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msSpecular.iImageID);
		if (pcImage->HasChannel(IMAGE_SPECULAR))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureSpecular, D3DFMT_L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_SPECULAR, CHANNEL_ZERO);
		}	
	}

	// **** Diffuse Level ****
	if (mpcMaterial->msDiffuseLevel.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msDiffuseLevel.iImageID);
		if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDiffuseLevel , D3DFMT_L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_GREY, CHANNEL_ZERO);
		}	
	}

	// **** Illumination ****
	if (mpcMaterial->msIllumination.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msIllumination.iImageID);
		if (pcImage->HasChannels(IMAGE_ILLUMINATION_BLUE, IMAGE_ILLUMINATION_GREEN, IMAGE_ILLUMINATION_RED, CHANNEL_ZERO))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureIllumination, D3DFMT_X8R8G8B8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_ILLUMINATION_BLUE, IMAGE_ILLUMINATION_GREEN, IMAGE_ILLUMINATION_RED, CHANNEL(IP_Illumination, CT_Ignored), CHANNEL_ZERO);
		}		
		else if (pcImage->HasChannel(IMAGE_ILLUMINATION))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureIllumination, D3DFMT_L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_ILLUMINATION, CHANNEL_ZERO);
		}
	}

	// **** Opacity ****
	if (mpcMaterial->msOpacity.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msOpacity.iImageID);
		if (pcImage->HasChannel(IMAGE_OPACITY))
		{
			eOpacity = UpdateOpacity(eOpacity, pcImage);
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureOpacity, D3DFMT_A8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_OPACITY, CHANNEL_ZERO);
		}
	}

	// **** Normal Map ****
	if (mpcMaterial->msBump.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msBump.iImageID);
		if (pcImage->HasChannels(IMAGE_NORMAL_Z, IMAGE_NORMAL_Y, IMAGE_NORMAL_X, CHANNEL_ZERO))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureNormal, D3DFMT_X8R8G8B8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_NORMAL_Z, IMAGE_NORMAL_Y, IMAGE_NORMAL_X, CHANNEL(IP_Normal, CT_Ignored), CHANNEL_ZERO);
		}
	}

	// **** Detail Map ****
	if (mpcMaterial->msDetail.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msDetail.iImageID);
		if (pcImage->HasChannels(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_ZERO))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDetail, D3DFMT_X8R8G8B8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL(IP_Diffuse, CT_Ignored), CHANNEL_ZERO);
		}
		else if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDetail, D3DFMT_L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar, IMAGE_DIFFUSE_GREY, CHANNEL_ZERO);
		}
	}

	// **** Decal ****
	if (mpcMaterial->msDecal.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msDecal.iImageID);
		if (pcImage->HasChannels(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_ZERO))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDecal, D3DFMT_X8R8G8B8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL(IP_Diffuse, CT_Ignored), CHANNEL_ZERO);
		}
		else if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDecal, D3DFMT_L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_GREY, CHANNEL_ZERO);
		}
		if (pcImage->HasChannel( IMAGE_MASK))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureDecalMask, D3DFMT_A8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_MASK, CHANNEL_ZERO);
		}
	}

	// **** Reflection ****
	if (mpcMaterial->msReflection.iImageID != -1)
	{
		pcImage = mpcSceneConverter->GetScene()->mcImageTracker.GetWithID(mpcMaterial->msReflection.iImageID);
		if (pcImage->HasChannels(IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL_ZERO))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureReflection, D3DFMT_X8R8G8B8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar,	IMAGE_DIFFUSE_BLUE, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_RED, CHANNEL(IP_Diffuse, CT_Ignored), CHANNEL_ZERO);
		}
		else if (pcImage->HasChannel(IMAGE_DIFFUSE_GREY))
		{
			bResult &= mpcSceneConverter->ConvertTexture(&pcGraphicsTextureReflection, D3DFMT_L8, pcImage, FALSE, pcImage->miWidth, pcImage->miHeight, PT_uchar, IMAGE_DIFFUSE_GREY, CHANNEL_ZERO);
		}
	}

	if (!bResult)
	{
		return FALSE;
	}

	ConvertMaterialProperties(pcGraphicsMaterial, &mpcMaterial->msProperties, eOpacity);

	int					iTexture;
	D3DTEXTUREOP		iDetailOp;
	int					iLighting;
	D3DTEXTUREOP		iLightingOp;

	iLightingOp = D3DTOP_SELECTARG2;
	iTexture = 0;
	if (pcGraphicsTextureNormal)
	{
		pcGraphicsMaterial->SetTexture(iTexture, pcGraphicsTextureNormal);
		ConvertSamplerState(pcGraphicsState->GetSamplerState(iTexture), &mpcMaterial->msDiffuse);
		pcGraphicsState->GetStageState(iTexture)->iColourOp = D3DTOP_DOTPRODUCT3;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_DIFFUSE;
		pcGraphicsState->GetStageState(iTexture)->iColourArg2 = D3DTA_TEXTURE;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_TEMP;
		pcGraphicsState->GetStageState(iTexture)->iTexCoord = mpcMaterial->msBump.eUVSource;
		iLightingOp = D3DTOP_MODULATE;
		iLighting = D3DTA_TEMP;
		iTexture++;
	}
	else
	{
		iLighting = D3DTA_DIFFUSE;
	}

	//Illumination goes somewhere around here.

	if ((pcGraphicsTextureDiffuse) && (pcGraphicsTextureDiffuseLevel))
	{
		pcGraphicsMaterial->SetTexture(iTexture, pcGraphicsTextureDiffuseLevel);
		ConvertSamplerState(pcGraphicsState->GetSamplerState(iTexture), &mpcMaterial->msDiffuseLevel);
		pcGraphicsState->GetStageState(iTexture)->iColourOp = D3DTOP_SELECTARG1;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_TEXTURE;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_CURRENT;  //Ignored
		pcGraphicsState->GetStageState(iTexture)->iTexCoord = mpcMaterial->msDiffuseLevel.eUVSource;
		iTexture++;

		pcGraphicsMaterial->SetTexture(iTexture, pcGraphicsTextureDiffuse);
		ConvertSamplerState(pcGraphicsState->GetSamplerState(iTexture), &mpcMaterial->msDiffuse);
		pcGraphicsState->GetStageState(iTexture)->iColourOp = D3DTOP_MODULATE;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_TEXTURE;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_CURRENT;
		pcGraphicsState->GetStageState(iTexture)->iTexCoord = mpcMaterial->msDiffuse.eUVSource;
		iTexture++;

		iLightingOp = D3DTOP_MODULATE;
		iDetailOp = D3DTOP_ADDSIGNED;

	}
	else if (pcGraphicsTextureDiffuse)
	{
		pcGraphicsMaterial->SetTexture(iTexture, pcGraphicsTextureDiffuse);
		ConvertSamplerState(pcGraphicsState->GetSamplerState(iTexture), &mpcMaterial->msDiffuse);
		pcGraphicsState->GetStageState(iTexture)->iColourOp = D3DTOP_SELECTARG1;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_TEXTURE;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_CURRENT;  //Ignored
		pcGraphicsState->GetStageState(iTexture)->iTexCoord = mpcMaterial->msDiffuse.eUVSource;
		iTexture++;

		iLightingOp = D3DTOP_MODULATE;
		iDetailOp = D3DTOP_ADDSIGNED;
	}
	else if (pcGraphicsTextureDiffuseLevel)
	{
		pcGraphicsMaterial->SetTexture(iTexture, pcGraphicsTextureDiffuseLevel);
		ConvertSamplerState(pcGraphicsState->GetSamplerState(iTexture), &mpcMaterial->msDiffuseLevel);
		pcGraphicsState->GetStageState(iTexture)->iColourOp = D3DTOP_SELECTARG1;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_TEXTURE;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_CURRENT;  //Ignored
		pcGraphicsState->GetStageState(iTexture)->iTexCoord = mpcMaterial->msDiffuseLevel.eUVSource;
		iTexture++;

		iLightingOp = D3DTOP_MODULATE;
		iDetailOp = D3DTOP_ADDSIGNED;
	}
	else
	{
		iDetailOp = D3DTOP_SELECTARG2;
	}

	if (pcGraphicsTextureDetail)
	{
		pcGraphicsMaterial->SetTexture(iTexture, pcGraphicsTextureDetail);
		ConvertSamplerState(pcGraphicsState->GetSamplerState(iTexture), &mpcMaterial->msDetail);
		pcGraphicsState->GetStageState(iTexture)->iColourOp = iDetailOp;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_CURRENT;
		pcGraphicsState->GetStageState(iTexture)->iColourArg2 = D3DTA_TEXTURE;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_CURRENT;
		pcGraphicsState->GetStageState(iTexture)->iTexCoord = mpcMaterial->msDetail.eUVSource;
		iTexture++;

		iLightingOp = D3DTOP_MODULATE;
	}

	pcGraphicsState->GetStageState(iTexture)->iColourOp = iLightingOp;
	pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_CURRENT;
	pcGraphicsState->GetStageState(iTexture)->iColourArg2 = iLighting;
	pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_CURRENT;  //Lighting is in current now.
	pcGraphicsState->GetStageState(iTexture)->iTexCoord = 0;
	if (iLighting == D3DTA_TEMP)
	{
		iTexture++;
	}

	if (pcGraphicsTextureSpecular)
	{
		pcGraphicsMaterial->SetTexture(iTexture, pcGraphicsTextureSpecular);
		ConvertSamplerState(pcGraphicsState->GetSamplerState(iTexture), &mpcMaterial->msSpecular);
		pcGraphicsState->GetStageState(iTexture)->iColourOp = D3DTOP_MODULATE;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_TEXTURE;
		pcGraphicsState->GetStageState(iTexture)->iColourArg2 = D3DTA_SPECULAR;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_TEMP;  //D3DPMISCCAPS_TSSARGTEMP
		pcGraphicsState->GetStageState(iTexture)->iTexCoord = 0;
		iTexture++;

		pcGraphicsState->GetStageState(iTexture)->iColourOp = D3DTOP_ADD;
		pcGraphicsState->GetStageState(iTexture)->iColourArg1 = D3DTA_CURRENT;
		pcGraphicsState->GetStageState(iTexture)->iColourArg2 = D3DTA_TEMP;
		pcGraphicsState->GetStageState(iTexture)->iResultArg = D3DTA_CURRENT;
		iTexture++;
	}

	pcGraphicsMaterial->SetNumTextures(iTexture);
	pcGraphicsState->SetNumTextures(iTexture);  
	pcGraphicsState->SetNumSamplers(iTexture);
	pcGraphicsState->SetDefaultSamplerStates(D3DTEXF_LINEAR);

	SafeAssign(ppcGraphicsMaterial, pcGraphicsMaterial);
	SafeAssign(ppcGraphicsState, pcGraphicsState);

	//Setup the scene
	mpcSceneConverter->GetMapper()->AddGraphicsMaterial(mpcMaterial->GetID(), pcGraphicsMaterial, pcGraphicsState);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialConverter::ConvertSamplerState(SSamplerState* psSamplerState, CMaterialImage* psMaterialImage)
{
	psSamplerState->iAddressU = ConvertTextureAddress(psMaterialImage->eAddressU);
	psSamplerState->iAddressV = ConvertTextureAddress(psMaterialImage->eAddressV);
	psSamplerState->iMagFilter = ConvertTextureFilterType(psMaterialImage->eMagFilter);
	psSamplerState->iMinFilter = ConvertTextureFilterType(psMaterialImage->eMinFilter);
	psSamplerState->iMipFilter = ConvertTextureFilterType(psMaterialImage->eMipFilter);
	psSamplerState->fMipmapLodBias = 0.0f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
_D3DTEXTUREADDRESS CMaterialConverter::ConvertTextureAddress(EUVAddress eUVAddress)
{
	if (eUVAddress == UVA_Tile)
	{
		return D3DTADDRESS_WRAP;
	}
	else if (eUVAddress == UVA_Mirror)
	{
		return D3DTADDRESS_MIRROR;
	}
	else if (eUVAddress == UVA_Clamp)
	{
		return D3DTADDRESS_CLAMP;
	}
	else if (eUVAddress == UVA_Border)
	{
		return D3DTADDRESS_BORDER;
	}
	return D3DTADDRESS_WRAP;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
_D3DTEXTUREFILTERTYPE CMaterialConverter::ConvertTextureFilterType(EUVFilter eUVFilter)
{
	if (eUVFilter == UVF_Linear)
	{
		return D3DTEXF_LINEAR;
	}
	else if (eUVFilter == UVF_Nearest)
	{
		return D3DTEXF_POINT;
	}
	else if (eUVFilter == UVF_Anisotropic)
	{
		return D3DTEXF_ANISOTROPIC;
	}
	else if (eUVFilter == UVF_Gaussian)
	{
		return D3DTEXF_GAUSSIANQUAD;
	}
	else if (eUVFilter == UVF_Pyramidal)
	{
		return D3DTEXF_PYRAMIDALQUAD;
	}
	return D3DTEXF_LINEAR;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EMaterialOpacity CMaterialConverter::GetOpacity(int iValue)
{
	if (iValue <= TRANSLUCENCY)
	{
		if (iValue > TRANSPARENCY)
		{
			return MO_Translucent;
		}
		else
		{
			return MO_Transparent;
		}
	}
	return MO_Opaque;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EImageOpacity CMaterialConverter::GetOpacity(CImage* pcImage)
{
	int						x, y;
	int						iOpaqueCount;
	int						iTranslucentCount;
	int						iTransparentCount;
	CImageAccessor*			pcAccessor;
	SImageColour*			psAlpha;
	BOOL					bLower;
	BOOL					bUpper;
	EPrimitiveTypes			eChannelType;
	unsigned char			ucTransparency;
	unsigned char			ucTranslucency;

	//Turn this into a proper histogram style of thing.
	iOpaqueCount = 0;
	iTranslucentCount = 0;
	iTransparentCount = 0;
	ucTransparency = TRANSPARENCY;
	ucTranslucency = TRANSLUCENCY;

	if (!pcImage->HasChannel(IMAGE_OPACITY))
	{
		return IO_Opaque;
	}

	pcAccessor = CImageAccessorCreator::Create(pcImage, IMAGE_OPACITY, CHANNEL_ZERO);
	eChannelType = pcAccessor->GetType();

	for (y = 0; y < pcImage->miHeight; y++)
	{
		for (x = 0; x < pcImage->miWidth; x++)
		{
			psAlpha = (SImageColour *)pcAccessor->Get(x, y);

			gcOperators.mcLessOrEqual.Do(&bLower, eChannelType, psAlpha, PT_uchar, &ucTransparency);
			gcOperators.mcGreaterOrEqual.Do(&bUpper, eChannelType, psAlpha, PT_uchar, &ucTranslucency);
			if (bLower)
			{
				iTransparentCount++;
			}
			else if (bUpper)
			{
				iOpaqueCount++;
			}
			else
			{
				iTranslucentCount++;
			}
		}
	}

	pcAccessor->Kill();

	if (iTranslucentCount > 0)
	{
		return IO_Translucent;
	}
	else if (iTransparentCount > 0)
	{
		return IO_Transparent;
	}
	else 
	{
		return IO_Opaque;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CMaterialConverter::ConvertMaterialProperties(CGraphicsMaterial* pcGraphicsMaterial, CMaterialProperties* psMaterialPropteries, EImageOpacity eOpacity)
{
	pcGraphicsMaterial->SetDiffuse(&psMaterialPropteries->sDiffuseColour);
	pcGraphicsMaterial->SetAmbient(&psMaterialPropteries->sAmbientColour);
	pcGraphicsMaterial->SetEmissive(&psMaterialPropteries->sEmissiveColour);
	pcGraphicsMaterial->SetSpecular(&psMaterialPropteries->sSpecularColour);
	pcGraphicsMaterial->SetPower(psMaterialPropteries->fShininessBase * 100.0f);
	pcGraphicsMaterial->ModulateSpecular(psMaterialPropteries->fShininessPower * 1.0f);
	pcGraphicsMaterial->SetRenderState(D3DRS_LIGHTING, psMaterialPropteries->bLit);

	if (eOpacity == IO_Translucent)
	{
		psMaterialPropteries->meOpacity = MO_Translucent;
	}
	else
	{
		psMaterialPropteries->meOpacity = GetOpacity((int)(psMaterialPropteries->sDiffuseColour.a*255));
		if (psMaterialPropteries->meOpacity != MO_Translucent)
		{
			if (eOpacity == IO_Transparent)
			{
				psMaterialPropteries->meOpacity = MO_Transparent;
			}
		}
	}

	if (psMaterialPropteries->meOpacity == MO_Translucent)
	{
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHAREF, TRANSPARENCY);
		pcGraphicsMaterial->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pcGraphicsMaterial->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}
	else if (psMaterialPropteries->meOpacity == MO_Transparent)
	{
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);
		pcGraphicsMaterial->SetRenderState(D3DRS_ALPHAREF, TRANSLUCENCY);
		pcGraphicsMaterial->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pcGraphicsMaterial->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	}

	if (psMaterialPropteries->fShininessPower > 0.01f)
	{
		pcGraphicsMaterial->SetRenderState(D3DRS_SPECULARENABLE, TRUE);
	}
}

