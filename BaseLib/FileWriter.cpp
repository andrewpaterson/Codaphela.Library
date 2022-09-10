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
#include <string.h>
#include "FileWriter.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteData(void* pvData, filePos iDataSize)
{
	CheckWrite(pvData, iDataSize);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteString(const char* szString)
{
	int		iStrLen;

	iStrLen = (int)strlen(szString)+1;
	WriteInt(iStrLen);
	CheckWrite(szString, iStrLen);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(int32 i)
{
	CheckWrite(&i, sizeof(int));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(uint32 ui)
{
	CheckWrite(&ui, sizeof(unsigned int));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteFloat(float32 f)
{
	CheckWrite(&f, sizeof(float32));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteFloat(float64 f)
{
	CheckWrite(&f, sizeof(float64));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteChar(char8 c)
{
	CheckWrite(&c, sizeof(char));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteChar(char16 c)
{
	CheckWrite(&c, sizeof(char16));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteBool(bool b)
{
	CheckWrite(&b, sizeof(bool));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(int16 i)
{
	CheckWrite(&i, sizeof(int16));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(uint16 i)
{
	CheckWrite(&i, sizeof(uint16));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(int8 i)
{
	CheckWrite(&i, sizeof(int8));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(uint8 i)
{
	CheckWrite(&i, sizeof(uint8));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(int64 i)
{
	CheckWrite(&i, sizeof(int64));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteInt(uint64 ulli)
{
	CheckWrite(&ulli, sizeof(uint64));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteBits(void* pvData, int iByteLength)
{
	CheckWrite(pvData, iByteLength);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CFileWriter::WriteIntArray(int* pai, int iLength)
{
	CheckWrite(pai, iLength * sizeof(int));
	return true;
}

