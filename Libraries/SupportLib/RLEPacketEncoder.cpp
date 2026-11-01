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
#include <string.h>
#include "RLEPacketEncoder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEPacketEncoder::Init(size uiElementSize)
{
	muiElementSize = uiElementSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEPacketEncoder::Kill(void)
{
	muiElementSize = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRLEPacketEncoder::IsSameElement(uint8* puiSource, int iFirst, int iSecond)
{
	return memcmp(&puiSource[iFirst * muiElementSize], &puiSource[iSecond * muiElementSize], muiElementSize) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// RLE compresses iNumElements elements into puiDest and returns the
// number of bytes written.  puiDest must be at least GetMaxEncodedSize
// bytes.
//
//////////////////////////////////////////////////////////////////////////
size CRLEPacketEncoder::Encode(uint8* puiSource, int iNumElements, uint8* puiDest)
{
	size	uiPos;
	int		x;
	int		iRun;
	int		iRaw;

	uiPos = 0;
	x = 0;
	while (x < iNumElements)
	{
		iRun = 1;
		while ((x + iRun < iNumElements) && (iRun < RLE_PACKET_MAX_LENGTH) && IsSameElement(puiSource, x, x + iRun))
		{
			iRun++;
		}

		if (iRun > 1)
		{
			puiDest[uiPos] = (uint8)(0x80 | (iRun - 1));
			memcpy(&puiDest[uiPos + 1], &puiSource[x * muiElementSize], muiElementSize);
			uiPos += 1 + muiElementSize;
			x += iRun;
		}
		else
		{
			//A raw packet ends where a run of two identical elements starts.
			iRaw = 1;
			while ((x + iRaw < iNumElements) && (iRaw < RLE_PACKET_MAX_LENGTH))
			{
				if ((x + iRaw + 1 < iNumElements) && IsSameElement(puiSource, x + iRaw, x + iRaw + 1))
				{
					break;
				}
				iRaw++;
			}
			puiDest[uiPos] = (uint8)(iRaw - 1);
			memcpy(&puiDest[uiPos + 1], &puiSource[(lsize)x * muiElementSize], (lsize)iRaw * muiElementSize);
			uiPos += 1 + iRaw * muiElementSize;
			x += iRaw;
		}
	}
	return uiPos;
}


//////////////////////////////////////////////////////////////////////////
//
// The worst case size is a one byte packet header for every element.
//
//////////////////////////////////////////////////////////////////////////
size CRLEPacketEncoder::GetMaxEncodedSize(int iNumElements)
{
	return (size)iNumElements * (muiElementSize + 1);
}
