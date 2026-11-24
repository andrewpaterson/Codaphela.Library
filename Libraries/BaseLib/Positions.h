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
#ifndef __POSITIONS_H__
#define __POSITIONS_H__
#include "ArrayInt.h"
#include "HashMapPrimitiveTemplate.h"
#include "Line.h"


//The ys at one x of a CPositions, in the order they were added.
class CPositionColumn
{
protected:
	int32		miX;
	CArrayInt	maiYs;

public:
	void		Init(int32 x);
	void		Kill(void);

	void		Add(int32 y);

	int32		GetX(void) const;
	size		NumYs(void) const;
	int32		GetY(size uiIndex);
};


typedef CArrayTemplate<CPositionColumn>	CArrayPositionColumn;


//A set of grid points grouped by x, ported from Logicim's Positions.  Columns and the ys in each keep the order they
//were added; each point is kept once.
class CPositions
{
protected:
	CArrayPositionColumn	macColumns;
	CHashMapIntTemplate<size>	mcColumnIndices;

public:
	void				Init(void);
	void				Init(CArrayLine* pacLines);
	void				Kill(void);

	void				Add(SInt32Vec2* ps);
	void				Add(int32 x, int32 y);

	size				NumColumns(void) const;
	CPositionColumn*	GetColumn(size uiIndex);
	size				NumPositions(void);
	void				CalculatePositions(CArrayInt32Vec2* pasPositions);
};


#endif // __POSITIONS_H__

