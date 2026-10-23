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
#include <string.h>
#include "HuffmanDecoder.h"


//////////////////////////////////////////////////////////////////////////
//
// Returns false if the lengths are over subscribed (more codes than the
// lengths allow).  An incomplete set of lengths is accepted, see
// IsComplete and IsSingleCode.
//
//////////////////////////////////////////////////////////////////////////
bool CHuffmanDecoder::Init(uint8* puiLengths, int iNumSymbols)
{
	int16	aiOffsets[DEFLATE_MAX_BITS + 1];
	int		iLength;
	int		iSymbol;
	int		iLeft;

	for (iLength = 0; iLength <= DEFLATE_MAX_BITS; iLength++)
	{
		maiCount[iLength] = 0;
	}
	for (iSymbol = 0; iSymbol < iNumSymbols; iSymbol++)
	{
		maiCount[puiLengths[iSymbol]]++;
	}
	miNumCodes = iNumSymbols - maiCount[0];

	//Each length doubles the number of possible codes.  Running out means the lengths are over subscribed.
	iLeft = 1;
	for (iLength = 1; iLength <= DEFLATE_MAX_BITS; iLength++)
	{
		iLeft <<= 1;
		iLeft -= maiCount[iLength];
		if (iLeft < 0)
		{
			miUnused = 0;
			return false;
		}
	}
	miUnused = iLeft;

	aiOffsets[1] = 0;
	for (iLength = 1; iLength < DEFLATE_MAX_BITS; iLength++)
	{
		aiOffsets[iLength + 1] = aiOffsets[iLength] + maiCount[iLength];
	}
	for (iSymbol = 0; iSymbol < iNumSymbols; iSymbol++)
	{
		if (puiLengths[iSymbol] != 0)
		{
			maiSymbols[aiOffsets[puiLengths[iSymbol]]] = (int16)iSymbol;
			aiOffsets[puiLengths[iSymbol]]++;
		}
	}

	InitFast(puiLengths, iNumSymbols);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Fills the lookup table for codes no longer than HUFFMAN_FAST_BITS.  A
// code's first bit is the first read, which the reader holds in the
// least significant bit, so a code is entered at its bits reversed and
// at every value of the bits that follow it.
//
//////////////////////////////////////////////////////////////////////////
void CHuffmanDecoder::InitFast(uint8* puiLengths, int iNumSymbols)
{
	uint32	auiNextCode[DEFLATE_MAX_BITS + 1];
	uint32	uiCode;
	uint32	uiReversed;
	uint32	uiIndex;
	uint16	uiEntry;
	int		iLength;
	int		iSymbol;
	int		i;

	memset(mauiFast, 0, sizeof(mauiFast));

	//maiCount[0] counts unused symbols, not codes, so the first length starts at zero.
	uiCode = 0;
	auiNextCode[1] = 0;
	for (iLength = 2; iLength <= DEFLATE_MAX_BITS; iLength++)
	{
		uiCode = (uiCode + maiCount[iLength - 1]) << 1;
		auiNextCode[iLength] = uiCode;
	}

	for (iSymbol = 0; iSymbol < iNumSymbols; iSymbol++)
	{
		iLength = puiLengths[iSymbol];
		if (iLength != 0)
		{
			uiCode = auiNextCode[iLength];
			auiNextCode[iLength]++;
			if (iLength <= HUFFMAN_FAST_BITS)
			{
				uiReversed = 0;
				for (i = 0; i < iLength; i++)
				{
					uiReversed = (uiReversed << 1) | ((uiCode >> i) & 1);
				}
				uiEntry = (uint16)((iSymbol << HUFFMAN_FAST_LENGTH_BITS) | iLength);
				for (uiIndex = uiReversed; uiIndex < HUFFMAN_FAST_SIZE; uiIndex += 1 << iLength)
				{
					mauiFast[uiIndex] = uiEntry;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CHuffmanDecoder::Kill(void)
{
	miNumCodes = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CHuffmanDecoder::IsComplete(void)
{
	return miUnused == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Deflate allows an incomplete distance or literal code only when it is
// a single code of one bit.
//
//////////////////////////////////////////////////////////////////////////
bool CHuffmanDecoder::IsSingleCode(void)
{
	return (miNumCodes == 1) && (maiCount[1] == 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Looks the next bits up in the table.  Bits not yet read look like
// zeros, so a code found that is no longer than the bits read is the
// right one; otherwise the code is longer than the bits read and another
// byte is needed.  No byte is read that the code doesn't use.  Returns
// false if the input ends or the bits are not a code.
//
//////////////////////////////////////////////////////////////////////////
bool CHuffmanDecoder::Decode(CBitReader* pcReader, int* piSymbol)
{
	uint16	uiEntry;
	int		iLength;

	for (;;)
	{
		uiEntry = mauiFast[pcReader->GetBits() & (HUFFMAN_FAST_SIZE - 1)];
		iLength = uiEntry & HUFFMAN_FAST_LENGTH_MASK;
		if (iLength == 0)
		{
			return DecodeSlow(pcReader, piSymbol);
		}
		if (iLength <= pcReader->NumBits())
		{
			pcReader->SkipBits(iLength);
			*piSymbol = uiEntry >> HUFFMAN_FAST_LENGTH_BITS;
			return true;
		}
		if (!pcReader->FillByte())
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Codes longer than the table, and bits that aren't a code, are decoded
// a bit at a time.  Codes are stored most significant bit first.
//
//////////////////////////////////////////////////////////////////////////
bool CHuffmanDecoder::DecodeSlow(CBitReader* pcReader, int* piSymbol)
{
	int		iCode;
	int		iFirst;
	int		iIndex;
	int		iLength;
	int		iCount;

	iCode = 0;
	iFirst = 0;
	iIndex = 0;
	for (iLength = 1; iLength <= DEFLATE_MAX_BITS; iLength++)
	{
		if ((iLength > pcReader->NumBits()) && !pcReader->FillByte())
		{
			return false;
		}
		iCode |= (int)((pcReader->GetBits() >> (iLength - 1)) & 1);
		iCount = maiCount[iLength];
		if (iCode - iCount < iFirst)
		{
			pcReader->SkipBits(iLength);
			*piSymbol = maiSymbols[iIndex + (iCode - iFirst)];
			return true;
		}
		iIndex += iCount;
		iFirst += iCount;
		iFirst <<= 1;
		iCode <<= 1;
	}
	return false;
}
