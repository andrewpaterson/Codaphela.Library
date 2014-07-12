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
#ifndef __CSV_FILE_EDITABLE_H__
#define __CSV_FILE_EDITABLE_H__
#include "Chars.h"
#include "FileIO.h"
#include "ArrayTemplateMinimal.h"
#include "CSVFile.h"
#include "ArrayString.h"


struct SCSVRowEditable
{
	int				iFileOffset;
	CArrayString	aszData;

	void 		Init(void);
	void 		Kill(void);
	void 		Dump(void);
	int	 		IndexOf(char* szString);
	int			NumCells(void);
	CChars*		Get(int iIndex);
};


typedef CArrayTemplate<SCSVRowEditable> CArrayCSVRow;


class CCSVFileEditable : public CCSVFile
{
protected:
	CArrayCSVRow	masCSVRows;

public:
	void 				Init(char cSeparator);
	void 				Kill(void);

	BOOL 				ReadLine(void);
	BOOL				ReadLine(CChars* szString);
	BOOL 				ReadLine(int iFileOffset, SCSVRowEditable* psCSVRow);
	void 				ReadAllLines(void);
	void 				Dump(void);
	int					NumRows(void);
	SCSVRowEditable*	Get(int iRowNum);

protected:
	int					ReadLine(char* snz);
};


#endif // __CSV_FILE_EDITABLE_H__

