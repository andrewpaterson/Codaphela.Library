/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __FILE_READER_H__
#define __FILE_READER_H__
#include "ErrorTypes.h"
#include "FilePosition.h"


#define CheckRead(pvData, iSize) 	if (Read(pvData, iSize, 1) != 1) { return false; }


class CFileReader
{
public:
	//General
						bool	ReadData(void* pvData, size iDataSize);

	//Strings.
						bool	ReadStringLength(size* piLength);
						bool	ReadStringChars(char* szString, size iLength);
						bool	ReadString(char* szString, size iMaxLength);

	//Helpers.		
						bool	ReadInt8(int8* pi);
						bool	ReadInt8(uint8* pi);
						bool	ReadInt16(int16* pi);
						bool	ReadInt16(uint16* pi);
						bool	ReadInt32(int32* pi);
						bool	ReadInt32(uint32* pi);
						bool	ReadInt64(int64* plli);
						bool	ReadInt64(uint64* plli);
						bool	ReadSInt(int* pi);
						bool	ReadSUInt(uint* pi);
						bool	ReadSize(size* pui);
						bool	ReadFloat(float32* pf);
						bool	ReadFloat(float64* pf);
						bool	ReadChar(char8* pc);
						bool	ReadWChar(char16* pc);
						bool	ReadBool(bool* pb);
						bool	ReadBits(void* pvData, size iByteLength);
						bool	ReadIntArray(int32* pai, size iLength);

protected:
	virtual				size	Read(void* pvDest, size iSize, size iCount) =0;
};


#endif // __FILE_READER_H__

