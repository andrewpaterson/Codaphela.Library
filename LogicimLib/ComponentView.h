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
#ifndef __COMPONENT_VIEW_H__
#define __COMPONENT_VIEW_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/Float64Vec2.h"
#include "BaseLib/XMLWriter.h"
#include "BaseLib/MarkupTag.h"
#include "Rotation.h"
#include "ComponentShapes.h"
#include "PortView.h"
#include "CircuitPainter.h"


class CCircuitComponent;
class CSubcircuitSimulation;
class CFamilyVoltageConfigurationStore;


//How a component is drawn and where its ports are, as Logicim's ComponentView.  A view is drawn facing North about
//its position and turned by its rotation.  It makes its component in a simulation (CreateComponent) and remembers it;
//one view drives one simulation, unlike Logicim's views, which keep a component per simulation path.
//
//A view saves itself as an XML element named by GetXMLName with its properties as attributes.  LoadXML is called on a
//view that has been allocated but not initialised: it reads the attributes and initialises the view from them.  If it
//fails the view is left uninitialised and is freed without Kill.
//
//The bounding box is the shapes and ports, never text, so a component's size doesn't depend on the font it is drawn
//with (Logicim measured its labels into the box).
class CComponentView : public CConstructable
{
CONSTRUCTABLE(CComponentView);
protected:
	SInt32Vec2				msPosition;
	ERotation				meRotation;
	CChars					mszName;
	CComponentShapes		mcShapes;
	CArrayPortViewPtr		mapcPortViews;
	SFloat64Vec2			msBoundsMinimum;
	SFloat64Vec2			msBoundsMaximum;
	bool					mbHasBounds;
	CCircuitComponent*		mpcComponent;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName);
	virtual void					Kill(void);

	virtual const char*				GetType(void) =0;
	virtual CCircuitComponent*		CreateComponent(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies) =0;
	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetXMLName(void) =0;
	virtual bool					LoadXML(CMarkupTag* pcTag) =0;
			void					SaveXML(CXMLWriter* pcWriter);

			bool					ConnectPortViews(CCircuitComponent* pcComponent);
			void					DisconnectComponent(void);
			CCircuitComponent*		GetComponent(void);

			void					SetPosition(int32 x, int32 y);
			SInt32Vec2				GetPosition(void);
			void					SetRotation(ERotation eRotation);
			ERotation				GetRotation(void);
			char*					GetName(void);
			CComponentShapes*		GetShapes(void);

			CPortView*				AddPortView(const char* szPortName, int32 x, int32 y);
			CPortView*				AddPortView(CArrayChars* paszPortNames, int32 x, int32 y);
			size					NumPortViews(void);
			CPortView*				GetPortView(size uiIndex);
			CPortView*				GetPortView(const char* szPortName);

			void					CalculateBounds(void);
			bool					GetLocalBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);
			bool					GetBoundingBox(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum);

protected:
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
	static	bool					LoadXMLPlacement(CMarkupTag* pcTag, int32* px, int32* py, ERotation* peRotation, char** pszName);
	virtual	bool					IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds);
			void					PaintShapes(CCircuitPainter* pcPainter);
			void					PaintPorts(CCircuitPainter* pcPainter);
};


typedef CArrayTemplatePtr<CComponentView>	CArrayComponentViewPtr;


#endif // __COMPONENT_VIEW_H__
