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

** ------------------------------------------------------------------------ **/
#include "RLEBitmapDecoder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEBitmapDecoder::Init(int iWidth, int iHeight, bool bFourBit)
{
	miWidth = iWidth;
	miHeight = iHeight;
	mbFourBit = bFourBit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEBitmapDecoder::Kill(void)
{
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Pixels outside the image are ignored.
//
//////////////////////////////////////////////////////////////////////////
void CRLEBitmapDecoder::SetIndex(uint8* puiIndices, int x, int y, uint8 uiIndex)
{
	if ((x < miWidth) && (y < miHeight))
	{
		puiIndices[y * miWidth + x] = uiIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes from the current position of pcFile into puiIndices (miWidth *
// miHeight bytes).  The rows are in file order (bottom up).  Pixels that
// are skipped are left unchanged.  The end of the file is treated as the
// end of the bitmap, except part way through a delta or absolute run.
//
//////////////////////////////////////////////////////////////////////////
bool CRLEBitmapDecoder::Decode(CFileReader* pcFile, uint8* puiIndices)
{
	int		x;
	int		y;
	uint8	uiCount;
	uint8	uiValue;
	uint8	uiDeltaX;
	uint8	uiDeltaY;
	uint8	uiByte;
	uint	i;

	x = 0;
	y = 0;
	for (;;)
	{
		if (!pcFile->ReadInt8(&uiCount) || !pcFile->ReadInt8(&uiValue))
		{
			return true;
		}

		if (uiCount > 0)
		{
			for (i = 0; (i < uiCount) && (x < miWidth); i++)
			{
				if (mbFourBit)
				{
					SetIndex(puiIndices, x, y, (uint8)((i & 1) ? (uiValue & 0xF) : (uiValue >> 4)));
				}
				else
				{
					SetIndex(puiIndices, x, y, uiValue);
				}
				x++;
			}
		}
		else if (uiValue == 0)
		{
			//End of line.
			x = 0;
			y++;
		}
		else if (uiValue == 1)
		{
			//End of bitmap.
			return true;
		}
		else if (uiValue == 2)
		{
			//Delta.
			if (!pcFile->ReadInt8(&uiDeltaX) || !pcFile->ReadInt8(&uiDeltaY))
			{
				return false;
			}
			x += uiDeltaX;
			y += uiDeltaY;
		}
		else
		{
			//Absolute run of uiValue pixels.  RLE4 has two pixels per byte.
			uiByte = 0;
			for (i = 0; i < uiValue; i++)
			{
				if (!mbFourBit || ((i & 1) == 0))
				{
					if (!pcFile->ReadInt8(&uiByte))
					{
						return false;
					}
				}
				if (mbFourBit)
				{
					SetIndex(puiIndices, x, y, (uint8)((i & 1) ? (uiByte & 0xF) : (uiByte >> 4)));
				}
				else
				{
					SetIndex(puiIndices, x, y, uiByte);
				}
				x++;
			}

			//Padded to a 16 bit boundary.
			if ((mbFourBit ? (uiValue + 1) / 2 : uiValue) & 1)
			{
				if (!pcFile->ReadInt8(&uiByte))
				{
					return true;
				}
			}
		}
	}
}
