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
#include <math.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "MicroprocessorLib/IntegratedCircuit.h"
#include "MicroprocessorLib/FamilyVoltageConfigurationStore.h"
#include "MicroprocessorLib/PowerSource.h"
#include "MicroprocessorLib/PowerInPort.h"
#include "MicroprocessorLib/PowerOutPort.h"
#include "MicroprocessorLib/PowerPinNames.h"
#include "MicroprocessorLib/SubcircuitSimulation.h"
#include "MicroprocessorLib/Circuit.h"
#include "ComponentStateXML.h"
#include "IntegratedCircuitView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts)
{
	CComponentView::Init(x, y, eRotation, szName);
	mszFamily.Init(szFamily);
	mbExplicitPowerPorts = bExplicitPowerPorts;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::Kill(void)
{
	mszFamily.Kill();
	CComponentView::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CComponentView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("family", mszFamily.Text());
	pcWriter->Attribute("explicitPowerPorts", mbExplicitPowerPorts);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentStateXML* CIntegratedCircuitView::GetComponentStateXML(void)
{
	return &gcIntegratedCircuitStateXML;
}


//////////////////////////////////////////////////////////////////////////
//
// Power ports are implicit unless the tag says otherwise.
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuitView::LoadXMLFamily(CMarkupTag* pcTag, char** pszFamily, bool* pbExplicitPowerPorts)
{
	*pszFamily = pcTag->GetStringAttribute("family");
	if ((*pszFamily == NULL) || ((*pszFamily)[0] == '\0'))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a family attribute.", NULL);
	}
	if (pcTag->GetStringAttribute("explicitPowerPorts") == NULL)
	{
		*pbExplicitPowerPorts = false;
	}
	else if (!pcTag->GetBoolAttribute("explicitPowerPorts", pbExplicitPowerPorts))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] explicitPowerPorts must be true or false.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the IC in the simulation and connects the port views to it.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CIntegratedCircuitView::CreateComponent(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CFamilyVoltageConfiguration*	pcFamily;
	CIntegratedCircuit*				pcIntegratedCircuit;

	if (mpcComponent != NULL)
	{
		gcLogger.Error2(__METHOD__, " [", GetType(), "] [", mszName.Text(), "] already has a component.", NULL);
		return NULL;
	}

	pcFamily = pcFamilies->Get(mszFamily.Text());
	if (pcFamily == NULL)
	{
		gcLogger.Error2(__METHOD__, " Cannot find family [", mszFamily.Text(), "].", NULL);
		return NULL;
	}

	pcIntegratedCircuit = CreateIntegratedCircuit(pcSimulation, pcFamily);
	if (pcIntegratedCircuit == NULL)
	{
		return NULL;
	}

	if (!mbExplicitPowerPorts)
	{
		if (!PowerImplicitly(pcIntegratedCircuit, pcFamily))
		{
			return NULL;
		}
	}

	if (!ConnectPortViews(pcIntegratedCircuit))
	{
		return NULL;
	}
	return pcIntegratedCircuit;
}


//////////////////////////////////////////////////////////////////////////
//
// A supply at the family's default voltage and a ground, on traces of
// their own.
//
//////////////////////////////////////////////////////////////////////////
bool CIntegratedCircuitView::PowerImplicitly(CIntegratedCircuit* pcIntegratedCircuit, CFamilyVoltageConfiguration* pcFamily)
{
	CVoltageConfiguration*	pcVoltage;
	CSubcircuitSimulation*	pcSimulation;
	CCircuit*				pcCircuit;
	CTrace*					pcVCC;
	CTrace*					pcGND;
	CPowerSource*			pcPower;

	pcVoltage = pcFamily->GetDefaultVoltageConfiguration(DEFAULT_LOGIC_LEVEL);
	if (pcVoltage == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Family [", mszFamily.Text(), "] has no default voltage; its power ports must be explicit.", NULL);
	}

	pcSimulation = pcIntegratedCircuit->GetContainingSubcircuitSimulation();
	pcCircuit = pcSimulation->GetCircuit();

	pcVCC = pcCircuit->AddTrace();
	pcIntegratedCircuit->GetPins()->GetVoltageCommon()->Connect(pcVCC, false);
	pcPower = NewMalloc<CPowerSource>();
	pcPower->Init(pcSimulation, "", pcVoltage->GetVcc());
	pcPower->GetPowerOutPort()->Connect(pcVCC, false);

	pcGND = pcCircuit->AddTrace();
	pcIntegratedCircuit->GetPins()->GetVoltageGround()->Connect(pcGND, false);
	pcPower = NewMalloc<CPowerSource>();
	pcPower->Init(pcSimulation, "", 0.0f);
	pcPower->GetPowerOutPort()->Connect(pcGND, false);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Call after the shapes are added.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::CreatePowerPortViews(void)
{
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	int32			iLeft;
	int32			iRight;

	if (!mbExplicitPowerPorts)
	{
		return;
	}

	if (!mcShapes.IncludeBounds(&sMinimum, &sMaximum, false))
	{
		sMinimum.Init(0, 0);
		sMaximum.Init(0, 0);
	}
	iLeft = (int32)floor(sMinimum.x);
	iRight = (int32)ceil(sMaximum.x);

	AddPortView(POWER_PIN_VCC, iLeft, 0);
	AddPortView(POWER_PIN_GND, iRight, 0);
	mcShapes.AddLine(iLeft, 0, iLeft + 1, 0);
	mcShapes.AddLine(iRight, 0, iRight - 1, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIntegratedCircuitView::GetFamily(void) { return mszFamily.Text(); }
bool CIntegratedCircuitView::IsExplicitPowerPorts(void) { return mbExplicitPowerPorts; }
CIntegratedCircuit* CIntegratedCircuitView::GetIntegratedCircuit(void) { return (CIntegratedCircuit*)mpcComponent; }

