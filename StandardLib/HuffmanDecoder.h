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
#ifndef __HUFFMAN_DECODER_H__
#define __HUFFMAN_DECODER_H__
#include "DeflateCommon.h"
#include "BitReader.h"


//Decodes canonical Huffman codes (RFC 1951 section 3.2.2) given the code length of each symbol.  A length of zero
//means the symbol is not used.
class CHuffmanDecoder
{
protected:
	int16	maiCount[DEFLATE_MAX_BITS + 1];		//Number of codes of each length.
	int16	maiSymbols[HUFFMAN_MAX_SYMBOLS];	//Symbols ordered by code.
	int		miNumCodes;
	int		miUnused;							//Codes left over by an incomplete set of lengths.

public:
	bool	Init(uint8* puiLengths, int iNumSymbols);
	void	Kill(void);

	bool	IsComplete(void);
	bool	IsSingleCode(void);

	bool	Decode(CBitReader* pcReader, int* piSymbol);
};


#endif // __HUFFMAN_DECODER_H__
