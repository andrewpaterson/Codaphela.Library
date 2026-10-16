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
#include "BitWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitWriter::Init(CFileWriter* pcFile)
{
	mpcFile = pcFile;
	muiBits = 0;
	miNumBits = 0;
	muiBufferUsed = 0;
	mbFailed = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitWriter::Kill(void)
{
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBitWriter::WriteByte(uint8 uiByte)
{
	if (muiBufferUsed == BIT_WRITER_BUFFER_SIZE)
	{
		if (!mpcFile->WriteData(mauiBuffer, muiBufferUsed))
		{
			mbFailed = true;
		}
		muiBufferUsed = 0;
	}
	mauiBuffer[muiBufferUsed] = uiByte;
	muiBufferUsed++;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the low iCount bits (up to 24) of uiValue, least significant
// first.
//
//////////////////////////////////////////////////////////////////////////
void CBitWriter::WriteBits(uint32 uiValue, int iCount)
{
	muiBits |= (uiValue & ((1u << iCount) - 1)) << miNumBits;
	miNumBits += iCount;
	while (miNumBits >= 8)
	{
		WriteByte((uint8)muiBits);
		muiBits >>= 8;
		miNumBits -= 8;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Pads the current byte with zero bits.
//
//////////////////////////////////////////////////////////////////////////
void CBitWriter::AlignToByte(void)
{
	if (miNumBits > 0)
	{
		WriteByte((uint8)muiBits);
	}
	muiBits = 0;
	miNumBits = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes whole bytes.  The writer must be byte aligned.
//
//////////////////////////////////////////////////////////////////////////
void CBitWriter::WriteBytes(uint8* puiData, size uiLength)
{
	size	i;

	for (i = 0; i < uiLength; i++)
	{
		WriteByte(puiData[i]);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Writes any buffered whole bytes to the file.  A partial byte stays
// buffered until AlignToByte.  Returns false if any write failed.
//
//////////////////////////////////////////////////////////////////////////
bool CBitWriter::Flush(void)
{
	if (muiBufferUsed > 0)
	{
		if (!mpcFile->WriteData(mauiBuffer, muiBufferUsed))
		{
			mbFailed = true;
		}
		muiBufferUsed = 0;
	}
	return !mbFailed;
}
