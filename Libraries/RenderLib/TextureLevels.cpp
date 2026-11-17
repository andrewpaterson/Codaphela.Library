/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela RenderLib

Codaphela RenderLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela RenderLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela RenderLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "SupportLib/MaterialImage.h"
#include "ImageTexels.h"
#include "TextureLevels.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 HalfTextureSize(int32 iSize)
{
	return (iSize + 1) / 2;
}


//////////////////////////////////////////////////////////////////////////
//
// Each channel summed over the texels covered and divided by how many.
//
//////////////////////////////////////////////////////////////////////////
void HalveTexels(uint32* puiDest, uint32* puiSource, int32 iWidth, int32 iHeight)
{
	int32	iHalfWidth;
	int32	iHalfHeight;
	int32	x;
	int32	y;
	int32	i;
	int32	j;
	int32	iCount;
	int32	iShift;
	uint32	auiSums[4];
	uint32	uiTexel;
	uint32	uiResult;

	iHalfWidth = HalfTextureSize(iWidth);
	iHalfHeight = HalfTextureSize(iHeight);
	for (y = 0; y < iHalfHeight; y++)
	{
		for (x = 0; x < iHalfWidth; x++)
		{
			auiSums[0] = 0;
			auiSums[1] = 0;
			auiSums[2] = 0;
			auiSums[3] = 0;
			iCount = 0;
			for (j = y * 2; (j < (y * 2) + 2) && (j < iHeight); j++)
			{
				for (i = x * 2; (i < (x * 2) + 2) && (i < iWidth); i++)
				{
					uiTexel = puiSource[((size)j * (size)iWidth) + (size)i];
					for (iShift = 0; iShift < 4; iShift++)
					{
						auiSums[iShift] += (uiTexel >> (iShift * 8)) & 0xFF;
					}
					iCount++;
				}
			}
			uiResult = 0;
			for (iShift = 0; iShift < 4; iShift++)
			{
				uiResult |= ((auiSums[iShift] + ((uint32)iCount / 2)) / (uint32)iCount) << (iShift * 8);
			}
			puiDest[((size)y * (size)iHalfWidth) + (size)x] = uiResult;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each level is laid down at the end and filled from the image's own
// level that size if it has one, from the level before if not.
//
//////////////////////////////////////////////////////////////////////////
bool CTextureLevels::Init(CMaterialImage* pcImage)
{
	CImage*	pcLevel;
	int32	iWidth;
	int32	iHeight;
	int32	iLevel;
	lsize	uiStart;
	bool	bOwn;

	mauiTexels.Init();
	maiStarts.Init();
	maiWidths.Init();
	maiHeights.Init();
	if ((pcImage == NULL) || pcImage->mpImage.IsNull() || (pcImage->mpImage->GetWidth() == 0) || (pcImage->mpImage->GetHeight() == 0))
	{
		return false;
	}

	iWidth = (int32)pcImage->mpImage->GetWidth();
	iHeight = (int32)pcImage->mpImage->GetHeight();
	for (iLevel = 0; ; iLevel++)
	{
		uiStart = mauiTexels.NumElements();
		maiStarts.Add((int)uiStart);
		maiWidths.Add(iWidth);
		maiHeights.Add(iHeight);
		mauiTexels.Resize(uiStart + ((lsize)iWidth * (lsize)iHeight));

		pcLevel = (iLevel == 0) ? &pcImage->mpImage : (((size)(iLevel - 1) < pcImage->mapLevels.NumElements()) ? &pcImage->mapLevels.Get((size)(iLevel - 1)) : NULL);
		bOwn = (pcLevel != NULL) && ((int32)pcLevel->GetWidth() == iWidth) && ((int32)pcLevel->GetHeight() == iHeight) && GetImageTexels(pcLevel, mauiTexels.GetData() + uiStart);
		if (!bOwn)
		{
			if (iLevel == 0)
			{
				mauiTexels.ReInit();
				maiStarts.ReInit();
				maiWidths.ReInit();
				maiHeights.ReInit();
				return false;
			}
			HalveTexels(mauiTexels.GetData() + uiStart, GetTexels(iLevel - 1), GetWidth(iLevel - 1), GetHeight(iLevel - 1));
		}
		if ((iWidth == 1) && (iHeight == 1))
		{
			return true;
		}
		iWidth = HalfTextureSize(iWidth);
		iHeight = HalfTextureSize(iHeight);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextureLevels::Kill(void)
{
	maiHeights.Kill();
	maiWidths.Kill();
	maiStarts.Kill();
	mauiTexels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 CTextureLevels::NumLevels(void) const { return (int32)maiWidths.NumElements(); }
uint32* CTextureLevels::GetTexels(int32 iLevel) { return mauiTexels.GetData() + maiStarts.GetValue((lsize)iLevel); }
int32 CTextureLevels::GetWidth(int32 iLevel) { return maiWidths.GetValue((lsize)iLevel); }
int32 CTextureLevels::GetHeight(int32 iLevel) { return maiHeights.GetValue((lsize)iLevel); }

