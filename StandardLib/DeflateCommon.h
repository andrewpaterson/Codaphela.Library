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
#ifndef __DEFLATE_COMMON_H__
#define __DEFLATE_COMMON_H__
#include "BaseLib/PrimitiveTypes.h"


//Deflate (RFC 1951) constants and tables shared by the inflater and the deflater.
#define DEFLATE_MAX_BITS				15
#define DEFLATE_MAX_CODE_LENGTH_BITS	7
#define DEFLATE_NUM_LITERAL_LENGTHS		286		//0-255 literals, 256 end of block, 257-285 lengths.
#define DEFLATE_NUM_FIXED_LITERALS		288		//The fixed code also defines 286 and 287, which are never valid.
#define DEFLATE_NUM_DISTANCES			30
#define DEFLATE_NUM_FIXED_DISTANCES		30
#define HUFFMAN_MAX_SYMBOLS				DEFLATE_NUM_FIXED_LITERALS
#define DEFLATE_NUM_CODE_LENGTHS		19
#define DEFLATE_END_OF_BLOCK			256
#define DEFLATE_FIRST_LENGTH_CODE		257
#define DEFLATE_NUM_LENGTH_CODES		29
#define DEFLATE_MIN_MATCH				3
#define DEFLATE_MAX_MATCH				258
#define DEFLATE_WINDOW_SIZE				32768
#define DEFLATE_MAX_STORED				65535

#define DEFLATE_BLOCK_STORED			0
#define DEFLATE_BLOCK_FIXED				1
#define DEFLATE_BLOCK_DYNAMIC			2

#define ZLIB_METHOD_DEFLATE				8
#define ZLIB_MAX_WINDOW_BITS			7		//CINFO, the log2 of the window size minus 8.
#define ZLIB_PRESET_DICTIONARY			0x20


extern const uint16	gauiDeflateLengthBase[DEFLATE_NUM_LENGTH_CODES];
extern const uint8	gauiDeflateLengthExtra[DEFLATE_NUM_LENGTH_CODES];
extern const uint16	gauiDeflateDistanceBase[DEFLATE_NUM_DISTANCES];
extern const uint8	gauiDeflateDistanceExtra[DEFLATE_NUM_DISTANCES];
extern const uint8	gauiDeflateCodeLengthOrder[DEFLATE_NUM_CODE_LENGTHS];


void	DeflateFixedLiteralLengths(uint8* puiLengths);
void	DeflateFixedDistanceLengths(uint8* puiLengths);


#endif // __DEFLATE_COMMON_H__
