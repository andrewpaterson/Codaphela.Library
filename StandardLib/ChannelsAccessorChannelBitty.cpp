/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "ChannelsAccessorChannelBitty.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CChannelsAccessorChannelBitty::Get(size iPos)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	void*				pvDest;
	void*				pvSource;
	size				iSourceBitPos;
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	pvSource = mpcChannels->GetData();
	iSourceBitPos = mpcChannels->GetBitStride() * iPos;
	pvDest = mpvGetBuffer;
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		pcAccessor->GetAsSub(pcAccessor->meAccessType, pvSource, iSourceBitPos, pvDest);
		pvDest = RemapSinglePointer(pvDest, pcAccessor->miAccessByteSize);
	}
	return mpvGetBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorChannelBitty::Set(size iPos, void* pvSource)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	void*				pvDest;
	size				iDestBitPos;
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	pvDest = mpcChannels->GetData();
	iDestBitPos = mpcChannels->GetBitStride() * iPos;
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		pcAccessor->SetFromSub(pcAccessor->meAccessType, pvDest, iDestBitPos, pvSource);
		pvSource = RemapSinglePointer(pvSource, pcAccessor->miAccessByteSize);
	}
}

