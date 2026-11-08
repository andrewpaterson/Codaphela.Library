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
#include "BaseLib/TypeConverter.h"
#include "ChannelAccessor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::Init(size iAccessByteSize, size iAccessBitSize, EPrimitiveType eAccessType, size iChannelByteOffset, size iChannelByteSize, EPrimitiveType eChannelType, bool bChannelReverse, size iChannelBitSize, size iChannelBitOffset, size iChannel)
{
	miAccessByteSize = iAccessByteSize;
	miAccessBitSize = iAccessBitSize;
	meAccessType = eAccessType;
	miChannelByteOffset = iChannelByteOffset;
	miChannelByteSize = iChannelByteSize;
	meChannelType = eChannelType;
	mbChannelReverse = bChannelReverse;
	miChannelBitSize = iChannelBitSize;
	miChannelBitOffset = iChannelBitOffset;
	miChannel = iChannel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetNative(void* pvBase, void* pvDest)
{
	pvBase = RemapSinglePointer(pvBase, miChannelByteOffset);
	memcpy(pvDest, pvBase, miChannelByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetAs(EPrimitiveType eType, void* pvBase, void* pvDest)
{
	uint8	auc[CHANNEL_SCRATCH_BYTES];

	//A reversed channel's bytes are put in order before they are converted, as SetFrom reverses them after.
	pvBase = RemapSinglePointer(pvBase, miChannelByteOffset);
	if (mbChannelReverse && (miChannelByteSize <= sizeof(auc)))
	{
		memcpy(auc, pvBase, miChannelByteSize);
		ReverseEndianness(auc, miChannelByteSize);
		pvBase = auc;
	}
	gcTypeConverter.Do(eType, pvDest, meChannelType, pvBase);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetAsSub(EPrimitiveType eType, void* pvBase, lsize iBitOffset, void* pvDest)
{
	char	c[CHANNEL_SCRATCH_BYTES];

	GetBits(pvBase, iBitOffset, miChannelBitSize, &c);
	gcTypeConverter.Do(eType, pvDest, meChannelType, &c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetBits(void* pvBase, lsize iBitOffset, size iNumBits, void* pvDest)
{
	if (!mbChannelReverse)
	{
		CopyBits(pvDest, 0, pvBase, iBitOffset + miChannelBitOffset, iNumBits, true);
	}
	else
	{
		CopyBitsReverseHiLo(pvDest, 0, pvBase, iBitOffset + miChannelBitOffset, iNumBits, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::SetNative(void* pvBase, void* pvSource)
{
	pvBase = RemapSinglePointer(pvBase, miChannelByteOffset);
	memcpy(pvBase, pvSource, miChannelByteSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::SetFrom(EPrimitiveType eType, void* pvBase, void* pvSource)
{
	pvBase = RemapSinglePointer(pvBase, miChannelByteOffset);
	gcTypeConverter.Do(meChannelType, pvBase, eType, pvSource);
	if (mbChannelReverse)
	{
		ReverseEndianness(pvBase, miChannelByteSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::SetFromSub(EPrimitiveType eType, void* pvBase, lsize iBitOffset, void* pvSource)
{
	uint8	c[CHANNEL_SCRATCH_BYTES];

	gcTypeConverter.Do(meChannelType, &c, eType, pvSource);
	SetBits(pvBase, iBitOffset, miChannelBitSize, &c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::SetBits(void* pvBase, lsize iBitOffset, size iNumBits, void* pvSource)
{
	if (!mbChannelReverse)
	{
		CopyBits(pvBase, iBitOffset + miChannelBitOffset, pvSource, 0, iNumBits, false);
	}
	else
	{
		CopyBitsReverseHiLo(pvBase, iBitOffset+ miChannelBitOffset, pvSource, 0, iNumBits, false);
	}
}

