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
#include "BaseLib/IntegerHelper.h"
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
	CChannel*	pcChannel;
	uint8		auc[16];

	pcChannel = mpcChannels->GetChannel(iChannel);
	if (!pcChannel)
	{
		return 0.0f;
	}

	CChannelAccessor::Init(pcChannel->miByteSize, pcChannel->miBitSize, pcChannel->eType, pcChannel->miByteOffset, pcChannel->miByteSize, pcChannel->eType, pcChannel->bReverse, pcChannel->miBitSize, pcChannel->miBitOffset, pcChannel->iChannel);

	memset(auc, 0, sizeof(auc));
	if ((miChannelByteOffset != MAX_SIZE) && (miChannelByteSize != MAX_SIZE))
	{
		CChannelAccessor::GetNative(mpcChannels->GetData(), auc);
		if (mbChannelReverse)
		{
			ReverseEndianness(auc, miChannelByteSize);
		}
	}
	else
	{
		CChannelAccessor::GetBits(mpcChannels->GetData(), iPos * mpcChannels->GetBitStride(), miChannelBitSize, auc);
	}

	switch (meChannelType)
	{
	case PT_int8:		return (float)*((int8*)auc);
	case PT_int16:		return (float)*((int16*)auc);
	case PT_int32:		return (float)*((int32*)auc);
	case PT_int64:		return (float)*((int64*)auc);
	case PT_uint8:		return (float)*((uint8*)auc);
	case PT_uint16:		return (float)*((uint16*)auc);
	case PT_uint32:		return (float)*((uint32*)auc);
	case PT_uint64:		return (float)*((uint64*)auc);
	case PT_char8:		return (float)*((char*)auc);
	case PT_float32:	return *((float32*)auc);
	case PT_float64:	return (float)*((float64*)auc);
	case PT_bit:
	case PT_crumb:
	case PT_tribble:
	case PT_nybble:
	case PT_nickle:
	case PT_sixbits:	return (float)auc[0];
	default:			return 0.0f;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSingleChannelAccessor::GetConvertTo(int iChannel, EPrimitiveType eType, void* pvDest, int iPos)
{
	CChannel*	pcChannel;
	size		iByteSize;
	size		iBitSize;
	bool		bSub;

	//Fix the mess that are this function and the one below it.
	pcChannel = mpcChannels->GetChannel(iChannel);
	if (pcChannel)
	{
		pcChannel = mpcChannels->GetChannel(iChannel);

		if (eType == PT_Undefined)
		{
			eType = pcChannel->eType;
		}

		iByteSize = gcTypeNames.GetByteSize(eType);
		iBitSize = gcTypeNames.GetBitSize(eType);
		if (iByteSize == 0)
		{
			iByteSize = MAX_SIZE;
			bSub = true;
		}
		else
		{
			bSub = false;
		}

		CChannelAccessor::Init(iByteSize, iBitSize, eType, pcChannel->miByteOffset, pcChannel->miByteSize, pcChannel->eType, pcChannel->bReverse, pcChannel->miBitSize, pcChannel->miBitOffset, pcChannel->iChannel);

		bSub |= miChannelByteOffset == MAX_SIZE;
		bSub |= miChannelByteSize == MAX_SIZE;

		if (!bSub)
		{
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
void CSingleChannelAccessor::GetNative(int iChannel, void* pvDest, int iPos)
{
	GetConvertTo(iChannel, PT_Undefined, pvDest, iPos);
}

