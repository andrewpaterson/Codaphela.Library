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
// Decodes into puiIndices (miWidth * miHeight bytes).  The rows are in
// file order (bottom up).  Pixels that are skipped are left unchanged.
//
//////////////////////////////////////////////////////////////////////////
bool CRLEBitmapDecoder::Decode(uint8* puiData, size uiDataSize, uint8* puiIndices)
{
	size	uiPos;
	int		x;
	int		y;
	int		iCount;
	int		iValue;
	int		i;
	size	uiBytes;

	uiPos = 0;
	x = 0;
	y = 0;
	while (uiPos + 1 < uiDataSize)
	{
		iCount = puiData[uiPos];
		iValue = puiData[uiPos + 1];
		uiPos += 2;
		if (iCount > 0)
		{
			for (i = 0; (i < iCount) && (x < miWidth); i++)
			{
				if (mbFourBit)
				{
					SetIndex(puiIndices, x, y, (uint8)((i & 1) ? (iValue & 0xF) : (iValue >> 4)));
				}
				else
				{
					SetIndex(puiIndices, x, y, (uint8)iValue);
				}
				x++;
			}
		}
		else if (iValue == 0)
		{
			//End of line.
			x = 0;
			y++;
		}
		else if (iValue == 1)
		{
			//End of bitmap.
			return true;
		}
		else if (iValue == 2)
		{
			//Delta.
			if (uiPos + 1 >= uiDataSize)
			{
				return false;
			}
			x += puiData[uiPos];
			y += puiData[uiPos + 1];
			uiPos += 2;
		}
		else
		{
			//Absolute run of iValue pixels, padded to a 16 bit boundary.
			uiBytes = mbFourBit ? (iValue + 1) / 2 : iValue;
			if (uiPos + uiBytes > uiDataSize)
			{
				return false;
			}
			for (i = 0; i < iValue; i++)
			{
				if (mbFourBit)
				{
					SetIndex(puiIndices, x, y, (uint8)((i & 1) ? (puiData[uiPos + i / 2] & 0xF) : (puiData[uiPos + i / 2] >> 4)));
				}
				else
				{
					SetIndex(puiIndices, x, y, puiData[uiPos + i]);
				}
				x++;
			}
			uiPos += uiBytes;
			if (uiBytes & 1)
			{
				uiPos++;
			}
		}
	}
	return true;
}
