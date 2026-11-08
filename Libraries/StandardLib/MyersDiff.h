/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __MYERS_DIFF_H__
#define __MYERS_DIFF_H__
#include "BaseLib/ArrayTemplate.h"


//A change between two sequences: elements [uiStart1, uiEnd1) of the first were replaced by elements [uiStart2, uiEnd2)
//of the second.  One side is empty for a pure insertion or deletion.
struct SDiffRange
{
	lsize	uiStart1;
	lsize	uiEnd1;
	lsize	uiStart2;
	lsize	uiEnd2;

	void	Init(lsize uiStart1, lsize uiEnd1, lsize uiStart2, lsize uiEnd2);
	lsize	Length1(void);
	lsize	Length2(void);
};


typedef CArrayLargeTemplate<SDiffRange>	CArrayDiffRange;


//Myers' O(ND) difference algorithm in linear space (the middle snake bisection from "An O(ND) Difference Algorithm and
//Its Variations", 1986), as IntelliJ and git use by default.  The sequences are ids: equal elements have equal ids, so
//lines, words or characters are interned to ids before they are compared.  Common prefixes and suffixes are trimmed
//before each bisection.  The halves left to compare are kept on an explicit stack, not the call stack, so very
//different sequences can't overflow it.
//
//The changes are appended in order and touching changes from the same Diff are merged.  Elements between the changes
//are matched.
class CMyersDiff
{
protected:
	lsize*						mpaui1;
	lsize*						mpaui2;
	CArrayDiffRange*			mpacChanges;
	lsize						muiFirstChange;  //Changes before this one were in mpacChanges before Diff and are never merged with.
	CArrayDiffRange				macPending;
	CArrayLargeTemplate<int64>	maiPaths;  //Bisect's forward then backward furthest paths.  It only grows: the first range is the largest.

public:
	void	Init(void);
	void	Kill(void);

	void	Diff(lsize* paui1, lsize uiLength1, lsize* paui2, lsize uiLength2, CArrayDiffRange* pacChanges);

protected:
	void	DiffRange(SDiffRange* psRange);
	bool	Bisect(SDiffRange* psRange, lsize* puiSplit1, lsize* puiSplit2);
	void	AddChange(lsize uiStart1, lsize uiEnd1, lsize uiStart2, lsize uiEnd2);
};


#endif // __MYERS_DIFF_H__
