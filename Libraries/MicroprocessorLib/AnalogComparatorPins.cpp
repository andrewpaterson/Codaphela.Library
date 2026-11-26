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
#include "LongTime.h"
#include "AnalogComparatorPins.h"


//The MAX9010's output, made when first asked for.  It holds no memory, so it is never killed.
CFixedVoltageConfiguration	gcAnalogComparatorVoltageConfiguration;
bool						gbAnalogComparatorVoltageConfiguration = false;


//////////////////////////////////////////////////////////////////////////
//
// A MAX9010's output at 5V (its supply is 4.5V to 5.5V): TTL, low out at
// 0.3V, high out at 3.3V, 5ns to switch; its inputs aren't read by level.
//
//////////////////////////////////////////////////////////////////////////
CVoltageConfigurationSource* GetAnalogComparatorVoltageConfiguration(void)
{
	if (!gbAnalogComparatorVoltageConfiguration)
	{
		gcAnalogComparatorVoltageConfiguration.Init(5.0f, 0.8f, 2.0f, 0.3f, 3.3f, NanosecondsToTime((int64)5), NanosecondsToTime((int64)5));
		gbAnalogComparatorVoltageConfiguration = true;
	}
	return &gcAnalogComparatorVoltageConfiguration;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparatorPins::Init(void)
{
	PreInit();
	CPins::Init();
	mpcPlus = NewMalloc<CAnalogComparatorInPort>();
	mpcPlus->Init(this, ANALOG_COMPARATOR_PIN_PLUS);
	mpcMinus = NewMalloc<CAnalogComparatorInPort>();
	mpcMinus->Init(this, ANALOG_COMPARATOR_PIN_MINUS);
	mpcPlus->SetOther(mpcMinus);
	mpcMinus->SetOther(mpcPlus);
	mpcOut = NewMalloc<CLogicPort>();
	mpcOut->Init(PT_Output, this, ANALOG_COMPARATOR_PIN_OUT, GetAnalogComparatorVoltageConfiguration());
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CAnalogComparatorPins::Class(void)
{
	CPins::Class();
	U_Pointer(mpcPlus);
	U_Pointer(mpcMinus);
	U_Pointer(mpcOut);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CAnalogComparatorInPort* CAnalogComparatorPins::GetPlus(void) const { return mpcPlus; }
CAnalogComparatorInPort* CAnalogComparatorPins::GetMinus(void) const { return mpcMinus; }
CLogicPort* CAnalogComparatorPins::GetOut(void) const { return mpcOut; }

