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
#ifndef __CSV_FILE_EDITABLE_H__
#define __CSV_FILE_EDITABLE_H__
#include "Chars.h"
#include "FileIO.h"
#include "ArrayTemplateMinimal.h"
#include "CSVFile.h"
#include "ArrayChars.h"


struct SCSVRowEditable
{
	filePos			iFileOffset;
	CArrayChars		aszData;

	void 		Init(void);
	void 		Kill(void);
	void 		Dump(void);
	size	 	IndexOf(char* szString);
	size		NumCells(void);
	CChars*		Get(size iIndex);
};


typedef CArrayTemplate<SCSVRowEditable> CArrayCSVRow;


class CCSVFileEditable : public CCSVFile
{
protected:
	CArrayCSVRow	masCSVRows;

public:
	void 				Init(char cSeparator);
	void 				Kill(void);

	bool 				ReadLine(void);
	bool				ReadLine(CChars* szString);
	bool 				ReadLine(filePos iFileOffset, SCSVRowEditable* psCSVRow);
	void 				ReadAllLines(void);
	void 				Dump(void);
	size				NumRows(void);
	SCSVRowEditable*	Get(size iRowNum);
};


#endif // __CSV_FILE_EDITABLE_H__

