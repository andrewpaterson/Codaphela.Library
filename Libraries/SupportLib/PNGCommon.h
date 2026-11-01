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
#ifndef __PNG_COMMON_H__
#define __PNG_COMMON_H__
#include "BaseLib/FileWriter.h"


#define PNG_SIGNATURE_SIZE		8
#define PNG_IHDR_SIZE			13

#define PNG_COLOUR_GREY			0
#define PNG_COLOUR_RGB			2
#define PNG_COLOUR_PALETTE		3
#define PNG_COLOUR_GREY_ALPHA	4
#define PNG_COLOUR_RGB_ALPHA	6

#define PNG_FILTER_NONE			0
#define PNG_FILTER_SUB			1
#define PNG_FILTER_UP			2
#define PNG_FILTER_AVERAGE		3
#define PNG_FILTER_PAETH		4
#define PNG_NUM_FILTERS			5

#define PNG_INTERLACE_NONE		0
#define PNG_INTERLACE_ADAM7		1


extern const uint8	gauiPNGSignature[PNG_SIGNATURE_SIZE];


uint8	PNGPaethPredictor(uint8 uiLeft, uint8 uiUp, uint8 uiUpLeft);
bool	PNGWriteChunk(CFileWriter* pcFile, const char* szType, uint8* puiData, lsize uiLength);
uint32	PNGChunkCRC(uint8* puiType, uint8* puiData, lsize uiLength);


#endif // __PNG_COMMON_H__

