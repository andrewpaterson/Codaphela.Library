/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "ChannelsAccessorTransformer.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorTransformer::Init(CChannels* pcChannels, CArrayChannelAccessor* pcAccessors, size iByteSize, size iBitSize)
{
	CChannelAccessor*	pcAccessor;

	CChannelsAccessor::Init(pcChannels, pcAccessors, iByteSize, iBitSize);

	pcAccessor = macAccessors.Get(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CChannelsAccessorTransformer::Get(size iPos)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorTransformer::Set(size iPos, void* pvSource)
{
	size				i;
	CChannelAccessor*	pcAccessor;
	void*				pvDest;
	size				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	pvDest = RemapSinglePointer(mpcChannels->GetData(), iPos * mpcChannels->GetByteStride());
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		pcAccessor->SetFrom(pcAccessor->meAccessType, pvDest, pvSource);
		pvSource = RemapSinglePointer(pvSource, pcAccessor->miAccessByteSize);
	}
}

