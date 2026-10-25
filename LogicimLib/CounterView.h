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
#ifndef __COUNTER_VIEW_H__
#define __COUNTER_VIEW_H__
#include "IntegratedCircuitView.h"


//An up counter of iBitWidth bits, as Logicim's CounterView: a 12 by 18 box titled "Up Counter" with the clock,
//reset, load, enables and data in down its left (grid x -6) and the count and terminal count out down its right,
//each labelled inside the box.  Reset and load are inverted.  A detail box shows the count in hexadecimal.
class CCounterView : public CIntegratedCircuitView
{
CONSTRUCTABLE(CCounterView);
protected:
	int32		miBitWidth;
	uint32		muiTerminalValue;
	size		muiValueText;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, uint32 uiTerminalValue);

	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual void					GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

			int32					GetBitWidth(void);
			uint32					GetTerminalValue(void);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
};


#endif // __COUNTER_VIEW_H__
