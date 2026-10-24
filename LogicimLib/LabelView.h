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
#ifndef __LABEL_VIEW_H__
#define __LABEL_VIEW_H__
#include "ComponentView.h"
#include "CircuitFonts.h"


#define LABEL_FONT_SIZE		10.0


//A line of text on the circuit, as Logicim's LabelView, with a box around it if it's filled or bordered.  The text is
//the view's name.  The box fits the text as it measured when the label was made, a quarter of the text's height past
//its ends and half its height either side, and is saved and loaded with it.  A label makes no component.
class CLabelView : public CComponentView
{
CONSTRUCTABLE(CLabelView);
protected:
	ETextAlignment	meAlignment;
	bool			mbBold;
	bool			mbFill;
	bool			mbBorder;
	SFloat64Vec2	msBoxMinimum;
	SFloat64Vec2	msBoxMaximum;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szText, ETextAlignment eAlignment, bool bBold, bool bFill, bool bBorder, CCircuitFonts* pcFonts);
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szText, ETextAlignment eAlignment, bool bBold, bool bFill, bool bBorder, SFloat64Vec2 sBoxMinimum, SFloat64Vec2 sBoxMaximum);

	virtual const char*				GetType(void);
	virtual bool					MakesComponent(void);
	virtual CCircuitComponent*		CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
	virtual const char*				GetXMLName(void);
	virtual bool					LoadXML(CMarkupTag* pcTag);

			ETextAlignment			GetAlignment(void);
			bool					IsBold(void);
			void					GetBox(SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum);

	static	void					MeasureBox(CCircuitFonts* pcFonts, const char* szText, ETextAlignment eAlignment, bool bBold, SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
};


#endif // __LABEL_VIEW_H__
