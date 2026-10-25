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
#ifndef __INVERTER_PACKAGE_VIEW_H__
#define __INVERTER_PACKAGE_VIEW_H__
#include "BufferView.h"


#define INVERTER_PACKAGE_FONT_SIZE		8.0
#define INVERTER_UNIT_FONT_SIZE			6.0
#define INVERTER_PACKAGE_MAX_UNITS		26


//A 74 series part of inverters (the 74LVC04 is six, the 74LVC14 six Schmitt triggered) drawn as one block, vaguely
//like the W65C816's: its inverters one above the other two grid units apart inside a box, each with its input out
//to the left of the box and its output out to the right.  Its description ("Hex Inverter") is at the top inside the
//box and its part number (74, its family and its type: "74LVC04") at the bottom.  It's one inverter IC of its family
//with an input and output for each inverter, "Input n" and "Output n", as an inverter view of that many.
class CInverterPackageView : public CIntegratedCircuitView
{
CONSTRUCTABLE(CInverterPackageView);
protected:
	CChars		mszPartType;
	CChars		mszDescription;
	int			miUnitCount;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType, int iUnitCount, const char* szDescription = NULL);
	virtual void					Kill(void);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual void					GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);

			char*					GetPartType(void);
			char*					GetDescription(void);
			int						GetUnitCount(void);
			void					GetPartNumber(CChars* psz);
			CPortView*				GetUnitInput(int iUnit);
			CPortView*				GetUnitOutput(int iUnit);

	static	void					GetPartNumber(CChars* psz, const char* szFamily, const char* szPartType);
	static	void					GetDescription(CChars* psz, const char* szPartType, int iUnitCount);
	static	int32					GetUnitY(int iUnit, int iUnitCount);
	static	int						GetPartUnitCount(const char* szPartType);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
			void					CreateShapes(void);
			void					CreatePackagePortViews(void);
};


//One inverter of a 74 series part of inverters, placed on its own: drawn as an inverter view of one, with its unit
//("Unit A" for the first) above it and its family below.  It knows its part's type, so the six units of a 74LVC04
//placed apart are still its units; each is an inverter of one.  Placing a unit in the editor goes on to the next
//(NewNextUnit) until the part's last.  Units with the same name and part type are one part's (IsUnitOfSamePart), and
//are deleted together; units without a name aren't any part's.
class CInverterUnitView : public CInverterView
{
CONSTRUCTABLE(CInverterUnitView);
protected:
	CChars		mszPartType;
	int			miUnit;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType, int iUnit);
	virtual void					Kill(void);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual void					GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);

			char*					GetPartType(void);
			int						GetUnit(void);
			char					GetUnitLetter(void);
			int						GetUnitCount(void);
	virtual CComponentView*			NewNextUnit(void);
	virtual bool					IsUnitOfSamePart(CComponentView* pcOther);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
};


#endif // __INVERTER_PACKAGE_VIEW_H__

