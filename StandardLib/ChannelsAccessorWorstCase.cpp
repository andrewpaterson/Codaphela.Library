/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/IntegerHelper.h"
#include "ChannelsAccessorWorstCase.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CChannelsAccessorWorstCase::Get(size iPos)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	void*				pvSource;
	size				iSourceBitPos;
	size				iDestBitPos;
	char				c[16];
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	pvSource = mpcChannels->GetData();
	iSourceBitPos = mpcChannels->GetBitStride() * iPos;
	iDestBitPos = 0;
	mpvGetBuffer[miBufferSize-1] = 0;
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		pcAccessor->GetAsSub(pcAccessor->meAccessType, pvSource, iSourceBitPos, c);
		CopyBits(mpvGetBuffer, iDestBitPos, c, 0, pcAccessor->miAccessBitSize, false);
		iDestBitPos += pcAccessor->miAccessBitSize;
	}
	return mpvGetBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorWorstCase::Set(size iPos, void* pvSource)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	void*				pvDest;
	size				iDestBitPos;
	size				iSourceBitPos;
	char				c[16 + 1];
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	pvDest = mpcChannels->GetData();
	iDestBitPos = mpcChannels->GetBitStride() * iPos;
	iSourceBitPos = 0;
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		c[pcAccessor->miAccessBitSize / 8] = 0;
		CopyBits(c, 0, pvSource, iSourceBitPos, pcAccessor->miAccessBitSize, false);
		pcAccessor->SetFromSub(pcAccessor->meAccessType, pvDest, iDestBitPos, c);
		iSourceBitPos += pcAccessor->miAccessBitSize;
	}
}

