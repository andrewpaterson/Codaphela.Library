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
#ifndef __I_R_OPTIMISER_H__
#define __I_R_OPTIMISER_H__
#include "IRPasses.h"
#include "IRPipeline.h"
#include "IROptimiserAddresses.h"
#include "IROptimiserBlocks.h"
#include "IROptimiserCounters.h"
#include "IROptimiserDivisions.h"
#include "IROptimiserFolding.h"
#include "IROptimiserIdioms.h"
#include "IROptimiserLoops.h"
#include "IROptimiserMemory.h"
#include "IROptimiserPromotion.h"
#include "IROptimiserRanges.h"


//The mid level passes, run on a function until nothing changes.  IRPipeline.h's IR_PASSES has a row for each, which
//says what it does in a line; the comment above each pass's function says the rest.
//
//The passes are in groups, a class each (CIRBlockPasses, CIRLoopPasses ...: the IROptimiser*.h files), which share
//what is in a CIRPassContext.  The optimiser owns them, has each pass as a CIRPass (a name and a group's function),
//and runs them as its pipeline says (SetPipeline: the target's; the standard one, every pass, unless one is set).
class CIROptimiser
{
protected:
	CIRPassContext			mcContext;
	CIRAddressPasses		mcAddressPasses;
	CIRBlockPasses			mcBlockPasses;
	CIRCounterPasses		mcCounterPasses;
	CIRDivisionPasses		mcDivisionPasses;
	CIRFoldingPasses		mcFoldingPasses;
	CIRIdiomPasses			mcIdiomPasses;
	CIRLoopPasses			mcLoopPasses;
	CIRMemoryPasses			mcMemoryPasses;
	CIRPromotionPasses		mcPromotionPasses;
	CIRRangePasses			mcRangePasses;
	CIRPass					macPasses[IRP_NumPasses];
	CIRPipeline				mcOwnPipeline;		//The standard pipeline (GetPipeline, to change it)...
	CIRPipeline*			mpcPipeline;		//...or the one set.

public:
	void			Init(CIRModule* pcModule);
	void			Kill(void);
	void			SetLibraryInt(SIRType sInt);
	void			SetLibrarySize(SIRType sSize);
	void			SetIndexedAddresses(bool bIndexedAddresses);
	void			SetWarnings(CChars* pszWarnings);
	void			SetDivideRemainders(uint32 uiDivideRemainders);
	void			SetDivisionCosts(CIRDivisionCosts* pcDivisionCosts);
	void			SetProfile(CIRInlineProfile* pcProfile);
	void			SetPipeline(CIRPipeline* pcPipeline);
	CIRPipeline*	GetPipeline(void) const;
	bool			Optimise(CIRFunction* pcFunction);
	bool			WritesMemory(CIRFunction* pcFunction);

protected:
	void			Begin(CIRFunction* pcFunction);
	void			InitPasses(void);
	bool			RunPass(EIRPass ePass, int iRound);
};


#endif // __I_R_OPTIMISER_H__

