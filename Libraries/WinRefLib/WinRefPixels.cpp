/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "BaseLib/FNVHash.h"
#include "BaseLib/PointerFunctions.h"
#include "SupportLib/PixelDifference.h"
#include "WinRefPixels.h"


//////////////////////////////////////////////////////////////////////////
//
// No pixels until SetSize.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefPixels::Init(void)
{
	mpuiData = NULL;
	miWidth = 0;
	miHeight = 0;
	muiPixelBytes = 0;
	muiRowBytes = 0;
	mbOwned = true;
}


//////////////////////////////////////////////////////////////////////////
//
// A view of pixels held elsewhere: they are read, or written, but not
// freed.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefPixels::Init(void* pvData, int32 iWidth, int32 iHeight, size uiPixelBytes, lsize uiRowBytes)
{
	mpuiData = (uint8*)pvData;
	miWidth = iWidth;
	miHeight = iHeight;
	muiPixelBytes = uiPixelBytes;
	muiRowBytes = uiRowBytes;
	mbOwned = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefPixels::Kill(void)
{
	if (mbOwned)
	{
		SafeFree(mpuiData);
	}
	mpuiData = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Black pixels of the new size, if it is new; the pixels are kept if not.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefPixels::SetSize(int32 iWidth, int32 iHeight, size uiPixelBytes)
{
	if ((iWidth == miWidth) && (iHeight == miHeight) && (uiPixelBytes == muiPixelBytes))
	{
		return;
	}

	SafeFree(mpuiData);
	miWidth = iWidth;
	miHeight = iHeight;
	muiPixelBytes = uiPixelBytes;
	muiRowBytes = (lsize)iWidth * uiPixelBytes;
	mpuiData = (uint8*)malloc(muiRowBytes * iHeight);
	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefPixels::Clear(void)
{
	memset(mpuiData, 0, muiRowBytes * miHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// pcSourceArea of pcSource copied with its top left at iDestX, iDestY,
// clipped to both.  The pixels must be the same size.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefPixels::Copy(CWinRefPixels* pcSource, CInt32Rect* pcSourceArea, int32 iDestX, int32 iDestY)
{
	CInt32Rect	cDest;
	CInt32Rect	cArea;
	int32		iOffsetX;
	int32		iOffsetY;
	lsize		uiRowBytes;
	int32		y;

	//Where the source area goes, clipped to the source's pixels and these.
	iOffsetX = iDestX - pcSourceArea->GetLeft();
	iOffsetY = iDestY - pcSourceArea->GetTop();
	cDest.Init(pcSourceArea);
	pcSource->GetArea(&cArea);
	cDest.ClipTo(&cArea);
	cDest.Init(cDest.GetLeft() + iOffsetX, cDest.GetTop() + iOffsetY, cDest.GetRight() + iOffsetX, cDest.GetBottom() + iOffsetY);
	GetArea(&cArea);
	cDest.ClipTo(&cArea);
	if ((cDest.GetWidth() <= 0) || (cDest.GetHeight() <= 0))
	{
		return;
	}

	uiRowBytes = (lsize)cDest.GetWidth() * muiPixelBytes;
	for (y = cDest.GetTop(); y < cDest.GetBottom(); y++)
	{
		memcpy(GetPixel(cDest.GetLeft(), y), pcSource->GetPixel(cDest.GetLeft() - iOffsetX, y - iOffsetY), uiRowBytes);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// pcArea's place and pixels, clipped to these, mixed into ulliHash.
//
//////////////////////////////////////////////////////////////////////////
uint64 CWinRefPixels::Hash(CInt32Rect* pcArea, uint64 ulliHash)
{
	CInt32Rect	cArea;
	CInt32Rect	cAll;
	int32		aiPlace[4];
	int32		y;

	cArea.Init(pcArea);
	GetArea(&cAll);
	cArea.ClipTo(&cAll);
	if ((cArea.GetWidth() <= 0) || (cArea.GetHeight() <= 0))
	{
		return ulliHash;
	}

	aiPlace[0] = cArea.GetLeft();
	aiPlace[1] = cArea.GetTop();
	aiPlace[2] = cArea.GetRight();
	aiPlace[3] = cArea.GetBottom();
	ulliHash = FNVHash64(aiPlace, sizeof(aiPlace), ulliHash);
	for (y = cArea.GetTop(); y < cArea.GetBottom(); y++)
	{
		ulliHash = FNVHash64(GetPixel(cArea.GetLeft(), y), (lsize)cArea.GetWidth() * muiPixelBytes, ulliHash);
	}
	return ulliHash;
}


//////////////////////////////////////////////////////////////////////////
//
// The pixels these and pcOther, the same size, have different.  Returns
// how many; pcDiffers is the rectangle around them.
//
//////////////////////////////////////////////////////////////////////////
lsize CWinRefPixels::FindDifference(CWinRefPixels* pcOther, CInt32Rect* pcDiffers)
{
	return FindPixelDifference(mpuiData, muiRowBytes, pcOther->mpuiData, pcOther->muiRowBytes, muiPixelBytes, miWidth, miHeight, pcDiffers);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefPixels::GetArea(CInt32Rect* pcDest)
{
	pcDest->Init(0, 0, miWidth, miHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint8* CWinRefPixels::GetPixel(int32 x, int32 y)
{
	return &mpuiData[(lsize)y * muiRowBytes + (lsize)x * muiPixelBytes];
}


int32 CWinRefPixels::GetWidth(void) { return miWidth; }
int32 CWinRefPixels::GetHeight(void) { return miHeight; }
size CWinRefPixels::GetPixelBytes(void) { return muiPixelBytes; }

