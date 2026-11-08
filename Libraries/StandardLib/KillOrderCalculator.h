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
#ifndef __KILL_ORDER_CALCULATOR_H__
#define __KILL_ORDER_CALCULATOR_H__
#include "BaseLib/ArrayTemplate.h"
#include "ObjectPointerArrays.h"


//A Nes pointed to object being ordered, as it is looked up by its address.
struct SKillOrderObject
{
	CBaseObject*	pcObject;
	size			uiFound;  //Its place in the order the objects were found.
};


//A waiting object (uiHolder, by where it was found) that holds a Nes pointer to another (uiHeld).
struct SKillOrderNeed
{
	size	uiHolder;
	size	uiHeld;
};


class CKillOrderCalculator
{
protected:
	CArrayBaseObjectPtr*	mpapcKilled;
	CArrayBaseObjectPtr		mapcKilledNesObjects;

public:
	void					Init(CArrayBaseObjectPtr* papcKilled);
	void					Kill(void);

	CArrayBaseObjectPtr*	Calculate(void);

protected:
	void					OrderNesObjects(void);
	void					FindNeeds(CArrayTemplate<SKillOrderNeed>* pasNeeds, CArrayTemplate<size>* pauiNumHolders);
};


#endif // __KILL_ORDER_CALCULATOR_H__

