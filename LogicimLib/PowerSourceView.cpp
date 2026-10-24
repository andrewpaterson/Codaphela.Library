/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "MicroprocessorLib/PowerSource.h"
#include "MicroprocessorLib/PowerPinNames.h"
#include "PowerSourceView.h"


//////////////////////////////////////////////////////////////////////////
//
// Subclasses add their shapes, the port view and then CalculateBounds.
//
//////////////////////////////////////////////////////////////////////////
void CPowerSourceView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, float32 fVoltage)
{
	CComponentView::Init(x, y, eRotation, szName);
	mfVoltage = fVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
// A supply has no family.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CPowerSourceView::CreateComponent(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CPowerSource*	pcPowerSource;

	if (mpcComponent != NULL)
	{
		gcLogger.Error2(__METHOD__, " [", GetType(), "] [", mszName.Text(), "] already has a component.", NULL);
		return NULL;
	}

	pcPowerSource = NewMalloc<CPowerSource>();
	pcPowerSource->Init(pcSimulation, mszName.Text(), mfVoltage);
	if (!ConnectPortViews(pcPowerSource))
	{
		return NULL;
	}
	return pcPowerSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
float32 CPowerSourceView::GetVoltage(void)
{
	return mfVoltage;
}


//////////////////////////////////////////////////////////////////////////
//
// Three shrinking bars under a stem down from the port.
//
//////////////////////////////////////////////////////////////////////////
void CGroundView::Init(int32 x, int32 y, ERotation eRotation, const char* szName)
{
	float64		fOffset;

	CPowerSourceView::Init(x, y, eRotation, szName, 0.0f);

	fOffset = 0.5;
	mcShapes.AddLine(-0.9, fOffset, 0.9, fOffset);
	mcShapes.AddLine(-0.6, fOffset + 0.333, 0.6, fOffset + 0.333);
	mcShapes.AddLine(-0.3, fOffset + 0.666, 0.3, fOffset + 0.666);
	mcShapes.AddLine(0, 0, 0, fOffset);
	AddPortView(POWER_PIN_POWER, 0, 0);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CGroundView::GetType(void)
{
	return "Ground";
}

