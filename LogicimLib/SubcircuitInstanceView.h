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
#ifndef __SUBCIRCUIT_INSTANCE_VIEW_H__
#define __SUBCIRCUIT_INSTANCE_VIEW_H__
#include "ComponentView.h"
#include "CircuitFonts.h"
#include "ValueView.h"


#define SUBCIRCUIT_TYPE_FONT_SIZE	13.0
#define SUBCIRCUIT_TEXT_FONT_SIZE	10.0


class CCircuitView;
struct SPinSide;


//One of a subcircuit's pins on an instance of it, as Logicim's SubcircuitPinView: the pin inside the subcircuit and
//the instance's port view for it.
struct SSubcircuitPinView
{
	CPinView*	pcPin;
	CPortView*	pcPortView;
};


typedef CArrayTemplate<SSubcircuitPinView>	CArraySubcircuitPinView;


//What Logicim measures with fonts when it lays an instance out: its box, in the instance's grid space, and where its
//type name, comment and name are down it (x is 0 for all three).  It's measured when an instance is
//made and then kept and saved, so a loaded instance keeps its shape whatever fonts it's drawn in.
struct SSubcircuitInstanceLayout
{
	SInt32Vec2	sBoxMinimum;
	SInt32Vec2	sBoxMaximum;
	float64		fTypeY;
	float64		fCommentY;
	float64		fNameY;
};


//An instance of a subcircuit placed in a circuit, as Logicim's SubcircuitInstanceView.  It's a box with the
//subcircuit's type name in bold, its comment and its name down the middle and the subcircuit's pins around its edges,
//each with its name, as each pin's alignment, anchour and weight say (see Logicim's QuadPinViewLists).  Each shown
//pin's port view holds the instance's trace ports for the pin, "Pin 0" up for a pin named "Pin".
//
//It makes a CSubcircuitInstance, whose trace ports join the nets outside to the nets of the pins inside, and a
//CSubcircuitInstanceSimulation that the subcircuit's own components are made in for the path through the instance.
//It's a circuit instance view: its instance circuit view is the subcircuit.
//
//An instance loaded from a file knows only its subcircuit's type name; its subcircuit is set afterwards
//(SetInstanceCircuitView), which lays out its pins.  It saves as <Subcircuit> with its type, comment and layout.
class CSubcircuitInstanceView : public CComponentView, public CCircuitInstanceView
{
CONSTRUCTABLE(CSubcircuitInstanceView);
protected:
	CChars						mszTypeName;
	CChars						mszComment;
	CCircuitView*				mpcInstanceCircuitView;
	SSubcircuitInstanceLayout	msLayout;
	CArraySubcircuitPinView		masPins;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szComment, CCircuitView* pcInstanceCircuitView, CCircuitFonts* pcFonts);
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szComment, const char* szTypeName, SSubcircuitInstanceLayout* psLayout);
	virtual void					Kill(void);

			bool					SetInstanceCircuitView(CCircuitView* pcInstanceCircuitView);

	virtual const char*				GetType(void);
	virtual const char*				GetXMLName(void);
	virtual void					GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext);
	virtual bool					LoadXML(CMarkupTag* pcTag);
	virtual CCircuitComponent*		CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies);
	virtual CCircuitInstanceView*	GetCircuitInstanceView(void);

	virtual CCircuitView*			GetInstanceCircuitView(void);
	virtual void					AppendDescription(CChars* psz);
	virtual void					GetPinPortJoins(CArrayPinPortJoin* pasJoins);

			char*					GetTypeName(void);
			char*					GetComment(void);
			SSubcircuitInstanceLayout*	GetLayout(void);
			size					NumPins(void);
			SSubcircuitPinView*		GetPin(size uiIndex);
			SSubcircuitPinView*		GetPin(const char* szPinName);

	static	void					MeasureLayout(CCircuitFonts* pcFonts, CCircuitView* pcInstanceCircuitView, const char* szName, const char* szComment, SSubcircuitInstanceLayout* psLayout);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
			void					LayOut(void);
			void					AddSidePins(SPinSide* psSide, int iOffset, int iOpposite, int iEdge, int iOutX, int iOutY, ETextAlignment eAlignment, int iRightRotations);
			void					AddPin(CPinView* pcPin, int32 x, int32 y, int iOutX, int iOutY, ETextAlignment eAlignment, int iRightRotations);
};


void	GetCircuitPinViews(CCircuitView* pcCircuit, CArrayTemplatePtr<CPinView>* papcPins);


#endif // __SUBCIRCUIT_INSTANCE_VIEW_H__

