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
#include "Unknowns.h"
#include "BaseLib/PointerFunctions.h"
#include "ChannelsAccessor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessor::Init(CChannels* pcChannels, CArrayChannelAccessor* pcAccessors, int iByteSize, int iBitSize, int iBufferSize)
{
	mpcChannels = pcChannels;
	memcpy(&macAccessors, pcAccessors, sizeof(CArrayChannelAccessor));
	memset(pcAccessors, 0, sizeof(CArrayChannelAccessor));
	miByteSize = iByteSize;
	miBitSize = iBitSize;
	mpvGetBuffer = (char*)malloc(iBufferSize);
	miBufferSize = iBufferSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelsAccessor::Kill(void)
{
	mpcChannels = NULL;
	macAccessors.Kill();
	SafeFree(mpvGetBuffer);
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
	int					i;
	CChannelAccessor*	pcAccessor;
	EPrimitiveType		eType;

	eType = PT_Undefined;
	for (i = 0; i < macAccessors.NumElements(); i++)
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
int CChannelsAccessor::GetBitSize(void)
{
	return miBitSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannelsAccessor::GetByteSize(void)
{
	return miByteSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CChannelsAccessor::GetBufferSize(void)
{
	return miBufferSize;
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

