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
#include "FileReader.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadData(void* pvData, filePos iDataSize)
{
	CheckRead(pvData, iDataSize);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadStringLength(int* piLength)
{
	CheckRead(piLength, sizeof(int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadStringChars(char* szString, int iLength)
{
	CheckRead(szString, iLength);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadString(char* szString, int iMaxLength)
{
	int		iLength;

	if (!ReadStringLength(&iLength))
	{
		return FALSE;
	}

	if (iLength > iMaxLength - 1)
	{
		iLength = iMaxLength - 1;
	}

	if (!ReadStringChars(szString, iLength))
	{
		return FALSE;
	}
	szString[iLength] = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadInt(int* pi)
{
	CheckRead(pi, sizeof(int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadInt(unsigned int* pui)
{
	CheckRead(pui, sizeof(unsigned int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadLong(long long int* plli)
{
	CheckRead(plli, sizeof(long long int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadLong(unsigned long long int* pulli)
{
	CheckRead(pulli, sizeof(unsigned long long int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadFloat(float* pf)
{
	CheckRead(pf, sizeof(float));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadDouble(double* pf)
{
	CheckRead(pf, sizeof(double));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadChar(char* pc)
{
	CheckRead(pc, sizeof(char));
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadChar(unsigned char* pc)
{
	CheckRead(pc, sizeof(unsigned char));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadBool(BOOL* pb)
{
	CheckRead(pb, sizeof(BOOL));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadShort(short int* pi)
{
	CheckRead(pi, sizeof(short int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadShort(unsigned short int* pi)
{
	CheckRead(pi, sizeof(unsigned short int));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadBits(void* pvData, int iByteLength)
{
	CheckRead(pvData, iByteLength);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CFileReader::ReadIntArray(int* pai, int iLength)
{
	CheckRead(pai, iLength * sizeof(int));
	return TRUE;
}
