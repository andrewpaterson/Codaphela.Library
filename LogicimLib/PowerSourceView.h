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
#ifndef __POWER_SOURCE_VIEW_H__
#define __POWER_SOURCE_VIEW_H__
#include "ComponentView.h"


//A view of a fixed voltage supply with one "Power" port, as Logicim's PowerSourceView.
class CPowerSourceView : public CComponentView
{
CONSTRUCTABLE(CPowerSourceView);
protected:
	float32		mfVoltage;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, float32 fVoltage);

	virtual CCircuitComponent*		CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
	virtual CComponentStateXML*		GetComponentStateXML(void);
			float32					GetVoltage(void);
};


//A 0V supply drawn as Logicim's ground symbol; the port is at the end of its stem.  It faces the way its stem points
//from the bars, so an upright ground, its port on top, faces North.
class CGroundView : public CPowerSourceView
{
CONSTRUCTABLE(CGroundView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
};


//A positive supply, as Logicim's PositivePowerView: a box with a point to its port and the voltage written upright in
//the box in the voltage's colour.  Over 4V the box is the dark fill.  It faces the way its point does, so an upright
//supply, its port below, faces South.
class CPositivePowerView : public CPowerSourceView
{
CONSTRUCTABLE(CPositivePowerView);
protected:
	size	muiVoltageText;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, float32 fVoltage);

	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
};


#endif // __POWER_SOURCE_VIEW_H__
