/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "AbstractFile.h"


#define CheckWrite(pvData, iSize)	if (Write(pvData, iSize, 1) != 1) { return FALSE; }


class CFileWriter
{
public:
	//General
						BOOL	WriteData(void* pvData, filePos iDataSize);

	//Strings.
						BOOL	WriteString(const char* szString);

	//Helpers.		
						BOOL	WriteInt(int8 i);
						BOOL	WriteInt(uint8 i);
						BOOL	WriteInt(int16 i);
						BOOL	WriteInt(uint16 i);
						BOOL	WriteInt(int32 i);
						BOOL	WriteInt(uint32 i);
						BOOL	WriteInt(int64 i);
						BOOL	WriteInt(uint64 i);
						BOOL	WriteFloat(float32 f);
						BOOL	WriteFloat(float64 f);
						BOOL	WriteChar(char8 c);
						BOOL	WriteChar(char16 c);
						BOOL	WriteBool(BOOL b);
						BOOL	WriteBits(void* pvData, int iBitLength);

						BOOL	WriteIntArray(int* pai, int iLength);

protected:
	virtual				filePos	Write(const void* pvSource, filePos iSize, filePos iCount) =0;
};


#endif // !__FILE_WRITER_H__

