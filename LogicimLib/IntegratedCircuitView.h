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
#ifndef __INTEGRATED_CIRCUIT_VIEW_H__
#define __INTEGRATED_CIRCUIT_VIEW_H__
#include "MicroprocessorLib/FamilyVoltageConfiguration.h"
#include "ComponentView.h"


class CIntegratedCircuit;


#define PORT_LABEL_FONT_SIZE	10.0


//A port view for CreateLabelledPortViews, as Logicim's PortViewCreator.  szName is the port's name (with " n" after it
//for each of iPortCount ports) and szText its label.
struct SPortViewCreator
{
	const char*	szText;
	const char*	szName;
	int			iPortCount;
	bool		bClock;
	bool		bInverted;
};


//A view of an integrated circuit of a logic family, as Logicim's IntegratedCircuitView and
//StandardIntegratedCircuitView.  Unless its power ports are explicit the IC is powered by its own supply at the
//family's default (TTL) voltage when it's made.  Explicit power ports are drawn as a VCC port on the left of the
//bounding box and a GND port on the right, each with a grid unit of line to the shapes.
class CIntegratedCircuitView : public CComponentView
{
CONSTRUCTABLE(CIntegratedCircuitView);
protected:
	CChars		mszFamily;
	bool		mbExplicitPowerPorts;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts);
	virtual void					Kill(void);

	virtual CCircuitComponent*		CreateComponent(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
	virtual CComponentStateXML*		GetComponentStateXML(void);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily) =0;

			char*					GetFamily(void);
			bool					IsExplicitPowerPorts(void);
			CIntegratedCircuit*		GetIntegratedCircuit(void);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
	static	bool					LoadXMLFamily(CMarkupTag* pcTag, char** pszFamily, bool* pbExplicitPowerPorts);
			void					CreatePowerPortViews(void);
			void					CreateLabelledPortViews(SPortViewCreator* asLeft, size uiNumLeft, SPortViewCreator* asRight, size uiNumRight, int32 iLeftOffset, int32 iRightOffset, int32 iLeftStart, int32 iRightStart, int32 iStep);
			CPortView*				CreateLabelledPortView(SPortViewCreator* psCreator, int32 x, int32 y, ERotation eInvertFacing);
			size					AddDetail(float64 x, int32 iWidth, const char* szName, const char* szValue);
			bool					PowerImplicitly(CIntegratedCircuit* pcIntegratedCircuit, CFamilyVoltageConfiguration* pcFamily);
};


class CPort;
bool PowerPortsImplicitly(CSubcircuitSimulation* pcSimulation, CPort* pcCommon, CPort* pcGround, CFamilyVoltageConfiguration* pcFamily);


#endif // __INTEGRATED_CIRCUIT_VIEW_H__
