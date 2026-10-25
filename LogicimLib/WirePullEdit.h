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
#ifndef __WIRE_PULL_EDIT_H__
#define __WIRE_PULL_EDIT_H__
#include "TraceLines.h"
#include "CircuitEdit.h"


//A press on a port or a wire, as Logicim's StartEditInPort: dragging pulls a wire out of it, releasing without moving
//selects as a click there would.
class CStartEditInPort : public CStatefulEdit
{
CONSTRUCTABLE(CStartEditInPort);
public:
	void			Init(void);

	void			Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Rotate(bool bRight, CCircuitEdit* pcEdit) override;
	void			Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	void			Discard(CCircuitEdit* pcEdit) override;
	bool			CanTransformComponents(void) override;
};


//Pulls a new wire out from where the edit started, as Logicim's WirePull: straight along the first way the pointer
//goes, then, once the pointer leaves that line, turning once to reach it.  So the wire is one or two lines, from the
//start to the middle and from the middle to the end.  It's drawn as a disconnected wire until the button is released,
//when it's added to the circuit (see CCircuitEditor::PullWire).
//
//Logicim waits for the pointer to move straight along a row or column before it picks the first way; here a first
//move that goes off both picks the way it went furthest.
class CWirePullEdit : public CStatefulEdit
{
CONSTRUCTABLE(CWirePullEdit);
protected:
	SInt32Vec2		msFirst;
	bool			mbHasMiddle;
	bool			mbFirstHorizontal;
	SInt32Vec2		msMiddle;
	bool			mbHasSecond;
	SInt32Vec2		msSecond;

public:
	void			Init(void);

	void			Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Rotate(bool bRight, CCircuitEdit* pcEdit) override;
	void			Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	void			Discard(CCircuitEdit* pcEdit) override;
	void			Paint(CCircuitPainter* pcPainter) override;
	bool			CanTransformComponents(void) override;

	void			Update(int32 x, int32 y);
	bool			IsEmpty(void);
	void			GetLines(CArrayTraceLine* pasLines);
	SInt32Vec2		GetFirst(void);
	SInt32Vec2		GetEnd(void);
};


#endif // __WIRE_PULL_EDIT_H__

