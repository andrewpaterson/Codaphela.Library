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
#include "ChannelsAccessorContiguous.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorContiguous::Init(CChannels* pcChannels, CArrayChannelAccessor* pcAccessors, int iByteSize, int iBitSize, int iBufferSize)
{
	CChannelAccessor*	pcAccessor;

	CChannelsAccessor::Init(pcChannels, pcAccessors, iByteSize, iBitSize, iBufferSize);

	pcAccessor = macAccessors.Get(0);
	miByteOffset = pcAccessor->miChannelByteOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CChannelsAccessorContiguous::Get(int iPos)
{
	void*				pvSource;

	pvSource = RemapSinglePointer(mpcChannels->GetData(), iPos * mpcChannels->GetByteStride() + miByteOffset);
	return pvSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessorContiguous::Set(int iPos, void* pvSource)
{
	void*				pvDest;

	pvDest = RemapSinglePointer(mpcChannels->GetData(), iPos * mpcChannels->GetByteStride() + miByteOffset);
	memcpy_fast(pvDest, pvSource, miByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannelsAccessorContiguous::IsContiguous(void)
{
	return TRUE;
}
