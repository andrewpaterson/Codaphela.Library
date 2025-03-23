/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "Channel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannel::Init(size iChannel, EPrimitiveType eType, bool bReverse)
{
	this->iChannel = iChannel;
	this->eType = eType;
	this->bReverse = bReverse;  //If type is bit'ty then reverse all bits otherwise reverse endianness.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannel::Init(size iChannel, EPrimitiveType eType, size iByteSize, size iBitSize, size iByteOffset, size iBitOffset, bool bReverse)
{
	this->iChannel = iChannel;
	this->eType = eType;
	this->bReverse = bReverse;  //If type is bit'ty then reverse all bits otherwise reverse endianness.
	miByteSize = iByteSize;  //CHANNEL_NON_ALIGNED_BYTES if bits not a multiple of 8.
	miBitSize = iBitSize;
	miByteOffset = iByteOffset;  //CHANNEL_NON_ALIGNED_BYTES if not byte aligned.
	miBitOffset = iBitOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CChannel::Is8BitAligned(void)
{
	return ((miByteSize != CHANNEL_NON_ALIGNED_BYTES) && (miByteOffset != CHANNEL_NON_ALIGNED_BYTES));
}

