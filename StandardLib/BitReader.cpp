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
#include "BitReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitReader::Init(CFileReader* pcFile)
{
	mpcFile = pcFile;
	muiBits = 0;
	miNumBits = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitReader::Kill(void)
{
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads up to 24 bits.  The first bit read is the least significant bit
// of the value.  Returns false if the file ends first.
//
//////////////////////////////////////////////////////////////////////////
bool CBitReader::ReadBits(int iCount, uint32* puiValue)
{
	uint8	uiByte;

	while (miNumBits < iCount)
	{
		if (!mpcFile->ReadInt8(&uiByte))
		{
			return false;
		}
		muiBits |= (uint32)uiByte << miNumBits;
		miNumBits += 8;
	}

	*puiValue = muiBits & ((1u << iCount) - 1);
	muiBits >>= iCount;
	miNumBits -= iCount;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBitReader::ReadBit(uint32* puiValue)
{
	uint8	uiByte;

	if (miNumBits == 0)
	{
		if (!mpcFile->ReadInt8(&uiByte))
		{
			return false;
		}
		muiBits = uiByte;
		miNumBits = 8;
	}

	*puiValue = muiBits & 1;
	muiBits >>= 1;
	miNumBits--;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Discards the rest of the current byte.  Fewer than 8 bits are ever
// buffered so the file is then at the next byte.
//
//////////////////////////////////////////////////////////////////////////
void CBitReader::AlignToByte(void)
{
	muiBits = 0;
	miNumBits = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the next byte above the bits already buffered, for when a caller
// needs to look at more bits than NumBits.  Returns false if the file
// ends.
//
//////////////////////////////////////////////////////////////////////////
bool CBitReader::FillByte(void)
{
	uint8	uiByte;

	if (!mpcFile->ReadInt8(&uiByte))
	{
		return false;
	}
	muiBits |= (uint32)uiByte << miNumBits;
	miNumBits += 8;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The buffered bits, the next to be read least significant.  Bits above
// NumBits are zero.
//
//////////////////////////////////////////////////////////////////////////
uint32 CBitReader::GetBits(void)
{
	return muiBits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBitReader::NumBits(void)
{
	return miNumBits;
}


//////////////////////////////////////////////////////////////////////////
//
// Discards iCount buffered bits.  There must be at least that many.
//
//////////////////////////////////////////////////////////////////////////
void CBitReader::SkipBits(int iCount)
{
	muiBits >>= iCount;
	miNumBits -= iCount;
}
