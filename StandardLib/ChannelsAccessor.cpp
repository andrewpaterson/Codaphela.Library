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
#include "Unknowns.h"
#include "BaseLib/PointerFunctions.h"
#include "ChannelsAccessor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessor::Init(CChannels* pcChannels, CArrayChannelAccessor* pcAccessors, uint32 iByteSize, uint32 iBitSize)
{
	mpcChannels = pcChannels;
	memcpy(&macAccessors, pcAccessors, sizeof(CArrayChannelAccessor));
	memset(pcAccessors, 0, sizeof(CArrayChannelAccessor));
	miByteSize = iByteSize;
	miBitSize = iBitSize;
	msBuffer.Init(GetBufferSize());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessor::Kill(void)
{
	mpcChannels = NULL;
	macAccessors.Kill();
	msBuffer.Kill();
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannelsAccessor::IsContiguous(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CChannelsAccessor::GetType(void)
{
	uint32				i;
	CChannelAccessor*	pcAccessor;
	EPrimitiveType		eType;
	uint32				uiNumAccessors;

	uiNumAccessors = macAccessors.NumElements();
	eType = PT_Undefined;
	for (i = 0; i < uiNumAccessors; i++)
	{
		pcAccessor = macAccessors.Get(i);
		if (eType == PT_Undefined)
		{
			eType = pcAccessor->meAccessType;
		}
		else if (eType != pcAccessor->meAccessType)
		{
			return PT_Undefined;
		}
	}
	return eType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CChannelsAccessor::GetBitSize(void)
{
	return miBitSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CChannelsAccessor::GetByteSize(void)
{
	return miByteSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CChannelsAccessor::GetBufferSize(void)
{
	if (miBitSize % 8 == 0)
	{
		return miByteSize;
	}
	else
	{
		return miBitSize / 8 + 1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CArrayChannelAccessor* CChannelsAccessor::GetAccessors(void)
{
	return &macAccessors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CChannels* CChannelsAccessor::GetChannels(void)
{
	return mpcChannels;
}

