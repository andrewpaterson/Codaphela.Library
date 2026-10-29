/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
						bool	WriteData(const void* pvData, lsize iDataSize);

	//Strings.
						bool	WriteString(const char* szString);

	//Helpers.		
						bool	WriteInt(int i);
						bool	WriteInt(uint i);
						bool	WriteInt8(int8 i);
						bool	WriteInt8(uint8 i);
						bool	WriteInt16(int16 i);
						bool	WriteInt16(uint16 i);
						bool	WriteInt32(int32 i);
						bool	WriteInt32(uint32 i);
#ifdef __W65C816__
						//An int is 16 bits on the W65C816: ints match these exactly (the 32 bit ones would all need a conversion, so none
						//would be chosen).
						bool	WriteInt32(int16 i);
						bool	WriteInt32(uint16 i);
#endif // __W65C816__
						bool	WriteInt64(int64 i);
						bool	WriteInt64(uint64 i);
						bool	WriteSize(lsize ui);  //Four bytes in 32 and 64 bit builds, see ReadSize.
						bool	WriteSizes(size* pai, size iLength);
						bool	WriteSInt(int i);
						bool	WriteSInt(uint i);
						bool	WriteFloat(float32 f);
						bool	WriteFloat(float64 f);
						bool	WriteChar(char8 c);
						bool	WriteWChar(char16 c);
						bool	WriteBool(bool b);
						bool	WriteBits(void* pvData, size iBitLength);

						bool	WriteIntArray(int32* pai, size iLength);

	//Least significant byte first, whatever the platform's byte order.
						bool	WriteLittleEndianInt16(int16 i);
						bool	WriteLittleEndianInt16(uint16 ui);
						bool	WriteLittleEndianInt32(int32 i);
						bool	WriteLittleEndianInt32(uint32 ui);

	//Most significant byte first, whatever the platform's byte order.
						bool	WriteBigEndianInt16(int16 i);
						bool	WriteBigEndianInt16(uint16 ui);
						bool	WriteBigEndianInt32(int32 i);
						bool	WriteBigEndianInt32(uint32 ui);

						template<class M>
						bool	WritePrimitive(M c);

protected:
	virtual				size	Write(const void* pvSource, lsize iSize, lsize iCount) =0;
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CFileWriter::WritePrimitive(M c)
{
	CheckWrite(&c, sizeof(M));
	return true;
}


#endif // __FILE_WRITER_H__

