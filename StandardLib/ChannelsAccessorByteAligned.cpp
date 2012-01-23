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
#include "ChannelsAccessorByteAligned.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CChannelsAccessorByteAligned::Get(int iPos)
{
	int					i;
	CChannelAccessor*	pcAccessor;
	void*				pvDest;
	void*				pvSource;

	pvSource = RemapSinglePointer(mpcChannels->GetData(), iPos * mpcChannels->GetByteStride());
	pvDest = mpvGetBuffer;
	for (i = 0; i < macAccessors.NumElements(); i++)
	{
		pcAccessor = macAccessors.Get(i);
		pcAccessor->GetNative(pvSource, pvDest);
		pvDest = RemapSinglePointer(pvDest, pcAccessor->miAccessByteSize);
	}
	return mpvGetBuffer;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorByteAligned::Set(int iPos, void* pvSource)
{
	int					i;
	CChannelAccessor*	pcAccessor;
	void*				pvDest;

	pvDest = RemapSinglePointer(mpcChannels->GetData(), iPos * mpcChannels->GetByteStride());
	for (i = 0; i < macAccessors.NumElements(); i++)
	{
		pcAccessor = macAccessors.Get(i);
		pcAccessor->SetNative(pvDest, pvSource);
		pvSource = RemapSinglePointer(pvSource, pcAccessor->miAccessByteSize);
	}
}

