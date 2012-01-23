/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "BaseLib/GeometricTypes.h"
#include "ColourARGB.h"
#include "ImageWriter.h"
#include "ImageAccessorCreator.h"
#include "ImageAccessor.h"
#include "BumpMapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void GenerateNormalMapFromMono(CImage* pcImage, SFloat3* pasNormals, EChannel eHeightChannel)
{
	int				x, y;
	SFloat3			sNormal;
	float			f1, f2, f3, f4;
	float			fdx1, fdx2;
	float			fdy1, fdy2;
	CImageAccessor*	pcAccessor;
	int				c;

	pcAccessor = CImageAccessorCreator::Create(pcImage, PT_uchar, eHeightChannel, CHANNEL_ZERO);
	c = 0;

	for (y = 0; y <= pcImage->miHeight; y++)
	{
		for (x = 0; x <= pcImage->miWidth; x++)
		{
			if ((x == 0) || (x == pcImage->miWidth) || (y == 0) || (y == pcImage->miHeight))
			{
				sNormal.Init(0.0f, 0.0f, 1.0f);
			}
			else
			{
				c = *((unsigned char*)pcAccessor->Get(x-1, y-1));
				f1 = Convert8BitColourToFloat(c);

				c = *((unsigned char*)pcAccessor->Get(x, y-1));
				f2 = Convert8BitColourToFloat(c);

				c = *((unsigned char*)pcAccessor->Get(x-1, y));
				f3 = Convert8BitColourToFloat(c);

				c = *((unsigned char*)pcAccessor->Get(x, y));
				f4 = Convert8BitColourToFloat(c);

				fdx1 = f1 - f2;
				fdx2 = f3 - f4;
				fdy1 = f1 - f3;
				fdy2 = f2 - f4;

				sNormal.Init(fdx1 + fdx2, fdy1 + fdy2, ((2.0f - fabsf(fdx1 + fdx2) + (2.0f - fabsf(fdy1 + fdy2)) / 4.0f)));
				sNormal.Normalize();
			}

			pasNormals[x + y * (pcImage->miWidth+1)] = sNormal;
		}
	}

	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AssignNormalMapFromNormals(CImage* pcImage, SFloat3* pasNormals)
{
	int				x, y;
	int				iColour;
	SFloat3			sNormal;
	SFloat3*		pn1; 
	SFloat3*		pn2; 
	SFloat3*		pn3; 
	SFloat3*		pn4; 
	CImageAccessor*	pcAccessor;

	pcAccessor = CImageAccessorCreator::Create(pcImage, PT_uchar, IMAGE_NORMAL_X, IMAGE_NORMAL_Y, IMAGE_NORMAL_Z, CHANNEL_ZERO);

	for (y = 0; y < pcImage->miHeight; y++)
	{
		for (x = 0; x < pcImage->miWidth; x++)
		{
			pn1 = &pasNormals[ x    +  y    * (pcImage->miWidth+1)];
			pn2 = &pasNormals[(x+1) +  y    * (pcImage->miWidth+1)];
			pn3 = &pasNormals[ x    + (y+1) * (pcImage->miWidth+1)];
			pn4 = &pasNormals[(x+1) + (y+1) * (pcImage->miWidth+1)];
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
void AssignBumpMapFromNormals(CImage* pcImage, SFloat3* pasNormals)
{
	int				x, y;
	int				iColour;
	SFloat3			sNormal;
	SFloat3*		pn1; 
	SFloat3*		pn2; 
	SFloat3*		pn3; 
	SFloat3*		pn4; 
	CImageAccessor*	pcAccessor;

	pcAccessor = CImageAccessorCreator::Create(pcImage, PT_uchar, IMAGE_BUMP_U, IMAGE_BUMP_V, CHANNEL_ZERO);

	for (y = 0; y < pcImage->miHeight; y++)
	{
		for (x = 0; x < pcImage->miWidth; x++)
		{
			pn1 = &pasNormals[ x    +  y    * (pcImage->miWidth+1)];
			pn2 = &pasNormals[(x+1) +  y    * (pcImage->miWidth+1)];
			pn3 = &pasNormals[ x    + (y+1) * (pcImage->miWidth+1)];
			pn4 = &pasNormals[(x+1) + (y+1) * (pcImage->miWidth+1)];
			sNormal = *pn1 + *pn2 + *pn3 + *pn4;
			sNormal.z /= 4.0f;
			sNormal.Normalize();

			iColour = Set32BitColour(0.0f, (1.0f + sNormal.x) / 2.0f, (1.0f + sNormal.y) / 2.0f, 0.0f);

			pcAccessor->Set(x, y, &iColour);
		}
	}

	pcAccessor->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void ConvertHeightMapTo(BOOL bNormalMap, BOOL bUVMapMap, CImage* pcImageDest, CImage* pcImageSource, EChannel eHeightChannel)
{
	int			iNumNormals;
	SFloat3*	pasNormals;
	SFloat3		sNormal;
	CImage		cDest;

	if (pcImageSource == pcImageDest)
	{
		pcImageDest = &cDest;
		cDest.Init();
	}

	pcImageDest->BeginChange();
	pcImageDest->SetSize(pcImageSource->GetWidth(), pcImageSource->GetHeight());

	if (bNormalMap)
	{
		pcImageDest->AddChannel(
			IMAGE_NORMAL_X, 
			IMAGE_NORMAL_Y, 
			IMAGE_NORMAL_Z, 
			PT_uchar);
	}

	if (bUVMapMap)
	{
		pcImageDest->AddChannel(
			IMAGE_BUMP_U,
			IMAGE_BUMP_V,
			PT_uchar);
	}

	pcImageDest->EndChange();
	if ((!bNormalMap) && (!bUVMapMap))
	{
		return;
	}

	iNumNormals = (pcImageDest->miWidth + 1) * (pcImageDest->miHeight + 1);
	pasNormals = (SFloat3*)malloc(sizeof(SFloat3) * iNumNormals);

	GenerateNormalMapFromMono(pcImageSource, pasNormals, eHeightChannel);
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

	if (pcImageDest == &cDest)
	{
		pcImageSource->Copy(pcImageDest);
		pcImageDest->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToNormals::Init(EChannel eHeightChannel)
{
	meHeightChannel = eHeightChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToNormals::Modify(CImage* pcImage)
{
	ConvertHeightMapTo(TRUE, FALSE, pcImage, pcImage, meHeightChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToNormals::Kill(void)
{
	CImageModifier::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToBumpUVs::Init(EChannel eHeightChannel)
{
	meHeightChannel = eHeightChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToBumpUVs::Modify(CImage* pcImage)
{
	ConvertHeightMapTo(FALSE, TRUE, pcImage, pcImage, meHeightChannel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageHeightToBumpUVs::Kill(void)
{
	CImageModifier::Kill();
}
