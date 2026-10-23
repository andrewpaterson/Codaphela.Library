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
#include "Positions.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositionColumn::Init(int32 x)
{
	miX = x;
	maiYs.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositionColumn::Kill(void)
{
	maiYs.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositionColumn::Add(int32 y)
{
	maiYs.AddIfUnique(y);
}


int32 CPositionColumn::GetX(void) { return miX; }
size CPositionColumn::NumYs(void) { return maiYs.NumElements(); }
int32 CPositionColumn::GetY(size uiIndex) { return maiYs.GetValue(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositions::Init(void)
{
	macColumns.Init();
	mcColumnIndices.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The start and end of every line.
//
//////////////////////////////////////////////////////////////////////////
void CPositions::Init(CArrayLine* pacLines)
{
	size	i;
	CLine*	pcLine;

	Init();
	for (i = 0; i < pacLines->NumElements(); i++)
	{
		pcLine = pacLines->Get(i);
		Add(pcLine->GetStart());
		Add(pcLine->GetEnd());
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositions::Kill(void)
{
	size	i;

	for (i = 0; i < macColumns.NumElements(); i++)
	{
		macColumns.Get(i)->Kill();
	}
	mcColumnIndices.Kill();
	macColumns.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositions::Add(SInt32Vec2* ps)
{
	Add(ps->x, ps->y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPositions::Add(int32 x, int32 y)
{
	size*				puiIndex;
	size				uiIndex;
	CPositionColumn*	pcColumn;

	puiIndex = mcColumnIndices.Get(x);
	if (puiIndex == NULL)
	{
		uiIndex = macColumns.NumElements();
		pcColumn = macColumns.Add();
		pcColumn->Init(x);
		mcColumnIndices.Put(x, &uiIndex);
	}
	else
	{
		pcColumn = macColumns.Get(*puiIndex);
	}
	pcColumn->Add(y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPositions::NumPositions(void)
{
	size	i;
	size	uiNumPositions;

	uiNumPositions = 0;
	for (i = 0; i < macColumns.NumElements(); i++)
	{
		uiNumPositions += macColumns.Get(i)->NumYs();
	}
	return uiNumPositions;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds every point to pasPositions sorted by x then y.
//
//////////////////////////////////////////////////////////////////////////
void CPositions::CalculatePositions(CArrayInt32Vec2* pasPositions)
{
	CArrayInt			aiXs;
	CArrayInt			aiYs;
	size				i;
	size				j;
	CPositionColumn*	pcColumn;
	SInt32Vec2*			psPosition;

	aiXs.Init();
	for (i = 0; i < macColumns.NumElements(); i++)
	{
		aiXs.Add(macColumns.Get(i)->GetX());
	}
	aiXs.QuickSort();

	for (i = 0; i < aiXs.NumElements(); i++)
	{
		pcColumn = macColumns.Get(*mcColumnIndices.Get(aiXs.GetValue(i)));
		aiYs.Init();
		for (j = 0; j < pcColumn->NumYs(); j++)
		{
			aiYs.Add(pcColumn->GetY(j));
		}
		aiYs.QuickSort();
		for (j = 0; j < aiYs.NumElements(); j++)
		{
			psPosition = pasPositions->Add();
			psPosition->Init(pcColumn->GetX(), aiYs.GetValue(j));
		}
		aiYs.Kill();
	}
	aiXs.Kill();
}


size CPositions::NumColumns(void) { return macColumns.NumElements(); }
CPositionColumn* CPositions::GetColumn(size uiIndex) { return macColumns.Get(uiIndex); }

