/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/MarkupTextParser.h"
#include "SubImageXML.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CSubImageXML::Import(CMarkupTag* pcTag, CSubImage* pcSubImage)
{
	CMarkupTag*		pcOffsetTopLeft;
	CMarkupTag*		pcX;
	CMarkupTag*		pcY;
	CMarkupTag*		pcOffsetBottomRight;
	CMarkupTag*		pcHorizontalAlignment;
	CMarkupTag*		pcVerticalAlignment;
	CMarkupTag*		pcSourceRectangle;
	CMarkupTag*		pcWidth;
	CMarkupTag*		pcHeight;
	bool			bResult;
	int 			iLeft; 
	int 			iTop; 
	int 			iRight; 
	int 			iBottom; 
	int 			iLeftOffset; 
	int 			iTopOffset; 
	int 			iRightOffset; 
	int 			iBottomOffset; 
	int 			iAlignment;

	pcOffsetTopLeft = CMarkupTextParser::GetTag(pcTag, "OffsetTopLeft");
	if (!pcOffsetTopLeft)
	{
		return false;
	}

	pcOffsetBottomRight = CMarkupTextParser::GetTag(pcTag, "OffsetBottomRight");
	if (!pcOffsetTopLeft)
	{
		return false;
	}

	pcHorizontalAlignment = CMarkupTextParser::GetTag(pcTag, "HorizontalAlignment");
	if (!pcHorizontalAlignment)
	{
		return false;
	}

	pcVerticalAlignment = CMarkupTextParser::GetTag(pcTag, "VerticalAlignment");
	if (!pcVerticalAlignment)
	{
		return false;
	}

	pcSourceRectangle = CMarkupTextParser::GetTag(pcTag, "SourceRectangle");
	if (!pcSourceRectangle)
	{
		return false;
	}

	iAlignment = SUB_IMAGE_ALIGNMENT_CENTER_HORIZONTAL | SUB_IMAGE_ALIGNMENT_CENTER_VERTICAL;

	pcX = CMarkupTextParser::GetTag(pcOffsetTopLeft, "X");
	if (!pcX)
	{
		return false;
	}
	pcY = CMarkupTextParser::GetTag(pcOffsetTopLeft, "Y");
	if (!pcY)
	{
		return false;
	}

	bResult = CMarkupTextParser::ReadInteger(pcX, &iLeftOffset);
	if (!bResult)
	{
		return false;
	}
	bResult = CMarkupTextParser::ReadInteger(pcY, &iTopOffset);
	if (!bResult)
	{
		return false;
	}

	pcX = CMarkupTextParser::GetTag(pcOffsetBottomRight, "X");
	if (!pcX )
	{
		return false;
	}
	pcY = CMarkupTextParser::GetTag(pcOffsetBottomRight, "Y");
	if (!pcY)
	{
		return false;
	}

	bResult = CMarkupTextParser::ReadInteger(pcX, &iRightOffset);
	if (!bResult)
	{
		return false;
	}
	bResult = CMarkupTextParser::ReadInteger(pcY, &iBottomOffset);
	if (!bResult)
	{
		return false;
	}

	pcX = CMarkupTextParser::GetTag(pcSourceRectangle, "X");
	if (!pcX )
	{
		return false;
	}
	pcY = CMarkupTextParser::GetTag(pcSourceRectangle, "Y");
	if (!pcY)
	{
		return false;
	}

	bResult = CMarkupTextParser::ReadInteger(pcX, &iLeft);
	if (!bResult)
	{
		return false;
	}
	bResult = CMarkupTextParser::ReadInteger(pcY, &iTop);
	if (!bResult)
	{
		return false;
	}

	pcWidth = CMarkupTextParser::GetTag(pcSourceRectangle, "Width");
	if (!pcWidth)
	{
		return false;
	}
	pcHeight = CMarkupTextParser::GetTag(pcSourceRectangle, "Height");
	if (!pcHeight)
	{
		return false;
	}

	bResult = CMarkupTextParser::ReadInteger(pcWidth, &iRight);
	if (!bResult)
	{
		return false;
	}
	bResult = CMarkupTextParser::ReadInteger(pcHeight, &iBottom);
	if (!bResult)
	{
		return false;
	}

	iRight += iLeft;
	iBottom += iTop;
	pcSubImage->Init(iLeft, iTop, iRight, iBottom, iLeftOffset, iTopOffset, iRightOffset, iBottomOffset, iAlignment);
	return true;
}

