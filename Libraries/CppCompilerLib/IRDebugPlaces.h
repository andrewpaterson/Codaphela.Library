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
#ifndef __I_R_DEBUG_PLACES_H__
#define __I_R_DEBUG_PLACES_H__
#include "IRLiveness.h"


//Which value each of a function's debug slots (its variables a debugger is told of, promoted: SIRDebugValue) holds
//before each of its instructions, as a back end writes them (DebuggerFPlan.txt F1-3).  A block starts with the
//values its predecessors all end with, or where they differ the phi that takes each one's (none if no phi does); its
//markers change them in order.  A value is only the slot's while it is live (CIRLiveness): after its last use
//its home may be another's.  The back end says where each value is (CIRBackend::DebugPlaces).
class CIRDebugPlaces
{
protected:
	CIRFunction*		mpcFunction;
	CIRLiveness			mcLiveness;
	size				muiSlots;
	CArrayIRReferences	masEntries;		//Per block, per slot: what it holds as the block starts.
	CArrayIRReferences	masValues;		//Per slot: what it holds now (Move).
	CIRBlock*			mpcBlock;		//Where Move last moved to...
	size				muiNext;		//...and its next marker.
	int32				miPosition;

public:
	void			Init(void);
	void			Kill(void);

	bool			Begin(CIRFunction* pcFunction, CIRDominators* pcDominators);
	void			Move(CIRBlock* pcBlock, size uiInstruction);
	size			NumSlots(void) const;
	SIRReference	Value(size uiSlot);

protected:
	void			ComputeEntries(void);
	bool			Enter(CIRBlock* pcBlock, SIRReference* psValues, CArrayIRReferences* pasExits, CArrayTemplate<bool>* pabReached);
	SIRReference	MatchPhi(CIRBlock* pcBlock, size uiSlot, CArrayIRReferences* pasExits, CArrayTemplate<bool>* pabReached);
	void			Apply(CIRBlock* pcBlock, size uiBefore, size* puiNext, SIRReference* psValues);
};


#endif // __I_R_DEBUG_PLACES_H__

