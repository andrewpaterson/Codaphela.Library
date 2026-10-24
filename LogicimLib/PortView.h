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
#ifndef __PORT_VIEW_H__
#define __PORT_VIEW_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/Chars.h"
#include "BaseLib/Int32Vec2.h"
#include "BaseLib/Float64Vec2.h"
#include "MicroprocessorLib/Port.h"
#include "Rotation.h"
#include "CircuitPainter.h"


#define PORT_BUBBLE_DIAMETER	0.9


class CComponentView;
class CCircuitComponent;


//Where a component's ports join the circuit, as Logicim's PortView.  One view can stand for several ports (the bits
//of a bus).  An inverting port has a bubble between the component and the port, which moves the port a grid unit
//out.  The dot on the port is coloured by the voltage the ports see.
class CPortView
{
protected:
	CComponentView*		mpcOwner;
	SInt32Vec2			msRelativePosition;
	CArrayChars			maszPortNames;
	CChars				mszText;
	bool				mbInverting;
	SFloat64Vec2		msBubbleCentre;
	float64				mfBubbleDiameter;
	bool				mbClock;
	CArrayPortPtr		mapcPorts;

public:
	void				Init(CComponentView* pcOwner, const char* szPortName, int32 x, int32 y);
	void				Init(CComponentView* pcOwner, CArrayChars* paszPortNames, int32 x, int32 y);
	void				Kill(void);

	void				SetInverting(ERotation eFacing);
	void				SetClock(bool bClock);
	void				SetText(const char* szText);
	void				Rotate(ERotation eRotation);

	bool				Connect(CCircuitComponent* pcComponent);
	void				Disconnect(void);

	void				Paint(CCircuitPainter* pcPainter);
	bool				IncludeBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds);

	SInt32Vec2			GetRelativePosition(void);
	SInt32Vec2			GetGridPosition(void);
	bool				IsInverting(void);
	bool				IsClock(void);
	char*				GetText(void);
	size				NumPortNames(void);
	char*				GetPortName(size uiIndex);
	size				NumPorts(void);
	CPort*				GetPort(size uiIndex);
	CArrayPortPtr*		GetPorts(void);
	CComponentView*		GetOwner(void);
};


typedef CArrayTemplatePtr<CPortView>	CArrayPortViewPtr;


#endif // __PORT_VIEW_H__
