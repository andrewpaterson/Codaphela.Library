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
#ifndef __INFLATER_H__
#define __INFLATER_H__
#include "BaseLib/FileReader.h"
#include "BaseLib/ArrayUint8.h"
#include "BitReader.h"
#include "HuffmanDecoder.h"


#define INFLATER_UNLIMITED	((size)-1)


//Decompresses deflate data (RFC 1951), optionally wrapped in a zlib stream (RFC 1950) whose header and Adler-32
//checksum are checked.  Input is read from a file; output is appended to an array.  Corrupt, truncated or
//over long input fails rather than reading or writing out of bounds.
class CInflater
{
protected:
	CFileReader*		mpcInput;
	bool				mbZLib;
	size				muiMaxOutput;
	CBitReader			mcBits;
	CArrayUint8*		mpacOutput;
	uint8*				mpuiOutput;
	size				muiOutputUsed;
	size				muiOutputStart;
	CHuffmanDecoder		mcLiteralLengths;
	CHuffmanDecoder		mcDistances;

public:
	void	Init(CFileReader* pcInput, bool bZLib, size uiMaxOutput = INFLATER_UNLIMITED);
	void	Kill(void);

	bool	Inflate(CArrayUint8* pacOutput);

protected:
	bool	ReadZLibHeader(void);
	bool	ReadZLibTrailer(void);
	bool	InflateBlocks(void);
	bool	InflateStored(void);
	bool	InflateFixed(void);
	bool	InflateDynamic(void);
	bool	InflateCodes(void);
	bool	Reserve(size uiLength);
};


#endif // __INFLATER_H__
