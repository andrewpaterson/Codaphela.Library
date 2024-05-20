/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __FILE_WRITER_H__
#define __FILE_WRITER_H__
#include "ErrorTypes.h"
#include "FilePosition.h"


#define CheckWrite(pvData, iSize)	if (Write(pvData, iSize, 1) != 1) { return false; }


class CFileWriter
{
public:
	//General
						bool	WriteData(void* pvData, size iDataSize);

	//Strings.
						bool	WriteString(const char* szString);

	//Helpers.		
						bool	WriteByte(int8 i);
						bool	WriteByte(uint8 i);
						bool	WriteShort(int16 i);
						bool	WriteShort(uint16 i);
						bool	WriteInt32(int32 i);
						bool	WriteInt32(uint32 i);
						bool	WriteLong(int64 i);
						bool	WriteLong(uint64 i);
						bool	WriteSize(size ui);
						bool	WriteInt(int i);
						bool	WriteFloat(float32 f);
						bool	WriteFloat(float64 f);
						bool	WriteChar(char8 c);
						bool	WriteWChar(char16 c);
						bool	WriteBool(bool b);
						bool	WriteBits(void* pvData, size iBitLength);

						bool	WriteIntArray(int32* pai, size iLength);

protected:
	virtual				size	Write(const void* pvSource, size iSize, size iCount) =0;
};


#endif // __FILE_WRITER_H__

