/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ConstructorCall.h"
#include "RCFilterPins.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRCFilterPins::Init(void)
{
	PreInit();
	CPins::Init();
	mpcIn = NewMalloc<CAnalogInPort>();
	mpcIn->Init(this, RC_FILTER_PIN_IN);
	mpcOut = NewMalloc<CLogicPort>();
	mpcOut->Init(PT_Output, this, RC_FILTER_PIN_OUT, NULL);
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRCFilterPins::Class(void)
{
	CPins::Class();
	U_Pointer(mpcIn);
	U_Pointer(mpcOut);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnalogInPort* CRCFilterPins::GetIn(void) const { return mpcIn; }
CLogicPort* CRCFilterPins::GetOut(void) const { return mpcOut; }

