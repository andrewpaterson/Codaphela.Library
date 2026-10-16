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
#include "RLEPacketDecoder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEPacketDecoder::Init(uint8* puiData, size uiDataSize, size uiElementSize)
{
	mpuiData = puiData;
	muiDataSize = uiDataSize;
	muiElementSize = uiElementSize;
	muiPosition = 0;
	miPacketRemaining = 0;
	mbRun = false;
	mpuiRunElement = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRLEPacketDecoder::Kill(void)
{
	mpuiData = NULL;
	muiDataSize = 0;
	mpuiRunElement = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the next decoded element, or NULL if the data is truncated.
// Every element of a run returns the same pointer so the caller can
// avoid converting it more than once.
//
//////////////////////////////////////////////////////////////////////////
uint8* CRLEPacketDecoder::Next(void)
{
	uint8*	puiElement;

	if (miPacketRemaining == 0)
	{
		if (muiPosition >= muiDataSize)
		{
			return NULL;
		}
		mbRun = (mpuiData[muiPosition] & 0x80) != 0;
		miPacketRemaining = (mpuiData[muiPosition] & 0x7F) + 1;
		muiPosition++;
		mpuiRunElement = NULL;
	}

	if (mbRun && (mpuiRunElement != NULL))
	{
		miPacketRemaining--;
		return mpuiRunElement;
	}

	if (muiPosition + muiElementSize > muiDataSize)
	{
		return NULL;
	}
	puiElement = &mpuiData[muiPosition];
	muiPosition += muiElementSize;
	miPacketRemaining--;
	if (mbRun)
	{
		mpuiRunElement = puiElement;
	}
	return puiElement;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of bytes of data consumed so far.
//
//////////////////////////////////////////////////////////////////////////
size CRLEPacketDecoder::GetPosition(void)
{
	return muiPosition;
}
