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
#include "DeflateCommon.h"


const uint16 gauiDeflateLengthBase[DEFLATE_NUM_LENGTH_CODES] =
{
	3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258
};

const uint8 gauiDeflateLengthExtra[DEFLATE_NUM_LENGTH_CODES] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
};

const uint16 gauiDeflateDistanceBase[DEFLATE_NUM_DISTANCES] =
{
	1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577
};

const uint8 gauiDeflateDistanceExtra[DEFLATE_NUM_DISTANCES] =
{
	0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
};

//The order the code length code lengths are stored in a dynamic block header.
const uint8 gauiDeflateCodeLengthOrder[DEFLATE_NUM_CODE_LENGTHS] =
{
	16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15
};


//////////////////////////////////////////////////////////////////////////
//
// puiLengths must hold DEFLATE_NUM_FIXED_LITERALS lengths.
//
//////////////////////////////////////////////////////////////////////////
void DeflateFixedLiteralLengths(uint8* puiLengths)
{
	int		i;

	for (i = 0; i < 144; i++)
	{
		puiLengths[i] = 8;
	}
	for (; i < 256; i++)
	{
		puiLengths[i] = 9;
	}
	for (; i < 280; i++)
	{
		puiLengths[i] = 7;
	}
	for (; i < DEFLATE_NUM_FIXED_LITERALS; i++)
	{
		puiLengths[i] = 8;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// puiLengths must hold DEFLATE_NUM_FIXED_DISTANCES lengths.
//
//////////////////////////////////////////////////////////////////////////
void DeflateFixedDistanceLengths(uint8* puiLengths)
{
	int		i;

	for (i = 0; i < DEFLATE_NUM_FIXED_DISTANCES; i++)
	{
		puiLengths[i] = 5;
	}
}
