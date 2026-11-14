/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_W65_C816_PLACES_H__
#define __I_R_W65_C816_PLACES_H__
#include "IRBackend.h"


class CIRW65C816Backend;


class CIRW65C816LiveRanges;
class CIRW65C816Groups;


//Where each of a function's temporaries lives while it runs, for the W65C816: Layout lays the frame out.  The
//temporaries that are never live at once share a place (PackTemporaries finds when each is live, GroupTemporaries
//and PlaceTemporaries put them down, the most used nearest the direct page's start), each slot has a place of its
//own, and the temporary that pays most is kept in X between its uses (AllocateX).  It writes what it decides into
//the back end's record of each temporary; OffsetOf and InX read it.
class CIRW65C816Places
{
protected:
	CIRW65C816Backend*	mpcBackend;

public:
	void	Init(CIRW65C816Backend* pcBackend);
	void	Kill(void);

	void			Layout(void);
	void			AddRegions(CIRW65C816LiveRanges* pcRanges);
	int32			PackTemporaries(void);
	void			AddPackEvents(CIRW65C816LiveRanges* pcRanges, struct SIR816PackEvents* psEvents);
	void			AddInstructionPackEvents(CIRW65C816LiveRanges* pcRanges, struct SIR816PackEvents* psEvents, CIRBlock* pcBlock, size uiInstruction, int32 iPosition, int32 iWeight);
	void			AddSuccessorPackEvents(CIRW65C816LiveRanges* pcRanges, struct SIR816PackEvents* psEvents, CIRBlock* pcBlock, int32 iPosition);
	void			OrderTemporaries(CIRW65C816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter, CArrayTemplate<int32>* paiOrder);
	int32			SharedLoopDepth(CIRBlock* pcFrom, CIRBlock* pcTo);
	int32			PlaceTemporaries(CIRW65C816LiveRanges* pcRanges, CArrayTemplate<int32>* paiOrder, CArrayTemplate<bool>* pabParameter);
	void			GroupTemporaries(CIRW65C816LiveRanges* pcRanges, CIRW65C816Groups* pcGroups, CArrayTemplate<int32>* paiOrder, CArrayTemplate<bool>* pabParameter);
	bool			IsGroupFree(CIRW65C816LiveRanges* pcRanges, CIRW65C816Groups* pcGroups, int32 iGroup, int32 iOffset, CArrayTemplate<bool>* pabParameter);
	void			PlaceTemporary(CIRW65C816LiveRanges* pcRanges, int32 iTemporary);
	int32			PlaceBytes(int32 iTemporary);
	void			AllocateX(CIRW65C816LiveRanges* pcRanges, CArrayTemplate<bool>* pabParameter);
	bool			InX(SIRReference sValue);
	int32			OffsetOf(SIRReference sTemporary);
};


#endif // __I_R_W65_C816_PLACES_H__
