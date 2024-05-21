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
#ifndef __ARRAY_INT_2D_H__
#define __ARRAY_INT_2D_H__
#include "ArrayInt.h"


//Make this inherit from CArrayTemplate2D<int>... After writing CArrayTemplate2D of course.
class CArrayInt2D
{
public:
	CArrayInt	mcArray;
	size		miWidth;
	size		miHeight;

	void	Init(void);
	void	Kill(void);

	size	AddColumn(void);
	size	AddRow(void);
	void	InsertColumn(size iColumn);  //iColumn is the column to insert *before*.
	void	InsertRow(size iRow);
	void	InsertColumns(size iColumn, size iCount);
	void	InsertRows(size iRow, size iCount);

	void	RemoveColumn(size iColumn);
	void	RemoveRow(size iRow);
	void	RemoveColumns(size iColumn, size iCount);
	void	RemoveRows(size iRow, size iCount);

	void	SetColumns(size iNumColumns);
	void	SetRows(size iNumRows);
	void	SetSize(size iNumColumns, size iNumRows);

	uint	Get(size x, size y);
	void	Set(size x, size y, uint iValue);
	uint	SafeGet(size x, size y, uint iNullValue);
	void	SafeSet(size x, size y, uint iValue);
	void	Fill(size iX1, size iY1, size iX2, size iY2, uint iValue);
	void	CountFill(size iX1, size iY1, size iX2, size iY2);
};


#endif // __ARRAY_INT_2D_H__

