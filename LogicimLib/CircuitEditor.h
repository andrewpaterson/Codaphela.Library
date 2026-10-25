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
#ifndef __CIRCUIT_EDITOR_H__
#define __CIRCUIT_EDITOR_H__
#include "CircuitViews.h"
#include "CircuitFonts.h"
#include "CircuitSelection.h"
#include "CircuitEdit.h"


class CCircuitSimulation;
class CFamilyVoltageConfigurationStore;


//Edits a project's circuits while one of them is simulated, as Logicim's editor (Logicim, CircuitEditor and
//SubcircuitEditor): selecting, placing, moving, turning and deleting components and wires, and loading and saving.
//Wires aren't drawn yet.  The editor owns the circuits and the simulation; the families and fonts are the caller's.
//
//The pointer is given in grid space.  Button one's press starts an edit (or ends a placement): on a selected view it
//moves the selection if dragged, anywhere else it drags a selection rectangle; its release ends the edit.  The
//selection mode follows the control and shift keys (see SetModifiers and CalculateSelectionMode).  With no edit and
//nothing selected the component under the pointer is hovered; turning and deleting act on it.
//
//The simulation runs on unchanged while views are edited.  Placed components are made in every simulation of every
//path through the circuit being edited, and started; deleted ones are taken out of them; and after any change each
//simulation is rewired (see CCircuitWiring::Rewire), so components that didn't move keep their state.  A simulation
//shouldn't run while an edit is in progress (CanRun).
//
//A saved project is a circuits file (see CircuitFile.h) with an <Editor> element naming the circuit being edited and
//the rotation new components are placed with.
class CCircuitEditor
{
protected:
	CCircuitViews*						mpcCircuits;
	CCircuitView*						mpcCircuit;
	CCircuitSimulation*					mpcSimulation;
	CFamilyVoltageConfigurationStore*	mpcFamilies;
	CCircuitFonts*						mpcFonts;
	CCircuitSelection					mcSelection;
	CCircuitEdit*						mpcEdit;
	ERotation							meCreationRotation;
	CComponentView*						mpcHover;
	SFloat64Vec2						msPointer;
	bool								mbPointer;
	bool								mbControl;
	bool								mbShift;

public:
	void				Init(CFamilyVoltageConfigurationStore* pcFamilies, CCircuitFonts* pcFonts);
	void				Kill(void);

	CCircuitView*		AddCircuit(const char* szTypeName);
	bool				Simulate(const char* szTypeName);

	void				ButtonPressed(SFloat64Vec2 sPosition);
	void				ButtonReleased(SFloat64Vec2 sPosition);
	void				PointerMoved(SFloat64Vec2 sPosition);
	void				SetModifiers(bool bControl, bool bShift);

	bool				StartPlaceComponent(const char* szXMLName);
	bool				StartPlaceSubcircuit(const char* szTypeName);
	void				RotateRight(void);
	void				RotateLeft(void);
	bool				Delete(void);
	void				StopEdit(void);

	bool				Save(const char* szFilename);
	bool				Load(const char* szFilename);

	void				Paint(CCircuitPainter* pcPainter);

	bool				CanRun(void);
	bool				IsEditing(void);
	CCircuitViews*		GetCircuits(void);
	CCircuitView*		GetCircuit(void);
	CViewPath*			GetViewPath(void);
	CCircuitSimulation*	GetSimulation(void);
	CCircuitSelection*	GetSelection(void);
	CComponentView*		GetHover(void);
	ERotation			GetCreationRotation(void);
	bool				IsControl(void);
	bool				IsShift(void);

	void				StartMoveViews(void);
	void				DoneMoveViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bSelect, bool bNew, bool bChanged);
	void				DeleteViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);
	bool				SelectInRectangle(SFloat64Vec2 sStart, SFloat64Vec2 sEnd, CCircuitSelection* pcPrevious);

protected:
	void				StartEdit(CStatefulEdit* pcEdit, SFloat64Vec2 sStart);
	void				DoneEdit(SFloat64Vec2 sPosition);
	void				DiscardEdit(void);
	void				KillEdit(void);
	bool				StartPlace(CComponentView* pcComponent);
	void				RotateComponents(bool bRight);
	void				UpdateHover(void);
	void				GetEditedPaths(CArrayViewPathPtr* papcViewPaths);
	void				CreateComponents(CArrayComponentViewPtr* papcComponents);
	bool				Rewire(void);
	void				KillProject(void);
	void				KillSimulation(void);
};


bool	CircuitUses(CCircuitView* pcCircuit, CCircuitView* pcSubcircuit);


#endif // __CIRCUIT_EDITOR_H__

