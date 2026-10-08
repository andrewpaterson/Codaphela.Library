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
#include "ImageRowBlitterFactory.h"
#include "ImageBlitter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Init(Ptr<CImageCel> pSourceCel, Ptr<CImage> pDestImage)
{
	PreInit();

	ValidatePtr(pSourceCel);
	ValidatePtr(pDestImage);

	mpSourceCel = pSourceCel;
	mpDestImage = pDestImage;

	mcFormat.Init();

	macRowBlitters.Init();
	mcContext.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Configure(CImageRowBlitterFactory* pcBlitterCache)
{
	bool					bResult;
	Ptr<CImage>				pSourceImage;

	pSourceImage = mpSourceCel->GetSourceImage();
	bResult = InitColourInfo(&mcFormat);
	if (!bResult)
	{
		return false;
	}

	bResult = InitOpacityInfo(&mcFormat);
	if (!bResult)
	{
		return false;
	}

	bResult = InitRowBlitters(&mcFormat, pcBlitterCache);
	if (!bResult)
	{
		return false;
	}

	InitContext(&mcContext);

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::InitColourInfo(CImageBlitterFormat* pcFormat)
{
	EColourOrder	eSourceColourOrder;
	ERGBColourBits	eSourceColourBits;
	EColourOrder	eDestColourOrder;
	ERGBColourBits	eDestColourBits;
	Ptr<CImage>		pSourceImage;
	
	pSourceImage = mpSourceCel->GetSourceImage();

	eSourceColourOrder = pSourceImage->GetColourOrder();
	if (eSourceColourOrder == CCO_Unknown)
	{
		return false;
	}

	mcFormat.meSourceColourFormat = pSourceImage->GetColourFormat();
	if (mcFormat.meSourceColourFormat == CFT_Unknown)
	{
		return false;
	}

	eSourceColourBits = pSourceImage->GetColourBits();
	if (eSourceColourBits == CRGB_Unknown)
	{
		return false;
	}

	mcFormat.meSourceAlphaBits = pSourceImage->GetAlphaBits();
	if (mcFormat.meSourceAlphaBits == ARGB_Unknown)
	{
		return false;
	}

	eDestColourOrder = mpDestImage->GetColourOrder();
	if (eDestColourOrder == CCO_Unknown)
	{
		return false;
	}

	mcFormat.meDestColourFormat = mpDestImage->GetColourFormat();
	if (mcFormat.meDestColourFormat == CFT_Unknown)
	{
		return false;
	}

	eDestColourBits = mpDestImage->GetColourBits();
	if (eDestColourBits == CRGB_Unknown)
	{
		return false;
	}

	mcFormat.meDestAlphaBits = mpDestImage->GetAlphaBits();
	if (mcFormat.meDestAlphaBits == ARGB_Unknown)
	{
		return false;
	}

	if (eSourceColourOrder != eDestColourOrder)
	{
		return false;
	}
	mcFormat.meColourOrder = eSourceColourOrder;

	if (eSourceColourBits != eDestColourBits)
	{
		return false;
	}
	mcFormat.meColourBits = eSourceColourBits;

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::InitOpacityInfo(CImageBlitterFormat* pcFormat)
{
	Ptr<CImage>			pSourceImage;
	CSubImage*			pcSubImage;
	CRectangle*			pcSourcePixelRect;
	size				x;
	size				y;
	CChannelsAccessor*	pcOpacityAccessor;
	size				uiImageWidth;
	CChannels*			pcChannels;
	float32				fAlpha;
	bool				bSolid;
	bool				bTransparent;
	bool				bTranslucent;
	size				uiBottom;
	size				uiRight;

	bSolid = false;
	bTransparent = false;
	bTranslucent = false;

	pSourceImage = mpSourceCel->GetSourceImage();
	pcChannels = pSourceImage->GetChannels();
	if (mcFormat.meSourceAlphaBits == ARGB_Unknown)
	{
		mcFormat.meSourceOpacity = CPO_None;
		return true;
	}

	if (mcFormat.meSourceAlphaBits != ARGB_None)
	{
		pcOpacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pcChannels, IMAGE_OPACITY, PT_float32);
		if (pcOpacityAccessor)
		{
			pcSubImage = mpSourceCel->GetSubImage();
			pcSourcePixelRect = &pcSubImage->mcImageRect;
			uiImageWidth = pSourceImage->GetWidth();
			uiBottom = pcSourcePixelRect->GetBottom();
			uiRight = pcSourcePixelRect->GetRight();
			for (y = pcSourcePixelRect->GetTop(); y < uiBottom; y++)
			{
				for (x = pcSourcePixelRect->GetLeft(); x < uiRight; x++)
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
		mcFormat.meSourceOpacity = CPO_Translucent;
	}
	else if (bTransparent)
	{
		mcFormat.meSourceOpacity = CPO_Transparent;
	}
	else
	{
		mcFormat.meSourceOpacity = CPO_Opaque;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::InitRowBlitters(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache)
{
	
	size						uiSourceByteStride;
	size						uiDestByteStride;
	Ptr<CBaseImageRowBlitter>	pcRowBlitter;
	Ptr<CImage>					pSourceImage;

	pSourceImage = mpSourceCel->GetSourceImage();
	uiSourceByteStride = pSourceImage->GetPixelByteStride();
	uiDestByteStride = mpDestImage->GetPixelByteStride();

	pcRowBlitter = NULL;
	if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && (uiSourceByteStride == uiDestByteStride) && (mcFormat.meDestAlphaBits == ARGB_None))
	{
		return CreateImageRowBlitterContiguous(pcFormat, pcBlitterCache);
	}
	else if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && (uiSourceByteStride == uiDestByteStride) && (mcFormat.meDestAlphaBits != ARGB_None))
	{
		return CreateImageRowBlitterByteAlignedOpaqueDestAlpha(pcFormat, pcBlitterCache);
	}
	else if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && (uiSourceByteStride != uiDestByteStride) && (mcFormat.meDestAlphaBits == ARGB_None))
	{
		return CreateImageRowBlitterByteAlignedOpaque(pcFormat, pcBlitterCache);
	}
	else if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && (uiSourceByteStride != uiDestByteStride) && (mcFormat.meDestAlphaBits != ARGB_None))
	{
		return CreateImageRowBlitterByteAlignedOpaqueDestAlpha(pcFormat, pcBlitterCache);
	}
	else if ((mcFormat.meSourceAlphaBits == ARGB_8bit) && (mcFormat.meColourBits == CRGB_24bit) && (mcFormat.meDestAlphaBits == ARGB_None))
	{
		return CreateImageRowBlitterRGBByteAlphaByteTranslucent(pcFormat, pcBlitterCache);
	}
	else if ((mcFormat.meSourceAlphaBits == ARGB_8bit) && (mcFormat.meColourBits == CRGB_24bit) && (mcFormat.meDestAlphaBits != ARGB_None))
	{
		return CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(pcFormat, pcBlitterCache);
	}
	else
	{
		//Fallback to an accessor based "blitter".
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterContiguous(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache)
{
	return CreateImageRowBlitterUseCacheFunc(pcFormat, pcBlitterCache, &CImageRowBlitterFactory::CreateImageRowBlitterContiguous);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterByteAlignedOpaque(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache)
{
	return CreateImageRowBlitterUseCacheFunc(pcFormat, pcBlitterCache, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaque);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterByteAlignedOpaqueDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache)
{
	return CreateImageRowBlitterUseCacheFunc(pcFormat, pcBlitterCache, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaqueDestAlpha);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, CreateImageRowBlitterFunc fCreate)
{
	CRectangle				cRect;
	size					y;
	size					uiBottom;
	size					xEnd;
	size					xStart;
	CBaseImageRowBlitter*	pcRowBlitter;

	mpSourceCel->GetImageSourceBounds(&cRect);
	uiBottom = cRect.GetBottom();
	xEnd = cRect.GetRight();
	xStart = cRect.GetLeft();
	for (y = cRect.GetTop(); y < uiBottom; y++)
	{
		pcRowBlitter = (pcBlitterCache->*fCreate)();
		AddBlitter(pcRowBlitter, xStart, xEnd, y);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterRGBByteAlphaByteTranslucent(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache)
{
	return CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(pcFormat, pcBlitterCache, &CImageRowBlitterFactory::CreateImageRowBlitterRGBByteAlphaByteTranslucent, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaque);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache)
{
	return CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(pcFormat, pcBlitterCache, &CImageRowBlitterFactory::CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaqueDestAlpha);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterCache, CreateImageRowBlitterFunc fCreateRGBByteAlphaByteTranslucent, CreateImageRowBlitterFunc fCreateByteAlignedOpaque)
{
	CRectangle				cRect;
	size					x, y;
	size					uiBottom;
	size					xEnd;
	size					xStart;
	CBaseImageRowBlitter*	pcRowBlitter;
	Ptr<CImage>				pSourceImage;
	CColourFormatHelper		cSourceFormatHelper;
	CColourFormatHelper		cDestFormatHelper;
	size					uiAlpha;
	CImageAccessor*			pcAccessorOpacity;
	size					uiLastType;
	size					uiThisType;

	pSourceImage = mpSourceCel->GetSourceImage();
	pcAccessorOpacity = CImageAccessorCreator::Create(&pSourceImage, PT_uint8, IMAGE_OPACITY, CHANNEL_STOP);

	cSourceFormatHelper.Init(mcFormat.meSourceColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meSourceAlphaBits);
	cDestFormatHelper.Init(mcFormat.meDestColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meDestAlphaBits);
	mpSourceCel->GetImageSourceBounds(&cRect);
	uiBottom = cRect.GetBottom();
	xEnd = cRect.GetRight();
	xStart = cRect.GetLeft();
	uiLastType = 0;
	for (y = cRect.GetTop(); y < uiBottom; y++)
	{
		xStart = cRect.GetLeft();
		for (x = xStart; x < xEnd; x++)
		{
			uiAlpha = *((uint8*)pcAccessorOpacity->Get(x, y));
			if (uiAlpha == 0)
			{
				uiThisType = 0;
			}
			else if (uiAlpha == 255)
			{
				uiThisType = 255;
			}
			else
			{
				uiThisType = 1;
			}

			if (uiLastType != uiThisType)
			{
				if (uiLastType == 1)
				{
					pcRowBlitter = (pcBlitterCache->*fCreateRGBByteAlphaByteTranslucent)();
					AddBlitter(pcRowBlitter, xStart, x, y);
				}
				else if (uiLastType == 255)
				{
					pcRowBlitter = (pcBlitterCache->*fCreateByteAlignedOpaque)();
					AddBlitter(pcRowBlitter, xStart, x, y);
				}
				xStart = x;
				uiLastType = uiThisType;
			}
		}

		if (uiLastType != 0)
		{
			if (uiLastType == 1)
			{
				pcRowBlitter = pcBlitterCache->CreateImageRowBlitterRGBByteAlphaByteTranslucent();
				AddBlitter(pcRowBlitter, xStart, x, y);
			}
			else if (uiLastType == 255)
			{
				pcRowBlitter = pcBlitterCache->CreateImageRowBlitterByteAlignedOpaque();
				AddBlitter(pcRowBlitter, xStart, x, y);
			}
			xStart = x;
		}
	}

	UFree(pcAccessorOpacity);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Free(void)
{
	macRowBlitters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Class(void)
{
	U_Data(CArrayImageRowBlitter, macRowBlitters);
	M_Pointer(mpSourceCel);
	M_Pointer(mpDestImage);
	U_Data(CImageBlitterFormat, mcFormat);
	U_Data(CImageBlitterContext, mcContext);
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
void CImageBlitter::AddBlitter(CBaseImageRowBlitter* pcBlitter, size xStart, size xEnd, size yOffset)
{
	CImageRowBlitter* psBlitter;

	psBlitter = macRowBlitters.Add();
	psBlitter->Init(pcBlitter, xStart, xEnd, yOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CImageBlitter::GetCel(void)
{
	return mpSourceCel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::InitContext(CImageBlitterContext* pcContext)
{
	CColourFormatHelper		cSourceFormatHelper;
	CColourFormatHelper		cDestFormatHelper;

	cSourceFormatHelper.Init(mcFormat.meSourceColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meSourceAlphaBits);
	cDestFormatHelper.Init(mcFormat.meDestColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meDestAlphaBits);

	pcContext->Init(mpSourceCel->GetSourceImage(), mpDestImage, &cSourceFormatHelper, &cDestFormatHelper);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Blit(int32 iDestX, int32 iDestY)
{
	size				uiNumElements;
	size				ui;
	CImageRowBlitter*	pcRowBlitter;
	int32				xSourceStart;
	int32				xSourceEnd;
	int32				ySource;
	CRectangle			cDestRect;
	CRectangle			cSourceRect;
	int32				yDest;
	int32				xDestStart;
	void*				pvSource;
	void*				pvDest;
	int32				iSourceLeft;
	int32				iSourceTop;
	int32				xDestOffset;
	int32				yDestOffset;
	int32				xLength;

	pvSource = mpSourceCel->GetSourceImage()->GetData();
	pvDest = mpDestImage->GetData();

	iSourceLeft = (size)mpSourceCel->GetSourceLeft();
	iSourceTop = (size)mpSourceCel->GetSourceTop();
	cDestRect.Init(0, 0, mpDestImage->GetWidth(), mpDestImage->GetHeight());
	mpSourceCel->GetImageDestBounds(iDestX, iDestY, &cSourceRect);

	if (cSourceRect.Outside(&cDestRect))
	{
		return true;
	}

	uiNumElements = macRowBlitters.NumElements();
	if (cSourceRect.Inside(&cDestRect))
	{
		for (ui = 0; ui < uiNumElements; ui++)
		{
			pcRowBlitter = macRowBlitters.Get(ui);

			ySource = pcRowBlitter->msOffset.y;
			yDestOffset = ySource - iSourceTop;
			yDest = iDestY + yDestOffset;

			xSourceStart = pcRowBlitter->msOffset.x;
			xSourceEnd = pcRowBlitter->muiXEnd;

			xDestOffset = xSourceStart - iSourceLeft;
			xDestStart = iDestX + xDestOffset;

			pcRowBlitter->mpcBlitter->Copy(&mcContext, pvSource, pvDest, xDestStart, yDest, xSourceStart, xSourceEnd, ySource);
		}

		return true;
	}
	else
	{
		for (ui = 0; ui < uiNumElements; ui++)
		{
			pcRowBlitter = macRowBlitters.Get(ui);

			ySource = pcRowBlitter->msOffset.y;
			yDestOffset = ySource - iSourceTop;
			yDest = iDestY + yDestOffset;

			if ((yDest >= cDestRect.miTop) && (yDest < cDestRect.miBottom))
			{
				xSourceStart = pcRowBlitter->msOffset.x;
				xSourceEnd = pcRowBlitter->muiXEnd;

				xDestOffset = xSourceStart - iSourceLeft;
				xDestStart = iDestX + xDestOffset;

				if (xDestStart < cDestRect.miLeft)
				{
					xSourceStart += cDestRect.miLeft - xDestStart;
					xDestStart = cDestRect.miLeft;
				}
				xLength = xSourceEnd - xSourceStart;
				if (xDestStart + xLength > cDestRect.miRight)
				{
					xSourceEnd -= xDestStart + xLength - cDestRect.miRight;
				}

				pcRowBlitter->mpcBlitter->Copy(&mcContext, pvSource, pvDest, xDestStart, yDest, xSourceStart, xSourceEnd, ySource);
			}
		}

		return true;
	}
}

