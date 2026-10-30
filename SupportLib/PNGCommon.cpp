/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include "StandardLib/CRC32.h"
#include "PNGCommon.h"


const uint8 gauiPNGSignature[PNG_SIGNATURE_SIZE] = { 0x89, 'P', 'N', 'G', '\r', '\n', 0x1A, '\n' };


//////////////////////////////////////////////////////////////////////////
//
// Predicts a byte from its neighbours to the left, above and above left
// (PNG specification section 9.4).
//
//////////////////////////////////////////////////////////////////////////
uint8 PNGPaethPredictor(uint8 uiLeft, uint8 uiUp, uint8 uiUpLeft)
{
	int		iEstimate;
	int		iLeftDistance;
	int		iUpDistance;
	int		iUpLeftDistance;

	iEstimate = (int)uiLeft + (int)uiUp - (int)uiUpLeft;
	iLeftDistance = abs(iEstimate - uiLeft);
	iUpDistance = abs(iEstimate - uiUp);
	iUpLeftDistance = abs(iEstimate - uiUpLeft);
	if ((iLeftDistance <= iUpDistance) && (iLeftDistance <= iUpLeftDistance))
	{
		return uiLeft;
	}
	else if (iUpDistance <= iUpLeftDistance)
	{
		return uiUp;
	}
	return uiUpLeft;
}


//////////////////////////////////////////////////////////////////////////
//
// The CRC covers the chunk type and data but not the length.
//
//////////////////////////////////////////////////////////////////////////
uint32 PNGChunkCRC(uint8* puiType, uint8* puiData, lsize uiLength)
{
	CCRC32	cCRC;

	cCRC.Init();
	cCRC.Update(puiType, 4);
	cCRC.Update(puiData, uiLength);
	return cCRC.Get();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a chunk: the data length, the four character type, the data and
// the CRC.
//
//////////////////////////////////////////////////////////////////////////
bool PNGWriteChunk(CFileWriter* pcFile, const char* szType, uint8* puiData, lsize uiLength)
{
	return	pcFile->WriteBigEndianInt32((uint32)uiLength) &&
			pcFile->WriteData(szType, 4) &&
			((uiLength == 0) || pcFile->WriteData(puiData, uiLength)) &&
			pcFile->WriteBigEndianInt32(PNGChunkCRC((uint8*)szType, puiData, uiLength));
}

