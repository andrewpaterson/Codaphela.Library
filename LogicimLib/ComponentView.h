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
#include "MicroprocessorLib/CircuitComponent.h"
#include "Rotation.h"
#include "ComponentShapes.h"
#include "PortView.h"
#include "CircuitPainter.h"
#include "ViewPath.h"


class CCircuitComponent;
class CSubcircuitSimulation;
class CFamilyVoltageConfigurationStore;
class CComponentStateXML;
class CComponentView;


//The component a view made in one circuit simulation of one view path, and the supplies made to power it implicitly
//(see PowerPortsImplicitly), which go with it.
struct SViewPathComponent
{
	CViewPath*					pcViewPath;
	CCircuitSimulation*			pcCircuitSimulation;
	CCircuitComponent*			pcComponent;
	CArrayCircuitComponentPtr	apcSupplies;
};


typedef CArrayTemplate<SViewPathComponent>	CArrayViewPathComponent;


//Two port bits a component joins onto one trace, as a splitter joins a bit of its bus to a bit of one of its ends.
struct SPortBitJoin
{
	CPortView*	pcFrom;
	size		uiFromBit;
	CPortView*	pcTo;
	size		uiToBit;
};


typedef CArrayTemplate<SPortBitJoin>	CArrayPortBitJoin;


//How a component is drawn and where its ports are, as Logicim's ComponentView.  A view is drawn facing East about its
//position, as it's normally read (-y up and -x left), and turned by its rotation.  As Logicim's views, it makes a component for each circuit simulation of
//each view path its circuit is on (CreateComponent), in the subcircuit simulation the path has for that circuit
//simulation, and remembers each by its path and circuit simulation.  A circuit used in many places, or simulated many
//times, has one set of views and a component of each per use.
//
//A view saves itself as an XML element named by GetXMLName with its properties as attributes.  LoadXML is called on a
//view that has been allocated but not initialised: it reads the attributes and initialises the view from them.  If it
//fails the view is left uninitialised and is freed without Kill.
//
//The bounding box is the shapes and ports, never text, so a component's size doesn't depend on the font it is drawn
//with (Logicim measured its labels into the box).  A component Logicim sized to its text (a constant's box is as long
//as its widest value) is sized by measuring the text when it's made, and its box is saved with it and loaded back,
//so a loaded circuit keeps its layout whatever fonts it's drawn in (see SaveXMLBox).
//
//A view that makes no component (a label or a tunnel) says so with MakesComponent.  A view with a connection name
//(a tunnel) joins its port views to those of every view with the same name.  A view can join bits of its port views
//onto the same traces (GetBitJoins), as a splitter does.  An instance of a subcircuit is a circuit instance view
//(GetCircuitInstanceView) too: the views of its subcircuit are on the path through it.
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
	CArrayViewPathComponent	masComponents;

public:
			void					Init(int32 x, int32 y, ERotation eRotation, const char* szName);
	virtual void					Kill(void);

	virtual const char*				GetType(void) =0;
	virtual CCircuitComponent*		CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies) =0;
	virtual bool					MakesComponent(void);
	virtual const char*				GetConnectionName(void);
	virtual void					GetBitJoins(CArrayPortBitJoin* pasJoins);
	virtual CCircuitInstanceView*	GetCircuitInstanceView(void);
	virtual void					Paint(CCircuitPainter* pcPainter);

	virtual const char*				GetXMLName(void) =0;
	virtual bool					LoadXML(CMarkupTag* pcTag) =0;
			void					SaveXML(CXMLWriter* pcWriter);
	virtual CComponentStateXML*		GetComponentStateXML(void);

			void					DisconnectComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
			void					DestroyComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
			CCircuitComponent*		GetComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
			CCircuitComponent*		GetComponent(CCircuitPainter* pcPainter);
			size					NumComponents(void);

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
			CSubcircuitSimulation*	CanCreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation);
			bool					PostCreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CCircuitComponent* pcComponent, CArrayCircuitComponentPtr* papcSupplies = NULL);
			SViewPathComponent*		FindComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size* puiIndex);
	virtual	void					SaveXMLAttributes(CXMLWriter* pcWriter);
	static	bool					LoadXMLPlacement(CMarkupTag* pcTag, int32* px, int32* py, ERotation* peRotation, char** pszName);
	static	void					SaveXMLBox(CXMLWriter* pcWriter, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum);
	static	bool					LoadXMLBox(CMarkupTag* pcTag, SInt32Vec2* psBoxMinimum, SInt32Vec2* psBoxMaximum);
	static	void					SaveXMLBox(CXMLWriter* pcWriter, SFloat64Vec2 sBoxMinimum, SFloat64Vec2 sBoxMaximum);
	static	bool					LoadXMLBox(CMarkupTag* pcTag, SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum);
	virtual	bool					IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds);
			void					PaintShapes(CCircuitPainter* pcPainter);
			void					PaintPorts(CCircuitPainter* pcPainter);
};


typedef CArrayTemplatePtr<CComponentView>	CArrayComponentViewPtr;


void DestroySimulatedComponent(CCircuitComponent* pcComponent, CCircuitSimulation* pcCircuitSimulation);


#endif // __COMPONENT_VIEW_H__
