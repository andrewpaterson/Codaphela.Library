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
#ifndef __I_R65_PLACES_H__
#define __I_R65_PLACES_H__
#include "IRBackend.h"


class CIR65Backend;


//Where each of a function's temporaries lives while it runs, for the 65C64: a register (X, Y, V, W or B), a
//pseudo register, or the frame.  Layout gives every temporary a place in the frame; Allocate then finds the
//temporaries that can share a home (Coalesce), weighs them, and gives the best homes to those that pay most
//(AllocateClasses).  PlaceOf and the questions beside it read what was decided.
class CIR65Places
{
protected:
	CIR65Backend*	mpcBackend;

public:
	void	Init(CIR65Backend* pcBackend);
	void	Kill(void);

	SIRInstruction*		FusedComparison(CIRBlock* pcBlock);
	void				MarkInA(void);
	int32				ParameterOffset(size uiParameter);
	void				ArrivalPlace(size uiParameter, SIR65Place* psPlace);
	void				FunctionArguments(void);
	int					ParameterRegister(size uiParameter);
	int					ArgumentUnits(SIRType sType);
	void				Layout(void);
	bool				NeedsHome(int32 iTemporary);
	void				Allocate(void);
	void				DiscountAddresses(CArrayIR65Weights* pasWeights);
	void				FindChanges(void);
	void				Coalesce(void);
	int32				Find(int32 iTemporary);
	bool				Interferes(int32 iLeader1, int32 iLeader2);
	void				Join(int32 iLeader1, int32 iLeader2);
	bool				Crosses(int32 iLeader, CArrayTemplate<int32>* paiPositions);
	bool				LiveAt(int32 iLeader, CArrayTemplate<int32>* paiPositions);
	bool				HasParameter(int32 iLeader);
	void				AddChunk(int32 iFrom, int32 iTo, size uiStart);
	bool				CheckHomeB(bool bChanges);
	bool				HomeFree(int32 iHome, int32 iLeader);
	int32				FindRange(CArrayIRRanges* pasRanges, SIRRange* psRange);
	void				TakeHome(int32 iHome, int32 iLeader);
	void				PlaceClass(int32 iLeader, uint8 uiKind, int32 iOffset, int32 iBytes);
	bool				TakeHomes(int32 iHome, int32 iUnits, int32 iLeader);
	bool				AllocateClass(int32 iLeader, uint32 uiWeight);
	bool				AllocateClasses(CArrayIR65Weights* pasWeights);
	void				ClearHomes(void);
	bool				CanUseV(void);
	bool				PaysV(void);
	bool				IsAddressRegister(SIR65Place* psPlace);
	bool				HasWidth(SIR65Place* psPlace);
	bool				Fits32BitRegister(int32 iLeader);
	int32				FindUnits(int32 iFirstHome, int32 iUnits, int32 iLeader);
	SIR65Place*			PlaceOf(SIRReference sValue);
	bool				IsHome(SIR65Place* psPlace);
	bool				IsMemoryHome(SIR65Place* psPlace);
	bool				SamePlace(SIRReference sValue1, SIRReference sValue2);
	bool				Overlaps(SIR65Place* psPlace1, SIR65Place* psPlace2);
	bool				IsDeferred(SIRReference sValue);
	bool				IsInA(SIRReference sValue);
	bool				InRegister(SIRReference sValue);
	bool				NarrowOnly(SIRReference sValue);
	void				FindNarrowOnly(void);
	const char*			RegisterName(SIR65Place* psPlace);
	bool				IsFrameOperand(CChars* pszOperand);
	bool				IsLeaf(void) const;
	bool				ArgumentsReadFirst(void);
	bool				IsArrivalPlace(SIRReference sValue);
};


#endif // __I_R65_PLACES_H__
