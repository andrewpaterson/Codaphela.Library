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
#ifndef __ARRAY_INT_2D_H__
#define __ARRAY_INT_2D_H__
#include "ArrayInt.h"


//Make this inherit from CArrayTemplate2D<int>... After writing CArrayTemplate2D of course.
class CArrayInt2D
{
public:
	CArrayInt	mcArray;
	int			miWidth;
	int			miHeight;

	void	Init(void);
	void	Init(int iChunkSize);
	void	Kill(void);

	int		AddColumn(void);
	int		AddRow(void);
	void	InsertColumn(int iColumn);  //iColumn is the column to insert *before*.
	void	InsertRow(int iRow);
	void	InsertColumns(int iColumn, int iCount);
	void	InsertRows(int iRow, int iCount);

	void	RemoveColumn(int iColumn);
	void	RemoveRow(int iRow);
	void	RemoveColumns(int iColumn, int iCount);
	void	RemoveRows(int iRow, int iCount);

	void	SetColumns(int iNumColumns);
	void	SetRows(int iNumRows);
	void	SetSize(int iNumColumns, int iNumRows);

	int		Get(int x, int y);
	void	Set(int x, int y, int iValue);
	int		SafeGet(int x, int y, int iNullValue);
	void	SafeSet(int x, int y, int iValue);
	void	Fill(int iX1, int iY1, int iX2, int iY2, int iValue);
	void	CountFill(int iX1, int iY1, int iX2, int iY2);
};


#endif // __ARRAY_INT_2D_H__

