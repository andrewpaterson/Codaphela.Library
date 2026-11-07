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
#include "BaseLib/PointerRemapper.h"
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
float CSingleChannelAccessor::GetConvertToFloat(size iChannel, lsize iPos)
{
	float	f;

	f = 0.0f;
	GetConvertTo(iChannel, PT_float32, &f, iPos);
	return f;
}


//////////////////////////////////////////////////////////////////////////
//
// The channel's value cast (not scaled) to a float.
//
//////////////////////////////////////////////////////////////////////////
float CSingleChannelAccessor::GetCastToFloat(size iChannel, lsize iPos)
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
	if (IsByteAligned())
	{
		CChannelAccessor::GetNative(GetElement(iPos), auc);
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
// The channel's value converted to eType (PT_Undefined is the channel's
// own type).
//
//////////////////////////////////////////////////////////////////////////
void CSingleChannelAccessor::GetConvertTo(size iChannel, EPrimitiveType eType, void* pvDest, lsize iPos)
{
	CChannel*	pcChannel;
	size		iByteSize;
	size		iBitSize;

	pcChannel = mpcChannels->GetChannel(iChannel);
	if (!pcChannel)
	{
		return;
	}

	if (eType == PT_Undefined)
	{
		eType = pcChannel->eType;
	}
	iByteSize = gcTypeNames.GetByteSize(eType);
	iBitSize = gcTypeNames.GetBitSize(eType);

	CChannelAccessor::Init((iByteSize != 0) ? iByteSize : MAX_SIZE, iBitSize, eType, pcChannel->miByteOffset, pcChannel->miByteSize, pcChannel->eType, pcChannel->bReverse, pcChannel->miBitSize, pcChannel->miBitOffset, pcChannel->iChannel);
	if ((iByteSize != 0) && IsByteAligned())
	{
		CChannelAccessor::GetAs(eType, GetElement(iPos), pvDest);
	}
	else
	{
		CChannelAccessor::GetAsSub(eType, mpcChannels->GetData(), iPos * mpcChannels->GetBitStride(), pvDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSingleChannelAccessor::GetNative(size iChannel, void* pvDest, lsize iPos)
{
	GetConvertTo(iChannel, PT_Undefined, pvDest, iPos);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSingleChannelAccessor::IsByteAligned(void)
{
	return (miChannelByteOffset != CHANNEL_NON_ALIGNED_BYTES) && (miChannelByteSize != CHANNEL_NON_ALIGNED_BYTES);
}


//////////////////////////////////////////////////////////////////////////
//
// The start of element iPos: the channel's byte offset is added by the
// accessor.
//
//////////////////////////////////////////////////////////////////////////
void* CSingleChannelAccessor::GetElement(lsize iPos)
{
	return RemapSinglePointer(mpcChannels->GetData(), (ptrdiff)(iPos * mpcChannels->GetByteStride()));
}
