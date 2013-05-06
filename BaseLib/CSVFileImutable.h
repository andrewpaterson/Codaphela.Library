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
#ifndef __CSV_FILE_IMMUTABLE_H__
#define __CSV_FILE_IMMUTABLE_H__
#include "CSVFile.h"
#include "ArrayPointer.h"


struct SCSVRowImmutable
{
	int		iTotalSize;  //The complete size of this struct.
	int		iNumFields;  //The numver of terminating zeros in the string.
	int		iRowLength;  //The number of characters in the string including all terminating zeros.

	char*	aszFields[1];	//This is just the first fields positions, the size of the struct will be increased to contain iNumFields char*'s.
	//...
	//The size of the struct will also include all the zero terminated strings lengths.

	void 		Dump(void);
	int	 		IndexOf(char* szString);
	char*		Get(int iIndex);
};


class CCSVFileImmutable : public CCSVFile
{
public:
	CArrayPointer		mapsCSVRows;

	void 				Init(char cSeparator);
	void 				Kill(void);

	BOOL 				ReadLine(void);
	BOOL				ReadLine(char* szString, int iMaxLength);
	BOOL 				ReadLine(int iFileOffset, SCSVRowImmutable** ppsCSVRow);
	SCSVRowImmutable*	AllocateRow(char* szText);
	void				FreeRow(SCSVRowImmutable* psCSVRow);
	void 				ReadAllLines(void);
	void 				Dump(void);
	int					NumRows(void);
	SCSVRowImmutable*	Get(int iRowNum);
};


#endif //__CSV_FILE_IMMUTABLE_H__

