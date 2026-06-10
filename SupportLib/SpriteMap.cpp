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
	mpCache = NULL;
	mpViewport = NULL;
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
	M_Pointer(mpCache);
	M_Pointer(mpViewport);
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
bool CSpriteMap::GetImageDestBounds(CRectangle* pcRect)
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
				if (pCel)
				{
					pcSubImage = pCel->GetSubImage();
					if (bFirst)
					{
						pcSubImage->GetImageDestBounds(pSprite->GetX(), pSprite->GetY(), &cBounding);
						bFirst = false;
					}
					else
					{
						pcSubImage->GetImageDestBounds(pSprite->GetX(), pSprite->GetY(), &cCelRect);
						cBounding.GrowToContain(&cCelRect);
					}
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
bool CSpriteMap::GetFullDestBounds(CRectangle* pcRect)
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
				if (pCel)
				{
					pcSubImage = pCel->GetSubImage();
					if (bFirst)
					{
						pcSubImage->GetFullDestBounds(pSprite->GetX(), pSprite->GetY(), &cBounding);
						bFirst = false;
					}
					else
					{
						pcSubImage->GetFullDestBounds(pSprite->GetX(), pSprite->GetY(), &cCelRect);
						cBounding.GrowToContain(&cCelRect);
					}
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
	int32						x;
	int32						y;
	bool						bResult;
	Ptr<CImageCelBlitterCache>	pOldCache;
	Ptr<CImage>					pOldViewport;
	Ptr<CImage>					pImage;
	bool						bExists;
	CRectangle					cBoundingRect;

	pOldViewport = mpViewport;
	pOldCache = mpCache;

	bExists = GetFullDestBounds(&cBoundingRect);
	if (!bExists)
	{
		return NULL;
	}

	pImage = CreateViewportImage(&cBoundingRect);

	mpViewport = pImage;
	mpCache = CreateBlitterCache();
	CreateCelBlitters();

	uiNumElements = maSprites.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pSprite = maSprites.Get(ui);
		pCel = pSprite->GetCel();
		if (pCel.IsNotNull())
		{
			x = pSprite->GetX();
			y = pSprite->GetY();
			bResult = pSprite->Blit(x - cBoundingRect.miLeft, y - cBoundingRect.miTop);
			if (!bResult)
			{
				mpViewport = pOldViewport;
				mpCache = pOldCache;
				return NULL;
			}
		}
	}

	mpViewport = pOldViewport;
	mpCache = pOldCache;

	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::CreateCelBlitters(void)
{
	size			ui;
	size			uiNumElements;
	Ptr<CSprite>	pSprite;
	bool			bResult;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			uiNumElements = maSprites.NumElements();
			for (ui = 0; ui < uiNumElements; ui++)
			{
				pSprite = maSprites.Get(ui);
				bResult = pSprite->CreateBlitter(mpCache);
				if (!bResult)
				{
					return false;
				}
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::ClearCelBlitters(void)
{
	size			ui;
	size			uiNumElements;
	Ptr<CSprite>	pSprite;

	uiNumElements = maSprites.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pSprite = maSprites.Get(ui);
		pSprite->ClearBlitter();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CSpriteMap::CreateViewportImage(CRectangle* pcBoundingRect)
{
	size			uiWidth;
	size			uiHeight;
	size			uiPadding;
	Ptr<CImage>		pDestImage;

	uiWidth = pcBoundingRect->GetWidth();
	uiHeight = pcBoundingRect->GetHeight();
	if (uiWidth % 8 != 0)
	{
		uiPadding = (8 - uiWidth % 8);
		uiWidth += uiPadding;
	}

	pDestImage = OMalloc<CImage>(uiWidth, uiHeight, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	if (pDestImage)
	{
		pDestImage->Clear();
	}
	return pDestImage;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CSpriteMap::CreateViewportImage(void)
{
	bool			bExists;
	CRectangle		cBoundingRect;

	bExists = GetFullDestBounds(&cBoundingRect);
	if (!bExists)
	{
		return NULL;
	}

	return CreateViewportImage(&cBoundingRect);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCelBlitterCache> CSpriteMap::CreateBlitterCache(void)
{
	if (mpViewport)
	{
		return OMalloc<CImageCelBlitterCache>(mpViewport);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::Blit(CRectangle* pcViewportRect)
{
	size			ui;
	size			uiNumElements;
	Ptr<CSprite>	pSprite;
	Ptr<CImageCel>	pCel;
	int32			x;
	int32			y;
	bool			bResult;
	Ptr<CImage>		pImage;

	uiNumElements = maSprites.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		pSprite = maSprites.Get(ui);
		pCel = pSprite->GetCel();
		if (pCel.IsNotNull())
		{
			x = pSprite->GetX();
			y = pSprite->GetY();
			bResult = pSprite->Blit(x - pcViewportRect->miLeft, y - pcViewportRect->miTop);
			if (!bResult)
			{
				return false;
			}
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void		CSpriteMap::SetBlitterCache(Ptr<CImageCelBlitterCache> pCache) { mpCache = pCache; }
void		CSpriteMap::SetViewport(Ptr<CImage> pViewport) { mpViewport = pViewport; }
Ptr<CImage> CSpriteMap::GetViewport(void) { return mpViewport; }

