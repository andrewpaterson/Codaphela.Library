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
#ifndef __HUFFMAN_ENCODER_H__
#define __HUFFMAN_ENCODER_H__
#include "DeflateCommon.h"
#include "BitWriter.h"


//Builds canonical Huffman codes (RFC 1951 section 3.2.2), either from symbol frequencies with a maximum code length
//or from given code lengths, and writes symbols with them.
class CHuffmanEncoder
{
protected:
	int		miNumSymbols;
	uint8	mauiLengths[HUFFMAN_MAX_SYMBOLS];
	uint16	mauiCodes[HUFFMAN_MAX_SYMBOLS];		//Bit reversed so they can be written least significant bit first.

public:
	void	Init(uint32* puiFrequencies, int iNumSymbols, int iMaxBits);
	void	Init(uint8* puiLengths, int iNumSymbols);
	void	Kill(void);

	void	Write(CBitWriter* pcWriter, int iSymbol);
	int		GetLength(int iSymbol);
	uint8*	GetLengths(void);
	int		GetNumSymbols(void);

protected:
	bool	BuildLengths(uint32* puiFrequencies, int iMaxBits);
	void	BuildCodes(void);
};


#endif // __HUFFMAN_ENCODER_H__
