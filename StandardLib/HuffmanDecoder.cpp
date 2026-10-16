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
	return true;
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
// Codes are stored most significant bit first, so are read a bit at a
// time.  Returns false if the input ends or the bits are not a code.
//
//////////////////////////////////////////////////////////////////////////
bool CHuffmanDecoder::Decode(CBitReader* pcReader, int* piSymbol)
{
	int		iCode;
	int		iFirst;
	int		iIndex;
	int		iLength;
	int		iCount;
	uint32	uiBit;

	iCode = 0;
	iFirst = 0;
	iIndex = 0;
	for (iLength = 1; iLength <= DEFLATE_MAX_BITS; iLength++)
	{
		if (!pcReader->ReadBit(&uiBit))
		{
			return false;
		}
		iCode |= (int)uiBit;
		iCount = maiCount[iLength];
		if (iCode - iCount < iFirst)
		{
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
