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
#include "BitmapPacker.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitRows::Init(int iWidth, int iHeight)
{
	miWidth = iWidth;
	miHeight = 0;
	miWords = (iWidth + 31) / 32;
	mauiWords.Init();
	SetHeight(iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitRows::Kill(void)
{
	mauiWords.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitRows::Set(int x, int y)
{
	lsize	uiWord;

	uiWord = ((lsize)y * (lsize)miWords) + (lsize)(x / 32);
	mauiWords.SetValue(uiWord, mauiWords.GetValue(uiWord) | ((uint32)1 << (x % 32)));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBitRows::Get(int x, int y)
{
	if ((x < 0) || (y < 0) || (x >= miWidth) || (y >= miHeight))
	{
		return false;
	}
	return (mauiWords.GetValue(((lsize)y * (lsize)miWords) + (lsize)(x / 32)) & ((uint32)1 << (x % 32))) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitRows::SetHeight(int iHeight)
{
	lsize	uiOld;
	lsize	uiNew;
	lsize	i;

	uiOld = (lsize)miHeight * (lsize)miWords;
	uiNew = (lsize)iHeight * (lsize)miWords;
	mauiWords.Resize(uiNew);
	for (i = uiOld; i < uiNew; i++)
	{
		mauiWords.SetValue(i, 0);
	}
	miHeight = iHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CBitRows::GetWord(int y, int iBit)
{
	uint32*	puiRow;
	uint32	uiLow;
	uint32	uiHigh;
	int		iWord;
	int		iShift;

	if ((y < 0) || (y >= miHeight) || (iBit >= miWidth))
	{
		return 0;
	}
	puiRow = mauiWords.GetData() + ((lsize)y * (lsize)miWords);
	iWord = iBit / 32;
	iShift = iBit % 32;
	uiLow = puiRow[iWord];
	if (iShift == 0)
	{
		return uiLow;
	}
	uiHigh = (iWord + 1 < miWords) ? puiRow[iWord + 1] : 0;
	return (uiLow >> iShift) | (uiHigh << (32 - iShift));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBitRows::Count(void)
{
	int		y;
	int		iCount;

	iCount = 0;
	for (y = 0; y < miHeight; y++)
	{
		iCount += CountRow(y);
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBitRows::CountRow(int y)
{
	uint32*	puiRow;
	uint32	uiWord;
	int		k;
	int		iCount;

	iCount = 0;
	puiRow = mauiWords.GetData() + ((lsize)y * (lsize)miWords);
	for (k = 0; k < miWords; k++)
	{
		for (uiWord = puiRow[k]; uiWord != 0; uiWord &= uiWord - 1)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
// pcDest is two bits wider and taller, so bit (x, y) is its (x + 1,
// y + 1).
//
//////////////////////////////////////////////////////////////////////////
void CBitRows::Grow(CBitRows* pcDest)
{
	int		x;
	int		y;
	int		i;
	int		j;
	bool	bSet;

	pcDest->Init(miWidth + 2, miHeight + 2);
	for (y = 0; y < pcDest->miHeight; y++)
	{
		for (x = 0; x < pcDest->miWidth; x++)
		{
			bSet = false;
			for (j = y - 2; (j <= y) && !bSet; j++)
			{
				for (i = x - 2; (i <= x) && !bSet; i++)
				{
					bSet = Get(i, j);
				}
			}
			if (bSet)
			{
				pcDest->Set(x, y);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitRows::Turn(CBitRows* pcDest)
{
	int		x;
	int		y;

	pcDest->Init(miHeight, miWidth);
	for (y = 0; y < miHeight; y++)
	{
		for (x = 0; x < miWidth; x++)
		{
			if (Get(x, y))
			{
				pcDest->Set(miHeight - 1 - y, x);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitmapPacker::Init(int iWidth)
{
	mcSpace.Init(iWidth, 0);
	maiClear.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitmapPacker::Kill(void)
{
	maiClear.Kill();
	mcSpace.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The shape's words against the space's at the same bits, row by row.
//
//////////////////////////////////////////////////////////////////////////
bool CBitmapPacker::Fits(CBitRows* pcShape, int x, int y)
{
	uint32*	puiRow;
	int		iRow;
	int		k;

	if ((x < 0) || (y < 0) || (x + pcShape->miWidth > mcSpace.miWidth))
	{
		return false;
	}
	for (iRow = 0; (iRow < pcShape->miHeight) && (y + iRow < mcSpace.miHeight); iRow++)
	{
		puiRow = pcShape->mauiWords.GetData() + ((lsize)iRow * (lsize)pcShape->miWords);
		for (k = 0; k < pcShape->miWords; k++)
		{
			if ((puiRow[k] & mcSpace.GetWord(y + iRow, x + (k * 32))) != 0)
			{
				return false;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Above the space's rows everything fits, so a shape no wider than the
// space always has a place.
//
//////////////////////////////////////////////////////////////////////////
bool CBitmapPacker::Find(CBitRows* pcShape, int* px, int* py)
{
	CArrayLargeInt	aiSet;
	int				x;
	int				y;
	int				iRow;
	bool			bRoom;

	if (pcShape->miWidth > mcSpace.miWidth)
	{
		return false;
	}
	aiSet.Init();
	for (iRow = 0; iRow < pcShape->miHeight; iRow++)
	{
		aiSet.Add(pcShape->CountRow(iRow));
	}
	for (y = 0; y <= mcSpace.miHeight; y++)
	{
		bRoom = true;
		for (iRow = 0; (iRow < pcShape->miHeight) && (y + iRow < mcSpace.miHeight) && bRoom; iRow++)
		{
			bRoom = (aiSet.GetValue((lsize)iRow) <= maiClear.GetValue((lsize)(y + iRow)));
		}
		for (x = 0; bRoom && (x + pcShape->miWidth <= mcSpace.miWidth); x++)
		{
			if (Fits(pcShape, x, y))
			{
				*px = x;
				*py = y;
				aiSet.Kill();
				return true;
			}
		}
	}
	aiSet.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitmapPacker::Add(CBitRows* pcShape, int x, int y)
{
	int		i;
	int		j;

	while (mcSpace.miHeight < y + pcShape->miHeight)
	{
		mcSpace.SetHeight(mcSpace.miHeight + 1);
		maiClear.Add(mcSpace.miWidth);
	}
	for (j = 0; j < pcShape->miHeight; j++)
	{
		for (i = 0; i < pcShape->miWidth; i++)
		{
			if (pcShape->Get(i, j))
			{
				mcSpace.Set(x + i, y + j);
			}
		}
		maiClear.SetValue((lsize)(y + j), maiClear.GetValue((lsize)(y + j)) - pcShape->CountRow(j));
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBitmapPacker::GetHeight(void)
{
	return mcSpace.miHeight;
}

