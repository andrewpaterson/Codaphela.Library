/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "TextAreaLayout.h"
#include "TextAreaShapes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaShapes::Init(CTextAreaLayout* pcLayout, CTextDocument* pcDocument)
{
	mpcLayout = pcLayout;
	mpcDocument = pcDocument;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaShapes::Kill(void)
{
	mpcLayout = NULL;
	mpcDocument = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The backgrounds, each over the ones before it, iWidth wide where they
// are across the text area.  Ranges need the text to have rows.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaShapes::AddBackgrounds(Ptr<CGeometryMap> pMap, CTextBackgrounds* pcBackgrounds, int32 iWidth)
{
	size					i;
	STextAreaBackground*	psBackground;

	for (i = 0; i < pcBackgrounds->NumBackgrounds(); i++)
	{
		psBackground = pcBackgrounds->Get(i);
		if (psBackground->eType == TAB_Range)
		{
			if (mpcLayout->HasRows())
			{
				AddRange(pMap, psBackground->uiStart, psBackground->uiEnd, psBackground->sColour);
			}
		}
		else if (psBackground->eType == TAB_Lines)
		{
			AddLines(pMap, psBackground, iWidth);
		}
		else
		{
			AddBoundary(pMap, psBackground, iWidth);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// One rectangle for each run of lines without gaps between them.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaShapes::AddLines(Ptr<CGeometryMap> pMap, STextAreaBackground* psBackground, int32 iWidth)
{
	size	uiLine;
	int32	iTop;
	int32	iBottom;
	int32	iRunTop;
	int32	iRunBottom;

	iRunTop = 0;
	iRunBottom = -1;
	for (uiLine = psBackground->uiStart; uiLine < psBackground->uiEnd; uiLine++)
	{
		if (!mpcLayout->GetLineRows(uiLine, &iTop, &iBottom))
		{
			break;
		}
		if (iTop != iRunBottom)
		{
			if (iRunBottom > iRunTop)
			{
				pMap->AddRectangle(0, iRunTop, iWidth, iRunBottom - iRunTop, psBackground->sColour);
			}
			iRunTop = iTop;
		}
		iRunBottom = iBottom;
	}
	if (iRunBottom > iRunTop)
	{
		pMap->AddRectangle(0, iRunTop, iWidth, iRunBottom - iRunTop, psBackground->sColour);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Centred on the line's top, but kept inside the text, so boundaries
// above the first line and below the last are whole.  A boundary with no
// line laid out is at the top.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaShapes::AddBoundary(Ptr<CGeometryMap> pMap, STextAreaBackground* psBackground, int32 iWidth)
{
	int32	iTop;
	int32	iBottom;
	int32	y;
	size	uiLast;

	y = 0;
	if (psBackground->uiStart < mpcDocument->NumLines())
	{
		if (mpcLayout->GetLineRows(psBackground->uiStart, &iTop, &iBottom))
		{
			y = iTop;
		}
	}
	else
	{
		uiLast = mpcDocument->NumLines() - 1;
		if (mpcLayout->GetLineRows(uiLast, &iTop, &iBottom))
		{
			y = iBottom + mpcLayout->GetGaps()->GetHeight(uiLast);
		}
	}

	y -= psBackground->iThickness / 2;
	if (y + psBackground->iThickness > mpcLayout->GetTextSize().y)
	{
		y = mpcLayout->GetTextSize().y - psBackground->iThickness;
	}
	if (y < 0)
	{
		y = 0;
	}
	pMap->AddRectangle(0, y, iWidth, psBackground->iThickness, psBackground->sColour);
}


//////////////////////////////////////////////////////////////////////////
//
// A block on each row uiStart to uiEnd is on, to the end of rows it goes
// past and a space further where they end at a line feed.  The text must
// have rows.
//
//////////////////////////////////////////////////////////////////////////
void CTextAreaShapes::AddRange(Ptr<CGeometryMap> pMap, size uiStart, size uiEnd, ARGB32 sColour)
{
	size				uiRow;
	size				uiStartRow;
	size				uiEndRow;
	int32				iLeft;
	int32				iRight;
	STextLayoutLine*	psLine;

	uiStartRow = mpcLayout->FindRow(uiStart, false);
	uiEndRow = mpcLayout->FindRow(uiEnd, true);
	for (uiRow = uiStartRow; uiRow <= uiEndRow; uiRow++)
	{
		psLine = mpcLayout->GetLayoutLine(uiRow);
		iLeft = (uiRow == uiStartRow) ? mpcLayout->GetPositionX(uiStart, false) : 0;
		if (uiRow == uiEndRow)
		{
			iRight = mpcLayout->GetPositionX(uiEnd, true);
		}
		else
		{
			iRight = mpcLayout->GetRowRight(uiRow);
			if (!mpcLayout->IsRowWrapped(uiRow))
			{
				iRight += mpcLayout->GetNewLineWidth();
			}
		}
		if (iRight > iLeft)
		{
			pMap->AddRectangle(iLeft, psLine->iTop, iRight - iLeft, psLine->iHeight, sColour);
		}
	}
}

