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
#ifndef __I_R_OPTIMISER_MEMORY_H__
#define __I_R_OPTIMISER_MEMORY_H__
#include "IRPasses.h"


//The passes over memory: slots split and their types unified, and loads of what is known to be there removed;
//and what may alias what, for the loop passes.
class CIRMemoryPasses : public CIRPasses
{
public:
	bool			SplitSlots(void);
	bool			UnifySlotTypes(void);
	bool			RemoveRedundantLoads(void);
	void			SplitAddress(SIRReference sAddress, SIRReference* psRoot, int64* plliOffset);
	bool			MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes);
	void			AddAvailable(CArrayIRAvailables* pasAvailable, SIRReference sRoot, int64 lliOffset, SIRType sType, SIRReference sValue);

protected:
	SIRType			SlotOffsetType(SIRReference sSlot, int64 lliOffset);
	SIRReference	ConstantData(SIRReference sRoot, int64 lliOffset, SIRType sType);
	void			MergeAvailable(CIRBlock* pcBlock, CArrayIRAvailables* pasAvailable, CArrayIRAvailables* pasEnds, CArrayTemplate<SIRBlockEnd>* pasBlockEnds);
	bool			IsFrameRoot(SIRReference sRoot);
	void			PointerBase(SIRReference sRoot, SIRPointerBase* psBase);
	bool			SlotOffset(SIRReference sAddress, SIRReference* psSlot, int64* plliOffset);
};


#endif // __I_R_OPTIMISER_MEMORY_H__

