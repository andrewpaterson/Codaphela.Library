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
#ifndef __BUFFER_VIEW_H__
#define __BUFFER_VIEW_H__
#include "LogicGateView.h"


//A row of buffers side by side, each a triangle pointing North from its input at grid y 1 to its output at y -1, as
//Logicim's BaseBufferView.  The input count is the number of buffers and the input width the bits each one carries.
class CBufferView : public CLogicGateView
{
CONSTRUCTABLE(CBufferView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iBufferCount, int iBufferWidth);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

protected:
			void					InitBuffer(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iBufferCount, int iBufferWidth, bool bNegated);
			void					CreateShapes(void);
			void					CreateBufferPortViews(void);
	virtual	bool					IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds);
};


//Buffers with bubbles on their outputs.
class CInverterView : public CBufferView
{
CONSTRUCTABLE(CInverterView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int iBufferCount, int iBufferWidth);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
};


#endif // __BUFFER_VIEW_H__
