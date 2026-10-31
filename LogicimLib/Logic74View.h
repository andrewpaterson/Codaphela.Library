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
#ifndef __LOGIC_74_VIEW_H__
#define __LOGIC_74_VIEW_H__
#include "Logic74Parts.h"
#include "LogicGateView.h"


#define LOGIC_74_FONT_SIZE			8.0
#define LOGIC_74_UNIT_FONT_SIZE		6.0
#define LOGIC_74_MAX_UNITS			26


//A whole real 74 series part (see SLogic74Part) drawn as one block, its part type saved with it.  A part of gates is a
//box with its gates drawn inside one above the other, each gate's inputs out to the left of the box and its output out
//to the right, named as the part's pins are ("1A", "1B" and "1Y").  Its description ("Quad 2-Input NAND") is at the
//top inside the box and its part number ("74LVC00") at the bottom.  Any other part is drawn as the block of its
//generic chip is (see CLogicBlockView), its pins labelled as the part's are ("D0", "CP", "OE"), with its description
//and part number.  It's simulated as one generic chip: a part of gates as a gate array, any other as its block's chip.
//A part type there's no row for draws an empty box and can't be simulated.
class CLogic74PackageView : public CLogicBlockView
{
CONSTRUCTABLE(CLogic74PackageView);
protected:
	CChars				mszPartType;
	SLogic74PartType	msType;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType);
	virtual void					Kill(void);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual void					GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

			char*					GetPartType(void);
			SLogic74Part*			GetPart(void);
			int						GetUnitCount(void);
			void					GetPartNumber(CChars* psz);
			void					GetDescription(CChars* psz);

	static	int32					GetGatePitch(int iInputCount);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
			void					CreateGates(void);
			void					CreatePartBlock(void);
			void					AddGateShapes(EGateFunction eFunction, float64 y, int iInputCount, int32 iInputX, int32 iOutputX);
			void					CreateTitles(float64 fTop, float64 fBottom);
};


//One unit of a real 74 series part, placed on its own as KiCad places a unit of a part drawn as units: a gate (or a
//74LVC74's flip flop) drawn as the generic one is, with its unit ("Unit A") above it and its family below.  It knows its part's type, so the
//four units of a 74LVC00 placed apart are still its units; each is simulated as a gate of its own.  Placing a unit in
//the editor goes on to the next (NewNextUnit) until the part's last.
class CLogic74UnitView : public CLogicGateView
{
CONSTRUCTABLE(CLogic74UnitView);
protected:
	CChars				mszPartType;
	SLogic74PartType	msType;
	int					miUnit;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType, int iUnit);
	virtual void					Kill(void);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual void					GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);
	virtual CComponentView*			NewNextUnit(void);

			char*					GetPartType(void);
			SLogic74Part*			GetPart(void);
			int						GetUnit(void);
			char					GetUnitLetter(void);
			int						GetUnitCount(void);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
	virtual	bool					IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds);
			void					CreateGate(void);
			void					CreateFlipFlop(void);
};


#endif // __LOGIC_74_VIEW_H__

