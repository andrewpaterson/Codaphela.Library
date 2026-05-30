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
#include "BaseLib/TypeNames.h"
#include "StandardLib/ChannelsAccessorCreator.h"
#include "ImageRowBlitterCache.h"
#include "ImageBlitter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Init(Ptr<CImageCel> pSourceCel, Ptr<CImage> pDestImage, Ptr<CImageRowBlitterCache> pBlitterCache)
{
	PreInit();

	bool					bResult;
	Ptr<CImage>				pSourceImage;
	CImageBlitterFormat		cFormat;

	cFormat.Init();

	macRowBlitters.Init();

	pSourceImage = pSourceCel->GetSourceImage();
	bResult = InitColourInfo(pSourceImage, pDestImage, &cFormat);
	if (!bResult)
	{
		return false;
	}

	bResult = InitOpacityInfo(pSourceCel, pDestImage, &cFormat);
	if (!bResult)
	{
		return false;
	}

	size						uiSourceByteStride;
	size						uiDestByteStride;
	Ptr<CBaseImageRowBlitter>	pcRowBlitter;
	CColourFormatHelper			cSourceFormatHelper;
	CColourFormatHelper			cDestFormatHelper;

	cSourceFormatHelper.Init(cFormat.meSourceColourFormat, cFormat.meColourOrder, cFormat.meColourBits, cFormat.meSourceAlphaBits);
	cDestFormatHelper.Init(cFormat.meDestColourFormat, cFormat.meColourOrder, cFormat.meColourBits, cFormat.meDestAlphaBits);
	uiSourceByteStride = pSourceImage->GetPixelByteStride();
	uiDestByteStride = pDestImage->GetPixelByteStride();

	pcRowBlitter = NULL;
	if (((cFormat.meSourceOpacity == CPO_None) || (cFormat.meSourceOpacity == CPO_Opaque)) && (uiSourceByteStride == uiDestByteStride))
	{
		CRectangle	cRect;
		size		uiY;
		size		uiBottom;

		pSourceCel->GetImageSourceBounds(&cRect);
		uiBottom = cRect.GetBottom();
		for (uiY = cRect.GetTop(); uiY < uiBottom; uiY++)
		{
			pcRowBlitter = pBlitterCache->CreateImageRowBlitterContiguous(pSourceImage, pDestImage);
			AddBlitter(pcRowBlitter, 0, uiY);
		}
	}
	else if (((cFormat.meSourceOpacity == CPO_None) || (cFormat.meSourceOpacity == CPO_Opaque)) && (uiSourceByteStride != uiDestByteStride))
	{
		pcRowBlitter = pBlitterCache->CreateImageRowBlitterByteAlignedOpaque(pSourceImage, pDestImage, &cSourceFormatHelper, &cDestFormatHelper);
	}
	else if ((cFormat.meSourceAlphaBits == ARGB_8bit) && (cFormat.meColourBits == CRGB_24bit))
	{
		pcRowBlitter = pBlitterCache->CreateImageRowBlitterRGBByteAlphaByteTranslucent(pSourceImage, pDestImage, &cSourceFormatHelper, &cDestFormatHelper);
	}
	else
	{
		//Fallback to an accessor based "blitter".
	}

	//CChannelsAccessor*			pcAccessor;
	//CChannelsAccessorCreator	cCreator;
	//size						uiNumChannels;
	//size						uiChannelIndex;
	//EChannel					eChannel;
	//EPrimitiveType				eType;
	//CColourFormatHelper			cFormatHelper;

	//uiNumChannels = cFormatHelper.GetNumChannels();

	//cCreator.Init(pSourceImage->GetChannels());
	//for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	//{
	//	eChannel = cFormatHelper.GetChannel(uiChannelIndex);
	//	eType = cFormatHelper.GetType(uiChannelIndex);
	//	if ((eChannel == IMAGE_CHANNEL_UNKNOWN) || (eType == PT_Undefined))
	//	{
	//		return false;
	//	}

	//	cCreator.AddAccess(eChannel, eType);
	//}
	// 
	//pcAccessor = cCreator.Create();

	PostInit();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::InitColourInfo(Ptr<CImage> pSource, Ptr<CImage> pDest, CImageBlitterFormat* pcFormat)
{
	EColourOrder	eSourceColourOrder;
	ERGBColourBits	eSourceColourBits;
	EColourOrder	eDestColourOrder;
	ERGBColourBits	eDestColourBits;

	eSourceColourOrder = GetColourOrder(pSource);
	if (eSourceColourOrder == CCO_Unknown)
	{
		return false;
	}

	pcFormat->meSourceColourFormat = GetColourFormat(pSource);
	if (pcFormat->meSourceColourFormat == CFT_Unknown)
	{
		return false;
	}

	eSourceColourBits = GetColourBits(pSource);
	if (eSourceColourBits == CRGB_Unknown)
	{
		return false;
	}

	pcFormat->meSourceAlphaBits = GetAlphaBits(pSource);
	if (pcFormat->meSourceAlphaBits == ARGB_Unknown)
	{
		return false;
	}

	eDestColourOrder = GetColourOrder(pDest);
	if (eDestColourOrder == CCO_Unknown)
	{
		return false;
	}

	pcFormat->meDestColourFormat = GetColourFormat(pDest);
	if (pcFormat->meDestColourFormat == CFT_Unknown)
	{
		return false;
	}

	eDestColourBits = GetColourBits(pDest);
	if (eDestColourBits == CRGB_Unknown)
	{
		return false;
	}

	pcFormat->meDestAlphaBits = GetAlphaBits(pDest);
	if (pcFormat->meDestAlphaBits == ARGB_Unknown)
	{
		return false;
	}

	if (eSourceColourOrder != eDestColourOrder)
	{
		return false;
	}
	pcFormat->meColourOrder = eSourceColourOrder;

	if (eSourceColourBits != eDestColourBits)
	{
		return false;
	}
	pcFormat->meColourBits = eSourceColourBits;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::InitOpacityInfo(Ptr<CImageCel> pSourceCel, Ptr<CImage> pDest, CImageBlitterFormat* pcFormat)
{
	Ptr<CImage>			pSourceImage;
	CSubImage*			pcSubImage;
	CRectangle*			pcSourcePixelRect;
	int32				x;
	int32				y;
	CChannelsAccessor*	pcOpacityAccessor;
	size				uiImageWidth;
	CChannels*			pcChannels;
	float32				fAlpha;
	bool				bSolid;
	bool				bTransparent;
	bool				bTranslucent;

	bSolid = false;
	bTransparent = false;
	bTranslucent = false;

	pSourceImage = pSourceCel->GetSourceImage();
	pcChannels = pSourceImage->GetChannels();
	if (pcFormat->meSourceAlphaBits == ARGB_Unknown)
	{
		pcFormat->meSourceOpacity = CPO_None;
		return true;
	}

	if (pcFormat->meSourceAlphaBits != ARGB_None)
	{
		pcOpacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pcChannels, IMAGE_OPACITY, PT_float32);
		if (pcOpacityAccessor)
		{
			pcSubImage = pSourceCel->GetSubImage();
			pcSourcePixelRect = &pcSubImage->mcImageRect;
			uiImageWidth = pSourceImage->GetWidth();
			for (y = pcSourcePixelRect->miTop; y <= pcSourcePixelRect->miBottom; y++)
			{
				for (x = pcSourcePixelRect->miTop; x <= pcSourcePixelRect->miBottom; x++)
				{
					fAlpha = *((float32*)pcOpacityAccessor->Get(x + y * uiImageWidth));
					if (fAlpha == 1.0f)
					{
						bSolid = true;
					}
					else if (fAlpha == 0.0f)
					{
						bTransparent = true;
					}
					else
					{
						bTranslucent = true;
					}
				}
			}
			UFree(pcOpacityAccessor);
		}
		else
		{
			return false;
		}
	}

	if (bTranslucent)
	{
		pcFormat->meSourceOpacity = CPO_Translucent;
	}
	else if (bTransparent)
	{
		pcFormat->meSourceOpacity = CPO_Transparent;
	}
	else
	{
		pcFormat->meSourceOpacity = CPO_Opaque;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Free(void)
{
	size				ui;
	size				uiNumElements;
	CImageRowBlitter*	psRowBlitter;

	uiNumElements = macRowBlitters.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		psRowBlitter = macRowBlitters.Get(ui);
		psRowBlitter->mpcBlitter->RemoveUsage();
	}
	macRowBlitters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Class(void)
{
	U_Data(CArrayImageRowBlitter, macRowBlitters);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EColourOrder CImageBlitter::GetColourOrder(Ptr<CImage> pImage)
{
	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	CArrayChannelOffset*	pcChannels;
	int						iOpacity;
	int						iIgnored;
	int						iRed;
	int						iGreen;
	int						iBlue;
	int						iRedGreen;
	int						iGreenBlue;
	int						iBlueRed;

	iOpacity = -1;
	iIgnored = -1;
	iRed = -1;
	iGreen = -1;
	iBlue = -1;

	pcChannels = pImage->GetChannelOffsets();
	uiNumChannels = pcChannels->NumElements();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = pcChannels->Get(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_DIFFUSE_RED)
		{
			iRed = (int)uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_GREEN)
		{
			iGreen = (int)uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_BLUE)
		{
			iBlue = (int)uiChannelIndex;
		}
	}

	if ((iRed == -1) || (iGreen == -1) || (iBlue == -1))
	{
		return CCO_Unknown;
	}

	iRedGreen = (int)iRed - (int)iGreen;
	iGreenBlue = (int)iGreen - (int)iBlue;
	iBlueRed = (int)iBlue - (int)iRed;

	if ((iRedGreen == -1) && (iGreenBlue == -1) && (iBlueRed == 2))
	{
		return CCO_RGB;
	}
	else if ((iBlueRed == -1) && (iRedGreen == -1) && (iGreenBlue == 2))
	{
		return CCO_BRG;
	}
	else if ((iGreenBlue == -1) && (iBlueRed == -1) && (iRedGreen == 2))
	{
		return CCO_GBR;
	}
	else if ((iBlueRed == -1) && (iGreenBlue == -1) && (iRedGreen == 2))
	{
		return CCO_BGR;
	}
	else if ((iBlueRed == -1) && (iGreenBlue == -1) && (iRedGreen == 2))
	{
		return CCO_RBG;
	}
	else if ((iBlueRed == -1) && (iGreenBlue == -1) && (iRedGreen == 2))
	{
		return CCO_GRB;
	}

	return CCO_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EColourFormat CImageBlitter::GetColourFormat(Ptr<CImage> pImage)
{
	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	CArrayChannelOffset*	pcChannels;
	int						iOpacity;
	int						iIgnored;
	int						iRed;

	//We already know the colour order at this point.
	iOpacity = -1;
	iIgnored = -1;
	iRed = -1;

	pcChannels = pImage->GetChannelOffsets();
	uiNumChannels = pcChannels->NumElements();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = pcChannels->Get(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_OPACITY)
		{
			iOpacity = uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_IGNORED)
		{
			iIgnored = uiChannelIndex;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_RED)
		{
			iRed = uiChannelIndex;
		}
	}

	if ((iOpacity != -1) && (iIgnored != -1))
	{
		return CFT_Unknown;
	}

	if (iOpacity != -1)
	{
		if (iRed > iOpacity)
		{
			return CFT_ARGB;
		}
		else
		{
			return CFT_RGBA;
		}
	}

	if (iIgnored != -1)
	{
		if (iRed > iIgnored)
		{
			return CFT_XRGB;
		}
		else
		{
			return CFT_RGBX;
		}
	}

	return CFT_RGB;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERGBColourBits CImageBlitter::GetColourBits(Ptr<CImage> pImage)
{
	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	CArrayChannelOffset*	pcChannels;
	EPrimitiveType			eRedType;
	EPrimitiveType			eGreenType;
	EPrimitiveType			eBlueType;

	eRedType = PT_Undefined;
	eGreenType = PT_Undefined;
	eBlueType = PT_Undefined;

	//We already know the colour exists at this point.
	pcChannels = pImage->GetChannelOffsets();
	uiNumChannels = pcChannels->NumElements();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = pcChannels->Get(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_DIFFUSE_RED)
		{
			eRedType = pcChannel->eType;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_GREEN)
		{
			eGreenType = pcChannel->eType;
		}
		else if (pcChannel->iChannel == IMAGE_DIFFUSE_BLUE)
		{
			eBlueType = pcChannel->eType;
		}
	}

	if ((eRedType == PT_uint8) && (eGreenType == PT_uint8) && (eBlueType == PT_uint8))
	{
		return CRGB_24bit;
	}
	else if ((eRedType == PT_uint16) && (eGreenType == PT_uint16) && (eBlueType == PT_uint16))
	{
		return CRGB_48bit;
	}
	else if ((eRedType == PT_float32) && (eGreenType == PT_float32) && (eBlueType == PT_float32))
	{
		return CRGB_Float3;
	}
	else if ((eRedType == PT_nickle) && (eGreenType == PT_nickle) && (eBlueType == PT_nickle))
	{
		return CRGB_15bit;
	}
	else if ((eRedType == PT_nickle) && (eGreenType == PT_sixbits) && (eBlueType == PT_nickle))
	{
		return CRGB_16bit;
	}
	else if ((eRedType == PT_crumb) && (eGreenType == PT_crumb) && (eBlueType == PT_crumb))
	{
		return CRGB_6bit;
	}
	else if ((eRedType == PT_tribble) && (eGreenType == PT_tribble) && (eBlueType == PT_crumb))
	{
		return CRGB_8bit332;
	}
	else if ((eRedType == PT_tribble) && (eGreenType == PT_crumb) && (eBlueType == PT_tribble))
	{
		return CRGB_8bit323;
	}
	else if ((eRedType == PT_crumb) && (eGreenType == PT_tribble) && (eBlueType == PT_tribble))
	{
		return CRGB_8bit323;
	}

	return CRGB_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERGBAlphaBits CImageBlitter::GetAlphaBits(Ptr<CImage> pImage)
{
	size					uiNumChannels;
	size					uiChannelIndex;
	CChannel*				pcChannel;
	CArrayChannelOffset*	pcChannels;
	EPrimitiveType			eOpacityType;
	EPrimitiveType			eIgnoredType;
	EPrimitiveType			eType;

	eIgnoredType = PT_Undefined;
	eOpacityType = PT_Undefined;

	//We already know the colour exists at this point.
	pcChannels = pImage->GetChannelOffsets();
	uiNumChannels = pcChannels->NumElements();
	for (uiChannelIndex = 0; uiChannelIndex < uiNumChannels; uiChannelIndex++)
	{
		pcChannel = pcChannels->Get(uiChannelIndex);

		if (pcChannel->iChannel == IMAGE_OPACITY)
		{
			eOpacityType = pcChannel->eType;
		}
		else if (pcChannel->iChannel == IMAGE_IGNORED)
		{
			eIgnoredType = pcChannel->eType;
		}
	}

	if ((eIgnoredType != PT_Undefined) && (eOpacityType != PT_Undefined))
	{
		return ARGB_Unknown;
	}
	else if ((eIgnoredType == PT_Undefined) && (eOpacityType == PT_Undefined))
	{
		return ARGB_None;
	}
	eType = eOpacityType;

	if (eType == PT_uint8)
	{
		return ARGB_8bit;
	}
	else if (eType == PT_uint16)
	{
		return ARGB_16bit;
	}
	else if (eType == PT_crumb)
	{
		return 	ARGB_2bit;
	}

	return ARGB_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::AddBlitter(Ptr<CBaseImageRowBlitter> pcBlitter, size xOffset, size yOffset)
{
	CImageRowBlitter* psBlitter;

	psBlitter = macRowBlitters.Add();
	psBlitter->Init(&pcBlitter, xOffset, yOffset);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Copy(int32 iDestX, int32 iDestY)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Copy(SImageCopy* psCopy)
{

}

