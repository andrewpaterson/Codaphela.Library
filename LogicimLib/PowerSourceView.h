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

	virtual CCircuitComponent*		CreateComponent(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
			float32					GetVoltage(void);
};


//A 0V supply drawn as Logicim's ground symbol; the port is at the top of its stem.
class CGroundView : public CPowerSourceView
{
CONSTRUCTABLE(CGroundView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
};


#endif // __POWER_SOURCE_VIEW_H__
