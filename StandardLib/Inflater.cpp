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
#include "Adler32.h"
#include "Inflater.h"


#define INFLATER_MIN_GROWTH		65536


//////////////////////////////////////////////////////////////////////////
//
// uiMaxOutput limits how many bytes Inflate may produce, so a small
// corrupt or malicious stream can't claim a huge amount of memory.
//
//////////////////////////////////////////////////////////////////////////
void CInflater::Init(CFileReader* pcInput, bool bZLib, lsize uiMaxOutput)
{
	mpcInput = pcInput;
	mbZLib = bZLib;
	muiMaxOutput = uiMaxOutput;
	mpacOutput = NULL;
	mpuiOutput = NULL;
	muiOutputUsed = 0;
	muiOutputStart = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInflater::Kill(void)
{
	mpcInput = NULL;
	mpacOutput = NULL;
	mpuiOutput = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the decompressed data to pacOutput.  Returns false if the
// input is corrupt, truncated, fails its checksum or would produce more
// than the maximum output.  The input is left just after the stream.
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::Inflate(CArrayLargeUint8* pacOutput)
{
	bool	bResult;

	mpacOutput = pacOutput;
	muiOutputStart = pacOutput->NumElements();
	muiOutputUsed = muiOutputStart;
	mpuiOutput = pacOutput->GetData();
	mcBits.Init(mpcInput);

	bResult = true;
	if (mbZLib)
	{
		bResult = ReadZLibHeader();
	}
	bResult = bResult && InflateBlocks();
	if (mbZLib)
	{
		bResult = bResult && ReadZLibTrailer();
	}

	mcBits.Kill();
	pacOutput->Resize(muiOutputUsed);
	mpuiOutput = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The two byte zlib header: compression method and window size, then
// flags with a check that makes the pair a multiple of 31.
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::ReadZLibHeader(void)
{
	uint8	uiMethod;
	uint8	uiFlags;

	if (!mpcInput->ReadInt8(&uiMethod) || !mpcInput->ReadInt8(&uiFlags))
	{
		return false;
	}
	if ((((uiMethod << 8) | uiFlags) % 31) != 0)
	{
		return false;
	}
	if (((uiMethod & 0xF) != ZLIB_METHOD_DEFLATE) || ((uiMethod >> 4) > ZLIB_MAX_WINDOW_BITS))
	{
		return false;
	}

	//A preset dictionary is never used by PNG and isn't supported.
	return (uiFlags & ZLIB_PRESET_DICTIONARY) == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The Adler-32 of the decompressed data, most significant byte first,
// starting at the next whole byte.
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::ReadZLibTrailer(void)
{
	uint32		uiExpected;
	CAdler32	cAdler;

	mcBits.AlignToByte();
	if (!mpcInput->ReadBigEndianInt32(&uiExpected))
	{
		return false;
	}

	cAdler.Init();
	cAdler.Update(&mpuiOutput[muiOutputStart], muiOutputUsed - muiOutputStart);
	return cAdler.Get() == uiExpected;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::InflateBlocks(void)
{
	uint32	uiLast;
	uint32	uiType;
	bool	bResult;

	do
	{
		if (!mcBits.ReadBit(&uiLast) || !mcBits.ReadBits(2, &uiType))
		{
			return false;
		}

		if (uiType == DEFLATE_BLOCK_STORED)
		{
			bResult = InflateStored();
		}
		else if (uiType == DEFLATE_BLOCK_FIXED)
		{
			bResult = InflateFixed();
		}
		else if (uiType == DEFLATE_BLOCK_DYNAMIC)
		{
			bResult = InflateDynamic();
		}
		else
		{
			bResult = false;
		}

		if (!bResult)
		{
			return false;
		}
	}
	while (uiLast == 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes room for uiLength more bytes of output.
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::Reserve(lsize uiLength)
{
	lsize	uiNeeded;
	lsize	uiCapacity;

	if (uiLength > muiMaxOutput - (muiOutputUsed - muiOutputStart))
	{
		return false;
	}

	uiNeeded = muiOutputUsed + uiLength;
	uiCapacity = mpacOutput->NumElements();
	if (uiNeeded > uiCapacity)
	{
		uiCapacity = uiCapacity * 2;
		if (uiCapacity < INFLATER_MIN_GROWTH)
		{
			uiCapacity = INFLATER_MIN_GROWTH;
		}
		if (uiCapacity < uiNeeded)
		{
			uiCapacity = uiNeeded;
		}
		if ((muiMaxOutput != INFLATER_UNLIMITED) && (uiCapacity > muiOutputStart + muiMaxOutput))
		{
			uiCapacity = muiOutputStart + muiMaxOutput;
		}
		mpacOutput->Resize(uiCapacity);
		if (mpacOutput->NumElements() != uiCapacity)
		{
			return false;
		}
		mpuiOutput = mpacOutput->GetData();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An uncompressed block: the length and its complement, then the bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::InflateStored(void)
{
	uint16	uiLength;
	uint16	uiComplement;

	mcBits.AlignToByte();
	if (!mpcInput->ReadLittleEndianInt16(&uiLength) || !mpcInput->ReadLittleEndianInt16(&uiComplement))
	{
		return false;
	}
	if (uiLength != (uint16)~uiComplement)
	{
		return false;
	}

	if (!Reserve(uiLength))
	{
		return false;
	}
	if ((uiLength > 0) && !mpcInput->ReadData(&mpuiOutput[muiOutputUsed], uiLength))
	{
		return false;
	}
	muiOutputUsed += uiLength;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::InflateFixed(void)
{
	uint8	auiLengths[DEFLATE_NUM_FIXED_LITERALS];

	DeflateFixedLiteralLengths(auiLengths);
	mcLiteralLengths.Init(auiLengths, DEFLATE_NUM_FIXED_LITERALS);
	DeflateFixedDistanceLengths(auiLengths);
	mcDistances.Init(auiLengths, DEFLATE_NUM_FIXED_DISTANCES);
	return InflateCodes();
}


//////////////////////////////////////////////////////////////////////////
//
// A block that starts with the code lengths of its own Huffman codes,
// themselves Huffman coded and run length encoded.
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::InflateDynamic(void)
{
	uint8				auiLengths[DEFLATE_NUM_LITERAL_LENGTHS + DEFLATE_NUM_DISTANCES];
	uint32				uiNumLiteralLengths;
	uint32				uiNumDistances;
	uint32				uiNumCodeLengths;
	uint32				uiValue;
	uint32				uiRepeat;
	uint32				uiTotal;
	uint32				i;
	int					iSymbol;
	uint8				uiRepeatLength;
	CHuffmanDecoder		cCodeLengths;

	if (!mcBits.ReadBits(5, &uiNumLiteralLengths) || !mcBits.ReadBits(5, &uiNumDistances) || !mcBits.ReadBits(4, &uiNumCodeLengths))
	{
		return false;
	}
	uiNumLiteralLengths += DEFLATE_FIRST_LENGTH_CODE;
	uiNumDistances += 1;
	uiNumCodeLengths += 4;
	if ((uiNumLiteralLengths > DEFLATE_NUM_LITERAL_LENGTHS) || (uiNumDistances > DEFLATE_NUM_DISTANCES))
	{
		return false;
	}

	for (i = 0; i < DEFLATE_NUM_CODE_LENGTHS; i++)
	{
		uiValue = 0;
		if ((i < uiNumCodeLengths) && !mcBits.ReadBits(3, &uiValue))
		{
			return false;
		}
		auiLengths[gauiDeflateCodeLengthOrder[i]] = (uint8)uiValue;
	}
	if (!cCodeLengths.Init(auiLengths, DEFLATE_NUM_CODE_LENGTHS) || !cCodeLengths.IsComplete())
	{
		return false;
	}

	uiTotal = uiNumLiteralLengths + uiNumDistances;
	i = 0;
	while (i < uiTotal)
	{
		if (!cCodeLengths.Decode(&mcBits, &iSymbol))
		{
			return false;
		}

		if (iSymbol < 16)
		{
			auiLengths[i] = (uint8)iSymbol;
			i++;
		}
		else
		{
			if (iSymbol == 16)
			{
				//Repeat the previous length 3 to 6 times.
				if (i == 0)
				{
					return false;
				}
				uiRepeatLength = auiLengths[i - 1];
				if (!mcBits.ReadBits(2, &uiRepeat))
				{
					return false;
				}
				uiRepeat += 3;
			}
			else if (iSymbol == 17)
			{
				//3 to 10 zeros.
				uiRepeatLength = 0;
				if (!mcBits.ReadBits(3, &uiRepeat))
				{
					return false;
				}
				uiRepeat += 3;
			}
			else
			{
				//11 to 138 zeros.
				uiRepeatLength = 0;
				if (!mcBits.ReadBits(7, &uiRepeat))
				{
					return false;
				}
				uiRepeat += 11;
			}

			if (i + uiRepeat > uiTotal)
			{
				return false;
			}
			while (uiRepeat > 0)
			{
				auiLengths[i] = uiRepeatLength;
				i++;
				uiRepeat--;
			}
		}
	}

	//Without an end of block code the block could never end.
	if (auiLengths[DEFLATE_END_OF_BLOCK] == 0)
	{
		return false;
	}

	if (!mcLiteralLengths.Init(auiLengths, uiNumLiteralLengths) || (!mcLiteralLengths.IsComplete() && !mcLiteralLengths.IsSingleCode()))
	{
		return false;
	}
	if (!mcDistances.Init(&auiLengths[uiNumLiteralLengths], uiNumDistances) || (!mcDistances.IsComplete() && !mcDistances.IsSingleCode()))
	{
		return false;
	}
	return InflateCodes();
}


//////////////////////////////////////////////////////////////////////////
//
// Decodes literals and length / distance pairs until the end of block.
// A distance may not reach back before the start of the output.
//
//////////////////////////////////////////////////////////////////////////
bool CInflater::InflateCodes(void)
{
	int		iSymbol;
	uint32	uiExtra;
	size	uiLength;
	size	uiDistance;
	size	i;
	uint8*	puiFrom;
	uint8*	puiTo;

	for (;;)
	{
		if (!mcLiteralLengths.Decode(&mcBits, &iSymbol))
		{
			return false;
		}

		if (iSymbol < DEFLATE_END_OF_BLOCK)
		{
			if (!Reserve(1))
			{
				return false;
			}
			mpuiOutput[muiOutputUsed] = (uint8)iSymbol;
			muiOutputUsed++;
		}
		else if (iSymbol == DEFLATE_END_OF_BLOCK)
		{
			return true;
		}
		else
		{
			iSymbol -= DEFLATE_FIRST_LENGTH_CODE;
			if (iSymbol >= DEFLATE_NUM_LENGTH_CODES)
			{
				return false;
			}
			if (!mcBits.ReadBits(gauiDeflateLengthExtra[iSymbol], &uiExtra))
			{
				return false;
			}
			uiLength = gauiDeflateLengthBase[iSymbol] + uiExtra;

			if (!mcDistances.Decode(&mcBits, &iSymbol))
			{
				return false;
			}
			if (iSymbol >= DEFLATE_NUM_DISTANCES)
			{
				return false;
			}
			if (!mcBits.ReadBits(gauiDeflateDistanceExtra[iSymbol], &uiExtra))
			{
				return false;
			}
			uiDistance = gauiDeflateDistanceBase[iSymbol] + uiExtra;
			if (uiDistance > muiOutputUsed - muiOutputStart)
			{
				return false;
			}

			if (!Reserve(uiLength))
			{
				return false;
			}

			//The source and destination overlap when the distance is shorter than the length, so copy forwards a byte at a time.
			puiFrom = &mpuiOutput[muiOutputUsed - uiDistance];
			puiTo = &mpuiOutput[muiOutputUsed];
			for (i = 0; i < uiLength; i++)
			{
				puiTo[i] = puiFrom[i];
			}
			muiOutputUsed += uiLength;
		}
	}
}
