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
#ifndef __VALUE_VIEW_H__
#define __VALUE_VIEW_H__
#include "IntegratedCircuitView.h"
#include "CircuitFonts.h"
#include "Radix.h"


#define VALUE_FONT_SIZE		10.0


class CPort;


//Where a subcircuit's pin is shown on an instance of it, as Logicim's SubcircuitPinAlignment and SubcircuitPinAnchour.
enum EPinAlignment
{
	PA_Cannot,
	PA_Left,
	PA_Right,
	PA_Top,
	PA_Bottom,
	PA_NotShown,
};


enum EPinAnchour
{
	PAN_Cannot,
	PAN_Negative,
	PAN_Centre,
	PAN_Positive,
};


struct SValueViewXML
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	int32		iBitWidth;
	ERadix		eRadix;
	SInt32Vec2	sBoxMinimum;
	SInt32Vec2	sBoxMaximum;
};


//A component that shows the value on its bus of iBitWidth ports, as Logicim's ConstantView and ProbeView.  Its port
//is at its position and its box runs from there down (grid y), two grid units wide and as long as its widest value
//in Logicim's monospaced 10 pixel font, measured when it's made (Logicim's createGraphics) and then kept and saved.
//The value is written in the middle of the box in its radix, turned with the box as Logicim turns it (the view is
//drawn as Logicim draws it turned twice to the right).
class CValueView : public CIntegratedCircuitView
{
CONSTRUCTABLE(CValueView);
protected:
	int32			miBitWidth;
	ERadix			meRadix;
	int				miMaxDigits;
	SInt32Vec2		msBoxMinimum;
	SInt32Vec2		msBoxMaximum;
	size			muiValueText;

public:
			void			Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum);

	virtual void			Paint(CCircuitPainter* pcPainter);

			int32			GetBitWidth(void);
			ERadix			GetRadix(void);
			void			GetBox(SInt32Vec2* psBoxMinimum, SInt32Vec2* psBoxMaximum);
			void			GetValueText(CChars* psz, int64 iTime);

	virtual	STraceValue		ReadPortValue(CPort* pcPort, int64 iTime);

	static	void			MeasureBox(CCircuitFonts* pcFonts, int32 iBitWidth, ERadix eRadix, SInt32Vec2* psBoxMinimum, SInt32Vec2* psBoxMaximum);
	static	bool			LoadValueXML(CMarkupTag* pcTag, SValueViewXML* psValue);

protected:
	virtual	void			SaveXMLAttributes(CXMLWriter* pcWriter);
			void			CreateValuePortViews(const char* szPortName);
			void			AddValueText(void);
			SFloat64Vec2	GetBoxCentre(void);
};


//A constant drives its value onto its bus; it's drawn as its box.
class CConstantView : public CValueView
{
CONSTRUCTABLE(CConstantView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, CCircuitFonts* pcFonts);
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);
};


//A probe reads the value on its bus; it's drawn as a circle when it has one digit and its box with round ends when it
//has more.
class CProbeView : public CValueView
{
CONSTRUCTABLE(CProbeView);
public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, CCircuitFonts* pcFonts);
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);
};


//A subcircuit's pin, as Logicim's PinView: drawn as a probe with its name in bold past the end of its box.  It's a
//passive component whose ports are a bus of trace ports, read by the voltages of their traces in its family's levels.
//Its alignment, anchour, weight, inverting and clock notch say how it's shown on an instance of its subcircuit;
//they're kept and saved for that.
class CPinView : public CValueView
{
CONSTRUCTABLE(CPinView);
protected:
	EPinAlignment					meAlignment;
	EPinAnchour						meAnchour;
	int32							miWeight;
	bool							mbInverting;
	bool							mbClockNotch;
	CFamilyVoltageConfiguration*	mpcFamily;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, CCircuitFonts* pcFonts);
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, int32 iBitWidth, ERadix eRadix, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum);
			void					SetSubcircuitAppearance(EPinAlignment eAlignment, EPinAnchour eAnchour, int32 iWeight, bool bInverting, bool bClockNotch);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CCircuitComponent*		CreateComponent(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
	virtual CIntegratedCircuit*		CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily);
	virtual CComponentStateXML*		GetComponentStateXML(void);
	virtual	STraceValue				ReadPortValue(CPort* pcPort, int64 iTime);

			EPinAlignment			GetAlignment(void);
			EPinAnchour				GetAnchour(void);
			int32					GetWeight(void);
			bool					IsInverting(void);
			bool					IsClockNotch(void);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
};


const char*		PinAlignmentToString(EPinAlignment eAlignment);
EPinAlignment	StringToPinAlignment(const char* szAlignment);
const char*		PinAnchourToString(EPinAnchour eAnchour);
EPinAnchour		StringToPinAnchour(const char* szAnchour);


#endif // __VALUE_VIEW_H__
