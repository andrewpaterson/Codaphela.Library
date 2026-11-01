/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "LinePositionCache.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineRow::Init(int32 iKey)
{
	miKey = iKey;
	macLines.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineRow::Kill(void)
{
	macLines.Kill();
}


int32 CLineRow::GetKey(void) { return miKey; }
CArrayLine* CLineRow::GetLines(void) { return &macLines; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::Init(CArrayLine* pacLines)
{
	size	i;
	CLine*	pcLine;

	macVerticalRows.Init();
	mcVerticalIndices.Init();
	macHorizontalRows.Init();
	mcHorizontalIndices.Init();

	for (i = 0; i < pacLines->NumElements(); i++)
	{
		pcLine = pacLines->Get(i);
		if (pcLine->IsNorthSouth())
		{
			AddVerticalLine(pcLine);
		}
		else if (pcLine->IsEastWest())
		{
			AddHorizontalLine(pcLine);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::Kill(void)
{
	KillRows(&macHorizontalRows);
	mcHorizontalIndices.Kill();
	KillRows(&macVerticalRows);
	mcVerticalIndices.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::KillRows(CArrayLineRow* pacRows)
{
	size	i;
	size	uiNumElements;

	uiNumElements = pacRows->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pacRows->Get(i)->Kill();
	}
	pacRows->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::AddVerticalLine(CLine* pcLine)
{
	AddLine(&macVerticalRows, &mcVerticalIndices, pcLine->GetX(), pcLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::AddHorizontalLine(CLine* pcLine)
{
	AddLine(&macHorizontalRows, &mcHorizontalIndices, pcLine->GetY(), pcLine);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::AddLine(CArrayLineRow* pacRows, CMapIntTemplate<size>* pcIndices, int32 iKey, CLine* pcLine)
{
	size*		puiIndex;
	size		uiIndex;
	CLineRow*	pcRow;

	puiIndex = pcIndices->Get(iKey);
	if (puiIndex == NULL)
	{
		uiIndex = pacRows->NumElements();
		pcRow = pacRows->Add();
		pcRow->Init(iKey);
		pcIndices->Put(iKey, &uiIndex);
	}
	else
	{
		pcRow = pacRows->Get(*puiIndex);
	}
	pcRow->GetLines()->Add(pcLine);
}


//////////////////////////////////////////////////////////////////////////
//
// pcLine must be in the row; the row keeps its place if it empties.
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::RemoveLine(CArrayLineRow* pacRows, CMapIntTemplate<size>* pcIndices, int32 iKey, CLine* pcLine)
{
	size*		puiIndex;
	CArrayLine*	pacLines;

	puiIndex = pcIndices->Get(iKey);
	if (puiIndex != NULL)
	{
		pacLines = pacRows->Get(*puiIndex)->GetLines();
		pacLines->RemoveAt(pacLines->GetIndex(pcLine));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The first line in the row through x, y.
//
//////////////////////////////////////////////////////////////////////////
CLine* CLinePositionCache::GetLine(CArrayLineRow* pacRows, CMapIntTemplate<size>* pcIndices, int32 iKey, int32 x, int32 y)
{
	size*		puiIndex;
	CArrayLine*	pacLines;
	CLine*		pcLine;
	size		i;
	size		uiNumElements;

	puiIndex = pcIndices->Get(iKey);
	if (puiIndex != NULL)
	{
		pacLines = pacRows->Get(*puiIndex)->GetLines();
		uiNumElements = pacLines->NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			pcLine = pacLines->Get(i);
			if (pcLine->IsPositionOn(x, y))
			{
				return pcLine;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLine* CLinePositionCache::GetHorizontalLine(int32 x, int32 y)
{
	return GetLine(&macHorizontalRows, &mcHorizontalIndices, y, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLine* CLinePositionCache::GetVerticalLine(int32 x, int32 y)
{
	return GetLine(&macVerticalRows, &mcVerticalIndices, x, x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::Split(SInt32Vec2* ps)
{
	Split(ps->x, ps->y);
}


//////////////////////////////////////////////////////////////////////////
//
// Splits the vertical and the horizontal line through x, y in two there,
// unless x, y is an end.  The halves go after the other lines in their row.
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::Split(int32 x, int32 y)
{
	CLine*	pcLine;
	int32	iMinimum;
	int32	iMaximum;
	CLine	cLine1;
	CLine	cLine2;

	pcLine = GetVerticalLine(x, y);
	if (pcLine != NULL)
	{
		iMinimum = pcLine->GetMinimumY();
		iMaximum = pcLine->GetMaximumY();
		if ((y > iMinimum) && (y < iMaximum))
		{
			RemoveLine(&macVerticalRows, &mcVerticalIndices, x, pcLine);
			cLine1.Init(x, iMinimum, x, y);
			cLine2.Init(x, y, x, iMaximum);
			AddVerticalLine(&cLine1);
			AddVerticalLine(&cLine2);
		}
	}

	pcLine = GetHorizontalLine(x, y);
	if (pcLine != NULL)
	{
		iMinimum = pcLine->GetMinimumX();
		iMaximum = pcLine->GetMaximumX();
		if ((x > iMinimum) && (x < iMaximum))
		{
			RemoveLine(&macHorizontalRows, &mcHorizontalIndices, y, pcLine);
			cLine1.Init(iMinimum, y, x, y);
			cLine2.Init(x, y, iMaximum, y);
			AddHorizontalLine(&cLine1);
			AddHorizontalLine(&cLine2);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the horizontal lines then the vertical ones, each row in the order
// it was first used.
//
//////////////////////////////////////////////////////////////////////////
void CLinePositionCache::GetLines(CArrayLine* pacLines)
{
	size		i;
	size		j;
	CArrayLine*	pacRowLines;
	size		uiNumElements;

	uiNumElements = macHorizontalRows.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pacRowLines = macHorizontalRows.Get(i)->GetLines();
		for (j = 0; j < pacRowLines->NumElements(); j++)
		{
			pacLines->Add(pacRowLines->Get(j));
		}
	}
	uiNumElements = macVerticalRows.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pacRowLines = macVerticalRows.Get(i)->GetLines();
		for (j = 0; j < pacRowLines->NumElements(); j++)
		{
			pacLines->Add(pacRowLines->Get(j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLinePositionCache::TouchesLine(int32 x, int32 y)
{
	return (GetVerticalLine(x, y) != NULL) || (GetHorizontalLine(x, y) != NULL);
}

