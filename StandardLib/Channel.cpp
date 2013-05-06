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
#include "CoreLib/TypeConverter.h"
#include "Channel.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannel::Init(int iChannel, EPrimitiveTypes eType, BOOL bReverse)
{
	this->iChannel = iChannel;
	this->eType = eType;
	this->bReverse = bReverse;  //If type is bit'ty then reverse all bits otherwise reverse endianness.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CChannel::Init(int iChannel, EPrimitiveTypes eType, int iByteSize, int iBitSize, int iByteOffset, int iBitOffset, BOOL bReverse)
{
	this->iChannel = iChannel;
	this->eType = eType;
	this->bReverse = bReverse;  //If type is bit'ty then reverse all bits otherwise reverse endianness.
	miByteSize = iByteSize;  //-1 if bits not a multiple of 8.
	miBitSize = iBitSize;
	miByteOffset = iByteOffset;  //-1 if not byte aligned.
	miBitOffset = iBitOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CChannel::Is8BitAligned(void)
{
	return ((miByteSize != -1) && (miByteOffset != -1));
}

