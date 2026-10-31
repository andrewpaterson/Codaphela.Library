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
#ifndef __MOVE_COMPONENTS_EDIT_H__
#define __MOVE_COMPONENTS_EDIT_H__
#include "ComponentView.h"
#include "TraceView.h"
#include "CircuitEdit.h"


//Where a moved component started.
struct SMovedComponent
{
	CComponentView*		pcComponent;
	SInt32Vec2			sPosition;
	ERotation			eRotation;
	bool				bMirrored;
};


typedef CArrayTemplate<SMovedComponent>	CArrayMovedComponent;


//Where a moved wire started.
struct SMovedTrace
{
	CTraceView*		pcTrace;
	SInt32Vec2		sStart;
	SInt32Vec2		sEnd;
};


typedef CArrayTemplate<SMovedTrace>	CArrayMovedTrace;


//Moves, turns and flips components and wires with the pointer, as Logicim's MoveComponents.  They turn and flip about
//where the edit started and move as far as the pointer has.  Moved connected, they keep their wires; otherwise they
//come off them (see CCircuitEditor::StartMoveViews).  New components (being placed) are already in the circuit but have no
//components in the simulation until the move is done; a discarded placement takes them out again, a discarded move
//puts them back where they were.
//
//A move of a list of views (the selection, duplicates) leaves them selected; a move of one component (being placed, or
//turned under the pointer) doesn't.
class CMoveComponentsEdit : public CStatefulEdit
{
CONSTRUCTABLE(CMoveComponentsEdit);
protected:
	CArrayMovedComponent	masComponents;
	CArrayMovedTrace		masTraces;
	bool					mbNew;
	bool					mbSelect;
	bool					mbConnected;
	SInt32Vec2				msLastDiff;  //Where the views were last moved to, so they're only moved again when it changes.
	int						miLastRightRotations;
	bool					mbLastMirrored;

public:
	void			Init(CComponentView* pcComponent, bool bNew, bool bConnected);
	void			Init(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bNew, bool bConnected);
	void			Kill(void) override;

	void			Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Transform(CCircuitEdit* pcEdit) override;
	void			Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	void			Discard(CCircuitEdit* pcEdit) override;
	bool			CanTransformComponents(void) override;

protected:
	void			InitMove(bool bNew, bool bSelect, bool bConnected);
	void			Follow(CCircuitEdit* pcEdit);
	void			AddComponent(CComponentView* pcComponent);
	void			AddTrace(CTraceView* pcTrace);
	void			MoveViews(bool bMirror, int iRightRotations, SInt32Vec2 sStart, SInt32Vec2 sDiff);
	void			GetViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);
};


SInt32Vec2	MoveAbout(SInt32Vec2 sPosition, bool bMirror, int iRightRotations, SInt32Vec2 sStart, SInt32Vec2 sDiff);


#endif // __MOVE_COMPONENTS_EDIT_H__

