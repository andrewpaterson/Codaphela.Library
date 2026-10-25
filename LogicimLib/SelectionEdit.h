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
#ifndef __SELECTION_EDIT_H__
#define __SELECTION_EDIT_H__
#include "CircuitSelection.h"
#include "CircuitEdit.h"


//Drags a selection rectangle, as Logicim's SelectionEdit.  The selection follows the rectangle as it's dragged, made
//from the selection before it by the selection mode (see CalculateSelectionMode).  The rectangle is dashed; a box by
//its corner shows whether it takes views it touches (dragged right to left) or only those wholly in it, and a plus
//or minus whether it adds to or takes from the selection.
class CSelectionEdit : public CStatefulEdit
{
CONSTRUCTABLE(CSelectionEdit);
protected:
	CCircuitEditor*		mpcEditor;
	CCircuitSelection	mcPrevious;
	SFloat64Vec2		msStart;
	SFloat64Vec2		msEnd;

public:
	void			Init(void);
	void			Kill(void) override;

	void			Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Rotate(bool bRight, CCircuitEdit* pcEdit) override;
	void			Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	void			Discard(CCircuitEdit* pcEdit) override;
	void			Paint(CCircuitPainter* pcPainter) override;
	bool			CanTransformComponents(void) override;
};


//A press on a selected view, as Logicim's StartEditInComponent: dragging moves the selection, releasing without moving
//selects as a click there would.
class CStartEditInComponent : public CStatefulEdit
{
CONSTRUCTABLE(CStartEditInComponent);
public:
	void			Init(void);

	void			Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	CStatefulEdit*	Rotate(bool bRight, CCircuitEdit* pcEdit) override;
	void			Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) override;
	void			Discard(CCircuitEdit* pcEdit) override;
	bool			CanTransformComponents(void) override;
};


#endif // __SELECTION_EDIT_H__

