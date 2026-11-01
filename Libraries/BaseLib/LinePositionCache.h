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
#ifndef __LINE_POSITION_CACHE_H__
#define __LINE_POSITION_CACHE_H__
#include "MapIntTemplate.h"
#include "Line.h"


//The lines on one vertical (keyed by x) or one horizontal (keyed by y) of a CLinePositionCache.
class CLineRow
{
protected:
	int32		miKey;
	CArrayLine	macLines;

public:
	void		Init(int32 iKey);
	void		Kill(void);

	int32		GetKey(void);
	CArrayLine*	GetLines(void);
};


typedef CArrayTemplate<CLineRow>	CArrayLineRow;


//Horizontal and vertical lines found by the points they pass through, ported from Logicim's LinePositionCache.  Lines
//that are neither are dropped.  The CLine pointers returned are only good until the cache next changes.
class CLinePositionCache
{
protected:
	CArrayLineRow			macVerticalRows;
	CMapIntTemplate<size>	mcVerticalIndices;
	CArrayLineRow			macHorizontalRows;
	CMapIntTemplate<size>	mcHorizontalIndices;

public:
	void		Init(CArrayLine* pacLines);
	void		Kill(void);

	void		Split(SInt32Vec2* ps);
	void		Split(int32 x, int32 y);

	CLine*		GetHorizontalLine(int32 x, int32 y);
	CLine*		GetVerticalLine(int32 x, int32 y);
	bool		TouchesLine(int32 x, int32 y);
	void		GetLines(CArrayLine* pacLines);

protected:
	void		AddVerticalLine(CLine* pcLine);
	void		AddHorizontalLine(CLine* pcLine);
	void		AddLine(CArrayLineRow* pacRows, CMapIntTemplate<size>* pcIndices, int32 iKey, CLine* pcLine);
	void		RemoveLine(CArrayLineRow* pacRows, CMapIntTemplate<size>* pcIndices, int32 iKey, CLine* pcLine);
	CLine*		GetLine(CArrayLineRow* pacRows, CMapIntTemplate<size>* pcIndices, int32 iKey, int32 x, int32 y);
	void		KillRows(CArrayLineRow* pacRows);
};


#endif // __LINE_POSITION_CACHE_H__

