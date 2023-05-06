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
#include "BaseLib/IntegerHelper.h"
#include "BaseLib/TypeConverter.h"
#include "BaseLib/Operators.h"
#include "ChannelAccessor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::Init(int iAccessByteSize, int iAccessBitSize, EPrimitiveType eAccessType, int iChannelByteOffset, int iChannelByteSize, EPrimitiveType eChannelType, bool bChannelReverse, int iChannelBitSize, int iChannelBitOffset, int iChannel)
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
	ga_memcpy_fast[miChannelByteSize](pvDest, pvBase);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetAs(EPrimitiveType eType, void* pvBase, void* pvDest)
{
	pvBase = RemapSinglePointer(pvBase, miChannelByteOffset);
	gcTypeConverter.Do(eType, pvDest, meChannelType, pvBase);
	if (mbChannelReverse)
	{
		ReverseEndianness(pvDest, miChannelByteSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetCast(EPrimitiveType eType, void* pvBase, void* pvDest)
{
	pvBase = RemapSinglePointer(pvBase, miChannelByteOffset);
	gcOperators.mcAssignment.Do(eType, pvDest, meChannelType, pvBase);
	if (mbChannelReverse)
	{
		ReverseEndianness(pvDest, miChannelByteSize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetAsSub(EPrimitiveType eType, void* pvBase, int iBitOffset, void* pvDest)
{
	char	c[16];

	GetBits(pvBase, iBitOffset, miChannelBitSize, &c);
	gcTypeConverter.Do(eType, pvDest, meChannelType, &c);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetCastSub(EPrimitiveType eType, void* pvBase, int iBitOffset, void* pvDest)
{
	char	c[16];

	GetBits(pvBase, iBitOffset, miChannelBitSize, &c);
	gcOperators.mcAssignment.Do(eType, pvDest, meChannelType, pvBase);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::GetBits(void* pvBase, int iBitOffset, int iNumBits, void* pvDest)
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
	ga_memcpy_fast[miChannelByteSize](pvBase, pvSource);
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
void CChannelAccessor::SetFromSub(EPrimitiveType eType, void* pvBase, int iBitOffset, void* pvSource)
{
	char	c[16];

	gcTypeConverter.Do(meChannelType, &c, eType, pvSource);
	SetBits(pvBase, iBitOffset, miChannelBitSize, &c);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannelAccessor::SetBits(void* pvBase, int iBitOffset, int iNumBits, void* pvSource)
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

