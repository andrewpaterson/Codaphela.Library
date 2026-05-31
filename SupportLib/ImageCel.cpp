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
#include "BaseLib/TypeConverter.h"
#include "ImageAccessorCreator.h"
#include "ImageColour.h"
#include "ColourARGB32.h"
#include "ImageCel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Init(Ptr<CImage> pcSourceImage, bool bCropTransparentBorders)
{
	Init(pcSourceImage, 0, 0, pcSourceImage->miWidth, pcSourceImage->miHeight);
	if (bCropTransparentBorders)
	{
		CropTransparentBorders();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Init(Ptr<CImage> pcSourceImage, CRectangle* pcRect)
{
	Init(pcSourceImage, pcRect->miLeft, pcRect->miTop, pcRect->miRight, pcRect->miBottom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Init(Ptr<CImage> pcSourceImage, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom)
{
	Init(pcSourceImage, iLeft, iTop, iRight, iBottom, 0, 0, 0, 0, SUB_IMAGE_ALIGNMENT_LEFT | SUB_IMAGE_ALIGNMENT_TOP);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Init(Ptr<CImage> pcSourceImage, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom, int32 iLeftOffset, int32 iTopOffset, int32 iRightOffset, int32 iBottomOffset, int32 iAlignment)
{
	PreInit();

	mcSubImage.Init(iLeft, iTop, iRight, iBottom, iLeftOffset, iTopOffset, iRightOffset, iBottomOffset, iAlignment);
	mpSourceImage = pcSourceImage;

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Class(void)
{
	U_Data(CSubImage, mcSubImage);
	M_Pointer(mpSourceImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCel::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCel::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Copy(CImageCel* pcSource)
{
	mcSubImage = *pcSource->GetSubImage();
	mpSourceImage = pcSource->GetSourceImage();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::CropTransparentBorders(void)
{
	CPixelOpacityChannel	cOpacity;

	cOpacity.Init(&mpSourceImage);
	CropTransparentBorders(&cOpacity);
	cOpacity.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::CropTransparentBorders(CPixelOpacityBase* pcPixelOpacity)
{
	int32 					iTop;
	int32 					iBottom;
	int32 					iLeft;
	int32 					iRight;
	CRectangle				mcSource;

	mcSubImage.GetImageSourceBounds(&mcSource);

	for (iTop = mcSource.miTop; iTop < mcSource.miBottom; iTop++)
	{
		if (!IsRowTransparent(pcPixelOpacity, iTop, mcSource.miLeft, mcSource.miRight))
		{
			break;
		}
	}

	if (iTop == mcSource.miBottom)
	{
		mcSubImage.AdjustToEmpty();
		return;
	}

	for (iBottom = mcSource.miBottom - 1; iBottom >= mcSource.miTop; iBottom--)
	{
		if (!IsRowTransparent(pcPixelOpacity, iBottom, mcSource.miLeft, mcSource.miRight))
		{
			break;
		}
	}

	for (iLeft = mcSource.miLeft; iLeft < mcSource.miRight; iLeft++)
	{
		if (!IsColumnTransparent(pcPixelOpacity, iLeft, iTop, iBottom + 1))
		{
			break;
		}
	}

	for (iRight = mcSource.miRight - 1; iRight >= mcSource.miLeft; iRight--)
	{
		if (!IsColumnTransparent(pcPixelOpacity, iRight, iTop, iBottom + 1))
		{
			break;
		}
	}
	
	mcSubImage.AdjustImageRect(iLeft, iTop, iRight+1, iBottom+1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::CopyParam(SImageCopy* psCopy, int32 iDestX, int32 iDestY, int32 iDestWith, int32 iDestHeight)
{
	SInt32Vec2	sPos;

	sPos = mcSubImage.GetImageDestPos(iDestX, iDestY);
	psCopy->Init(sPos.x, sPos.y, mcSubImage.mcImageRect.miLeft, mcSubImage.mcImageRect.miTop, mcSubImage.mcImageRect.miRight, mcSubImage.mcImageRect.miBottom, iDestWith, iDestHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::GetImageSourceBounds(CRectangle* pcReturn)
{
	mcSubImage.GetImageSourceBounds(pcReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Print(CChars* psz)
{
	CImageAccessor*		pcAccessorRGB;
	CImageAccessor*		pcAccessorOpacity;
	ARGB32				sRGB;
	uint				uiGrey;
	char				c;
	uint8				uiOpacity;
	SImageCopy			sCopy;
	uint				isx;
	uint				isy;
	uint				idx;
	uint				idy;

	CopyParam(&sCopy, 0, 0, mcSubImage.GetFullWidth(), mcSubImage.GetFullHeight());
	if (!sCopy.bValid)
	{
		return;
	}

	pcAccessorRGB = CImageAccessorCreator::Create(&mpSourceImage, PT_uint8, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	pcAccessorOpacity = CImageAccessorCreator::Create(&mpSourceImage, PT_uint8, IMAGE_OPACITY, CHANNEL_STOP);

	pcAccessorRGB->SyncDataCache();

	for (isy = sCopy.uiSourceY1; isy < sCopy.uiSourceY2; isy++)
	{
		idy = sCopy.uiDestY + isy - sCopy.uiSourceY1;
		for (isx = sCopy.uiSourceX1; isx < sCopy.uiSourceX2; isx++)
		{
			idx = sCopy.uiDestX + isx - sCopy.uiSourceX1;

			sRGB = 0;
			pcAccessorRGB->Get(isx, isy, &sRGB);
			pcAccessorOpacity->Get(isx, isy, &uiOpacity);
			uiGrey = (uint)Get8BitRedColour(sRGB) + (uint)Get8BitGreenColour(sRGB) + (uint)Get8BitBlueColour(sRGB);
			uiGrey /= 3;
			if (uiOpacity > 0)
			{
				if (uiGrey <= 51)
				{
					c = '·';
				}
				else if (uiGrey <= 102)
				{
					c = '•';
				}
				else if (uiGrey <= 154)
				{
					c = 'o';
				}
				else if (uiGrey <= 205)
				{
					c = 'O';
				}
				else
				{
					c = '@';
				}
			}
			else
			{
				c = ' ';
			}
			psz->Append(c);
		}
		psz->AppendNewLine();
	}

	pcAccessorRGB->Kill();
	pcAccessorOpacity->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCel::IsColumnTransparent(CPixelOpacityBase* pcOpacity, int32 x, int32 y1, int32 y2)
{
	int32		y;

	for (y = y1; y < y2; y++)
	{
		if (!pcOpacity->IsTransparent(x, y))
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
bool CImageCel::IsRowTransparent(CPixelOpacityBase* pcOpacity, int32 y, int32 x1, int32 x2)
{
	int32		x;

	for (x = x1; x < x2; x++)
	{
		if (!pcOpacity->IsTransparent(x, y))
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
void CImageCel::GetAllChannels(CArrayChannel* pasChannels)
{
	mpSourceImage->GetAllChannels(pasChannels);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CImageCel::MustFixDrawOpacity(void)
{
	if (mpSourceImage->HasChannel(IMAGE_OPACITY))
	{
		//If we already have an opacity channel then it has been used correctly
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::FixDrawOpacity(CImageAccessor* pcDestOpacity, int32 iDestTop, int32 iDestLeft)
{
	CPixelOpacityChannel	cOpacity;

	cOpacity.Init(&mpSourceImage);
	FixDrawOpacity(&cOpacity, pcDestOpacity, iDestLeft, iDestTop);
	cOpacity.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::FixDrawOpacity(CPixelOpacityBase* pcPixelOpacity, CImageAccessor* pcDestOpacity, int32 iDestTop, int32 iDestLeft)
{
	int32						x;
	int32						y;
	int32						iDestX;
	int32						iDestY;
	SImageColour			sOneAlpha;
	SImageColour			sZeroAlpha;
	CImageColourOpacity		cA;

	cA.Init(1.0f);
	pcDestOpacity->MakeColour(&sOneAlpha, &cA);
	sZeroAlpha.Zero();

	iDestY = iDestTop;
	for (y = mcSubImage.mcImageRect.miTop; y < mcSubImage.mcImageRect.miBottom; y++)
	{
		iDestX = iDestLeft;
		for (x = mcSubImage.mcImageRect.miLeft; x < mcSubImage.mcImageRect.miRight; x++)
		{
			if (pcPixelOpacity->IsTransparent(x, y))
			{
				pcDestOpacity->Set(iDestX, iDestY, &sZeroAlpha);
			}
			else
			{
				pcDestOpacity->Set(iDestX, iDestY, &sOneAlpha);
			}
			iDestX++;
		}
		iDestY++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSubImage* CImageCel::GetSubImage(void)
{
	return &mcSubImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::SetSubImage(CSubImage* pcSubImage)
{
	mcSubImage.Copy(pcSubImage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageCel::GetImageDestBounds(int32 x, int32 y, CRectangle* pcReturn)
{
	mcSubImage.GetImageDestBounds(x, y, pcReturn);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CImageCel::GetSourceImage(void)
{
	return mpSourceImage;
}

