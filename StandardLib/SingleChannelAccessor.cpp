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
#include "BaseLib/TypeNames.h"
#include "SingleChannelAccessor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSingleChannelAccessor::Init(CChannels* pcChannels)
{
	mpcChannels = pcChannels;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CSingleChannelAccessor::GetConvertToFloat(int iChannel, int iPos)
{
	float				f;

	GetConvertTo(iChannel, PT_float32, &f, iPos);
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float CSingleChannelAccessor::GetCastToFloat(int iChannel, int iPos)
{
	float				f;

	GetCastTo(iChannel, PT_float32, &f, iPos);
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSingleChannelAccessor::GetConvertTo(int iChannel, EPrimitiveType eType, void* pvDest, int iPos)
{
	CChannel*			pcChannel;
	void*				pvData;
	int					iIndex;
	int					iByteSize;
	int					iBitSize;
	bool				bSub;

	//Fix the mess that are this function and the one below it.
	pcChannel = mpcChannels->GetChannel(iChannel);
	if (pcChannel)
	{
		pcChannel = mpcChannels->GetChannel(iChannel);
		iIndex = mpcChannels->GetIndexOfChannel(pcChannel);

		if (eType == PT_Undefined)
		{
			eType = pcChannel->eType;
		}

		iByteSize = gcTypeNames.GetByteSize(eType);
		iBitSize = gcTypeNames.GetBitSize(eType);
		if (iByteSize == 0)
		{
			iByteSize = -1;
			bSub = true;
		}
		else
		{
			bSub = false;
		}

		CChannelAccessor::Init(iByteSize, iBitSize, eType, pcChannel->miByteOffset, pcChannel->miByteSize, pcChannel->eType, pcChannel->bReverse, pcChannel->miBitSize, pcChannel->miBitOffset, pcChannel->iChannel);

		bSub |= miChannelByteOffset == -1;
		bSub |= miChannelByteSize == -1;

		if (!bSub)
		{
			pvData = RemapSinglePointer(mpcChannels->GetData(), iPos * mpcChannels->GetByteStride());
			CChannelAccessor::GetAs(eType, mpcChannels->GetData(), pvDest);
		}
		else
		{
			CChannelAccessor::GetAsSub(eType, mpcChannels->GetData(), iPos * mpcChannels->GetBitStride(), pvDest);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSingleChannelAccessor::GetCastTo(int iChannel, EPrimitiveType eType, void* pvDest, int iPos)
{
	CChannel*			pcChannel;
	void*				pvData;
	int					iIndex;
	int					iByteSize;
	int					iBitSize;
	bool				bSub;

	pcChannel = mpcChannels->GetChannel(iChannel);
	if (pcChannel)
	{
		pcChannel = mpcChannels->GetChannel(iChannel);
		iIndex = mpcChannels->GetIndexOfChannel(pcChannel);

		if (eType == PT_Undefined)
		{
			eType = pcChannel->eType;
		}

		iByteSize = gcTypeNames.GetByteSize(eType);
		iBitSize = gcTypeNames.GetBitSize(eType);
		if (iByteSize ==0)
		{
			iByteSize = -1;
			bSub = true;
		}
		else
		{
			bSub = false;
		}

		CChannelAccessor::Init(iByteSize, iBitSize, eType, pcChannel->miByteOffset, pcChannel->miByteSize, pcChannel->eType, pcChannel->bReverse, pcChannel->miBitSize, pcChannel->miBitOffset, pcChannel->iChannel);

		bSub |= miChannelByteOffset == -1;
		bSub |= miChannelByteSize == -1;

		if (!bSub)
		{
			pvData = RemapSinglePointer(mpcChannels->GetData(), iPos * mpcChannels->GetByteStride());
			CChannelAccessor::GetCast(eType, mpcChannels->GetData(), pvDest);
		}
		else
		{
			CChannelAccessor::GetCastSub(eType, mpcChannels->GetData(), iPos * mpcChannels->GetBitStride(), pvDest);
		}
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSingleChannelAccessor::GetNative(int iChannel, void* pvDest, int iPos)
{
	GetConvertTo(iChannel, PT_Undefined, pvDest, iPos);
}

