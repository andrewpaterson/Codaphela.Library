/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "ImageDividerNumbers.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDividerNumbers::InitGeneral(int iCelWidth, int iCelHeight, int iColumnCount, int iRowCount, int iLeftOffset, int iTopOffset, int iWidthSpacing, int iHeightSpacing)
{
	this->miCelWidth =		iCelWidth;
	this->miCelHeight =		iCelHeight;
	this->miColumnCount =	iColumnCount;
	this->miRowCount =		iRowCount;
	this->miLeftOffset =		iLeftOffset;
	this->miTopOffset =		iTopOffset;
	this->miWidthSpacing =	iWidthSpacing;
	this->miHeightSpacing =	iHeightSpacing;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDividerNumbers::InitFromCelSize(int iCelWidth, int iCelHeight, int iLeftOffset, int iTopOffset, int iWidthSpacing, int iHeightSpacing)
{
	InitGeneral(iCelWidth, iCelHeight, -1, -1, iLeftOffset, iTopOffset, iWidthSpacing, iHeightSpacing);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDividerNumbers::InitFromRowsColumns(int iColumnCount, int iRowCount, int iLeftOffset, int iTopOffset, int iWidthSpacing, int iHeightSpacing)
{
	InitGeneral(-1, -1, iColumnCount, iRowCount, iLeftOffset, iTopOffset, iWidthSpacing, iHeightSpacing);
}


void CImageDividerNumbers::InitFromSingle(int iLeftOffset, int iTopOffset, int iRightOffset, int iBottomOffset)
{
	InitGeneral(-1, -1, 1, 1, iLeftOffset, iTopOffset, iRightOffset, iBottomOffset);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageDividerNumbers::Calculate(int iImageWidth, int iImageHeight)
{
	int					iTotalWidth;
	int					iTotalHeight;
	int					iRemainder;

	if ((miColumnCount == -1) && (miCelWidth == -1))
	{
		miCelWidth = iImageWidth - miLeftOffset - miWidthSpacing;
		miColumnCount = 1;
	}
	else
	{
		if (miColumnCount == -1)
		{
			iTotalWidth = miCelWidth + miWidthSpacing;
			miColumnCount = (iImageWidth - miLeftOffset) / iTotalWidth;

			iRemainder = (iImageWidth - miLeftOffset) % iTotalWidth;
			if (iRemainder >= miCelWidth)
			{
				miColumnCount++;
			}
		}
		else if (miCelWidth == -1)
		{
			miCelWidth = (iImageWidth - miLeftOffset - miWidthSpacing * (miColumnCount-1)) / miColumnCount;
		}
	}
	if ((miRowCount == -1) && (miCelHeight == -1))
	{
		miCelHeight = iImageHeight;
		miRowCount = 1;
	}
	else
	{
		if (miRowCount == -1)
		{
			iTotalHeight = miCelHeight + miHeightSpacing;
			miRowCount = (iImageHeight - miTopOffset) / iTotalHeight;

			iRemainder = (iImageHeight - miTopOffset) % iTotalHeight;
			if (iRemainder >= miCelHeight)
			{
				miRowCount++;
			}
		}
		if (miCelHeight == -1)
		{
			miCelHeight = (iImageHeight - miTopOffset - miHeightSpacing * (miRowCount-1)) / miRowCount;
		}
	}
}

