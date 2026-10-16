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
#ifndef __DEFLATER_H__
#define __DEFLATER_H__
#include "BaseLib/FileWriter.h"
#include "BitWriter.h"
#include "HuffmanEncoder.h"


#define DEFLATER_HASH_BITS			15
#define DEFLATER_HASH_SIZE			(1 << DEFLATER_HASH_BITS)
#define DEFLATER_BLOCK_SYMBOLS		16384


//Compresses data with deflate (RFC 1951), optionally wrapped in a zlib stream (RFC 1950).  Matches are found with
//hash chains and lazy evaluation.  Each block is written stored, with the fixed code or with its own dynamic codes,
//whichever is smallest.  The same input always gives the same output.
class CDeflater
{
protected:
	CFileWriter*	mpcOutput;
	bool			mbZLib;
	CBitWriter		mcBits;

	uint8*			mpuiData;
	size			muiDataSize;
	int32*			maiHead;		//Most recent position of each hash.
	int32*			maiPrevious;	//Previous position with the same hash, by position modulo the window.

	uint16*			mauiSymbolLengths;		//Match length, or 0 for a literal.
	uint16*			mauiSymbolValues;		//Match distance, or the literal byte.
	size			muiNumSymbols;
	size			muiBlockStart;
	size			muiBlockLength;
	uint32			mauiLiteralFrequencies[DEFLATE_NUM_LITERAL_LENGTHS];
	uint32			mauiDistanceFrequencies[DEFLATE_NUM_DISTANCES];

public:
	bool	Init(CFileWriter* pcOutput, bool bZLib);
	void	Kill(void);

	bool	Deflate(uint8* puiData, size uiSize);

protected:
	int32	InsertHash(size uiPosition);
	int		FindMatch(size uiPosition, int32 iChain, int iPreviousLength, size* puiDistance);
	void	AddLiteral(uint8 uiLiteral);
	void	AddMatch(int iLength, size uiDistance);
	void	ClearBlock(void);
	void	WriteBlock(bool bLast);
	void	WriteStored(bool bLast);
	void	WriteSymbols(CHuffmanEncoder* pcLiteralLengths, CHuffmanEncoder* pcDistances);
	size	CountSymbolBits(uint8* puiLiteralLengths, uint8* puiDistances);
	size	BuildCodeLengthSymbols(uint8* puiLengths, int iNumLengths, uint8* puiSymbols, uint8* puiExtras);
};


int	DeflateLengthCode(int iLength);
int	DeflateDistanceCode(size uiDistance);


#endif // __DEFLATER_H__
