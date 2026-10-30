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
#include "LineSplitter.h"


//////////////////////////////////////////////////////////////////////////
//
// Splits pacLines where a horizontal and a vertical line meet at one of
// pcPositions and adds the pieces to pacOutput (Logicim's LineSplitter).
//
//////////////////////////////////////////////////////////////////////////
void SplitLines(CArrayLine* pacLines, CPositions* pcPositions, CArrayLine* pacOutput)
{
	CLinePositionCache	cCache;

	cCache.Init(pacLines);
	SplitLines(&cCache, pcPositions, NULL, pacOutput);
	cCache.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Splits the lines in pcCache at the junctions among pcPositions and at
// pasAdditionalJunctions, if not NULL, and adds them all to pacOutput.
// pcCache is left split.
//
//////////////////////////////////////////////////////////////////////////
void SplitLines(CLinePositionCache* pcCache, CPositions* pcPositions, CArrayInt32Vec2* pasAdditionalJunctions, CArrayLine* pacOutput)
{
	CArrayInt32Vec2	asJunctions;
	size			i;
	size			uiNumElements;

	asJunctions.Init();
	CalculateLineJunctions(pcPositions, pcCache, &asJunctions);
	if (pasAdditionalJunctions != NULL)
	{
		uiNumElements = pasAdditionalJunctions->NumElements();
		for (i = 0; i < uiNumElements; i++)
		{
			asJunctions.AddIfUnique(pasAdditionalJunctions->Get(i));
		}
	}

	uiNumElements = asJunctions.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcCache->Split(asJunctions.Get(i));
	}
	asJunctions.Kill();

	pcCache->GetLines(pacOutput);
}


//////////////////////////////////////////////////////////////////////////
//
// The positions where a horizontal and a vertical line meet with at least
// one of them passing through (not ending there).  Each is added once.
//
//////////////////////////////////////////////////////////////////////////
void CalculateLineJunctions(CPositions* pcPositions, CLinePositionCache* pcCache, CArrayInt32Vec2* pasJunctions)
{
	size				i;
	size				j;
	CPositionColumn*	pcColumn;
	int32				x;
	int32				y;
	CLine*				pcHorizontal;
	CLine*				pcVertical;
	SInt32Vec2			sJunction;

	for (i = 0; i < pcPositions->NumColumns(); i++)
	{
		pcColumn = pcPositions->GetColumn(i);
		x = pcColumn->GetX();
		for (j = 0; j < pcColumn->NumYs(); j++)
		{
			y = pcColumn->GetY(j);
			pcHorizontal = pcCache->GetHorizontalLine(x, y);
			if (pcHorizontal != NULL)
			{
				pcVertical = pcCache->GetVerticalLine(x, y);
				if (pcVertical != NULL)
				{
					if (((x > pcHorizontal->GetMinimumX()) && (x < pcHorizontal->GetMaximumX())) ||
						((y > pcVertical->GetMinimumY()) && (y < pcVertical->GetMaximumY())))
					{
						sJunction.Init(x, y);
						pasJunctions->AddIfUnique(&sJunction);
					}
				}
			}
		}
	}
}

