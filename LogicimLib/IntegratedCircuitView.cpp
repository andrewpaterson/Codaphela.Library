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
// Makes the IC in the path's subcircuit simulation and connects the port
// views to it.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CIntegratedCircuitView::CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	CFamilyVoltageConfiguration*	pcFamily;
	CIntegratedCircuit*				pcIntegratedCircuit;
	CSubcircuitSimulation*			pcSimulation;
	CArrayCircuitComponentPtr		apcSupplies;
	bool							bResult;

	pcSimulation = CanCreateComponent(pcViewPath, pcCircuitSimulation);
	if (pcSimulation == NULL)
	{
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

	apcSupplies.Init();
	if (!mbExplicitPowerPorts)
	{
		if (!PowerImplicitly(pcIntegratedCircuit, pcFamily, &apcSupplies))
		{
			apcSupplies.Kill();
			return NULL;
		}
	}

	bResult = PostCreateComponent(pcViewPath, pcCircuitSimulation, pcIntegratedCircuit, &apcSupplies);
	apcSupplies.Kill();
	if (!bResult)
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
bool CIntegratedCircuitView::PowerImplicitly(CIntegratedCircuit* pcIntegratedCircuit, CFamilyVoltageConfiguration* pcFamily, CArrayCircuitComponentPtr* papcSupplies)
{
	return PowerPortsImplicitly(pcIntegratedCircuit->GetContainingSubcircuitSimulation(), pcIntegratedCircuit->GetPins()->GetVoltageCommon(), pcIntegratedCircuit->GetPins()->GetVoltageGround(), pcFamily, papcSupplies);
}


//////////////////////////////////////////////////////////////////////////
//
// A supply at the family's default voltage on pcCommon and a ground on
// pcGround, on traces of their own.  The supplies are added to
// papcSupplies.
//
//////////////////////////////////////////////////////////////////////////
bool PowerPortsImplicitly(CSubcircuitSimulation* pcSimulation, CPort* pcCommon, CPort* pcGround, CFamilyVoltageConfiguration* pcFamily, CArrayCircuitComponentPtr* papcSupplies)
{
	CVoltageConfiguration*	pcVoltage;
	CCircuit*				pcCircuit;
	CTrace*					pcVCC;
	CTrace*					pcGND;
	CPowerSource*			pcPower;

	pcVoltage = pcFamily->GetDefaultVoltageConfiguration(DEFAULT_LOGIC_LEVEL);
	if (pcVoltage == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Family [", pcFamily->GetFamily(), "] has no default voltage; its power ports must be explicit.", NULL);
	}

	pcCircuit = pcSimulation->GetCircuit();

	pcVCC = pcCircuit->AddTrace();
	pcCommon->Connect(pcVCC, false);
	pcPower = NewMalloc<CPowerSource>();
	pcPower->Init(pcSimulation, "", pcVoltage->GetVcc());
	pcPower->GetPowerOutPort()->Connect(pcVCC, false);
	papcSupplies->Add(pcPower);

	pcGND = pcCircuit->AddTrace();
	pcGround->Connect(pcGND, false);
	pcPower = NewMalloc<CPowerSource>();
	pcPower->Init(pcSimulation, "", 0.0f);
	pcPower->GetPowerOutPort()->Connect(pcGND, false);
	papcSupplies->Add(pcPower);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// VCC on the top of the shapes and GND on the bottom, in the middle.  Call
// after the shapes are added.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::CreatePowerPortViews(void)
{
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	int32			iTop;
	int32			iBottom;

	if (!mbExplicitPowerPorts)
	{
		return;
	}

	if (!mcShapes.IncludeBounds(&sMinimum, &sMaximum, false))
	{
		sMinimum.Init(0, 0);
		sMaximum.Init(0, 0);
	}
	iTop = (int32)floor(sMinimum.y);
	iBottom = (int32)ceil(sMaximum.y);

	AddPortView(POWER_PIN_VCC, 0, iTop);
	AddPortView(POWER_PIN_GND, 0, iBottom);
	mcShapes.AddLine(0, iTop, 0, iTop + 1);
	mcShapes.AddLine(0, iBottom, 0, iBottom - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// Ports down the left and right of the component with a label each, as
// Logicim's createLeftAndRightPortViews and createPortLabels.  The left
// ports are at x iLeftX from y iLeftStart down, iStep apart, facing West,
// and the right ports at x iRightX from y iRightStart, facing East.
// Labels are 0.6 inside the ports, reading inwards.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::CreateLabelledPortViews(SPortViewCreator* asLeft, size uiNumLeft, SPortViewCreator* asRight, size uiNumRight, int32 iLeftX, int32 iRightX, int32 iLeftStart, int32 iRightStart, int32 iStep)
{
	size		i;
	int32		y;
	CPortView*	pcPortView;
	float64		fLeftLabel;
	float64		fRightLabel;

	fLeftLabel = (iLeftX < iRightX) ? iLeftX + 0.6 : iLeftX - 0.6;
	fRightLabel = (iLeftX < iRightX) ? iRightX - 0.6 : iRightX + 0.6;

	y = iLeftStart;
	for (i = 0; i < uiNumLeft; i++)
	{
		pcPortView = CreateLabelledPortView(&asLeft[i], iLeftX, y, ROT_West);
		mcShapes.AddText(fLeftLabel, y, pcPortView->GetText(), CF_SansSerif, PORT_LABEL_FONT_SIZE, TA_Left);
		y += iStep;
	}

	y = iRightStart;
	for (i = 0; i < uiNumRight; i++)
	{
		pcPortView = CreateLabelledPortView(&asRight[i], iRightX, y, ROT_East);
		mcShapes.AddText(fRightLabel, y, pcPortView->GetText(), CF_SansSerif, PORT_LABEL_FONT_SIZE, TA_Right);
		y += iStep;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// eInvertFacing is the way the port faces out of the component: an
// inverted port's bubble and a clock's notch go that way.
//
//////////////////////////////////////////////////////////////////////////
CPortView* CIntegratedCircuitView::CreateLabelledPortView(SPortViewCreator* psCreator, int32 x, int32 y, ERotation eInvertFacing)
{
	CPortView*		pcPortView;
	CArrayChars		aszNames;
	CChars			szName;
	int				i;

	if (psCreator->iPortCount == 1)
	{
		pcPortView = AddPortView(psCreator->szName, x, y);
	}
	else
	{
		aszNames.Init();
		for (i = 0; i < psCreator->iPortCount; i++)
		{
			szName.Init(psCreator->szName);
			szName.Append(' ');
			szName.Append(i);
			aszNames.Add(szName.Text());
			szName.Kill();
		}
		pcPortView = AddPortView(&aszNames, x, y);
		aszNames.Kill();
	}
	pcPortView->SetText(psCreator->szText);
	if (psCreator->bClock)
	{
		pcPortView->SetClock(eInvertFacing);
	}
	if (psCreator->bInverted)
	{
		pcPortView->SetInverting(eInvertFacing);
	}
	return pcPortView;
}


//////////////////////////////////////////////////////////////////////////
//
// A named value in a small box on the line at y, as Logicim's DetailView:
// the name left of the middle and the value in a box running iWidth grid
// units right from it.  Returns the value text's shape.
//
//////////////////////////////////////////////////////////////////////////
size CIntegratedCircuitView::AddDetail(float64 y, int32 iWidth, const char* szName, const char* szValue)
{
	CChars				szLabel;
	SComponentShape*	psShape;
	size				uiValueText;

	szLabel.Init(szName);
	szLabel.Append(": ");
	mcShapes.AddText(0, y, szLabel.Text(), CF_SansSerif, PORT_LABEL_FONT_SIZE, TA_Right);
	szLabel.Kill();

	psShape = mcShapes.AddRectangle(0, y - 0.6, iWidth, y + 0.6, true, true);
	psShape->fLineWidth = 1;
	psShape->eFill = SC_Background;

	uiValueText = mcShapes.NumShapes();
	psShape = mcShapes.AddText(0.6, y, szValue, CF_Monospaced, PORT_LABEL_FONT_SIZE, TA_Left);
	psShape->eBorder = SC_CommentText;
	return uiValueText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIntegratedCircuitView::GetFamily(void) { return mszFamily.Text(); }
bool CIntegratedCircuitView::IsExplicitPowerPorts(void) { return mbExplicitPowerPorts; }
CIntegratedCircuit* CIntegratedCircuitView::GetIntegratedCircuit(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation) { return (CIntegratedCircuit*)GetComponent(pcViewPath, pcCircuitSimulation); }


//////////////////////////////////////////////////////////////////////////
//
// Its family, a choice of the families there are, as Logicim's
// FamilyPropertyEditor; the family it has is a choice even if it isn't
// there.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::AddFamilyProperty(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	CArrayChars		aszFamilies;
	size			i;

	aszFamilies.Init();
	for (i = 0; i < psContext->pcFamilies->NumFamilies(); i++)
	{
		aszFamilies.Add(psContext->pcFamilies->GetFamily(i)->GetFamily());
	}
	if (!aszFamilies.Contains(mszFamily.Text()))
	{
		aszFamilies.Add(mszFamily.Text());
	}
	pcProperties->AddChoice("family", &aszFamilies, mszFamily.Text());
	aszFamilies.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Its family and whether its power ports are explicit, as Logicim's
// StandardIntegratedCircuitProperties.
//
//////////////////////////////////////////////////////////////////////////
void CIntegratedCircuitView::AddPowerProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	AddFamilyProperty(pcProperties, psContext);
	pcProperties->AddBoolean("explicitPowerPorts", mbExplicitPowerPorts);
}
