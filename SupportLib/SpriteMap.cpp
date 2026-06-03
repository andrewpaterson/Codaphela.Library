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
#include "StandardLib/Objects.h"
#include "MultiImageCopier.h"
#include "ImageCelBlitterCache.h"
#include "SpriteMap.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::Init(void)
{
	PreInit();

	CBlockMap::Init();
	maSprites.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::Free(void)
{
	CBlockMap::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::Class(void)
{
	CBlockMap::Class();
	M_Embedded(maSprites);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::AddSprite(Ptr<CSprite> pSprite)
{
	maSprites.Add(pSprite);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSpriteMap::NumSprites(void)
{
	return maSprites.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::Clear(void)
{
	maSprites.Clear();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::BeginChange(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::EndChange(void)
{
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::FindImageCels(CRectangle* pcRectangle, MapImageCelFunction pSpriteFunction)
{
	size						uiNumSprites;
	size						uiSprite;
	Ptr<CSprite>				pSprite;
	CRectangle					cCelRect;
	SIntVec2					sOffset;

	uiNumSprites = maSprites.NumElements();
	for (uiSprite = 0; uiSprite < uiNumSprites; uiSprite++)
	{
		pSprite = maSprites.Get(uiSprite);
		pSprite->GetImageDestBounds(&cCelRect);
		if (pcRectangle->Intersect(&cCelRect))
		{
			pSpriteFunction(pSprite->GetCel(), cCelRect.miLeft - pcRectangle->miLeft, cCelRect.miTop - pcRectangle->miTop);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::GetImageDestBounds(int32 x, int32 y, CRectangle* pcRect)
{
	size			ui;
	size			uiNumElements;
	Ptr<CSprite>	pSprite;
	Ptr<CImageCel>	pCel;
	CSubImage*		pcSubImage;
	CRectangle		cBounding;
	CRectangle		cCelRect;
	bool			bFirst;

	if (pcRect)
	{
		cBounding.Init();
		bFirst = true;
		uiNumElements = maSprites.NumElements();
		if (uiNumElements > 0)
		{
			for (ui = 0; ui < uiNumElements; ui++)
			{
				pSprite = maSprites.Get(ui);
				pCel = pSprite->GetCel();
				pcSubImage = pCel->GetSubImage();
				if (bFirst)
				{
					pcSubImage->GetImageDestBounds(pSprite->GetX() + x, pSprite->GetY() + y, &cBounding);
					bFirst = false;
				}
				else
				{
					pcSubImage->GetImageDestBounds(pSprite->GetX() + x, pSprite->GetY() + y, &cCelRect);
					cBounding.GrowToContain(&cCelRect);
				}
			}
			pcRect->Init(&cBounding);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::GetFullDestBounds(int32 x, int32 y, CRectangle* pcRect)
{
	size			ui;
	size			uiNumElements;
	Ptr<CSprite>	pSprite;
	Ptr<CImageCel>	pCel;
	CSubImage*		pcSubImage;
	CRectangle		cBounding;
	CRectangle		cCelRect;
	bool			bFirst;

	if (pcRect)
	{
		cBounding.Init();
		bFirst = true;
		uiNumElements = maSprites.NumElements();
		if (uiNumElements > 0)
		{
			for (ui = 0; ui < uiNumElements; ui++)
			{
				pSprite = maSprites.Get(ui);
				pCel = pSprite->GetCel();
				pcSubImage = pCel->GetSubImage();
				if (bFirst)
				{
					pcSubImage->GetFullDestBounds(pSprite->GetX() + x, pSprite->GetY() + y, &cBounding);
					bFirst = false;
				}
				else
				{
					pcSubImage->GetFullDestBounds(pSprite->GetX() + x, pSprite->GetY() + y, &cCelRect);
					cBounding.GrowToContain(&cCelRect);
				}
			}
			pcRect->Init(&cBounding);
			return true;
		}

	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CSpriteMap::WriteToImage(void)
{
	size						ui;
	size						uiNumElements;
	Ptr<CSprite>				pSprite;
	Ptr<CImageCel>				pCel;
	Ptr<CImage>					pDestImage;
	CRectangle					cBoundingRect;
	bool						bExists;
	size						uiWidth;
	size						uiHeight;
	int32						x;
	int32						y;
	bool						bResult;
	Ptr<CImageCelBlitterCache>	pCache;
	size						uiPadding;

	bExists = GetFullDestBounds(0, 0, &cBoundingRect);
	if (!bExists)
	{
		return NULL;
	}

	uiWidth = cBoundingRect.GetWidth();
	uiHeight = cBoundingRect.GetHeight();
	if (uiWidth % 8 != 0)
	{
		uiPadding = (8 - uiWidth % 8);
		uiWidth += uiPadding;
	}

	pDestImage = OMalloc<CImage>(uiWidth, uiHeight, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	if (pDestImage.IsNull())
	{
		return NULL;
	}

	pDestImage->Clear();

	pCache = OMalloc<CImageCelBlitterCache>(pDestImage);

	uiNumElements = maSprites.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pSprite = maSprites.Get(ui);
		bResult = pSprite->CreateBlitter(pCache);
		if (!bResult)
		{
			pDestImage = NULL;
			return NULL;
		}
	}

	for (ui = 0; ui < uiNumElements; ui++)
	{
		pSprite = maSprites.Get(ui);
		pCel = pSprite->GetCel();
		if (pCel.IsNotNull())
		{
			x = pSprite->GetX();
			y = pSprite->GetY();
			bResult = pSprite->Blit(x - cBoundingRect.GetLeft(), y - cBoundingRect.GetTop());
			if (!bResult)
			{
				pDestImage = NULL;
				return NULL;
			}
		}
	}

	for (ui = 0; ui < uiNumElements; ui++)
	{
		pSprite = maSprites.Get(ui);
		pSprite->ClearBlitter();
	}

	pCache.Kill();

	return pDestImage;
}

