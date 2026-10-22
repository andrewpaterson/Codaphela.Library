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
#include <math.h>
#include "StandardLib/ClassDefines.h"
#include "BaseLib/GeometricTypes.h"
#include "ColourARGB32.h"
#include "ImageWriter.h"
#include "ImageAccessorCreator.h"
#include "ImageAccessor.h"
#include "BumpMapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GenerateNormalMapFromMono(CImage* pcImage, SFloat32Vec3* pasNormals, EChannel eHeightChannel)
{
	uint32			x, y;
	SFloat32Vec3	sNormal;
	float32			f1, f2, f3, f4;
	float32			fdx1, fdx2;
	float32			fdy1, fdy2;
	CImageAccessor*	pcAccessor;
	int				c;
	uint32			iHeight;
	uint32			iWidth;

	pcAccessor = CImageAccessorCreator::Create(pcImage, PT_uint8, eHeightChannel, CHANNEL_STOP);
	c = 0;

	iHeight = pcImage->GetImageHeight();
	iWidth = pcImage->GetImageWidth();
	for (y = 0; y <= iHeight; y++)
	{
		for (x = 0; x <= iWidth; x++)
		{
			if ((x == 0) || (x == iWidth) || (y == 0) || (y == iHeight))
			{
				sNormal.Init(0.0f, 0.0f, 1.0f);
			}
			else
			{
				c = *((uint8*)pcAccessor->Get(x-1, y-1));
				f1 = Convert8BitColourToFloat(c);

				c = *((uint8*)pcAccessor->Get(x, y-1));
				f2 = Convert8BitColourToFloat(c);

				c = *((uint8*)pcAccessor->Get(x-1, y));
				f3 = Convert8BitColourToFloat(c);

				c = *((uint8*)pcAccessor->Get(x, y));
				f4 = Convert8BitColourToFloat(c);

				fdx1 = f1 - f2;
				fdx2 = f3 - f4;
				fdy1 = f1 - f3;
				fdy2 = f2 - f4;

				sNormal.Init(fdx1 + fdx2, fdy1 + fdy2, ((2.0f - fabsf(fdx1 + fdx2) + (2.0f - fabsf(fdy1 + fdy2)) / 4.0f)));
				sNormal.Normalize();
			}

			pasNormals[x + y * (iWidth + 1)] = sNormal;
		}
	}

	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssignNormalMapFromNormals(Ptr<CImage> pcImage, SFloat32Vec3* pasNormals)
{
	uint32				x, y;
	int					iColour;
	SFloat32Vec3		sNormal;
	SFloat32Vec3*		pn1; 
	SFloat32Vec3*		pn2; 
	SFloat32Vec3*		pn3; 
	SFloat32Vec3*		pn4; 
	CImageAccessor*		pcAccessor;
	uint32				iHeight;
	uint32				iWidth;
	uint32				iImageWidth;

	pcAccessor = CImageAccessorCreator::Create(&pcImage, PT_uint8, IMAGE_NORMAL_X, IMAGE_NORMAL_Y, IMAGE_NORMAL_Z, CHANNEL_STOP);

	iWidth = (pcImage->GetImageWidth() + 1);
	iImageWidth = pcImage->GetImageWidth();
	iHeight = pcImage->GetImageHeight();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iImageWidth; x++)
		{
			pn1 = &pasNormals[ x    +  y    * iWidth];
			pn2 = &pasNormals[(x+1) +  y    * iWidth];
			pn3 = &pasNormals[ x    + (y+1) * iWidth];
			pn4 = &pasNormals[(x+1) + (y+1) * iWidth];
			sNormal = *pn1 + *pn2 + *pn3 + *pn4;
			sNormal.z /= 4.0f;
			sNormal.Normalize();

			iColour = Set32BitColour((1.0f + sNormal.x) / 2.0f, (1.0f + sNormal.y) / 2.0f, sNormal.z, 0.0f);

			pcAccessor->Set(x, y, &iColour);
		}
	}

	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssignBumpMapFromNormals(Ptr<CImage> pcImage, SFloat32Vec3* pasNormals)
{
	uint32				x, y;
	uint32				iColour;
	SFloat32Vec3		sNormal;
	SFloat32Vec3*		pn1; 
	SFloat32Vec3*		pn2; 
	SFloat32Vec3*		pn3; 
	SFloat32Vec3*		pn4; 
	CImageAccessor*		pcAccessor;
	uint32				iWidth;
	uint32				iHeight;
	uint32				iImageWidth;

	pcAccessor = CImageAccessorCreator::Create(&pcImage, PT_uint8, IMAGE_BUMP_U, IMAGE_BUMP_V, CHANNEL_STOP);

	iWidth = (pcImage->GetImageWidth() + 1);
	iHeight = pcImage->GetImageHeight();
	iImageWidth = pcImage->GetImageWidth();
	for (y = 0; y < iHeight; y++)
	{
		for (x = 0; x < iImageWidth; x++)
		{
			pn1 = &pasNormals[ x    +  y    * iWidth];
			pn2 = &pasNormals[(x+1) +  y    * iWidth];
			pn3 = &pasNormals[ x    + (y+1) * iWidth];
			pn4 = &pasNormals[(x+1) + (y+1) * iWidth];
			sNormal = *pn1 + *pn2 + *pn3 + *pn4;
			sNormal.z /= 4.0f;
			sNormal.Normalize();

			iColour = Set32BitColour((1.0f + sNormal.x) / 2.0f, (1.0f + sNormal.y) / 2.0f, 0.0f, 0.0f);  //U then V, the first two bytes.

			pcAccessor->Set(x, y, &iColour);
		}
	}

	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> ConvertHeightMapTo(bool bNormalMap, bool bUVMapMap, Ptr<CImage> pcImageSource, EChannel eHeightChannel)
{
	int				iNumNormals;
	SFloat32Vec3*		pasNormals;
	SFloat32Vec3			sNormal;
	Ptr<CImage>		pcImageDest;

	pcImageDest = OMalloc<CImage>();
	pcImageDest->BeginChange();
	pcImageDest->SetSize(pcImageSource->GetWidth(), pcImageSource->GetHeight());

	if (bNormalMap)
	{
		pcImageDest->AddChannel(
			IMAGE_NORMAL_X, 
			IMAGE_NORMAL_Y, 
			IMAGE_NORMAL_Z, 
			PT_uint8);
	}

	if (bUVMapMap)
	{
		pcImageDest->AddChannel(
			IMAGE_BUMP_U,
			IMAGE_BUMP_V,
			PT_uint8);
	}

	pcImageDest->EndChange();
	if ((!bNormalMap) && (!bUVMapMap))
	{
		return NULL;
	}

	iNumNormals = (pcImageDest->GetImageWidth() + 1) * (pcImageDest->GetImageHeight() + 1);
	pasNormals = (SFloat32Vec3*)malloc(sizeof(SFloat32Vec3) * iNumNormals);

	GenerateNormalMapFromMono(&pcImageSource, pasNormals, eHeightChannel);
	if (bNormalMap)
	{
		AssignNormalMapFromNormals(pcImageDest, pasNormals);
	}
	if (bUVMapMap)
	{
		AssignBumpMapFromNormals(pcImageDest, pasNormals);
	}
	
	if (pasNormals)
	{
		free(pasNormals);
	}

	return pcImageDest;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToNormals::Init(EChannel eHeightChannel)
{
	PreInit();

	meHeightChannel = eHeightChannel;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageHeightToNormals::Modify(Ptr<CImage> pcImage)
{
	return ConvertHeightMapTo(true, false, pcImage, meHeightChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToNormals::Class(void)
{
	CImageModifier::Class();
	U_Enum(meHeightChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToBumpUVs::Init(EChannel eHeightChannel)
{
	PreInit();

	meHeightChannel = eHeightChannel;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageHeightToBumpUVs::Modify(Ptr<CImage> pcImage)
{
	return ConvertHeightMapTo(false, true, pcImage, meHeightChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToBumpUVs::Class(void)
{
	CImageModifier::Class();
	U_Enum(meHeightChannel);
}
