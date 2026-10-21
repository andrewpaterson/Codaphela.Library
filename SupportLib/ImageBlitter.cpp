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

	mpcSourceCel = (CImageCel*)pSourceCel.Object();
	mpDestImage = pDestImage;

	mcFormat.Init();

	macRowBlitters.Init();
	mcContext.Init();
	maiRowStarts.Init();

	mpcSizing = NULL;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Configure(CImageRowBlitterFactory* pcBlitterFactory, bool bAllowSizing)
{
	bool			bResult;
	Ptr<CImage>		pSourceImage;

	if (bAllowSizing)
	{
		EnableSizing(pcBlitterFactory);
	}

	pSourceImage = mpcSourceCel->GetSourceImage();
	bResult = PopulateColourFormat(&mcFormat);
	if (!bResult)
	{
		return false;
	}

	bResult = PopulateOpacityFormat(&mcFormat);
	if (!bResult)
	{
		return false;
	}

	bResult = CreateImageRowBlitters(&mcFormat, pcBlitterFactory, bAllowSizing);
	if (!bResult)
	{
		return false;
	}

	PopulateContext(&mcContext);
	PopulateRowStarts();

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::AllowsSizing(void)
{
	return mpcSizing != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::EnableSizing(CImageRowBlitterFactory* pcBlitterCache)
{
	size							iNumBlitters;
	size							i;
	COffsetImageRowBlitter*			pcOffsetImageRowBlitter;
	CBaseImageRowSizingBlitter*		pcImageRowSizingBlitter;

	if (mpcSizing == NULL)
	{
		mpcSizing = (CSizingImageBlitterSize*)malloc(sizeof(CSizingImageBlitterSize));
		mpcSizing->Init();

		iNumBlitters = macRowBlitters.NumElements();
		for (i = 0; i < iNumBlitters; i++)
		{
			pcOffsetImageRowBlitter = macRowBlitters.Get(i);

			pcImageRowSizingBlitter = pcBlitterCache->CreateImageRowSizingBlitter(pcOffsetImageRowBlitter->mpcBlitter);
			pcOffsetImageRowBlitter->SetImageRowSizingBlitter(pcImageRowSizingBlitter);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::PopulateColourFormat(CImageBlitterFormat* pcFormat)
{
	EColourOrder	eSourceColourOrder;
	ERGBColourBits	eSourceColourBits;
	EColourOrder	eDestColourOrder;
	ERGBColourBits	eDestColourBits;
	Ptr<CImage>		pSourceImage;
	
	pSourceImage = mpcSourceCel->GetSourceImage();

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
bool CImageBlitter::PopulateOpacityFormat(CImageBlitterFormat* pcFormat)
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
	bool				bTransparent;
	bool				bTranslucent;
	size				uiBottom;
	size				uiRight;

	bTransparent = false;
	bTranslucent = false;

	pSourceImage = mpcSourceCel->GetSourceImage();
	pcChannels = pSourceImage->GetChannels();
	if (mcFormat.meSourceAlphaBits == ARGB_Unknown)
	{
		mcFormat.meSourceOpacity = CPO_None;
		return true;
	}

	//An image with an ignored channel (e.g. RGBX) has alpha bits but no opacity channel.
	if (!pSourceImage->HasChannel(IMAGE_OPACITY))
	{
		mcFormat.meSourceOpacity = CPO_None;
		return true;
	}

	if (mcFormat.meSourceAlphaBits != ARGB_None)
	{
		pcOpacityAccessor = CChannelsAccessorCreator::CreateSingleChannelAccessor(pcChannels, IMAGE_OPACITY, PT_float32);
		if (pcOpacityAccessor)
		{
			pcSubImage = mpcSourceCel->GetSubImage();
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
bool CImageBlitter::CreateImageRowBlitters(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, bool bAllowSizing)
{
	size			uiSourceByteStride;
	size			uiDestByteStride;
	Ptr<CImage>		pSourceImage;
	bool			bDestOpacity;
	bool			bSameLayout;

	pSourceImage = mpcSourceCel->GetSourceImage();
	uiSourceByteStride = pSourceImage->GetPixelByteStride();
	uiDestByteStride = mpDestImage->GetPixelByteStride();

	//The alpha bits of an image with an ignored channel (e.g. RGBX) are the size of the ignored channel.  Only write alpha if the destination has an opacity channel.
	bDestOpacity = mpDestImage->HasChannel(IMAGE_OPACITY);

	//Pixels can only be copied whole if the channels are in the same place in both images (e.g. not XRGB into RGBX).
	bSameLayout = (uiSourceByteStride == uiDestByteStride) && (mcFormat.meSourceColourFormat == mcFormat.meDestColourFormat);

	if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && bSameLayout && !bDestOpacity)
	{
		return CreateImageRowBlitterContiguous(pcFormat, pcBlitterFactory, bAllowSizing);
	}
	else if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && bSameLayout && bDestOpacity)
	{
		return CreateImageRowBlitterByteAlignedOpaqueDestAlpha(pcFormat, pcBlitterFactory, bAllowSizing);
	}
	else if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && !bSameLayout && !bDestOpacity)
	{
		return CreateImageRowBlitterByteAlignedOpaque(pcFormat, pcBlitterFactory, bAllowSizing);
	}
	else if (((mcFormat.meSourceOpacity == CPO_None) || (mcFormat.meSourceOpacity == CPO_Opaque)) && !bSameLayout && bDestOpacity)
	{
		return CreateImageRowBlitterByteAlignedOpaqueDestAlpha(pcFormat, pcBlitterFactory, bAllowSizing);
	}
	else if ((mcFormat.meSourceAlphaBits == ARGB_8bit) && (mcFormat.meColourBits == CRGB_24bit) && !bDestOpacity)
	{
		return CreateImageRowBlitterRGBByteAlphaByteTranslucent(pcFormat, pcBlitterFactory, bAllowSizing);
	}
	else if ((mcFormat.meSourceAlphaBits == ARGB_8bit) && (mcFormat.meColourBits == CRGB_24bit) && bDestOpacity)
	{
		return CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(pcFormat, pcBlitterFactory, bAllowSizing);
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
bool CImageBlitter::CreateImageRowBlitterContiguous(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, bool bAllowSizing)
{
	return CreateImageRowBlitterUseCacheFunc(pcFormat, pcBlitterFactory, &CImageRowBlitterFactory::CreateImageRowBlitterContiguous, bAllowSizing);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterByteAlignedOpaque(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, bool bAllowSizing)
{
	return CreateImageRowBlitterUseCacheFunc(pcFormat, pcBlitterFactory, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaque, bAllowSizing);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterByteAlignedOpaqueDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, bool bAllowSizing)
{
	return CreateImageRowBlitterUseCacheFunc(pcFormat, pcBlitterFactory, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaqueDestAlpha, bAllowSizing);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, CreateImageRowBlitterFunc fCreate, bool bAllowSizing)
{
	CRectangle				cRect;
	size					y;
	size					uiBottom;
	size					xEnd;
	size					xStart;
	CBaseImageRowBlitter*	pcRowBlitter;

	mpcSourceCel->GetImageSourceBounds(&cRect);
	uiBottom = cRect.GetBottom();
	xEnd = cRect.GetRight();
	xStart = cRect.GetLeft();
	for (y = cRect.GetTop(); y < uiBottom; y++)
	{
		pcRowBlitter = (pcBlitterFactory->*fCreate)();
		AddBlitter(pcRowBlitter, pcBlitterFactory, xStart, xEnd, y, bAllowSizing);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterRGBByteAlphaByteTranslucent(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, bool bAllowSizing)
{
	return CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(pcFormat, pcBlitterFactory, &CImageRowBlitterFactory::CreateImageRowBlitterRGBByteAlphaByteTranslucent, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaque, bAllowSizing);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, bool bAllowSizing)
{
	return CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(pcFormat, pcBlitterFactory, &CImageRowBlitterFactory::CreateImageRowBlitterRGBByteAlphaByteTranslucentDestAlpha, &CImageRowBlitterFactory::CreateImageRowBlitterByteAlignedOpaqueDestAlpha, bAllowSizing);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::CreateImageRowBlitterRGBByteAlphaByteUseCacheFunc(CImageBlitterFormat* pcFormat, CImageRowBlitterFactory* pcBlitterFactory, CreateImageRowBlitterFunc fCreateRGBByteAlphaByteTranslucent, CreateImageRowBlitterFunc fCreateByteAlignedOpaque, bool bAllowSizing)
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

	pSourceImage = mpcSourceCel->GetSourceImage();
	pcAccessorOpacity = CImageAccessorCreator::Create(&pSourceImage, PT_uint8, IMAGE_OPACITY, CHANNEL_STOP);

	cSourceFormatHelper.Init(mcFormat.meSourceColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meSourceAlphaBits);
	cDestFormatHelper.Init(mcFormat.meDestColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meDestAlphaBits);
	mpcSourceCel->GetImageSourceBounds(&cRect);
	uiBottom = cRect.GetBottom();
	xEnd = cRect.GetRight();
	xStart = cRect.GetLeft();
	uiLastType = 0;
	for (y = cRect.GetTop(); y < uiBottom; y++)
	{
		xStart = cRect.GetLeft();
		for (x = xStart; x < xEnd; x++)
		{
			uiAlpha = *((uint8*)pcAccessorOpacity->Get((int32)x, (int32)y));
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
					pcRowBlitter = (pcBlitterFactory->*fCreateRGBByteAlphaByteTranslucent)();
					AddBlitter(pcRowBlitter, pcBlitterFactory, xStart, x, y, bAllowSizing);
				}
				else if (uiLastType == 255)
				{
					pcRowBlitter = (pcBlitterFactory->*fCreateByteAlignedOpaque)();
					AddBlitter(pcRowBlitter, pcBlitterFactory, xStart, x, y, bAllowSizing);
				}
				xStart = x;
				uiLastType = uiThisType;
			}
		}

		if (uiLastType != 0)
		{
			if (uiLastType == 1)
			{
				pcRowBlitter = (pcBlitterFactory->*fCreateRGBByteAlphaByteTranslucent)();
				AddBlitter(pcRowBlitter, pcBlitterFactory, xStart, x, y, bAllowSizing);
			}
			else if (uiLastType == 255)
			{
				pcRowBlitter = (pcBlitterFactory->*fCreateByteAlignedOpaque)();
				AddBlitter(pcRowBlitter, pcBlitterFactory, xStart, x, y, bAllowSizing);
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
	SafeKill(mpcSizing);
	maiRowStarts.Kill();
	macRowBlitters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::Class(void)
{
	U_Data(CArrayOffsetImageRowBlitter, macRowBlitters);
	U_Pointer(mpcSourceCel);
	M_Pointer(mpDestImage);
	U_Data(CImageBlitterFormat, mcFormat);
	U_Data(CImageBlitterContext, mcContext);
	U_Data(CArrayInt32, maiRowStarts);
	U_Pointer(mpcSizing);
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
void CImageBlitter::AddBlitter(CBaseImageRowBlitter* pcImageRowBlitter, CImageRowBlitterFactory* pcBlitterFactory, size xStart, size xEnd, size yOffset, bool bAllowSizing)
{
	COffsetImageRowBlitter*			pcOffsetImageRowBlitter;
	CBaseImageRowSizingBlitter*		pcImageRowSizingBlitter;

	if (bAllowSizing)
	{
		pcImageRowSizingBlitter = pcBlitterFactory->CreateImageRowSizingBlitter(pcImageRowBlitter);
	}
	else
	{
		pcImageRowSizingBlitter = NULL;
	}

	pcOffsetImageRowBlitter = macRowBlitters.Add();
	pcOffsetImageRowBlitter->Init(pcImageRowBlitter, pcImageRowSizingBlitter, xStart, xEnd, yOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CImageBlitter::GetCel(void)
{
	return mpcSourceCel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::PopulateContext(CImageBlitterContext* pcContext)
{
	CColourFormatHelper		cSourceFormatHelper;
	CColourFormatHelper		cDestFormatHelper;

	cSourceFormatHelper.Init(mcFormat.meSourceColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meSourceAlphaBits);
	cDestFormatHelper.Init(mcFormat.meDestColourFormat, mcFormat.meColourOrder, mcFormat.meColourBits, mcFormat.meDestAlphaBits);

	pcContext->Init(mpcSourceCel->GetSourceImage(), mpDestImage, &cSourceFormatHelper, &cDestFormatHelper);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitter::PopulateRowStarts(void)
{
	CRectangle					cRect;
	int32						iHeight;
	int32						y;
	size						ui;
	size						uiNumElements;
	int32*						paiRowStarts;
	COffsetImageRowBlitter*		pcOffsetImageRowBlitter;

	//The segments were added a row at a time from the top so the segments of a row follow on from the row above.
	mpcSourceCel->GetImageSourceBounds(&cRect);
	iHeight = cRect.GetHeight();
	maiRowStarts.Resize(iHeight + 1);
	paiRowStarts = maiRowStarts.GetData();

	uiNumElements = macRowBlitters.NumElements();
	ui = 0;
	for (y = 0; y <= iHeight; y++)
	{
		while (ui < uiNumElements)
		{
			pcOffsetImageRowBlitter = macRowBlitters.Get(ui);
			if ((int32)pcOffsetImageRowBlitter->msOffset.y >= cRect.GetTop() + y)
			{
				break;
			}
			ui++;
		}
		paiRowStarts[y] = (int32)ui;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::PopulateSizingTables(int32 iDestWidth, int32 iDestHeight)
{
	return mpcSizing->PopulateSizingTables(mpcSourceCel->GetSubImage(), iDestWidth, iDestHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::ClipDestRect(CRectangle* pcClip, CRectangle* pcDestRect)
{
	pcDestRect->Init(0, 0, mpDestImage->GetWidth(), mpDestImage->GetHeight());

	if (pcClip)
	{
		if (pcClip->msTopLeft.x > pcDestRect->msTopLeft.x)
		{
			pcDestRect->msTopLeft.x = pcClip->msTopLeft.x;
		}
		if (pcClip->msTopLeft.y > pcDestRect->msTopLeft.y)
		{
			pcDestRect->msTopLeft.y = pcClip->msTopLeft.y;
		}
		if (pcClip->msBottomRight.x < pcDestRect->msBottomRight.x)
		{
			pcDestRect->msBottomRight.x = pcClip->msBottomRight.x;
		}
		if (pcClip->msBottomRight.y < pcDestRect->msBottomRight.y)
		{
			pcDestRect->msBottomRight.y = pcClip->msBottomRight.y;
		}
		if ((pcDestRect->msBottomRight.x <= pcDestRect->msTopLeft.x) || 
			(pcDestRect->msBottomRight.y <= pcDestRect->msTopLeft.y))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Blit(int32 iDestX, int32 iDestY, CRectangle* pcClip)
{
	size						uiNumElements;
	size						ui;
	COffsetImageRowBlitter*		pcOffsetImageRowBlitter;
	int32						xSourceStart;
	int32						xSourceEnd;
	int32						ySource;
	CRectangle					cDestRect;
	CRectangle					cSourceRect;
	int32						yDest;
	int32						xDestStart;
	void*						pvSource;
	void*						pvDest;
	int32						iSourceLeft;
	int32						iSourceTop;
	int32						xDestOffset;
	int32						yDestOffset;
	int32						xLength;
	CSubImage*					pcSubImage;

	pvSource = mpcSourceCel->GetSourceImage()->GetData();
	pvDest = mpDestImage->GetData();

	//The cel's alignment moves where it is drawn as well as where it is clipped (see GetImageDestBounds).
	pcSubImage = mpcSourceCel->GetSubImage();
	iSourceLeft = mpcSourceCel->GetSourceLeft() - pcSubImage->msAlignment.x;
	iSourceTop = mpcSourceCel->GetSourceTop() - pcSubImage->msAlignment.y;
	if (!ClipDestRect(pcClip, &cDestRect))
	{
		return true;
	}
	mpcSourceCel->GetImageDestBounds(iDestX, iDestY, &cSourceRect);

	if (cSourceRect.Outside(&cDestRect))
	{
		return true;
	}

	uiNumElements = macRowBlitters.NumElements();
	if (cSourceRect.Inside(&cDestRect))
	{
		for (ui = 0; ui < uiNumElements; ui++)
		{
			pcOffsetImageRowBlitter = macRowBlitters.Get(ui);

			ySource = (int32)pcOffsetImageRowBlitter->msOffset.y;
			yDestOffset = ySource - iSourceTop;
			yDest = iDestY + yDestOffset;

			xSourceStart = (int32)pcOffsetImageRowBlitter->msOffset.x;
			xSourceEnd = (int32)pcOffsetImageRowBlitter->muiXEnd;

			xDestOffset = xSourceStart - iSourceLeft;
			xDestStart = iDestX + xDestOffset;

			pcOffsetImageRowBlitter->mpcBlitter->Copy(&mcContext, pvSource, pvDest, xDestStart, yDest, xSourceStart, xSourceEnd, ySource);
		}

		return true;
	}
	else
	{
		for (ui = 0; ui < uiNumElements; ui++)
		{
			pcOffsetImageRowBlitter = macRowBlitters.Get(ui);

			ySource = (int32)pcOffsetImageRowBlitter->msOffset.y;
			yDestOffset = ySource - iSourceTop;
			yDest = iDestY + yDestOffset;

			if ((yDest >= cDestRect.msTopLeft.y) && (yDest < cDestRect.msBottomRight.y))
			{
				xSourceStart = (int32)pcOffsetImageRowBlitter->msOffset.x;
				xSourceEnd = (int32)pcOffsetImageRowBlitter->muiXEnd;

				xDestOffset = xSourceStart - iSourceLeft;
				xDestStart = iDestX + xDestOffset;

				if (xDestStart < cDestRect.msTopLeft.x)
				{
					xSourceStart += cDestRect.msTopLeft.x - xDestStart;
					xDestStart = cDestRect.msTopLeft.x;
				}
				xLength = xSourceEnd - xSourceStart;
				if (xDestStart + xLength > cDestRect.msBottomRight.x)
				{
					xSourceEnd -= xDestStart + xLength - cDestRect.msBottomRight.x;
				}

				//A row segment can lie entirely outside the clip even when the rest of the cel does not.
				if (xSourceStart >= xSourceEnd)
				{
					continue;
				}

				pcOffsetImageRowBlitter->mpcBlitter->Copy(&mcContext, pvSource, pvDest, xDestStart, yDest, xSourceStart, xSourceEnd, ySource);
			}
		}

		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageBlitter::Blit(int32 iDestX, int32 iDestY, int32 iDestWidth, int32 iDestHeight, CRectangle* pcClip)
{
	CSubImage*					pcSubImage;
	int32						iFullWidth;
	int32						iFullHeight;
	int32						iImageHeight;
	int32						iSourceLeft;
	CRectangle					cDestRect;
	int32						iLeft;
	int32						iTop;
	int32						xVisibleStart;
	int32						xVisibleEnd;
	int32						yVisibleStart;
	int32						yVisibleEnd;
	int32						y;
	int32						iImageRow;
	int32						xStart;
	int32						xEnd;
	int32*						paiSourceX;
	int32*						paiSourceY;
	int32*						paiDestX;
	int32*						paiRowStarts;
	size						ui;
	size						uiEnd;
	COffsetImageRowBlitter*		pcRowBlitter;
	void*						pvSource;
	void*						pvDest;

	if (!AllowsSizing())
	{
		return false;
	}

	//The whole cel, transparent borders included, is drawn iDestWidth by iDestHeight.  Rows and columns of the cel are
	//repeated to stretch it and dropped to shrink it.
	pcSubImage = mpcSourceCel->GetSubImage();
	iFullWidth = pcSubImage->GetFullWidth();
	iFullHeight = pcSubImage->GetFullHeight();
	if ((iDestWidth == iFullWidth) && (iDestHeight == iFullHeight))
	{
		return Blit(iDestX, iDestY, pcClip);
	}

	if ((iDestWidth <= 0) || (iDestHeight <= 0) || (iFullWidth <= 0) || (iFullHeight <= 0))
	{
		return true;
	}

	if (!ClipDestRect(pcClip, &cDestRect))
	{
		return true;
	}

	//The alignment is sized with the cel so a centred cel stays centred on its position.
	iLeft = iDestX + (int32)(((int64)pcSubImage->msAlignment.x * iDestWidth) / iFullWidth);
	iTop = iDestY + (int32)(((int64)pcSubImage->msAlignment.y * iDestHeight) / iFullHeight);

	//The visible part of the sized cel relative to its top left.
	xVisibleStart = cDestRect.msTopLeft.x - iLeft;
	if (xVisibleStart < 0)
	{
		xVisibleStart = 0;
	}
	xVisibleEnd = cDestRect.msBottomRight.x - iLeft;
	if (xVisibleEnd > iDestWidth)
	{
		xVisibleEnd = iDestWidth;
	}
	yVisibleStart = cDestRect.msTopLeft.y - iTop;
	if (yVisibleStart < 0)
	{
		yVisibleStart = 0;
	}
	yVisibleEnd = cDestRect.msBottomRight.y - iTop;
	if (yVisibleEnd > iDestHeight)
	{
		yVisibleEnd = iDestHeight;
	}
	if ((xVisibleStart >= xVisibleEnd) || (yVisibleStart >= yVisibleEnd))
	{
		return true;
	}

	PopulateSizingTables(iDestWidth, iDestHeight);

	pvSource = mpcSourceCel->GetSourceImage()->GetData();
	pvDest = mpDestImage->GetData();
	paiSourceX = mpcSizing->GetSizedSourceX();
	paiSourceY = mpcSizing->GetSizedSourceY();
	paiDestX = mpcSizing->GetSizedDestX();
	paiRowStarts = maiRowStarts.GetData();
	iSourceLeft = pcSubImage->mcImageRect.msTopLeft.x - pcSubImage->msOffsetTopLeft.x;
	iImageHeight = pcSubImage->GetImageHeight();

	for (y = yVisibleStart; y < yVisibleEnd; y++)
	{
		//Rows in the cel's transparent borders have nothing to draw.
		iImageRow = paiSourceY[y];
		if ((iImageRow < 0) || (iImageRow >= iImageHeight))
		{
			continue;
		}

		uiEnd = paiRowStarts[iImageRow + 1];
		for (ui = paiRowStarts[iImageRow]; ui < uiEnd; ui++)
		{
			pcRowBlitter = macRowBlitters.Get(ui);

			xStart = paiDestX[(int32)pcRowBlitter->msOffset.x - iSourceLeft];
			xEnd = paiDestX[(int32)pcRowBlitter->muiXEnd - iSourceLeft];
			if (xStart < xVisibleStart)
			{
				xStart = xVisibleStart;
			}
			if (xEnd > xVisibleEnd)
			{
				xEnd = xVisibleEnd;
			}

			//Shrinking can drop every column of a segment and clipping can leave a segment entirely outside.
			if (xStart >= xEnd)
			{
				continue;
			}

			if (iDestWidth == iFullWidth)
			{
				//Only the height changes so the columns are copied one for one by the faster blitters.
				pcRowBlitter->mpcBlitter->Copy(&mcContext, pvSource, pvDest, iLeft + xStart, iTop + y, paiSourceX[xStart], paiSourceX[xStart] + (xEnd - xStart), pcRowBlitter->msOffset.y);
			}
			else
			{
				pcRowBlitter->mpcSizingBlitter->Copy(&mcContext, pvSource, pvDest, iLeft + xStart, iLeft + xEnd, iTop + y, &paiSourceX[xStart], pcRowBlitter->msOffset.y);
			}
		}
	}

	return true;
}

