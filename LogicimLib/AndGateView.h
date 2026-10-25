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
#ifndef __AND_GATE_VIEW_H__
#define __AND_GATE_VIEW_H__
#include "LogicGateView.h"


//A flat backed AND gate with a half ellipse front, as Logicim's BaseAndGateView.
class CAndGateView : public CLogicGateView
{
CONSTRUCTABLE(CAndGateView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

protected:
			void					InitAnd(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth, bool bNegated);
			void					CreateShapes(void);
};


//An AND gate with a bubble on its output.
class CNandGateView : public CAndGateView
{
CONSTRUCTABLE(CNandGateView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iInputCount, int iInputWidth);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
};


#endif // __AND_GATE_VIEW_H__
