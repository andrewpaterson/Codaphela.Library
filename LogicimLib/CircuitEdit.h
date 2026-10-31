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
#ifndef __CIRCUIT_EDIT_H__
#define __CIRCUIT_EDIT_H__
#include "BaseLib/Constructable.h"
#include "BaseLib/Float64Vec2.h"
#include "BaseLib/Int32Vec2.h"


class CCircuitEdit;
class CCircuitEditor;
class CCircuitPainter;


//What an edit is doing, as Logicim's StatefulEdit: selecting, moving or placing.  Move (and Transform, when the edit
//has been turned or flipped) can hand back a different stateful edit to carry on with, which is then started; the edit
//frees the one it replaced.  Positions are grid positions.
class CStatefulEdit : public CConstructable
{
CONSTRUCTABLE(CStatefulEdit);
public:
	virtual void			Kill(void);

	virtual void			Start(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) =0;
	virtual CStatefulEdit*	Move(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) =0;
	virtual CStatefulEdit*	Transform(CCircuitEdit* pcEdit) =0;
	virtual void			Done(SFloat64Vec2 sPosition, CCircuitEdit* pcEdit) =0;
	virtual void			Discard(CCircuitEdit* pcEdit) =0;
	virtual void			Paint(CCircuitPainter* pcPainter);
	virtual bool			CanTransformComponents(void) =0;
};


//An edit in progress from a mouse press (or the start of a placement) to its end, as Logicim's Edit.  It knows where
//it started (rounded to the grid), how far the pointer has moved from there and how it has been turned and flipped
//about there, and passes the pointer to its stateful edit.  The turns and flips are kept as a mirror top to bottom (if
//IsMirrored) followed by GetRightRotations turns right, as a component's are (see TransformOrientation).
class CCircuitEdit
{
protected:
	CStatefulEdit*		mpcEdit;
	CCircuitEditor*		mpcEditor;
	SInt32Vec2			msStart;
	SInt32Vec2			msDiff;
	bool				mbHadDiff;
	int					miRightRotations;
	bool				mbMirrored;

public:
	void				Init(SFloat64Vec2 sStart, CStatefulEdit* pcEdit, CCircuitEditor* pcEditor);
	void				Kill(void);

	void				Move(SFloat64Vec2 sPosition);
	void				Rotate(bool bRight);
	void				Flip(bool bHorizontally);
	void				Done(SFloat64Vec2 sPosition);
	void				Discard(void);
	void				Paint(CCircuitPainter* pcPainter);

	bool				HasDiff(void);
	int					GetRightRotations(void);
	bool				IsMirrored(void);
	SInt32Vec2			GetStart(void);
	SInt32Vec2			GetDiff(void);
	CCircuitEditor*		GetEditor(void);
	bool				CanTransformComponents(void);

protected:
	void				Transform(void);
};


#endif // __CIRCUIT_EDIT_H__

