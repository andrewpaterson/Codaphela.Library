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
#ifndef __FILE_READER_H__
#define __FILE_READER_H__
#include "ErrorTypes.h"
#include "AbstractFile.h"


#define CheckRead(pvData, iSize) 	if (Read(pvData, iSize, 1) != 1) { return FALSE; }


class CFileReader
{
public:
	//General
						BOOL	ReadData(void* pvData, filePos iDataSize);

	//Strings.
						BOOL	ReadStringLength(int* piLength);
						BOOL	ReadStringChars(char* szString, int iLength);
						BOOL	ReadString(char* szString, int iMaxLength);

	//Helpers.		
						BOOL	ReadInt(int* pi);
						BOOL	ReadLong(long long int* plli);
						BOOL	ReadFloat(float* pf);
						BOOL	ReadDouble(double* pf);
						BOOL	ReadChar(char* pc);
						BOOL	ReadChar(unsigned char* pc);
						BOOL	ReadBool(BOOL* pb);
						BOOL	ReadShort(short int* pi);
						BOOL	ReadShort(unsigned short int* pi);
						BOOL	ReadBits(void* pvData, int iByteLength);
						BOOL	ReadIntArray(int* pai, int iLength);

protected:
	virtual				filePos	Read(void* pvDest, filePos iSize, filePos iCount) =0;
};


#endif // __FILE_READER_H__

