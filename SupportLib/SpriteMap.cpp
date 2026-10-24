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
#include "BaseLib/PointerFunctions.h"
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
	masSprites.Init();
	maCels.Init();
	maBlitters.Init();
	mcCelIndices.Init(&ComparePtrPtr);

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::Free(void)
{
	mcCelIndices.Kill();
	masSprites.Kill();
	CBlockMap::Free();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::Class(void)
{
	CBlockMap::Class();
	U_Data(CArraySprite, masSprites);
	M_Embedded(maCels);
	M_Embedded(maBlitters);
	U_Data(CMapImageCelIndex, mcCelIndices);
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
bool CSpriteMap::AddSprite(Ptr<CImageCel> pCel, int32 x, int32 y)
{
	SSprite*	psSprite;

	if (pCel.IsNull())
	{
		return false;
	}

	psSprite = masSprites.Add();
	psSprite->sPosition.Init(x, y);
	psSprite->uiCel = AddCel(pCel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
// The index of the cel in maCels, adding it if it isn't there yet.		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSpriteMap::AddCel(Ptr<CImageCel> pCel)
{
	CImageCel*	pcCel;
	size*		puiCel;
	size		uiCel;

	pcCel = (CImageCel*)pCel.Object();
	puiCel = mcCelIndices.Get(pcCel);
	if (puiCel)
	{
		return *puiCel;
	}

	uiCel = maCels.NumElements();
	maCels.Add(pCel);
	maBlitters.Add(NULL);
	mcCelIndices.Put(pcCel, &uiCel);
	return uiCel;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSpriteMap::NumSprites(void)
{
	return masSprites.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSprite* CSpriteMap::GetSprite(size uiIndex)
{
	return masSprites.SafeGet(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSpriteMap::NumCels(void)
{
	return maCels.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImageCel> CSpriteMap::GetCel(size uiCel)
{
	return maCels.Get(uiCel);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSpriteMap::Clear(void)
{
	masSprites.ReInit();
	maCels.Clear();
	maBlitters.Clear();
	mcCelIndices.ReInit();
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
void CSpriteMap::FindImageCels(CInt32Rect* pcRectangle, MapImageCelFunction pSpriteFunction)
{
	size			uiNumSprites;
	size			uiSprite;
	SSprite*		psSprite;
	Ptr<CImageCel>	pCel;
	CInt32Rect		cCelRect;

	uiNumSprites = masSprites.NumElements();
	for (uiSprite = 0; uiSprite < uiNumSprites; uiSprite++)
	{
		psSprite = masSprites.Get(uiSprite);
		pCel = maCels.Get(psSprite->uiCel);
		pCel->GetImageDestBounds(psSprite->sPosition.x, psSprite->sPosition.y, &cCelRect);
		if (pcRectangle->Intersect(&cCelRect))
		{
			pSpriteFunction(pCel, cCelRect.msTopLeft.x - pcRectangle->msTopLeft.x, cCelRect.msTopLeft.y - pcRectangle->msTopLeft.y);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::GetImageDestBounds(CInt32Rect* pcRect)
{
	size		ui;
	size		uiNumElements;
	SSprite*	psSprite;
	CImageCel*	pcCel;
	CSubImage*	pcSubImage;
	CInt32Rect	cBounding;
	CInt32Rect	cCelRect;

	if (pcRect)
	{
		uiNumElements = masSprites.NumElements();
		if (uiNumElements > 0)
		{
			cBounding.Init();
			for (ui = 0; ui < uiNumElements; ui++)
			{
				psSprite = masSprites.Get(ui);
				pcCel = (CImageCel*)maCels.UnsafeGetObject(psSprite->uiCel);
				pcSubImage = pcCel->GetSubImage();
				if (ui == 0)
				{
					pcSubImage->GetImageDestBounds(psSprite->sPosition.x, psSprite->sPosition.y, &cBounding);
				}
				else
				{
					pcSubImage->GetImageDestBounds(psSprite->sPosition.x, psSprite->sPosition.y, &cCelRect);
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
bool CSpriteMap::GetFullDestBounds(CInt32Rect* pcRect)
{
	size		ui;
	size		uiNumElements;
	SSprite*	psSprite;
	CImageCel*	pcCel;
	CSubImage*	pcSubImage;
	CInt32Rect	cBounding;
	CInt32Rect	cCelRect;

	if (pcRect)
	{
		uiNumElements = masSprites.NumElements();
		if (uiNumElements > 0)
		{
			cBounding.Init();
			for (ui = 0; ui < uiNumElements; ui++)
			{
				psSprite = masSprites.Get(ui);
				pcCel = (CImageCel*)maCels.UnsafeGetObject(psSprite->uiCel);
				pcSubImage = pcCel->GetSubImage();
				if (ui == 0)
				{
					pcSubImage->GetFullDestBounds(psSprite->sPosition.x, psSprite->sPosition.y, &cBounding);
				}
				else
				{
					pcSubImage->GetFullDestBounds(psSprite->sPosition.x, psSprite->sPosition.y, &cCelRect);
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
	bool						bResult;
	Ptr<CImageCelBlitterCache>	pOldCache;
	Ptr<CImage>					pOldViewport;
	Ptr<CImage>					pImage;
	bool						bExists;
	CInt32Rect					cBoundingRect;

	bExists = GetFullDestBounds(&cBoundingRect);
	if (!bExists)
	{
		return NULL;
	}

	pOldViewport = mpViewport;
	pOldCache = mpCache;

	pImage = CreateViewportImage(&cBoundingRect);

	mpViewport = pImage;
	mpCache = CreateBlitterCache();
	ClearCelBlitters();

	bResult = BlitSprites(cBoundingRect.msTopLeft.x, cBoundingRect.msTopLeft.y, true);

	ClearCelBlitters();
	mpViewport = pOldViewport;
	mpCache = pOldCache;

	if (!bResult)
	{
		return NULL;
	}
	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::CreateCelBlitters(void)
{
	size	ui;
	size	uiNumElements;
	bool	bResult;

	if (mpCache && mpViewport)
	{
		uiNumElements = maCels.NumElements();
		for (ui = 0; ui < uiNumElements; ui++)
		{
			bResult = CreateCelBlitter(ui);
			if (!bResult)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::HasCelBlitters(void)
{
	size	ui;
	size	uiNumElements;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			uiNumElements = maBlitters.NumElements();
			for (ui = 0; ui < uiNumElements; ui++)
			{
				if (maBlitters.UnsafeGetObject(ui) == NULL)
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
bool CSpriteMap::UsesCelBlitters(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::CreateCelBlitter(size uiCel)
{
	Ptr<CImageBlitter>	pBlitter;

	if (mpCache && mpViewport)
	{
		if (mpCache->GetDestImage() == mpViewport)
		{
			pBlitter = mpCache->CreateImageBlitter(maCels.Get(uiCel));
			if (pBlitter.IsNull())
			{
				return false;
			}
			maBlitters.Set(uiCel, pBlitter);
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
	size	ui;
	size	uiNumElements;

	uiNumElements = maBlitters.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		maBlitters.Set(ui, NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> CSpriteMap::CreateViewportImage(CInt32Rect* pcBoundingRect)
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

	pDestImage = OMalloc<CImage>((uint32)uiWidth, (uint32)uiHeight, PT_uint8, IMAGE_OPACITY, IMAGE_DIFFUSE_RED, IMAGE_DIFFUSE_GREEN, IMAGE_DIFFUSE_BLUE, CHANNEL_STOP);
	if (pDestImage)
	{
		pDestImage->Black();
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
	CInt32Rect		cBoundingRect;

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
bool CSpriteMap::Blit(CInt32Rect* pcViewportRect, bool bEnsureBlitters)
{
	return BlitSprites(pcViewportRect->msTopLeft.x, pcViewportRect->msTopLeft.y, bEnsureBlitters);
}


//////////////////////////////////////////////////////////////////////////
//																		//
// Blits every sprite with iLeft, iTop of the map at the viewport's		//
// top left.															//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSpriteMap::BlitSprites(int32 iLeft, int32 iTop, bool bEnsureBlitters)
{
	size			ui;
	size			uiNumElements;
	SSprite*		psSprite;
	CImageBlitter*	pcBlitter;
	bool			bResult;

	uiNumElements = masSprites.NumElements();
	for (ui = 0; ui < uiNumElements; ui++)
	{
		psSprite = masSprites.Get(ui);
		pcBlitter = (CImageBlitter*)maBlitters.UnsafeGetObject(psSprite->uiCel);
		if (pcBlitter == NULL)
		{
			if (!bEnsureBlitters)
			{
				return false;
			}
			bResult = CreateCelBlitter(psSprite->uiCel);
			if (!bResult)
			{
				return false;
			}
			pcBlitter = (CImageBlitter*)maBlitters.UnsafeGetObject(psSprite->uiCel);
		}

		bResult = pcBlitter->Blit(psSprite->sPosition.x - iLeft, psSprite->sPosition.y - iTop);
		if (!bResult)
		{
			return false;
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

