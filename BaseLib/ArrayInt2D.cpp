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
#include "ArrayTemplate.h"
#include "ArrayInt2D.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::Init(void)
{
	mcArray.Init();
	miWidth = 0;
	miHeight = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::Kill(void)
{
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayInt2D::AddColumn(void)
{
	size	iOldWidth;

	iOldWidth = miWidth;
	InsertColumns(miWidth - 1, 1);
	return iOldWidth;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CArrayInt2D::AddRow(void)
{
	size	iOldHeight;

	iOldHeight = miHeight;
	InsertRows(miHeight - 1, 1);
	return iOldHeight;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::InsertColumn(size iColumn)
{
	InsertColumns(iColumn, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::InsertRow(size iRow)
{
	InsertRows(iRow, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::RemoveColumn(size iColumn)
{
	RemoveColumns(iColumn, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::RemoveRow(size iRow)
{
	RemoveRows(iRow, 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::SetColumns(size iNumColumns)
{
	if (miHeight == 0)
	{
		miWidth = iNumColumns;
	}
	else
	{
		if (iNumColumns > miWidth)
		{
			if (miWidth > 0)
			{
				InsertColumns(miWidth, iNumColumns-miWidth);
			}
			else
			{
				InsertColumns(0, iNumColumns);
			}
		}
		else if (iNumColumns < miWidth)
		{
			RemoveColumns(iNumColumns, miWidth-iNumColumns);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::SetRows(size iNumRows)
{
	if (miWidth == 0)
	{
		miHeight = iNumRows;
	}
	else
	{
		if (iNumRows > miHeight)
		{
			if (miHeight > 0)
			{
				InsertRows(miHeight, iNumRows-miHeight);
			}
			else
			{
				InsertRows(0, iNumRows);
			}
		}
		else if (iNumRows < miHeight)
		{
			RemoveRows(iNumRows, miHeight-iNumRows);
		}	
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::SetSize(size iNumColumns, size iNumRows)
{
	SetColumns(iNumColumns);
	SetRows(iNumRows);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::InsertColumns(size iColumn, size iCount)
{
	//Naive implementation, fix as necessary.
	size		i;

	i = miHeight;
	if (i != 0)
	{
		do
		{
			i--;
			mcArray.InsertNumAt(iCount, i * miWidth + iColumn);
		}
		while (i != 0);
	}
	miWidth += iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::InsertRows(size iRow, size iCount)
{
	mcArray.InsertNumAt(iCount * miWidth, iRow * miWidth);
	miHeight += iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::RemoveColumns(size iColumn, size iCount)
{
	//Naive implementation, fix as necessary.
	size		i;

	i = miHeight;
	if (i != 0)
	{
		do
		{
			i--;
			mcArray.RemoveRange(i * miWidth + iColumn, i * miWidth + iColumn + iCount);
		}
		while (i != 0);
	}
	miWidth -= iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::RemoveRows(size iRow, size iCount)
{
	mcArray.RemoveRange(iRow * miWidth, (iRow+iCount) * miWidth, true);
	miHeight -= iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint CArrayInt2D::Get(size x, size y)
{
	return mcArray.GetValue(x + y*miWidth);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::Set(size x, size y, uint iValue)
{
	mcArray.SetValue(x + y * miWidth, iValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint CArrayInt2D::SafeGet(size x, size y, uint iNullValue)
{
	if ((x < miWidth) && (y < miHeight))
	{
		return mcArray.GetValue(x + y * miWidth);
	}
	return iNullValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::SafeSet(size x, size y, uint iValue)
{
	if (((x >= 0) && (x < miWidth)) && ((y >= 0) && (y < miHeight)))
	{
		return mcArray.SetValue(x + y*miWidth, iValue);
	}
}

	
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::Fill(size iX1, size iY1, size iX2, size iY2, uint iValue)
{
	size	x;
	size	y;

	for (y = iY1; y < iY2; y++)
	{
		for (x = iX1; x < iX2; x++)
		{
			mcArray.SetValue(x + y*miWidth, iValue);
		}
	}
}

	
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayInt2D::CountFill(size iX1, size iY1, size iX2, size iY2)
{
	size	x;
	size	y;
	uint	iValue;

	iValue = 0;
	for (y = iY1; y < iY2; y++)
	{
		for (x = iX1; x < iX2; x++)
		{
			mcArray.SetValue(x + y*miWidth, iValue);
			iValue++;
		}
	}
}
