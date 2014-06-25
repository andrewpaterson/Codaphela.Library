/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
BOOL CFileWriter::WriteData(void* pvData, filePos iDataSize)
{
	CheckWrite(pvData, iDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteString(char* szString)
{
	int		iStrLen;

	iStrLen = (int)strlen(szString)+1;
	CheckWrite(&iStrLen, sizeof(int));
	CheckWrite(szString, iStrLen);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteInt(int i)
{
	CheckWrite(&i, sizeof(int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteFloat(float f)
{
	CheckWrite(&f, sizeof(float));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteDouble(double f)
{
	CheckWrite(&f, sizeof(double));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteChar(char c)
{
	CheckWrite(&c, sizeof(char));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteChar(unsigned char c)
{
	CheckWrite(&c, sizeof(unsigned char));
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteBool(BOOL b)
{
	CheckWrite(&b, sizeof(BOOL));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteShort(short int i)
{
	CheckWrite(&i, sizeof(short int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteShort(unsigned short int i)
{
	CheckWrite(&i, sizeof(unsigned short int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteLong(long long int i)
{
	CheckWrite(&i, sizeof(long long int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteBits(void* pvData, int iByteLength)
{
	CheckWrite(pvData, iByteLength);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileWriter::WriteIntArray(int* pai, int iLength)
{
	CheckWrite(pai, iLength * sizeof(int));
	return TRUE;
}

