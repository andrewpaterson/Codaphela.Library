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
#ifndef __TUNNEL_VIEW_H__
#define __TUNNEL_VIEW_H__
#include "ComponentView.h"
#include "CircuitFonts.h"


#define TUNNEL_FONT_SIZE	11.0


//A named tag that joins its wire to the wires of every other tunnel of the same name, as Logicim's TunnelView.  Names
//are matched with their case but without the spaces around them (Logicim ignored case).  Its point is at its position and its name runs away from it
//(to the right, facing East).  A double sided tunnel has a second point at its far end, a whole number of grid units away,
//that's joined to the first.  How long it is depends on its name as it measured when the tunnel was made; its box is
//saved and loaded with it.  A tunnel makes no component.
class CTunnelView : public CComponentView
{
CONSTRUCTABLE(CTunnelView);
protected:
	bool			mbDoubleSided;
	CChars			mszConnectionName;
	SFloat64Vec2	msBoxMinimum;
	SFloat64Vec2	msBoxMaximum;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, bool bDoubleSided, CCircuitFonts* pcFonts);
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, bool bDoubleSided, SFloat64Vec2 sBoxMinimum, SFloat64Vec2 sBoxMaximum);
	virtual void					Kill(void);

	virtual const char*				GetType(void);
	virtual bool					MakesComponent(void);
	virtual CCircuitComponent*		CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
	virtual const char*				GetConnectionName(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);

			bool					IsDoubleSided(void);
			void					GetBox(SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum);

	static	void					MeasureBox(CCircuitFonts* pcFonts, const char* szName, bool bDoubleSided, SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
};


#endif // __TUNNEL_VIEW_H__
