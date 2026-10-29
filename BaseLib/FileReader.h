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
#ifndef __FILE_READER_H__
#define __FILE_READER_H__
#include "ErrorTypes.h"
#include "FilePosition.h"


#define CheckRead(pvData, iSize) 	if (Read(pvData, iSize, 1) != 1) { return false; }


class CFileReader
{
public:
	//General
						bool	ReadData(void* pvData, lsize iDataSize);

	//Strings.
						bool	ReadStringLength(lsize* piLength);
#ifndef SIZE_IS_LSIZE
						bool	ReadStringLength(size* piLength);
#endif // SIZE_IS_LSIZE
						bool	ReadStringChars(char* szString, size iLength);
						bool	ReadString(char* szString, size iMaxLength);

	//Helpers.		
						bool	ReadInt8(int8* pi);
						bool	ReadInt8(uint8* pi);
						bool	ReadInt16(int16* pi);
						bool	ReadInt16(uint16* pi);
						bool	ReadInt32(int32* pi);
						bool	ReadInt32(uint32* pi);
#ifdef __W65C816__
						//An int is 16 bits on the W65C816: the 32 bit value is read into an int.
						bool	ReadInt32(int16* pi);
						bool	ReadInt32(uint16* pi);
#endif // __W65C816__
						bool	ReadInt64(int64* plli);
						bool	ReadInt64(uint64* plli);
						bool	ReadSInt(int* pi);
						bool	ReadSInt(uint* pi);
						bool	ReadSize(lsize* pui);  //Four bytes in 32 and 64 bit builds, sign extended so 0xffffffff is SIZE_MAX.
#ifndef SIZE_IS_LSIZE
						bool	ReadSize(size* pui);  //The same four bytes; fails if the value does not fit (all ones reads as MAX_SIZE).
#endif // SIZE_IS_LSIZE
						bool	ReadSizes(size* pai, size iLength);
						bool	ReadFloat(float32* pf);
						bool	ReadFloat(float64* pf);
						bool	ReadChar(char8* pc);
						bool	ReadWChar(char16* pc);
						bool	ReadBool(bool* pb);
						bool	ReadBits(void* pvData, size iByteLength);
						bool	ReadIntArray(int32* pai, size iLength);

	//Least significant byte first, whatever the platform's byte order.
						bool	ReadLittleEndianInt16(int16* pi);
						bool	ReadLittleEndianInt16(uint16* pui);
						bool	ReadLittleEndianInt32(int32* pi);
						bool	ReadLittleEndianInt32(uint32* pui);

	//Most significant byte first, whatever the platform's byte order.
						bool	ReadBigEndianInt16(int16* pi);
						bool	ReadBigEndianInt16(uint16* pui);
						bool	ReadBigEndianInt32(int32* pi);
						bool	ReadBigEndianInt32(uint32* pui);

						template<class M>
						bool	ReadPrimitive(M* pc);

protected:
	virtual				size	Read(void* pvDest, lsize iSize, lsize iCount) =0;
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CFileReader::ReadPrimitive(M* pc)
{
	CheckRead(pc, sizeof(M));
	return true;
}


#endif // __FILE_READER_H__

