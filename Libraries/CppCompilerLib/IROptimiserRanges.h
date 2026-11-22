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
#ifndef __I_R_OPTIMISER_RANGES_H__
#define __I_R_OPTIMISER_RANGES_H__
#include "IRPasses.h"


//The passes that go by how big a value can be and whether a pointer is null (CIRRanges, which every group asks):
//narrowed truncations, multiplies and comparisons, and null tests folded.
class CIRRangePasses : public CIRPasses
{
public:
	bool			NarrowTruncations(void);
	bool			NarrowMultiplies(void);
	bool			FoldNullTests(void);
	bool			NarrowComparison(SIRInstruction* psInstruction);

protected:
	SIRInstruction*	ExtendedMultiple(SIRReference sValue, int64* plliMultiple, int iDepth);
	bool			BoundIndices(void);
	SIRReference	Unextended(SIRReference sValue, uint16 uiOpcode, SIRType* psType);
	void			ComparisonRange(SIRInstruction* psInstruction);
	SIRReference	FreeNarrow(SIRReference sValue, SIRType sType);
	SIRReference	Narrowed(SIRReference sValue, SIRType sType, CArrayIRInstructions* pasNew, int iDepth);
};


#endif // __I_R_OPTIMISER_RANGES_H__

