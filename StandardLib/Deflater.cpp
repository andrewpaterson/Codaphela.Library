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
#include <stdlib.h>
#include <string.h>
#include "Adler32.h"
#include "Deflater.h"


#define DEFLATER_WINDOW_MASK		(DEFLATE_WINDOW_SIZE - 1)
#define DEFLATER_MAX_CHAIN			128		//Most chain links followed looking for a match.
#define DEFLATER_GOOD_LENGTH		8		//Follow fewer links when the previous match is already this long.
#define DEFLATER_NICE_LENGTH		128		//Stop looking when a match is this long.
#define DEFLATER_LAZY_LENGTH		16		//Don't look for a longer match after one this long.
#define DEFLATER_TOO_FAR			4096	//A three byte match further back than this costs more than three literals.

#define ZLIB_DEFAULT_HEADER_METHOD	0x78	//Deflate with a 32K window.
#define ZLIB_DEFAULT_HEADER_FLAGS	0x9C	//Default compression level, no dictionary.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int DeflateLengthCode(int iLength)
{
	int		iCode;

	for (iCode = DEFLATE_NUM_LENGTH_CODES - 1; iCode > 0; iCode--)
	{
		if (iLength >= gauiDeflateLengthBase[iCode])
		{
			return iCode;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int DeflateDistanceCode(size uiDistance)
{
	int		iCode;

	for (iCode = DEFLATE_NUM_DISTANCES - 1; iCode > 0; iCode--)
	{
		if (uiDistance >= gauiDeflateDistanceBase[iCode])
		{
			return iCode;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if the match finding tables can't be allocated.
//
//////////////////////////////////////////////////////////////////////////
bool CDeflater::Init(CFileWriter* pcOutput, bool bZLib)
{
	mpcOutput = pcOutput;
	mbZLib = bZLib;
	mpuiData = NULL;
	muiDataSize = 0;

	maiHead = (int32*)malloc(DEFLATER_HASH_SIZE * sizeof(int32));
	maiPrevious = (int32*)malloc(DEFLATE_WINDOW_SIZE * sizeof(int32));
	mauiSymbolLengths = (uint16*)malloc(DEFLATER_BLOCK_SYMBOLS * sizeof(uint16));
	mauiSymbolValues = (uint16*)malloc(DEFLATER_BLOCK_SYMBOLS * sizeof(uint16));
	if ((maiHead == NULL) || (maiPrevious == NULL) || (mauiSymbolLengths == NULL) || (mauiSymbolValues == NULL))
	{
		Kill();
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDeflater::Kill(void)
{
	free(maiHead);
	free(maiPrevious);
	free(mauiSymbolLengths);
	free(mauiSymbolValues);
	maiHead = NULL;
	maiPrevious = NULL;
	mauiSymbolLengths = NULL;
	mauiSymbolValues = NULL;
	mpcOutput = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Compresses all of puiData as one stream.  Returns false if writing
// fails.
//
//////////////////////////////////////////////////////////////////////////
bool CDeflater::Deflate(uint8* puiData, size uiSize)
{
	size		uiPosition;
	size		uiEnd;
	size		p;
	int32		iChain;
	int			iLength;
	size		uiDistance;
	bool		bAvailable;
	int			iPreviousLength;
	size		uiPreviousDistance;
	CAdler32	cAdler;
	uint32		uiAdler;

	mpuiData = puiData;
	muiDataSize = uiSize;
	memset(maiHead, 0xFF, DEFLATER_HASH_SIZE * sizeof(int32));
	mcBits.Init(mpcOutput);

	if (mbZLib)
	{
		mcBits.WriteBits(ZLIB_DEFAULT_HEADER_METHOD, 8);
		mcBits.WriteBits(ZLIB_DEFAULT_HEADER_FLAGS, 8);
	}

	muiBlockStart = 0;
	ClearBlock();

	//Lazy matching: a match is only used if the match starting at the next byte isn't longer.
	uiPosition = 0;
	bAvailable = false;
	iPreviousLength = 0;
	uiPreviousDistance = 0;
	while (uiPosition < uiSize)
	{
		iChain = InsertHash(uiPosition);
		iLength = 0;
		uiDistance = 0;
		if ((iChain >= 0) && (iPreviousLength < DEFLATER_LAZY_LENGTH))
		{
			iLength = FindMatch(uiPosition, iChain, bAvailable ? iPreviousLength : 0, &uiDistance);
		}

		if (bAvailable && (iPreviousLength >= DEFLATE_MIN_MATCH) && (iLength <= iPreviousLength))
		{
			//The match starts at the previous byte.
			AddMatch(iPreviousLength, uiPreviousDistance);
			uiEnd = uiPosition - 1 + iPreviousLength;
			for (p = uiPosition + 1; p < uiEnd; p++)
			{
				InsertHash(p);
			}
			uiPosition = uiEnd;
			bAvailable = false;
			iPreviousLength = 0;
		}
		else
		{
			if (bAvailable)
			{
				AddLiteral(puiData[uiPosition - 1]);
			}
			bAvailable = true;
			iPreviousLength = iLength;
			uiPreviousDistance = uiDistance;
			uiPosition++;
		}
	}
	if (bAvailable)
	{
		AddLiteral(puiData[uiPosition - 1]);
	}
	WriteBlock(true);
	mcBits.AlignToByte();

	if (mbZLib)
	{
		cAdler.Init();
		cAdler.Update(puiData, uiSize);
		uiAdler = cAdler.Get();
		mcBits.WriteBits(uiAdler >> 24, 8);
		mcBits.WriteBits(uiAdler >> 16, 8);
		mcBits.WriteBits(uiAdler >> 8, 8);
		mcBits.WriteBits(uiAdler, 8);
	}

	mpuiData = NULL;
	return mcBits.Flush();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds uiPosition to the chain for the three bytes starting there and
// returns the previous position in that chain, or -1.
//
//////////////////////////////////////////////////////////////////////////
int32 CDeflater::InsertHash(size uiPosition)
{
	uint32	uiHash;
	int32	iPrevious;

	if (uiPosition + 2 >= muiDataSize)
	{
		return -1;
	}

	uiHash = (((uint32)mpuiData[uiPosition] << 10) ^ ((uint32)mpuiData[uiPosition + 1] << 5) ^ mpuiData[uiPosition + 2]) & (DEFLATER_HASH_SIZE - 1);
	iPrevious = maiHead[uiHash];
	maiPrevious[uiPosition & DEFLATER_WINDOW_MASK] = iPrevious;
	maiHead[uiHash] = (int32)uiPosition;
	return iPrevious;
}


//////////////////////////////////////////////////////////////////////////
//
// Follows the hash chain from iChain for a match longer than
// iPreviousLength.  Returns its length, or 0 if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
int CDeflater::FindMatch(size uiPosition, int32 iChain, int iPreviousLength, size* puiDistance)
{
	int		iMaxLength;
	int		iBest;
	int		iFound;
	int		iChainLeft;
	int		iLength;
	uint8*	puiCandidate;
	uint8*	puiCurrent;
	int32	iNext;

	iMaxLength = DEFLATE_MAX_MATCH;
	if (muiDataSize - uiPosition < (size)iMaxLength)
	{
		iMaxLength = (int)(muiDataSize - uiPosition);
	}
	iBest = (iPreviousLength > DEFLATE_MIN_MATCH - 1) ? iPreviousLength : DEFLATE_MIN_MATCH - 1;
	if (iBest >= iMaxLength)
	{
		return 0;
	}

	iChainLeft = (iPreviousLength >= DEFLATER_GOOD_LENGTH) ? DEFLATER_MAX_CHAIN / 4 : DEFLATER_MAX_CHAIN;
	iFound = 0;
	puiCurrent = &mpuiData[uiPosition];
	while ((iChain >= 0) && (iChainLeft > 0) && (uiPosition - iChain <= DEFLATE_WINDOW_SIZE))
	{
		puiCandidate = &mpuiData[iChain];
		if ((puiCandidate[iBest] == puiCurrent[iBest]) && (puiCandidate[0] == puiCurrent[0]) && (puiCandidate[1] == puiCurrent[1]))
		{
			iLength = 2;
			while ((iLength < iMaxLength) && (puiCandidate[iLength] == puiCurrent[iLength]))
			{
				iLength++;
			}
			if (iLength > iBest)
			{
				iBest = iLength;
				iFound = iLength;
				*puiDistance = uiPosition - iChain;
				if ((iLength >= DEFLATER_NICE_LENGTH) || (iLength >= iMaxLength))
				{
					break;
				}
			}
		}

		//An entry older than the window has been overwritten by a later position, which ends the chain.
		iNext = maiPrevious[iChain & DEFLATER_WINDOW_MASK];
		if (iNext >= iChain)
		{
			break;
		}
		iChain = iNext;
		iChainLeft--;
	}

	if ((iFound == DEFLATE_MIN_MATCH) && (*puiDistance > DEFLATER_TOO_FAR))
	{
		return 0;
	}
	return iFound;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDeflater::ClearBlock(void)
{
	muiNumSymbols = 0;
	muiBlockLength = 0;
	memset(mauiLiteralFrequencies, 0, sizeof(mauiLiteralFrequencies));
	memset(mauiDistanceFrequencies, 0, sizeof(mauiDistanceFrequencies));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDeflater::AddLiteral(uint8 uiLiteral)
{
	mauiSymbolLengths[muiNumSymbols] = 0;
	mauiSymbolValues[muiNumSymbols] = uiLiteral;
	muiNumSymbols++;
	mauiLiteralFrequencies[uiLiteral]++;
	muiBlockLength++;

	if (muiNumSymbols == DEFLATER_BLOCK_SYMBOLS)
	{
		WriteBlock(false);
		muiBlockStart += muiBlockLength;
		ClearBlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDeflater::AddMatch(int iLength, size uiDistance)
{
	mauiSymbolLengths[muiNumSymbols] = (uint16)iLength;
	mauiSymbolValues[muiNumSymbols] = (uint16)uiDistance;
	muiNumSymbols++;
	mauiLiteralFrequencies[DEFLATE_FIRST_LENGTH_CODE + DeflateLengthCode(iLength)]++;
	mauiDistanceFrequencies[DeflateDistanceCode(uiDistance)]++;
	muiBlockLength += iLength;

	if (muiNumSymbols == DEFLATER_BLOCK_SYMBOLS)
	{
		WriteBlock(false);
		muiBlockStart += muiBlockLength;
		ClearBlock();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The number of bits the block's symbols and end of block take with the
// given code lengths.
//
//////////////////////////////////////////////////////////////////////////
size CDeflater::CountSymbolBits(uint8* puiLiteralLengths, uint8* puiDistances)
{
	size	uiBits;
	size	i;
	int		iCode;

	uiBits = puiLiteralLengths[DEFLATE_END_OF_BLOCK];
	for (i = 0; i < muiNumSymbols; i++)
	{
		if (mauiSymbolLengths[i] == 0)
		{
			uiBits += puiLiteralLengths[mauiSymbolValues[i]];
		}
		else
		{
			iCode = DeflateLengthCode(mauiSymbolLengths[i]);
			uiBits += puiLiteralLengths[DEFLATE_FIRST_LENGTH_CODE + iCode] + gauiDeflateLengthExtra[iCode];
			iCode = DeflateDistanceCode(mauiSymbolValues[i]);
			uiBits += puiDistances[iCode] + gauiDeflateDistanceExtra[iCode];
		}
	}
	return uiBits;
}


//////////////////////////////////////////////////////////////////////////
//
// Run length encodes code lengths into code length symbols (0-15 a
// length, 16 repeat the previous length, 17 and 18 runs of zeros) and
// their extra bits.  Returns the number of symbols.
//
//////////////////////////////////////////////////////////////////////////
size CDeflater::BuildCodeLengthSymbols(uint8* puiLengths, int iNumLengths, uint8* puiSymbols, uint8* puiExtras)
{
	size	uiCount;
	int		i;
	int		iRun;
	int		iChunk;
	uint8	uiLength;

	uiCount = 0;
	i = 0;
	while (i < iNumLengths)
	{
		uiLength = puiLengths[i];
		iRun = 1;
		while ((i + iRun < iNumLengths) && (puiLengths[i + iRun] == uiLength))
		{
			iRun++;
		}

		if (uiLength == 0)
		{
			while (iRun >= 11)
			{
				iChunk = (iRun < 138) ? iRun : 138;
				puiSymbols[uiCount] = 18;
				puiExtras[uiCount] = (uint8)(iChunk - 11);
				uiCount++;
				i += iChunk;
				iRun -= iChunk;
			}
			if (iRun >= 3)
			{
				puiSymbols[uiCount] = 17;
				puiExtras[uiCount] = (uint8)(iRun - 3);
				uiCount++;
				i += iRun;
				iRun = 0;
			}
		}
		else
		{
			puiSymbols[uiCount] = uiLength;
			puiExtras[uiCount] = 0;
			uiCount++;
			i++;
			iRun--;
			while (iRun >= 3)
			{
				iChunk = (iRun < 6) ? iRun : 6;
				puiSymbols[uiCount] = 16;
				puiExtras[uiCount] = (uint8)(iChunk - 3);
				uiCount++;
				i += iChunk;
				iRun -= iChunk;
			}
		}

		while (iRun > 0)
		{
			puiSymbols[uiCount] = uiLength;
			puiExtras[uiCount] = 0;
			uiCount++;
			i++;
			iRun--;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the buffered symbols as whichever of a stored, fixed or dynamic
// block is smallest.
//
//////////////////////////////////////////////////////////////////////////
void CDeflater::WriteBlock(bool bLast)
{
	CHuffmanEncoder		cLiteralLengths;
	CHuffmanEncoder		cDistances;
	CHuffmanEncoder		cCodeLengths;
	CHuffmanEncoder		cFixedLiteralLengths;
	CHuffmanEncoder		cFixedDistances;
	uint8				auiFixedLiteralLengths[DEFLATE_NUM_FIXED_LITERALS];
	uint8				auiFixedDistances[DEFLATE_NUM_FIXED_DISTANCES];
	uint8				auiLengths[DEFLATE_NUM_LITERAL_LENGTHS + DEFLATE_NUM_DISTANCES];
	uint8				auiCodeLengthSymbols[DEFLATE_NUM_LITERAL_LENGTHS + DEFLATE_NUM_DISTANCES];
	uint8				auiCodeLengthExtras[DEFLATE_NUM_LITERAL_LENGTHS + DEFLATE_NUM_DISTANCES];
	uint32				auiCodeLengthFrequencies[DEFLATE_NUM_CODE_LENGTHS];
	static const uint8	auiCodeLengthExtraBits[3] = { 2, 3, 7 };
	int					iNumLiteralLengths;
	int					iNumDistances;
	int					iNumCodeLengths;
	size				uiNumCodeLengthSymbols;
	size				uiDynamicBits;
	size				uiFixedBits;
	size				uiStoredBits;
	size				uiNumStored;
	size				i;
	uint8				uiSymbol;

	mauiLiteralFrequencies[DEFLATE_END_OF_BLOCK]++;

	cLiteralLengths.Init(mauiLiteralFrequencies, DEFLATE_NUM_LITERAL_LENGTHS, DEFLATE_MAX_BITS);
	cDistances.Init(mauiDistanceFrequencies, DEFLATE_NUM_DISTANCES, DEFLATE_MAX_BITS);

	iNumLiteralLengths = DEFLATE_NUM_LITERAL_LENGTHS;
	while ((iNumLiteralLengths > DEFLATE_FIRST_LENGTH_CODE) && (cLiteralLengths.GetLength(iNumLiteralLengths - 1) == 0))
	{
		iNumLiteralLengths--;
	}
	iNumDistances = DEFLATE_NUM_DISTANCES;
	while ((iNumDistances > 1) && (cDistances.GetLength(iNumDistances - 1) == 0))
	{
		iNumDistances--;
	}

	memcpy(auiLengths, cLiteralLengths.GetLengths(), iNumLiteralLengths);
	memcpy(&auiLengths[iNumLiteralLengths], cDistances.GetLengths(), iNumDistances);
	uiNumCodeLengthSymbols = BuildCodeLengthSymbols(auiLengths, iNumLiteralLengths + iNumDistances, auiCodeLengthSymbols, auiCodeLengthExtras);

	memset(auiCodeLengthFrequencies, 0, sizeof(auiCodeLengthFrequencies));
	for (i = 0; i < uiNumCodeLengthSymbols; i++)
	{
		auiCodeLengthFrequencies[auiCodeLengthSymbols[i]]++;
	}
	cCodeLengths.Init(auiCodeLengthFrequencies, DEFLATE_NUM_CODE_LENGTHS, DEFLATE_MAX_CODE_LENGTH_BITS);
	iNumCodeLengths = DEFLATE_NUM_CODE_LENGTHS;
	while ((iNumCodeLengths > 4) && (cCodeLengths.GetLength(gauiDeflateCodeLengthOrder[iNumCodeLengths - 1]) == 0))
	{
		iNumCodeLengths--;
	}

	uiDynamicBits = 3 + 5 + 5 + 4 + 3 * iNumCodeLengths;
	for (i = 0; i < uiNumCodeLengthSymbols; i++)
	{
		uiSymbol = auiCodeLengthSymbols[i];
		uiDynamicBits += cCodeLengths.GetLength(uiSymbol);
		if (uiSymbol >= 16)
		{
			uiDynamicBits += auiCodeLengthExtraBits[uiSymbol - 16];
		}
	}
	uiDynamicBits += CountSymbolBits(cLiteralLengths.GetLengths(), cDistances.GetLengths());

	DeflateFixedLiteralLengths(auiFixedLiteralLengths);
	DeflateFixedDistanceLengths(auiFixedDistances);
	uiFixedBits = 3 + CountSymbolBits(auiFixedLiteralLengths, auiFixedDistances);

	//Each stored block has a 3 bit header, up to 7 bits of padding and a 4 byte length.
	uiNumStored = (muiBlockLength + DEFLATE_MAX_STORED - 1) / DEFLATE_MAX_STORED;
	if (uiNumStored == 0)
	{
		uiNumStored = 1;
	}
	uiStoredBits = (muiBlockLength + uiNumStored * 5) * 8;

	if ((uiStoredBits <= uiFixedBits) && (uiStoredBits <= uiDynamicBits))
	{
		WriteStored(bLast);
	}
	else if (uiFixedBits <= uiDynamicBits)
	{
		mcBits.WriteBits(bLast ? 1 : 0, 1);
		mcBits.WriteBits(DEFLATE_BLOCK_FIXED, 2);
		cFixedLiteralLengths.Init(auiFixedLiteralLengths, DEFLATE_NUM_FIXED_LITERALS);
		cFixedDistances.Init(auiFixedDistances, DEFLATE_NUM_FIXED_DISTANCES);
		WriteSymbols(&cFixedLiteralLengths, &cFixedDistances);
		cFixedDistances.Kill();
		cFixedLiteralLengths.Kill();
	}
	else
	{
		mcBits.WriteBits(bLast ? 1 : 0, 1);
		mcBits.WriteBits(DEFLATE_BLOCK_DYNAMIC, 2);
		mcBits.WriteBits(iNumLiteralLengths - DEFLATE_FIRST_LENGTH_CODE, 5);
		mcBits.WriteBits(iNumDistances - 1, 5);
		mcBits.WriteBits(iNumCodeLengths - 4, 4);
		for (i = 0; i < (size)iNumCodeLengths; i++)
		{
			mcBits.WriteBits(cCodeLengths.GetLength(gauiDeflateCodeLengthOrder[i]), 3);
		}
		for (i = 0; i < uiNumCodeLengthSymbols; i++)
		{
			uiSymbol = auiCodeLengthSymbols[i];
			cCodeLengths.Write(&mcBits, uiSymbol);
			if (uiSymbol >= 16)
			{
				mcBits.WriteBits(auiCodeLengthExtras[i], auiCodeLengthExtraBits[uiSymbol - 16]);
			}
		}
		WriteSymbols(&cLiteralLengths, &cDistances);
	}

	cCodeLengths.Kill();
	cDistances.Kill();
	cLiteralLengths.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the block's input uncompressed, split into stored blocks of at
// most 65535 bytes.
//
//////////////////////////////////////////////////////////////////////////
void CDeflater::WriteStored(bool bLast)
{
	size	uiPosition;
	size	uiRemaining;
	size	uiChunk;

	uiPosition = muiBlockStart;
	uiRemaining = muiBlockLength;
	do
	{
		uiChunk = (uiRemaining < DEFLATE_MAX_STORED) ? uiRemaining : DEFLATE_MAX_STORED;
		mcBits.WriteBits((bLast && (uiChunk == uiRemaining)) ? 1 : 0, 1);
		mcBits.WriteBits(DEFLATE_BLOCK_STORED, 2);
		mcBits.AlignToByte();
		mcBits.WriteBits((uint32)uiChunk, 16);
		mcBits.WriteBits((uint32)~uiChunk, 16);
		mcBits.WriteBytes(&mpuiData[uiPosition], uiChunk);
		uiPosition += uiChunk;
		uiRemaining -= uiChunk;
	}
	while (uiRemaining > 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDeflater::WriteSymbols(CHuffmanEncoder* pcLiteralLengths, CHuffmanEncoder* pcDistances)
{
	size	i;
	int		iLength;
	size	uiDistance;
	int		iCode;

	for (i = 0; i < muiNumSymbols; i++)
	{
		iLength = mauiSymbolLengths[i];
		if (iLength == 0)
		{
			pcLiteralLengths->Write(&mcBits, mauiSymbolValues[i]);
		}
		else
		{
			iCode = DeflateLengthCode(iLength);
			pcLiteralLengths->Write(&mcBits, DEFLATE_FIRST_LENGTH_CODE + iCode);
			mcBits.WriteBits(iLength - gauiDeflateLengthBase[iCode], gauiDeflateLengthExtra[iCode]);

			uiDistance = mauiSymbolValues[i];
			iCode = DeflateDistanceCode(uiDistance);
			pcDistances->Write(&mcBits, iCode);
			mcBits.WriteBits((uint32)(uiDistance - gauiDeflateDistanceBase[iCode]), gauiDeflateDistanceExtra[iCode]);
		}
	}
	pcLiteralLengths->Write(&mcBits, DEFLATE_END_OF_BLOCK);
}
