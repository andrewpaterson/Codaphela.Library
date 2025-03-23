/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
	this->iCelWidth =		iCelWidth;
	this->iCelHeight =		iCelHeight;
	this->iColumnCount =	iColumnCount;
	this->iRowCount =		iRowCount;
	this->iLeftOffset =		iLeftOffset;
	this->iTopOffset =		iTopOffset;
	this->iWidthSpacing =	iWidthSpacing;
	this->iHeightSpacing =	iHeightSpacing;
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

	if ((iColumnCount == -1) && (iCelWidth == -1))
	{
		iCelWidth = iImageWidth - iLeftOffset - iWidthSpacing;
		iColumnCount = 1;
	}
	else
	{
		if (iColumnCount == -1)
		{
			iTotalWidth = iCelWidth + iWidthSpacing;
			iColumnCount = (iImageWidth - iLeftOffset) / iTotalWidth;

			iRemainder = (iImageWidth - iLeftOffset) % iTotalWidth;
			if (iRemainder >= iCelWidth)
			{
				iColumnCount++;
			}
		}
		else if (iCelWidth == -1)
		{
			iCelWidth = (iImageWidth - iLeftOffset - iWidthSpacing * (iColumnCount-1)) / iColumnCount;
		}
	}
	if ((iRowCount == -1) && (iCelHeight == -1))
	{
		iCelHeight = iImageHeight;
		iRowCount = 1;
	}
	else
	{
		if (iRowCount == -1)
		{
			iTotalHeight = iCelHeight + iHeightSpacing;
			iRowCount = (iImageHeight - iTopOffset) / iTotalHeight;

			iRemainder = (iImageHeight - iTopOffset) % iTotalHeight;
			if (iRemainder >= iCelHeight)
			{
				iRowCount++;
			}
		}
		if (iCelHeight == -1)
		{
			iCelHeight = (iImageHeight - iTopOffset - iHeightSpacing * (iRowCount-1)) / iRowCount;
		}
	}
}

