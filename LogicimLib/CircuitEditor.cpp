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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "BaseLib/XMLFile.h"
#include "BaseLib/XMLWriter.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "CircuitPainter.h"
#include "CircuitWiring.h"
#include "CircuitFile.h"
#include "SimulationStateXML.h"
#include "ComponentPlacement.h"
#include "MoveComponentsEdit.h"
#include "SelectionEdit.h"
#include "WirePullEdit.h"
#include "TraceAutoConnector.h"
#include "CircuitEditor.h"


//////////////////////////////////////////////////////////////////////////
//
// No circuits and nothing simulated.  New components face East, as
// Logicim's (whose West is East here).
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::Init(CFamilyVoltageConfigurationStore* pcFamilies, CCircuitFonts* pcFonts)
{
	mpcCircuits = NewMalloc<CCircuitViews>();
	mpcCircuits->Init();
	mpcCircuit = NULL;
	mpcSimulation = NULL;
	mpcFamilies = pcFamilies;
	mpcFonts = pcFonts;
	mcSelection.Init();
	mpcEdit = NULL;
	meCreationRotation = ROT_East;
	ClearHover();
	masLeads.Init();
	mapcFollowing.Init();
	msPointer.Init(0, 0);
	mbPointer = false;
	mbControl = false;
	mbShift = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::Kill(void)
{
	KillProject();
	mapcFollowing.Kill();
	masLeads.Kill();
	mcSelection.Kill();
	mpcFonts = NULL;
	mpcFamilies = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An edit in progress is dropped as it is, not discarded.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::KillProject(void)
{
	KillEdit();
	KillLeads();
	mcSelection.Clear();
	ClearHover();
	KillSimulation();
	if (mpcCircuits != NULL)
	{
		mpcCircuits->Kill();
		free(mpcCircuits);
		mpcCircuits = NULL;
	}
	mpcCircuit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The views forget the simulation before it's killed.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::KillSimulation(void)
{
	if (mpcSimulation != NULL)
	{
		if (mpcCircuit != NULL)
		{
			mpcCircuit->DisconnectSimulation(GetViewPath(), mpcSimulation);
		}
		mpcSimulation->Kill();
		free(mpcSimulation);
		mpcSimulation = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new, empty circuit (see CCircuitViews::Add).
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CCircuitEditor::AddCircuit(const char* szTypeName)
{
	return mpcCircuits->Add(szTypeName);
}


//////////////////////////////////////////////////////////////////////////
//
// Edits the circuit of the type and simulates it, in a new simulation
// started afresh; the last one is killed.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Simulate(const char* szTypeName)
{
	CCircuitView*	pcCircuit;

	pcCircuit = mpcCircuits->Get(szTypeName);
	if (pcCircuit == NULL)
	{
		return gcLogger.Error2(__METHOD__, " There is no circuit [", szTypeName, "] to simulate.", NULL);
	}

	DiscardEdit();
	mcSelection.Clear();
	KillSimulation();
	mpcCircuit = pcCircuit;
	mpcSimulation = NewMalloc<CCircuitSimulation>();
	mpcSimulation->Init(szTypeName);
	UpdateHover();
	return mpcCircuit->CreateSimulation(GetViewPath(), mpcSimulation, mpcFamilies, true);
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's mousePressed for button one: a press during an edit (a
// placement) ends it, otherwise it starts one.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::ButtonPressed(SFloat64Vec2 sPosition)
{
	CStartEditInComponent*	pcStartEdit;
	CStartEditInPort*		pcStartPort;
	CSelectionEdit*			pcSelectionEdit;

	msPointer = sPosition;
	mbPointer = true;
	if (mpcCircuit == NULL)
	{
		return;
	}

	UpdateHover();
	if (mpcEdit != NULL)
	{
		DoneEdit(sPosition);
	}
	else if (mcSelection.IsOn(sPosition))
	{
		pcStartEdit = NewMalloc<CStartEditInComponent>();
		pcStartEdit->Init();
		StartEdit(pcStartEdit, sPosition);
	}
	else if (mbHoverConnection)
	{
		pcStartPort = NewMalloc<CStartEditInPort>();
		pcStartPort->Init();
		StartEdit(pcStartPort, sPosition);
	}
	else
	{
		pcSelectionEdit = NewMalloc<CSelectionEdit>();
		pcSelectionEdit->Init();
		StartEdit(pcSelectionEdit, sPosition);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::ButtonReleased(SFloat64Vec2 sPosition)
{
	msPointer = sPosition;
	mbPointer = true;
	DoneEdit(sPosition);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::PointerMoved(SFloat64Vec2 sPosition)
{
	msPointer = sPosition;
	mbPointer = true;
	if (mpcEdit != NULL)
	{
		mpcEdit->Move(sPosition);
	}
	UpdateHover();
}


//////////////////////////////////////////////////////////////////////////
//
// A change to the keys changes the selection being dragged, as Logicim's
// mouseModifierKeys.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::SetModifiers(bool bControl, bool bShift)
{
	if ((bControl != mbControl) || (bShift != mbShift))
	{
		mbControl = bControl;
		mbShift = bShift;
		if (mbPointer)
		{
			PointerMoved(msPointer);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A new component of the element name (see NewPlacedComponentView) under
// the pointer, following it until the button is pressed, as Logicim's
// startPlaceComponent.  Fails if the pointer hasn't been over the circuit.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::StartPlaceComponent(const char* szXMLName)
{
	CComponentView*		pcComponent;

	if (!mbPointer || (mpcCircuit == NULL))
	{
		return false;
	}

	DiscardEdit();
	pcComponent = NewPlacedComponentView(szXMLName, RoundToGrid(msPointer.x), RoundToGrid(msPointer.y), meCreationRotation, mpcCircuit, mpcFonts);
	if (pcComponent == NULL)
	{
		return gcLogger.Error2(__METHOD__, " Cannot place a [", szXMLName, "].", NULL);
	}
	return StartPlace(pcComponent);
}


//////////////////////////////////////////////////////////////////////////
//
// An instance of the subcircuit, as Logicim's startPlaceSubcircuit.  A
// circuit can't hold an instance of itself, or of a subcircuit that holds
// it.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::StartPlaceSubcircuit(const char* szTypeName)
{
	CCircuitView*		pcSubcircuit;
	CComponentView*		pcComponent;

	if (!mbPointer || (mpcCircuit == NULL))
	{
		return false;
	}

	pcSubcircuit = mpcCircuits->Get(szTypeName);
	if (pcSubcircuit == NULL)
	{
		return gcLogger.Error2(__METHOD__, " There is no subcircuit [", szTypeName, "].", NULL);
	}
	if ((pcSubcircuit == mpcCircuit) || CircuitUses(pcSubcircuit, mpcCircuit))
	{
		return gcLogger.Error2(__METHOD__, " Circuit [", mpcCircuit->GetTypeName(), "] cannot hold an instance of [", szTypeName, "], which would hold it.", NULL);
	}

	DiscardEdit();
	pcComponent = NewPlacedSubcircuitInstanceView(pcSubcircuit, RoundToGrid(msPointer.x), RoundToGrid(msPointer.y), meCreationRotation, mpcFonts);
	return StartPlace(pcComponent);
}


//////////////////////////////////////////////////////////////////////////
//
// The component is in the circuit from now, but not simulated until it's
// placed.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::StartPlace(CComponentView* pcComponent)
{
	CMoveComponentsEdit*	pcMove;

	mcSelection.Clear();
	mpcCircuit->Add(pcComponent);
	pcMove = NewMalloc<CMoveComponentsEdit>();
	pcMove->Init(pcComponent, true);
	StartEdit(pcMove, msPointer);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Turns what's being moved or placed, or the component under the pointer,
// or the selection, a quarter clockwise about the pointer, as Logicim's
// rotateRight.  Components placed afterwards are turned too.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RotateRight(void)
{
	RotateComponents(true);
	meCreationRotation = ::RotateRight(meCreationRotation);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RotateLeft(void)
{
	RotateComponents(false);
	meCreationRotation = ::RotateLeft(meCreationRotation);
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's rotateMoveComponents.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RotateComponents(bool bRight)
{
	CMoveComponentsEdit*	pcMove;

	if (mpcEdit != NULL)
	{
		mpcEdit->Rotate(bRight);
		return;
	}
	if (!mbPointer)
	{
		return;
	}

	if (mpcHover != NULL)
	{
		pcMove = NewMalloc<CMoveComponentsEdit>();
		pcMove->Init(mpcHover, false);
	}
	else if (!mcSelection.IsEmpty())
	{
		pcMove = NewMalloc<CMoveComponentsEdit>();
		pcMove->Init(mcSelection.GetComponents(), mcSelection.GetTraces(), false);
	}
	else
	{
		return;
	}
	StartEdit(pcMove, msPointer);
	mpcEdit->Rotate(bRight);
	DoneEdit(msPointer);
}


//////////////////////////////////////////////////////////////////////////
//
// Deletes the selection or, with nothing selected, what's under the
// pointer, as Logicim's editActionDeleteComponentIfPossible: the wire a
// hovered point is on; the wires ending at a hovered connection, or the
// hovered component if none do.  Fails if there's nothing to delete or an
// edit is in progress.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Delete(void)
{
	CArrayComponentViewPtr	apcComponents;
	CArrayTraceViewPtr		apcTraces;
	size					i;

	if (mpcEdit != NULL)
	{
		return false;
	}

	apcComponents.Init();
	apcTraces.Init();
	if (mcSelection.IsEmpty())
	{
		if (mpcHoverTrace != NULL)
		{
			apcTraces.Add(mpcHoverTrace);
		}
		else if (mbHoverConnection)
		{
			GetTracesEndingAt(msHoverConnection, &apcTraces);
		}
		if ((apcTraces.NumElements() == 0) && (mpcHover != NULL))
		{
			apcComponents.Add(mpcHover);
		}
	}
	else
	{
		for (i = 0; i < mcSelection.NumComponents(); i++)
		{
			apcComponents.Add(mcSelection.GetComponent(i));
		}
		for (i = 0; i < mcSelection.NumTraces(); i++)
		{
			apcTraces.Add(mcSelection.GetTrace(i));
		}
	}

	if ((apcComponents.NumElements() == 0) && (apcTraces.NumElements() == 0))
	{
		apcTraces.Kill();
		apcComponents.Kill();
		return false;
	}
	DeleteViews(&apcComponents, &apcTraces);
	apcTraces.Kill();
	apcComponents.Kill();
	UpdateHover();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Escape: drops the edit (taking out what was being placed) and the
// selection, as Logicim's stopSimulatorEdit.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::StopEdit(void)
{
	DiscardEdit();
	mcSelection.Clear();
	UpdateHover();
}


//////////////////////////////////////////////////////////////////////////
//
// An edit in progress is discarded first, so what's saved is what's
// simulated.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Save(const char* szFilename)
{
	CXMLWriter	cWriter;
	bool		bResult;

	DiscardEdit();

	cWriter.Init();
	cWriter.Begin("Logicim");
	cWriter.Attribute("version", (int32)CIRCUIT_FILE_VERSION);
	cWriter.Begin("Editor");
	if (mpcCircuit != NULL)
	{
		cWriter.Attribute("circuit", mpcCircuit->GetTypeName());
	}
	cWriter.Attribute("rotation", RotationToString(meCreationRotation));
	cWriter.End();
	mpcCircuits->SaveXML(&cWriter);
	if (mpcSimulation != NULL)
	{
		SaveSimulationXML(&cWriter, mpcCircuit, GetViewPath(), mpcSimulation);
	}
	cWriter.End();
	bResult = cWriter.Write(szFilename);
	cWriter.Kill();
	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Could not write [", szFilename, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the project with the file's, as Logicim's load: its circuits,
// the circuit being edited (the first if the file has no <Editor>) and its
// simulation as saved.  If the file can't be loaded the project is left
// as it was.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Load(const char* szFilename)
{
	CXMLFile			cFile;
	CMarkupTag*			pcRoot;
	CMarkupTag*			pcEditorTag;
	CCircuitViews*		pcCircuits;
	CCircuitView*		pcCircuit;
	CCircuitSimulation*	pcSimulation;
	ERotation			eRotation;
	char*				szCircuit;
	char*				szRotation;
	bool				bResult;

	cFile.Init();
	pcRoot = ReadCircuitFileRoot(&cFile, szFilename, "Circuits");
	if (pcRoot == NULL)
	{
		cFile.Kill();
		return false;
	}

	pcCircuits = NewMalloc<CCircuitViews>();
	pcCircuits->Init();
	bResult = pcCircuits->LoadXML(pcRoot->GetTag("Circuits"));

	eRotation = ROT_East;
	szCircuit = NULL;
	pcEditorTag = pcRoot->GetTag("Editor");
	if (pcEditorTag != NULL)
	{
		szCircuit = pcEditorTag->GetStringAttribute("circuit");
		szRotation = pcEditorTag->GetStringAttribute("rotation");
		if (szRotation != NULL)
		{
			eRotation = StringToRotation(szRotation);
			if (eRotation == ROT_Cannot)
			{
				bResult = gcLogger.Error2(__METHOD__, " [", szFilename, "] has an unknown rotation [", szRotation, "].", NULL);
			}
		}
	}

	pcCircuit = NULL;
	if (bResult)
	{
		pcCircuit = (szCircuit != NULL) ? pcCircuits->Get(szCircuit) : pcCircuits->Get((size)0);
		if (pcCircuit == NULL)
		{
			bResult = gcLogger.Error2(__METHOD__, " [", szFilename, "] has no circuit to edit.", NULL);
		}
	}

	pcSimulation = NULL;
	if (bResult)
	{
		pcSimulation = NewMalloc<CCircuitSimulation>();
		pcSimulation->Init(pcCircuit->GetTypeName());
		bResult = LoadCircuitFileSimulation(pcRoot, pcCircuit, pcCircuits->GetTopPath(pcCircuit), pcSimulation, mpcFamilies);
	}
	cFile.Kill();

	if (!bResult)
	{
		if (pcSimulation != NULL)
		{
			pcCircuit->DisconnectSimulation(pcCircuits->GetTopPath(pcCircuit), pcSimulation);
			pcSimulation->Kill();
			free(pcSimulation);
		}
		pcCircuits->Kill();
		free(pcCircuits);
		return false;
	}

	KillProject();
	mpcCircuits = pcCircuits;
	mpcCircuit = pcCircuit;
	mpcSimulation = pcSimulation;
	meCreationRotation = eRotation;
	UpdateHover();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit, the component under the pointer, the edit and the
// selection, as Logicim's paint.  The painter must be for the editor's
// path and simulation.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::Paint(CCircuitPainter* pcPainter)
{
	if (mpcCircuit == NULL)
	{
		return;
	}

	mpcCircuit->Paint(pcPainter);
	if (mbHoverConnection)
	{
		pcPainter->ClearTransform();
		pcPainter->Circle(msHoverConnection.x, msHoverConnection.y, pcPainter->GetViewport()->GetCircleRadius() * 5 / pcPainter->GetViewport()->GetPixelsPerGrid(), false, true, 1, pcPainter->GetColours()->msPortHover, pcPainter->GetColours()->msPortHover);
	}
	else if (mpcHover != NULL)
	{
		PaintSelectionMarkers(pcPainter, mpcHover, pcPainter->GetColours()->msViewHover);
	}
	if (mpcEdit != NULL)
	{
		mpcEdit->Paint(pcPainter);
	}
	mcSelection.Paint(pcPainter);
	pcPainter->ClearTransform();
}


//////////////////////////////////////////////////////////////////////////
//
// The simulation doesn't run during an edit, as Logicim's canRun.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::CanRun(void)
{
	return mpcEdit == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::IsEditing(void) { return mpcEdit != NULL; }
CCircuitViews* CCircuitEditor::GetCircuits(void) { return mpcCircuits; }
CCircuitView* CCircuitEditor::GetCircuit(void) { return mpcCircuit; }
CCircuitSimulation* CCircuitEditor::GetSimulation(void) { return mpcSimulation; }
CCircuitSelection* CCircuitEditor::GetSelection(void) { return &mcSelection; }
CComponentView* CCircuitEditor::GetHover(void) { return mpcHover; }
CTraceView* CCircuitEditor::GetHoverTrace(void) { return mpcHoverTrace; }


//////////////////////////////////////////////////////////////////////////
//
// False if no connection is hovered.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::GetHoverConnection(SInt32Vec2* psConnection)
{
	*psConnection = msHoverConnection;
	return mbHoverConnection;
}
ERotation CCircuitEditor::GetCreationRotation(void) { return meCreationRotation; }
bool CCircuitEditor::IsControl(void) { return mbControl; }
bool CCircuitEditor::IsShift(void) { return mbShift; }


//////////////////////////////////////////////////////////////////////////
//
// The top path of the circuit being edited; NULL if there's none.
//
//////////////////////////////////////////////////////////////////////////
CViewPath* CCircuitEditor::GetViewPath(void)
{
	if (mpcCircuit == NULL)
	{
		return NULL;
	}
	return mpcCircuits->GetTopPath(mpcCircuit);
}


//////////////////////////////////////////////////////////////////////////
//
// Views being moved aren't selected while they move.  The leads of
// components already in the circuit are found, to be routed again when
// the move is done; the wires of leads that end on nothing are added to
// papcFollowing, to be moved with them.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::StartMoveViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bNew, CArrayTraceViewPtr* papcFollowing)
{
	size			i;
	size			j;
	SMoveLead*		psLead;
	CTraceView*		pcTrace;

	mcSelection.Clear();
	KillLeads();
	if (!bNew)
	{
		FindLeads(papcComponents, papcTraces);
	}

	for (i = masLeads.NumElements(); i > 0; i--)
	{
		psLead = masLeads.Get(i - 1);
		if ((psLead->pcTargetPort == NULL) && !IsAnythingAt(psLead->sTarget, &psLead->apcTraces))
		{
			for (j = 0; j < psLead->apcTraces.NumElements(); j++)
			{
				pcTrace = psLead->apcTraces.GetPtr(j);
				papcFollowing->Add(pcTrace);
				mapcFollowing.Add(pcTrace);
			}
			psLead->apcTraces.Kill();
			masLeads.RemoveAt(i - 1, true);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a port, or a wire other than the excluded ones, is on the grid
// point.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::IsAnythingAt(SInt32Vec2 sPoint, CArrayTraceViewPtr* papcExcluded)
{
	size			i;
	CTraceView*		pcTrace;

	if (mpcCircuit->GetPortViewAt(sPoint) != NULL)
	{
		return true;
	}
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (pcTrace->Contains(sPoint) && (papcExcluded->Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's doneMoveComponents: new components are made and started
// in the simulations.  If anything changed the moved wires, and the wires
// at the moved components' ports, are merged and split again with the
// wires they touch, and the simulations are rewired.  The views are
// selected if bSelect: the components and the wires along where the moved
// wires went.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::DoneMoveViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bSelect, bool bNew, bool bChanged)
{
	size				i;
	size				j;
	size				k;
	CArrayTraceLine		asMovedLines;
	CArrayTraceViewPtr	apcSeeds;
	CArrayTraceViewPtr	apcAdded;
	CArrayTraceLine		asNoLines;
	CTraceView*			pcTrace;
	CComponentView*		pcComponent;
	STraceLine*			psLine;
	STraceLine			sAdded;
	SInt32Vec2			sPort;

	ClearHover();
	if (bNew)
	{
		CreateComponents(papcComponents);
	}

	asMovedLines.Init();
	apcAdded.Init();
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		pcTrace = papcTraces->GetPtr(i);
		if (mapcFollowing.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND)
		{
			psLine = asMovedLines.Add();
			psLine->Init(pcTrace->GetStart(), pcTrace->GetEnd());
		}
	}
	mapcFollowing.ReInit();

	apcSeeds.Init();
	if (bChanged)
	{
		RerouteLeads(&apcSeeds);
	}
	KillLeads();

	if (bNew || bChanged)
	{
		for (i = 0; i < papcTraces->NumElements(); i++)
		{
			apcSeeds.Add(papcTraces->GetPtr(i));
		}
		for (i = 0; i < papcComponents->NumElements(); i++)
		{
			pcComponent = papcComponents->GetPtr(i);
			for (j = 0; j < pcComponent->NumPortViews(); j++)
			{
				sPort = pcComponent->GetPortView(j)->GetGridPosition();
				for (k = 0; k < mpcCircuit->NumTraces(); k++)
				{
					pcTrace = mpcCircuit->GetTrace(k);
					if (pcTrace->Contains(sPort) && (apcSeeds.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
					{
						apcSeeds.Add(pcTrace);
					}
				}
			}
		}
		asNoLines.Init();
		mpcCircuit->ReplaceWires(&asNoLines, &apcSeeds, &apcAdded);
		asNoLines.Kill();
		Rewire();
	}
	else
	{
		for (i = 0; i < papcTraces->NumElements(); i++)
		{
			apcAdded.Add(papcTraces->GetPtr(i));
		}
	}
	apcSeeds.Kill();

	mcSelection.Clear();
	if (bSelect)
	{
		for (i = 0; i < papcComponents->NumElements(); i++)
		{
			mcSelection.Add(papcComponents->GetPtr(i));
		}
		for (i = 0; i < apcAdded.NumElements(); i++)
		{
			pcTrace = apcAdded.GetPtr(i);
			sAdded.Init(pcTrace->GetStart(), pcTrace->GetEnd());
			for (j = 0; j < asMovedLines.NumElements(); j++)
			{
				if (sAdded.Overlaps(asMovedLines.Get(j), false))
				{
					mcSelection.Add(pcTrace);
					break;
				}
			}
		}
	}
	apcAdded.Kill();
	asMovedLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the views' components out of every simulation, then the views out
// of the circuit (with the paths through deleted instances) and frees
// them.  The wires that touched deleted wires are merged and split again
// (as Logicim's deleteTraceViews) and the simulations rewired.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::DeleteViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces)
{
	CArrayViewPathPtr		apcViewPaths;
	size					i;
	size					j;
	size					k;
	CViewPath*				pcViewPath;
	CCircuitSimulation*		pcSimulation;
	CComponentView*			pcComponent;
	CCircuitInstanceView*	pcInstanceView;
	CTraceView*				pcTrace;
	CTraceView*				pcOther;
	CArrayTraceViewPtr		apcTouching;
	CArrayTraceLine			asNoLines;

	ClearHover();
	apcTouching.Init();
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		pcTrace = papcTraces->GetPtr(i);
		for (j = 0; j < mpcCircuit->NumTraces(); j++)
		{
			pcOther = mpcCircuit->GetTrace(j);
			if (pcOther->Touches(pcTrace) && (papcTraces->Find(pcOther) == ARRAY_ELEMENT_NOT_FOUND) && (apcTouching.Find(pcOther) == ARRAY_ELEMENT_NOT_FOUND))
			{
				apcTouching.Add(pcOther);
			}
		}
	}

	apcViewPaths.Init();
	GetEditedPaths(&apcViewPaths);
	for (i = 0; i < apcViewPaths.NumElements(); i++)
	{
		pcViewPath = apcViewPaths.GetPtr(i);
		for (j = 0; j < pcViewPath->NumCircuitSimulations(); j++)
		{
			pcSimulation = pcViewPath->GetCircuitSimulation(j);
			for (k = 0; k < papcComponents->NumElements(); k++)
			{
				mpcCircuit->DestroyComponent(papcComponents->GetPtr(k), pcViewPath, pcSimulation);
			}
			for (k = 0; k < papcTraces->NumElements(); k++)
			{
				papcTraces->GetPtr(k)->ClearTraces(pcViewPath, pcSimulation);
			}
		}
	}
	apcViewPaths.Kill();

	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		pcComponent = papcComponents->GetPtr(i);
		mcSelection.Remove(pcComponent);
		pcInstanceView = pcComponent->GetCircuitInstanceView();
		if (pcInstanceView != NULL)
		{
			mpcCircuits->GetViewPaths()->RemovePathsThrough(pcInstanceView);
		}
		mpcCircuit->Remove(pcComponent);
		SafeKill(pcComponent);
	}
	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		pcTrace = papcTraces->GetPtr(i);
		mcSelection.Remove(pcTrace);
		mpcCircuit->RemoveTrace(pcTrace);
		SafeKill(pcTrace);
	}

	if (apcTouching.NumElements() > 0)
	{
		for (i = 0; i < apcTouching.NumElements(); i++)
		{
			mcSelection.Remove(apcTouching.GetPtr(i));
		}
		asNoLines.Init();
		mpcCircuit->ReplaceWires(&asNoLines, &apcTouching, NULL);
		asNoLines.Kill();
	}
	apcTouching.Kill();

	Rewire();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the pulled wire to the circuit, merged and split with the wires it
// touches, and rewires the simulations.  A wire pulled from one
// component's port to another's is routed around the components instead
// (see CTraceAutoConnector), unless no route can be found.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::PullWire(CWirePullEdit* pcPull)
{
	CArrayTraceLine		asLines;

	ClearHover();
	mcSelection.Clear();
	if (!RouteWire(pcPull->GetFirst(), pcPull->GetEnd()))
	{
		asLines.Init();
		pcPull->GetLines(&asLines);
		mpcCircuit->ReplaceWires(&asLines, NULL, NULL);
		asLines.Kill();
	}
	Rewire();
}


//////////////////////////////////////////////////////////////////////////
//
// The lead of each port of each component, see SMoveLead.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FindLeads(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces)
{
	size				i;
	size				j;
	CComponentView*		pcComponent;

	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		pcComponent = papcComponents->GetPtr(i);
		for (j = 0; j < pcComponent->NumPortViews(); j++)
		{
			FindLead(pcComponent->GetPortView(j), papcComponents, papcTraces);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Follows the one wire from the port on through the points where only two
// wires meet (and nothing else), to the next junction or port.  There's no
// lead if the port isn't on exactly one wire's end, if the wires go
// through a moved wire, or if they end on a moved component's port.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FindLead(CPortView* pcPort, CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces)
{
	CArrayTraceViewPtr	apcChain;
	CArrayTraceViewPtr	apcAt;
	SInt32Vec2			sPoint;
	CTraceView*			pcTrace;
	CPortView*			pcPortAt;
	SMoveLead*			psLead;
	size				i;
	bool				bLead;

	apcChain.Init();
	apcAt.Init();
	sPoint = pcPort->GetGridPosition();
	GetTracesEndingAt(sPoint, &apcAt);
	bLead = (apcAt.NumElements() == 1);
	pcTrace = bLead ? apcAt.GetPtr(0) : NULL;
	pcPortAt = NULL;
	while (bLead)
	{
		if ((papcTraces->Find(pcTrace) != ARRAY_ELEMENT_NOT_FOUND) || (apcChain.Find(pcTrace) != ARRAY_ELEMENT_NOT_FOUND))
		{
			bLead = false;
			break;
		}
		apcChain.Add(pcTrace);
		sPoint = (pcTrace->GetStart() == sPoint) ? pcTrace->GetEnd() : pcTrace->GetStart();

		apcAt.ReInit();
		for (i = 0; i < mpcCircuit->NumTraces(); i++)
		{
			if ((mpcCircuit->GetTrace(i) != pcTrace) && mpcCircuit->GetTrace(i)->Contains(sPoint))
			{
				apcAt.Add(mpcCircuit->GetTrace(i));
			}
		}
		pcPortAt = mpcCircuit->GetPortViewAt(sPoint);
		if ((pcPortAt != NULL) || (apcAt.NumElements() != 1) || !apcAt.GetPtr(0)->IsEnd(sPoint))
		{
			break;
		}
		pcTrace = apcAt.GetPtr(0);
	}

	if (bLead && (pcPortAt != NULL) && (papcComponents->Find(pcPortAt->GetOwner()) != ARRAY_ELEMENT_NOT_FOUND))
	{
		bLead = false;
	}
	if (bLead)
	{
		psLead = masLeads.Add();
		psLead->pcPort = pcPort;
		psLead->sTarget = sPoint;
		psLead->apcTraces.Init();
		for (i = 0; i < apcChain.NumElements(); i++)
		{
			psLead->apcTraces.Add(apcChain.GetPtr(i));
		}
		psLead->pcTargetPort = (pcPortAt != NULL) ? pcPortAt : FindNetPort(sPoint, &apcChain, papcComponents);
	}
	apcAt.Kill();
	apcChain.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A port of a component that isn't moving on the wires joined to those
// through the grid point, not counting the excluded wires.  NULL if there
// is none.
//
//////////////////////////////////////////////////////////////////////////
CPortView* CCircuitEditor::FindNetPort(SInt32Vec2 sPoint, CArrayTraceViewPtr* papcExcluded, CArrayComponentViewPtr* papcComponents)
{
	CArrayTraceViewPtr	apcNet;
	CArrayPortViewPtr	apcPortViews;
	CTraceView*			pcTrace;
	CPortView*			pcPortView;
	CPortView*			pcFound;
	size				i;
	size				j;

	apcNet.Init();
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (pcTrace->Contains(sPoint) && (papcExcluded->Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
		{
			apcNet.Add(pcTrace);
		}
	}
	for (i = 0; i < apcNet.NumElements(); i++)
	{
		for (j = 0; j < mpcCircuit->NumTraces(); j++)
		{
			pcTrace = mpcCircuit->GetTrace(j);
			if (pcTrace->Touches(apcNet.GetPtr(i)) && (papcExcluded->Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND) && (apcNet.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
			{
				apcNet.Add(pcTrace);
			}
		}
	}

	pcFound = NULL;
	apcPortViews.Init();
	mpcCircuit->CollectPortViews(&apcPortViews);
	for (i = 0; (i < apcPortViews.NumElements()) && (pcFound == NULL); i++)
	{
		pcPortView = apcPortViews.GetPtr(i);
		if (papcComponents->Find(pcPortView->GetOwner()) == ARRAY_ELEMENT_NOT_FOUND)
		{
			for (j = 0; j < apcNet.NumElements(); j++)
			{
				if (apcNet.GetPtr(j)->Contains(pcPortView->GetGridPosition()))
				{
					pcFound = pcPortView;
					break;
				}
			}
		}
	}
	apcPortViews.Kill();
	apcNet.Kill();
	return pcFound;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out the leads' wires and routes each moved port again to where its
// lead went, or wires it there straight (along and then across) if it
// can't be routed.  The new wires, and those at the leads' ends, are added
// to papcSeeds to be merged and split.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RerouteLeads(CArrayTraceViewPtr* papcSeeds)
{
	size					i;
	size					j;
	size					uiConnection;
	SMoveLead*				psLead;
	CTraceView*				pcTrace;
	CArrayTraceViewPtr		apcBefore;
	CArrayInt				aiConnections;
	CArrayTraceLine			asLines;
	CTraceAutoConnector		cConnector;
	SInt32Vec2				sPort;
	STraceLine*				psLine;
	bool					bRouted;

	if (masLeads.NumElements() == 0)
	{
		return;
	}

	for (i = 0; i < masLeads.NumElements(); i++)
	{
		psLead = masLeads.Get(i);
		for (j = 0; j < psLead->apcTraces.NumElements(); j++)
		{
			pcTrace = psLead->apcTraces.GetPtr(j);
			if (mpcCircuit->RemoveTrace(pcTrace))
			{
				mcSelection.Remove(pcTrace);
				SafeKill(pcTrace);
			}
		}
	}

	apcBefore.Init();
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		apcBefore.Add(mpcCircuit->GetTrace(i));
	}

	aiConnections.Init();
	cConnector.Init(mpcCircuit);
	uiConnection = 0;
	for (i = 0; i < masLeads.NumElements(); i++)
	{
		psLead = masLeads.Get(i);
		if ((psLead->pcTargetPort != NULL) && cConnector.AddConnection(cConnector.AddNet(), psLead->pcPort, psLead->pcTargetPort, TRACE_AUTO_CONNECT_NO_BEND))
		{
			aiConnections.Add((int)uiConnection);
			uiConnection++;
		}
		else
		{
			aiConnections.Add(-1);
		}
	}
	cConnector.Connect();

	asLines.Init();
	for (i = 0; i < masLeads.NumElements(); i++)
	{
		psLead = masLeads.Get(i);
		bRouted = (aiConnections.GetValue(i) != -1) && cConnector.IsConnected((size)aiConnections.GetValue(i));
		if (!bRouted)
		{
			sPort = psLead->pcPort->GetGridPosition();
			psLine = asLines.Add();
			psLine->Init(sPort.x, sPort.y, psLead->sTarget.x, sPort.y);
			psLine = asLines.Add();
			psLine->Init(psLead->sTarget.x, sPort.y, psLead->sTarget.x, psLead->sTarget.y);
		}
	}
	cConnector.Kill();
	aiConnections.Kill();

	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (apcBefore.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND)
		{
			papcSeeds->Add(pcTrace);
		}
		else
		{
			for (j = 0; j < masLeads.NumElements(); j++)
			{
				if (pcTrace->Contains(masLeads.Get(j)->sTarget))
				{
					papcSeeds->Add(pcTrace);
					break;
				}
			}
		}
	}
	apcBefore.Kill();

	if (asLines.NumElements() > 0)
	{
		mpcCircuit->ReplaceWires(&asLines, papcSeeds, NULL);
		papcSeeds->ReInit();
	}
	asLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::KillLeads(void)
{
	size	i;

	for (i = 0; i < masLeads.NumElements(); i++)
	{
		masLeads.Get(i)->apcTraces.Kill();
	}
	masLeads.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// Routes a wire from the port on sFrom to the port on sTo, if they're
// ports of different components, and merges and splits the new wires with
// those they touch.  Fails, adding nothing, if they aren't or if there's
// no route.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::RouteWire(SInt32Vec2 sFrom, SInt32Vec2 sTo)
{
	CPortView*			pcFrom;
	CPortView*			pcTo;
	CTraceAutoConnector	cConnector;
	CArrayTraceViewPtr	apcBefore;
	CArrayTraceViewPtr	apcAdded;
	CArrayTraceLine		asNoLines;
	CTraceView*			pcTrace;
	size				i;
	bool				bConnected;

	pcFrom = mpcCircuit->GetPortViewAt(sFrom);
	pcTo = mpcCircuit->GetPortViewAt(sTo);
	if ((pcFrom == NULL) || (pcTo == NULL) || (pcFrom->GetOwner() == pcTo->GetOwner()))
	{
		return false;
	}

	apcBefore.Init();
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		apcBefore.Add(mpcCircuit->GetTrace(i));
	}

	cConnector.Init(mpcCircuit);
	cConnector.AddConnection(cConnector.AddNet(), pcFrom, pcTo, TRACE_AUTO_CONNECT_NO_BEND);
	cConnector.Connect();
	bConnected = cConnector.IsConnected(0);
	cConnector.Kill();

	apcAdded.Init();
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (apcBefore.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND)
		{
			apcAdded.Add(pcTrace);
		}
	}
	apcBefore.Kill();

	if (apcAdded.NumElements() > 0)
	{
		asNoLines.Init();
		mpcCircuit->ReplaceWires(&asNoLines, &apcAdded, NULL);
		asNoLines.Kill();
	}
	apcAdded.Kill();
	return bConnected;
}


//////////////////////////////////////////////////////////////////////////
//
// Selects the views a rectangle picks, from the selection before it by the
// selection mode, as Logicim's calculateSelection.  Returns whether the
// selection changed.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::SelectInRectangle(SFloat64Vec2 sStart, SFloat64Vec2 sEnd, CCircuitSelection* pcPrevious)
{
	CArrayComponentViewPtr	apcComponents;
	CArrayTraceViewPtr		apcTraces;

	apcComponents.Init();
	apcTraces.Init();
	mpcCircuit->GetViewsInRectangle(sStart, sEnd, &apcComponents, &apcTraces);
	mcSelection.Set(&apcComponents, &apcTraces, pcPrevious, CalculateSelectionMode(mbControl, mbShift));
	apcTraces.Kill();
	apcComponents.Kill();
	return !mcSelection.Equals(pcPrevious);
}


//////////////////////////////////////////////////////////////////////////
//
// The edit owns the stateful edit.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::StartEdit(CStatefulEdit* pcEdit, SFloat64Vec2 sStart)
{
	mpcEdit = (CCircuitEdit*)malloc(sizeof(CCircuitEdit));
	mpcEdit->Init(sStart, pcEdit, this);
	UpdateHover();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's editDone.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::DoneEdit(SFloat64Vec2 sPosition)
{
	if (mpcEdit != NULL)
	{
		mpcEdit->Done(sPosition);
		KillEdit();
	}
	UpdateHover();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's discardEdit.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::DiscardEdit(void)
{
	if (mpcEdit != NULL)
	{
		mpcEdit->Discard();
		KillEdit();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::KillEdit(void)
{
	if (mpcEdit != NULL)
	{
		mpcEdit->Kill();
		free(mpcEdit);
		mpcEdit = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The component under the pointer and the connection or point on a wire,
// as Logicim's calculateHighlightedPort; none during an edit or while
// anything is selected.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::UpdateHover(void)
{
	SInt32Vec2	sPoint;

	ClearHover();
	if ((mpcEdit == NULL) && mcSelection.IsEmpty() && mbPointer && (mpcCircuit != NULL))
	{
		mpcHover = mpcCircuit->GetComponentAt(msPointer);
		sPoint.Init(RoundToGrid(msPointer.x), RoundToGrid(msPointer.y));
		if ((mpcHover != NULL) && mpcCircuit->IsConnectionAt(sPoint))
		{
			mbHoverConnection = true;
			msHoverConnection = sPoint;
		}
		else
		{
			mpcHoverTrace = mpcCircuit->GetTraceAt(sPoint);
			if (mpcHoverTrace != NULL)
			{
				mbHoverConnection = true;
				msHoverConnection = sPoint;
				if (mpcCircuit->IsConnectionAt(sPoint))
				{
					mpcHoverTrace = NULL;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::ClearHover(void)
{
	mpcHover = NULL;
	mbHoverConnection = false;
	msHoverConnection.Init(0, 0);
	mpcHoverTrace = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The wires with an end on the grid point.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::GetTracesEndingAt(SInt32Vec2 sPoint, CArrayTraceViewPtr* papcTraces)
{
	size			i;
	CTraceView*		pcTrace;

	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (pcTrace->IsEnd(sPoint))
		{
			papcTraces->Add(pcTrace);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Every path that ends in the circuit being edited.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::GetEditedPaths(CArrayViewPathPtr* papcViewPaths)
{
	mpcCircuits->GetViewPaths()->GetPathsEndingWith(mpcCircuit, papcViewPaths);
}


//////////////////////////////////////////////////////////////////////////
//
// Makes the views' components, and those of the subcircuits of instances
// among them, in every simulation of every path through the circuit being
// edited, and starts them as a simulation starts its components.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::CreateComponents(CArrayComponentViewPtr* papcComponents)
{
	CArrayViewPathPtr			apcViewPaths;
	CArrayCircuitComponentPtr	apcCreated;
	size						i;
	size						j;
	size						k;
	CViewPath*					pcViewPath;
	CCircuitSimulation*			pcSimulation;

	apcViewPaths.Init();
	GetEditedPaths(&apcViewPaths);
	for (i = 0; i < apcViewPaths.NumElements(); i++)
	{
		pcViewPath = apcViewPaths.GetPtr(i);
		for (j = 0; j < pcViewPath->NumCircuitSimulations(); j++)
		{
			pcSimulation = pcViewPath->GetCircuitSimulation(j);
			apcCreated.Init();
			mpcCircuit->CreateComponents(pcViewPath, pcSimulation, mpcFamilies, papcComponents, &apcCreated);
			for (k = 0; k < apcCreated.NumElements(); k++)
			{
				apcCreated.GetPtr(k)->Reset(pcSimulation->GetTimeline());
			}
			apcCreated.Kill();
		}
	}
	apcViewPaths.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Rewires each simulation of each path through the circuit being edited,
// once for each simulation's top path.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Rewire(void)
{
	CArrayViewPathPtr						apcViewPaths;
	CArrayViewPathPtr						apcTops;
	CArrayTemplatePtr<CCircuitSimulation>	apcSimulations;
	size									i;
	size									j;
	size									k;
	CViewPath*								pcViewPath;
	CViewPath*								pcTop;
	CCircuitSimulation*						pcSimulation;
	bool									bDone;
	CCircuitWiring							cWiring;
	bool									bResult;

	apcViewPaths.Init();
	apcTops.Init();
	apcSimulations.Init();
	GetEditedPaths(&apcViewPaths);
	for (i = 0; i < apcViewPaths.NumElements(); i++)
	{
		pcViewPath = apcViewPaths.GetPtr(i);
		pcTop = pcViewPath->GetTop();
		for (j = 0; j < pcViewPath->NumCircuitSimulations(); j++)
		{
			pcSimulation = pcViewPath->GetCircuitSimulation(j);
			bDone = false;
			for (k = 0; k < apcTops.NumElements(); k++)
			{
				if ((apcTops.GetPtr(k) == pcTop) && (apcSimulations.GetPtr(k) == pcSimulation))
				{
					bDone = true;
					break;
				}
			}
			if (!bDone)
			{
				apcTops.Add(pcTop);
				apcSimulations.Add(pcSimulation);
			}
		}
	}
	apcViewPaths.Kill();

	bResult = true;
	for (k = 0; k < apcTops.NumElements(); k++)
	{
		cWiring.Init(apcSimulations.GetPtr(k));
		bResult &= cWiring.Rewire(apcTops.GetPtr(k));
		cWiring.Kill();
	}
	apcSimulations.Kill();
	apcTops.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the circuit holds an instance of the subcircuit, or of a
// subcircuit that does, all the way down.
//
//////////////////////////////////////////////////////////////////////////
bool CircuitUses(CCircuitView* pcCircuit, CCircuitView* pcSubcircuit)
{
	size					i;
	CCircuitInstanceView*	pcInstanceView;
	CCircuitView*			pcInstanceCircuit;

	for (i = 0; i < pcCircuit->NumComponents(); i++)
	{
		pcInstanceView = pcCircuit->GetComponent(i)->GetCircuitInstanceView();
		if (pcInstanceView != NULL)
		{
			pcInstanceCircuit = pcInstanceView->GetInstanceCircuitView();
			if ((pcInstanceCircuit == pcSubcircuit) || ((pcInstanceCircuit != NULL) && CircuitUses(pcInstanceCircuit, pcSubcircuit)))
			{
				return true;
			}
		}
	}
	return false;
}

