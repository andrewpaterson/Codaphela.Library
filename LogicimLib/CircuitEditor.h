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
class CWirePullEdit;


//The wires from a port of a component being moved to the next junction or port, and where they end: a port of a
//component that isn't moving (pcTargetPort) or a junction of the rest of the net (pcTargetPort is then a port of the
//net, NULL if it has none).
struct SMoveLead
{
	CPortView*			pcPort;
	SInt32Vec2			sTarget;
	CPortView*			pcTargetPort;
	CArrayTraceViewPtr	apcTraces;
};


typedef CArrayTemplate<SMoveLead>	CArrayMoveLead;


//An end of a wire being moved that was joined to something that isn't moving (a wire or a port), and where it was.
//While the wire moves it's joined back there by a stretch of new wire, see CCircuitEditor::GetAnchorLines.
struct SMoveAnchor
{
	CTraceView*		pcTrace;
	bool			bStart;
	SInt32Vec2		sOriginal;
};


typedef CArrayTemplate<SMoveAnchor>	CArrayMoveAnchor;


//Edits a project's circuits while one of them is simulated, as Logicim's editor (Logicim, CircuitEditor and
//SubcircuitEditor): selecting, placing, moving, turning and deleting components and wires, drawing wires, and loading
//and saving.  The editor owns the circuits and the simulation; the families and fonts are the caller's.
//
//The pointer is given in grid space.  Button one's press starts an edit (or ends a placement): on a selected view it
//moves the selection if dragged; on a hovered connection it pulls a wire out if dragged (see CWirePullEdit); anywhere
//else it drags a selection rectangle; its release ends the edit.  The selection mode follows the control and shift
//keys (see SetModifiers and CalculateSelectionMode).
//
//With no edit and nothing selected the component under the pointer is hovered, and a connection (a port or a wire's
//end) on the grid point under the pointer in it, or else a point on a wire there, as Logicim's
//calculateHighlightedPort.  Turning acts on the hovered component.  Deleting a hovered point on a wire deletes the
//wire; a hovered connection, the wires ending there or else the component.
//
//A wire pulled from one component's port and released on another's is routed around the components (see
//CTraceAutoConnector) rather than drawn as pulled, which Logicim doesn't do.
//
//Moving components drags their wires with them, which Logicim doesn't: the wires from each moved port to the next
//junction or port (its lead) are found when the move starts and, when it's done, are taken out and the port routed
//again to where its lead went (see CTraceAutoConnector), or wired there straight if there's no route.  A lead that
//ends on nothing moves with its component instead.  Leads between moved components, and leads with moved wires in
//them, stay as they are.
//
//Moving wires stretches them: each end of a moved wire that was joined to something that isn't moving is joined back
//to where it was by new wire, along and then across, drawn as a disconnected wire while it moves and added when the
//move is done.  The circuit and the simulations aren't changed until then.
//
//Wires are kept as Logicim keeps them (see MergeTraceLines): those along each other are merged and they're split
//where one ends on another and at ports, whenever wires are drawn, moved or deleted, or components moved.
//
//The simulation runs on unchanged while views are edited.  Placed components are made in every simulation of every
//path through the circuit being edited, and started; deleted ones are taken out of them; and after any change each
//simulation is rewired (see CCircuitWiring::Rewire), so components that didn't move keep their state.  A simulation
//shouldn't run while an edit is in progress (CanRun).
//
//A component's properties (see CComponentProperties) are edited as Logicim's property editor dialog edits them: the
//hovered component's, or else the selected one's, are listed by GetProperties and a new component made from them (and
//turned to a rotation) replaces it, in the circuit and in the running simulations, by SetProperties.
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
	CComponentView*						mpcNextUnit;
	ERotation							meCreationRotation;
	CComponentView*						mpcHover;
	bool								mbHoverConnection;
	SInt32Vec2							msHoverConnection;
	CTraceView*							mpcHoverTrace;  //The wire the hovered connection is a point on, NULL if it's a connection.
	CArrayMoveLead						masLeads;  //Of the components being moved.
	CArrayTraceViewPtr					mapcFollowing;  //Wires that only hang off the components being moved, moving with them.
	CArrayMoveAnchor					masAnchors;  //Of the wires being moved.
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

	CComponentView*		GetPropertiesComponent(void);
	void				GetProperties(CComponentView* pcComponent, CComponentProperties* pcProperties);
	CComponentView*		SetProperties(CComponentView* pcComponent, CComponentProperties* pcProperties, ERotation eRotation);

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
	bool				GetHoverConnection(SInt32Vec2* psConnection);
	CTraceView*			GetHoverTrace(void);
	ERotation			GetCreationRotation(void);
	bool				IsControl(void);
	bool				IsShift(void);

	void				StartMoveViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bNew, CArrayTraceViewPtr* papcFollowing);
	void				DoneMoveViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bSelect, bool bNew, bool bChanged);
	void				DeleteViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);
	bool				SelectInRectangle(SFloat64Vec2 sStart, SFloat64Vec2 sEnd, CCircuitSelection* pcPrevious);
	void				PullWire(CWirePullEdit* pcPull);

protected:
	void				StartEdit(CStatefulEdit* pcEdit, SFloat64Vec2 sStart);
	void				DoneEdit(SFloat64Vec2 sPosition);
	void				DiscardEdit(void);
	void				KillEdit(void);
	bool				StartPlace(CComponentView* pcComponent);
	void				RotateComponents(bool bRight);
	void				UpdateHover(void);
	void				ClearHover(void);
	void				GetTracesEndingAt(SInt32Vec2 sPoint, CArrayTraceViewPtr* papcTraces);
	bool				RouteWire(SInt32Vec2 sFrom, SInt32Vec2 sTo);
	void				FindLeads(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);
	void				FindLead(CPortView* pcPort, CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);
	CPortView*			FindNetPort(SInt32Vec2 sPoint, CArrayTraceViewPtr* papcExcluded, CArrayComponentViewPtr* papcComponents);
	void				RerouteLeads(CArrayTraceViewPtr* papcSeeds);
	void				KillLeads(void);
	void				InitPropertiesContext(SComponentPropertiesContext* psContext);
	bool				IsAnythingAt(SInt32Vec2 sPoint, CArrayTraceViewPtr* papcExcluded);
	void				FindAnchors(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);
	bool				IsJoinedAt(SInt32Vec2 sPoint, CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces);
	void				GetAnchorLines(SMoveAnchor* psAnchor, CArrayTraceLine* pasLines);
	void				PaintAnchors(CCircuitPainter* pcPainter);
	void				GetEditedPaths(CArrayViewPathPtr* papcViewPaths);
	void				CreateComponents(CArrayComponentViewPtr* papcComponents);
	bool				Rewire(void);
	void				KillProject(void);
	void				KillSimulation(void);
};


bool	CircuitUses(CCircuitView* pcCircuit, CCircuitView* pcSubcircuit);


#endif // __CIRCUIT_EDITOR_H__

