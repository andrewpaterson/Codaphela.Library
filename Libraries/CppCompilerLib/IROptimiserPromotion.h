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
#ifndef __I_R_OPTIMISER_PROMOTION_H__
#define __I_R_OPTIMISER_PROMOTION_H__
#include "IRPasses.h"


//Slots promoted to values (SSA construction).
class CIRPromotionPasses : public CIRPasses
{
protected:
	CArrayIRPromotions		masPromotions;
	CArrayTemplate<int32>	maiPromotion;		//Per temporary: the promotion its address is, -1 for none.
	CArrayTemplate<int32>	maiPhiPromotion;	//Per temporary: the promotion a phi made by it is for, -1 for none.
	CArrayIRReferences		masValues;			//The values being renamed, one stack per promotion...
	CArrayTemplate<int32>	maiValueLinks;		//...as a list through this (the previous value's index, -1).
	CArrayTemplate<int32>	maiTops;			//Per promotion: the top value's index, -1 for none.
	CArrayTemplate<int32>	maiPushedPromotions;	//The promotion of each value pushed, in order.

public:
	void			Init(CIRPassContext* pcContext) override;
	void			Kill(void) override;

	bool			PromoteSlots(void);

protected:
	int32			PromotionOf(SIRReference sAddress);
	void			FindPromotions(void);
	void			PlacePhis(void);
	void			PushValue(int32 iPromotion, SIRReference sValue);
	void			PopValue(void);
	SIRReference	CurrentValue(int32 iPromotion);
	void			Rename(void);
	void			RenameBlock(CIRBlock* pcBlock, size* puiPushed);
	void			FillSuccessorPhis(CIRBlock* pcBlock);
	void			RemovePromoted(void);
};


#endif // __I_R_OPTIMISER_PROMOTION_H__

