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
#ifndef __IMAGE_READER_COMMON_H__
#define __IMAGE_READER_COMMON_H__
#include "BaseLib/PrimitiveTypes.h"


uint16	ReadLittleEndianUInt16(uint8* pui);
uint32	ReadLittleEndianUInt32(uint8* pui);

//Reads the whole file into memory.  The caller frees the result.  Logs an error and returns NULL if it can't be read.
uint8*	LoadFileIntoMemory(char* szFilename, size* puiFileSize);


#endif // __IMAGE_READER_COMMON_H__
