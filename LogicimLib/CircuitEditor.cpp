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
#include "BaseLib/Markup.h"
#include "BaseLib/LogString.h"
#include "BaseLib/ArrayElementNotFound.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "CircuitPainter.h"
#include "CircuitWiring.h"
#include "CircuitFile.h"
#include "SimulationStateXML.h"
#include "ComponentPlacement.h"
#include "ComponentViewFactory.h"
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
	mcUndo.Init();
	mpcEdit = NULL;
	mpcNextUnit = NULL;
	meCreationRotation = ROT_East;
	mbCreationMirrored = false;
	ClearHover();
	masLeads.Init();
	mapcFollowing.Init();
	masAnchors.Init();
	mbDetachedMove = false;
	mbLeadsLifted = false;
	mapcLiftedLeads.Init();
	maiLiftedLeadIndices.Init();
	mapcLeadRoutes.Init();
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
	SafeKill(mpcNextUnit);
	KillProject();
	mcUndo.Kill();
	mapcLeadRoutes.Kill();
	maiLiftedLeadIndices.Kill();
	mapcLiftedLeads.Kill();
	masAnchors.Kill();
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
	masAnchors.ReInit();
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
// started afresh; the last one is killed.  Starting to edit a circuit can
// be undone; simulating the same one again, as a reset does, can't.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Simulate(const char* szTypeName)
{
	CCircuitView*	pcCircuit;
	CCircuitView*	pcPrevious;

	pcCircuit = mpcCircuits->Get(szTypeName);
	if (pcCircuit == NULL)
	{
		return gcLogger.Error2(__METHOD__, " There is no circuit [", szTypeName, "] to simulate.", NULL);
	}

	DiscardEdit();
	mcSelection.Clear();
	KillSimulation();
	pcPrevious = mpcCircuit;
	mpcCircuit = pcCircuit;
	mpcSimulation = NewMalloc<CCircuitSimulation>();
	mpcSimulation->Init(szTypeName);
	UpdateHover();
	if (!mpcCircuit->CreateSimulation(GetViewPath(), mpcSimulation, mpcFamilies, true))
	{
		return false;
	}
	if ((mcUndo.NumSnapshots() == 0) || (pcCircuit != pcPrevious))
	{
		PushUndo(false);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's mousePressed for button one: a press during an edit (a
// placement) ends it, otherwise it starts one.  A press on a component
// that isn't selected, without shift (which adds to the selection), moves
// just that component if it's dragged; on its port it's a wire pull, as a
// hovered connection is.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::ButtonPressed(SFloat64Vec2 sPosition)
{
	CStartEditInComponent*	pcStartEdit;
	CStartEditInPort*		pcStartPort;
	CSelectionEdit*			pcSelectionEdit;
	CComponentView*			pcPressed;
	SInt32Vec2				sPoint;

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
		pcStartEdit->Init(NULL);
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
		sPoint.Init(RoundToGrid(sPosition.x), RoundToGrid(sPosition.y));
		pcPressed = mbShift ? NULL : mpcCircuit->GetComponentAt(sPosition);
		if ((pcPressed != NULL) && (mpcCircuit->GetPortViewAt(sPoint) == NULL))
		{
			pcStartEdit = NewMalloc<CStartEditInComponent>();
			pcStartEdit->Init(pcPressed);
			StartEdit(pcStartEdit, sPosition);
		}
		else
		{
			pcSelectionEdit = NewMalloc<CSelectionEdit>();
			pcSelectionEdit->Init();
			StartEdit(pcSelectionEdit, sPosition);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::ButtonReleased(SFloat64Vec2 sPosition)
{
	CComponentView*		pcNextUnit;

	msPointer = sPosition;
	mbPointer = true;
	DoneEdit(sPosition);

	//A unit placed by the press is followed by placing the part's next unit under the pointer, once the button is up
	//(the release would otherwise place it straight away).
	if (mpcNextUnit != NULL)
	{
		pcNextUnit = mpcNextUnit;
		mpcNextUnit = NULL;
		if (mpcEdit == NULL)
		{
			pcNextUnit->SetPosition(RoundToGrid(msPointer.x), RoundToGrid(msPointer.y));
			StartPlace(pcNextUnit);
		}
		else
		{
			SafeKill(pcNextUnit);
		}
	}
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
// placed.  It's turned as new components are and is mirrored if they are.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::StartPlace(CComponentView* pcComponent)
{
	CMoveComponentsEdit*	pcMove;

	mcSelection.Clear();
	pcComponent->SetMirrored(mbCreationMirrored);
	mpcCircuit->Add(pcComponent);
	pcMove = NewMalloc<CMoveComponentsEdit>();
	pcMove->Init(pcComponent, true, true);
	StartEdit(pcMove, msPointer);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Turns what's being moved or placed, or the component under the pointer,
// or the selection, a quarter clockwise about the pointer, as Logicim's
// rotateRight.  Components placed afterwards are turned too, which is
// done first so the undo snapshot the turn saves has it.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RotateRight(void)
{
	meCreationRotation = ::RotateRight(meCreationRotation);
	TransformComponents(ET_RotateRight);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RotateLeft(void)
{
	meCreationRotation = ::RotateLeft(meCreationRotation);
	TransformComponents(ET_RotateLeft);
}


//////////////////////////////////////////////////////////////////////////
//
// Mirrors what's being moved or placed, or the component under the
// pointer, or the selection, left to right on the screen about the
// pointer.  Components placed afterwards are flipped too.  Logicim had no
// flipping.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FlipHorizontally(void)
{
	TransformOrientation(&meCreationRotation, &mbCreationMirrored, true, 2);
	TransformComponents(ET_FlipHorizontally);
}


//////////////////////////////////////////////////////////////////////////
//
// As FlipHorizontally, top to bottom.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FlipVertically(void)
{
	TransformOrientation(&meCreationRotation, &mbCreationMirrored, true, 0);
	TransformComponents(ET_FlipVertically);
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's rotateMoveComponents: the edit in progress is turned or
// flipped, or else an edit moving the component under the pointer, or the
// selection, is started, turned or flipped and done.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::TransformComponents(EEditTransform eTransform)
{
	CMoveComponentsEdit*	pcMove;
	bool					bStarted;

	bStarted = false;
	if (mpcEdit == NULL)
	{
		if (!mbPointer)
		{
			return;
		}

		if (mpcHover != NULL)
		{
			pcMove = NewMalloc<CMoveComponentsEdit>();
			pcMove->Init(mpcHover, false, true);
		}
		else if (!mcSelection.IsEmpty())
		{
			pcMove = NewMalloc<CMoveComponentsEdit>();
			pcMove->Init(mcSelection.GetComponents(), mcSelection.GetTraces(), false, true);
		}
		else
		{
			return;
		}
		StartEdit(pcMove, msPointer);
		bStarted = true;
	}

	switch (eTransform)
	{
	case ET_RotateRight:
		mpcEdit->Rotate(true);
		break;
	case ET_RotateLeft:
		mpcEdit->Rotate(false);
		break;
	case ET_FlipHorizontally:
		mpcEdit->Flip(true);
		break;
	case ET_FlipVertically:
		mpcEdit->Flip(false);
		break;
	}

	if (bStarted)
	{
		DoneEdit(msPointer);
	}
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
	PushUndo(false);
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
// The component whose properties are edited, as Logicim's getComponent:
// the hovered one, or else the only selected one.  NULL if there's neither
// or an edit is in progress.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CCircuitEditor::GetPropertiesComponent(void)
{
	if (mpcEdit != NULL)
	{
		return NULL;
	}
	if (mpcHover != NULL)
	{
		return mpcHover;
	}
	if (mcSelection.NumTraces() == 0)
	{
		return mcSelection.GetSingleComponent();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The component's properties, with their current values, added to the
// (initialised) properties.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::GetProperties(CComponentView* pcComponent, CComponentProperties* pcProperties)
{
	SComponentPropertiesContext	sContext;

	InitPropertiesContext(&sContext);
	pcComponent->GetProperties(pcProperties, &sContext);
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the component with a new one of the same kind made from the
// properties (see CComponentView::InitFromProperties), turned to the
// rotation and mirrored or not, as Logicim's PropertyEditEvent.  The old
// component is taken out of every simulation and freed; the new one is
// made and started in them, its wires merged and split, and the
// simulations rewired.  The new one is selected if the old one was.
// Returns the new component, or NULL (leaving the old one) if it can't be
// made, or an instance would be of a subcircuit that holds the circuit
// being edited.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CCircuitEditor::SetProperties(CComponentView* pcComponent, CComponentProperties* pcProperties, ERotation eRotation, bool bMirrored)
{
	SComponentPropertiesContext	sContext;
	CComponentView*				pcNew;
	CCircuitInstanceView*		pcInstanceView;
	CCircuitView*				pcSubcircuit;
	CArrayComponentViewPtr		apcComponents;
	CArrayTraceViewPtr			apcSeeds;
	CArrayTraceLine				asNoLines;
	CArrayViewPathPtr			apcViewPaths;
	CViewPath*					pcViewPath;
	CTraceView*					pcTrace;
	SInt32Vec2					sPort;
	bool						bSelected;
	size						i;
	size						j;

	if ((mpcEdit != NULL) || (mpcCircuit == NULL))
	{
		return NULL;
	}

	InitPropertiesContext(&sContext);
	pcNew = NewComponentView(pcComponent->GetXMLName());
	if ((pcNew == NULL) || !pcNew->InitFromProperties(pcComponent->GetPosition().x, pcComponent->GetPosition().y, eRotation, pcProperties, &sContext))
	{
		if (pcNew != NULL)
		{
			free(pcNew);
		}
		return NULL;
	}
	pcNew->SetMirrored(bMirrored);
	pcInstanceView = pcNew->GetCircuitInstanceView();
	if (pcInstanceView != NULL)
	{
		pcSubcircuit = pcInstanceView->GetInstanceCircuitView();
		if ((pcSubcircuit == mpcCircuit) || CircuitUses(pcSubcircuit, mpcCircuit))
		{
			SafeKill(pcNew);
			gcLogger.Error2(__METHOD__, " Circuit [", mpcCircuit->GetTypeName(), "] cannot hold an instance of [", pcSubcircuit->GetTypeName(), "], which would hold it.", NULL);
			return NULL;
		}
	}

	ClearHover();
	apcViewPaths.Init();
	GetEditedPaths(&apcViewPaths);
	for (i = 0; i < apcViewPaths.NumElements(); i++)
	{
		pcViewPath = apcViewPaths.GetPtr(i);
		for (j = 0; j < pcViewPath->NumCircuitSimulations(); j++)
		{
			mpcCircuit->DestroyComponent(pcComponent, pcViewPath, pcViewPath->GetCircuitSimulation(j));
		}
	}
	apcViewPaths.Kill();
	pcInstanceView = pcComponent->GetCircuitInstanceView();
	if (pcInstanceView != NULL)
	{
		mpcCircuits->GetViewPaths()->RemovePathsThrough(pcInstanceView);
	}

	bSelected = mcSelection.Contains(pcComponent);
	mcSelection.Remove(pcComponent);
	mpcCircuit->Replace(pcComponent, pcNew);
	SafeKill(pcComponent);
	if (bSelected)
	{
		mcSelection.Add(pcNew);
	}

	apcComponents.Init();
	apcComponents.Add(pcNew);
	CreateComponents(&apcComponents);
	apcComponents.Kill();

	apcSeeds.Init();
	for (i = 0; i < pcNew->NumPortViews(); i++)
	{
		sPort = pcNew->GetPortView(i)->GetGridPosition();
		for (j = 0; j < mpcCircuit->NumTraces(); j++)
		{
			pcTrace = mpcCircuit->GetTrace(j);
			if (pcTrace->Contains(sPort) && (apcSeeds.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
			{
				apcSeeds.Add(pcTrace);
			}
		}
	}
	if (apcSeeds.NumElements() > 0)
	{
		for (i = 0; i < apcSeeds.NumElements(); i++)
		{
			mcSelection.Remove(apcSeeds.GetPtr(i));
		}
		asNoLines.Init();
		mpcCircuit->ReplaceWires(&asNoLines, &apcSeeds, NULL);
		asNoLines.Kill();
	}
	apcSeeds.Kill();

	Rewire();
	UpdateHover();
	PushUndo(false);
	return pcNew;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::InitPropertiesContext(SComponentPropertiesContext* psContext)
{
	psContext->pcFamilies = mpcFamilies;
	psContext->pcFonts = mpcFonts;
	psContext->pcCircuits = mpcCircuits;
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
	SaveXML(&cWriter);
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
// The whole project as Save writes it; the writer must be initialised and
// empty.  An edit in progress isn't discarded, so call it when there's
// none or it's done.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::SaveXML(CXMLWriter* pcWriter)
{
	size	i;

	pcWriter->Begin("Logicim");
	pcWriter->Attribute("version", (int32)CIRCUIT_FILE_VERSION);
	pcWriter->Begin("Editor");
	if (mpcCircuit != NULL)
	{
		pcWriter->Attribute("circuit", mpcCircuit->GetTypeName());
	}
	pcWriter->Attribute("rotation", RotationToString(meCreationRotation));
	if (mbCreationMirrored)
	{
		pcWriter->Attribute("mirrored", true);
	}
	if (mpcCircuit != NULL)
	{
		for (i = 0; i < mcSelection.NumComponents(); i++)
		{
			pcWriter->Begin("Selected");
			pcWriter->Attribute("component", (int64)FindComponentIndex(mcSelection.GetComponent(i)));
			pcWriter->End();
		}
		for (i = 0; i < mcSelection.NumTraces(); i++)
		{
			pcWriter->Begin("Selected");
			pcWriter->Attribute("trace", (int64)FindTraceIndex(mcSelection.GetTrace(i)));
			pcWriter->End();
		}
	}
	pcWriter->End();
	mpcCircuits->SaveXML(pcWriter);
	if (mpcSimulation != NULL)
	{
		SaveSimulationXML(pcWriter, mpcCircuit, GetViewPath(), mpcSimulation);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the project with the file's, as Logicim's loadFile, which is
// the first change that can be undone back to (see LoadXML).
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Load(const char* szFilename)
{
	CXMLFile	cFile;
	CMarkupTag*	pcRoot;
	bool		bResult;

	cFile.Init();
	pcRoot = ReadCircuitFileRoot(&cFile, szFilename, "Circuits");
	bResult = (pcRoot != NULL) && LoadXML(pcRoot, szFilename);
	cFile.Kill();
	if (bResult)
	{
		PushUndo(false);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces the project with the one under pcRoot, as Logicim's load: its
// circuits, the circuit being edited (the first if there's no <Editor>),
// the selection in it and its simulation as saved.  szSource names the
// document in errors.  If it can't be loaded the project is left as it
// was.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::LoadXML(CMarkupTag* pcRoot, const char* szSource)
{
	CMarkupTag*					pcEditorTag;
	CMarkupTag*					pcTag;
	STagIterator				sIter;
	CCircuitViews*				pcCircuits;
	CCircuitView*				pcCircuit;
	CCircuitSimulation*			pcSimulation;
	ERotation					eRotation;
	bool						bMirrored;
	char*						szCircuit;
	char*						szRotation;
	char*						szMirrored;
	CArrayTemplate<int64>		aiComponents;
	CArrayTemplate<int64>		aiTraces;
	int64						iIndex;
	size						i;
	bool						bResult;

	pcCircuits = NewMalloc<CCircuitViews>();
	pcCircuits->Init();
	bResult = pcCircuits->LoadXML(pcRoot->GetTag("Circuits"));

	eRotation = ROT_East;
	bMirrored = false;
	szCircuit = NULL;
	aiComponents.Init();
	aiTraces.Init();
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
				bResult = gcLogger.Error2(__METHOD__, " [", szSource, "] has an unknown rotation [", szRotation, "].", NULL);
			}
		}
		szMirrored = pcEditorTag->GetStringAttribute("mirrored");
		if ((szMirrored != NULL) && !pcEditorTag->GetBoolAttribute("mirrored", &bMirrored))
		{
			bResult = gcLogger.Error2(__METHOD__, " [", szSource, "] has a mirrored that isn't true or false [", szMirrored, "].", NULL);
		}

		pcTag = pcEditorTag->GetTag("Selected", &sIter);
		while (pcTag != NULL)
		{
			if (pcTag->GetInt64Attribute("component", &iIndex))
			{
				aiComponents.Add(&iIndex);
			}
			else if (pcTag->GetInt64Attribute("trace", &iIndex))
			{
				aiTraces.Add(&iIndex);
			}
			else
			{
				bResult = gcLogger.Error2(__METHOD__, " [", szSource, "] has a [Selected] that is neither a component nor a trace.", NULL);
			}
			pcTag = pcEditorTag->GetNextTag(&sIter);
		}
	}

	pcCircuit = NULL;
	if (bResult)
	{
		pcCircuit = (szCircuit != NULL) ? pcCircuits->Get(szCircuit) : pcCircuits->Get((size)0);
		if (pcCircuit == NULL)
		{
			bResult = gcLogger.Error2(__METHOD__, " [", szSource, "] has no circuit to edit.", NULL);
		}
	}

	if (bResult)
	{
		for (i = 0; i < aiComponents.NumElements(); i++)
		{
			iIndex = *aiComponents.Get(i);
			if ((iIndex < 0) || ((size)iIndex >= pcCircuit->NumComponents()))
			{
				bResult = gcLogger.Error2(__METHOD__, " [", szSource, "] selects component [", LongToString(iIndex), "], which isn't in the circuit.", NULL);
			}
		}
		for (i = 0; i < aiTraces.NumElements(); i++)
		{
			iIndex = *aiTraces.Get(i);
			if ((iIndex < 0) || ((size)iIndex >= pcCircuit->NumTraces()))
			{
				bResult = gcLogger.Error2(__METHOD__, " [", szSource, "] selects trace [", LongToString(iIndex), "], which isn't in the circuit.", NULL);
			}
		}
	}

	pcSimulation = NULL;
	if (bResult)
	{
		pcSimulation = NewMalloc<CCircuitSimulation>();
		pcSimulation->Init(pcCircuit->GetTypeName());
		bResult = LoadCircuitFileSimulation(pcRoot, pcCircuit, pcCircuits->GetTopPath(pcCircuit), pcSimulation, mpcFamilies);
	}

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
		aiTraces.Kill();
		aiComponents.Kill();
		return false;
	}

	KillProject();
	mpcCircuits = pcCircuits;
	mpcCircuit = pcCircuit;
	mpcSimulation = pcSimulation;
	meCreationRotation = eRotation;
	mbCreationMirrored = bMirrored;
	for (i = 0; i < aiComponents.NumElements(); i++)
	{
		mcSelection.Add(mpcCircuit->GetComponent((size)*aiComponents.Get(i)));
	}
	for (i = 0; i < aiTraces.NumElements(); i++)
	{
		mcSelection.Add(mpcCircuit->GetTrace((size)*aiTraces.Get(i)));
	}
	aiTraces.Kill();
	aiComponents.Kill();
	UpdateHover();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Saves the project as it is now onto the undo stack, after a change.
// Nothing is saved until a circuit is being edited.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::PushUndo(bool bSelectionOnly)
{
	CXMLWriter	cWriter;

	if (mpcCircuit == NULL)
	{
		return;
	}

	cWriter.Init();
	SaveXML(&cWriter);
	mcUndo.Push(cWriter.GetText(), bSelectionOnly);
	cWriter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Ctrl+Z goes back to before the last change that wasn't only to the
// selection; bStep (Ctrl+Shift+Z) goes back one change, whatever it was.
// An edit in progress is discarded first.  False if there's nothing to
// undo or the snapshot couldn't be loaded, when nothing changes.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Undo(bool bStep)
{
	return LoadUndo(mcUndo.FindUndo(bStep));
}


//////////////////////////////////////////////////////////////////////////
//
// Ctrl+Y goes forward to after the next change that wasn't only to the
// selection, or to the last change if they all were; bStep (Ctrl+Shift+Y)
// goes forward one change.  As Undo otherwise.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Redo(bool bStep)
{
	return LoadUndo(mcUndo.FindRedo(bStep));
}


//////////////////////////////////////////////////////////////////////////
//
// Not while an edit is in progress.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::CanUndo(bool bStep)
{
	return (mpcEdit == NULL) && (mcUndo.FindUndo(bStep) != ARRAY_ELEMENT_NOT_FOUND);
}


//////////////////////////////////////////////////////////////////////////
//
// Not while an edit is in progress.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::CanRedo(bool bStep)
{
	return (mpcEdit == NULL) && (mcUndo.FindRedo(bStep) != ARRAY_ELEMENT_NOT_FOUND);
}


//////////////////////////////////////////////////////////////////////////
//
// Loads the snapshot at uiIndex and makes it the undo stack's current
// one.  A part's next unit waiting to be placed is dropped.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::LoadUndo(size uiIndex)
{
	CMarkup		cMarkup;
	CMarkupTag*	pcRoot;
	bool		bResult;

	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}

	DiscardEdit();
	SafeKill(mpcNextUnit);

	cMarkup.Init();
	pcRoot = ReadCircuitTextRoot(&cMarkup, mcUndo.Get(uiIndex), "Undo", "Circuits");
	bResult = (pcRoot != NULL) && LoadXML(pcRoot, "Undo");
	cMarkup.Kill();
	if (bResult)
	{
		mcUndo.MoveTo(uiIndex);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The component's index in the circuit being edited.
//
//////////////////////////////////////////////////////////////////////////
size CCircuitEditor::FindComponentIndex(CComponentView* pcComponent)
{
	size	i;

	for (i = 0; i < mpcCircuit->NumComponents(); i++)
	{
		if (mpcCircuit->GetComponent(i) == pcComponent)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
// The wire's index in the circuit being edited.
//
//////////////////////////////////////////////////////////////////////////
size CCircuitEditor::FindTraceIndex(CTraceView* pcTrace)
{
	size	i;

	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		if (mpcCircuit->GetTrace(i) == pcTrace)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
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
		PaintAnchors(pcPainter);
	}
	mcSelection.Paint(pcPainter);
	pcPainter->ClearTransform();
}


//////////////////////////////////////////////////////////////////////////
//
// The stretches of wire that will join the moved wires back to where they
// were, drawn as disconnected wires, as a pulled wire is.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::PaintAnchors(CCircuitPainter* pcPainter)
{
	CArrayTraceLine		asLines;
	size				i;
	STraceLine*			psLine;
	ARGB32				sColour;

	if (masAnchors.NumElements() == 0)
	{
		return;
	}
	asLines.Init();
	for (i = 0; i < masAnchors.NumElements(); i++)
	{
		GetAnchorLines(masAnchors.Get(i), &asLines);
	}
	pcPainter->ClearTransform();
	sColour = pcPainter->GetColours()->msDisconnectedTrace;
	for (i = 0; i < asLines.NumElements(); i++)
	{
		psLine = asLines.Get(i);
		pcPainter->Line(psLine->sStart.x, psLine->sStart.y, psLine->sEnd.x, psLine->sEnd.y, SHAPE_LINE_WIDTH, sColour);
	}
	asLines.Kill();
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
CUndoStack* CCircuitEditor::GetUndoStack(void) { return &mcUndo; }
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
bool CCircuitEditor::IsCreationMirrored(void) { return mbCreationMirrored; }
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
// Views being moved aren't selected while they move.  Moved bConnected,
// the leads of components already in the circuit are found, to be routed
// again as they move (see MovedViews); the wires of leads that end on
// nothing, or on another moved port, are added to papcFollowing, to be
// moved with them.  Otherwise
// views already in the circuit are detached from their wires.  New views
// (being placed) are in no simulation yet, so neither is done.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::StartMoveViews(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, bool bNew, bool bConnected, CArrayTraceViewPtr* papcFollowing)
{
	size			i;
	size			j;
	SMoveLead*		psLead;
	CTraceView*		pcTrace;

	mcSelection.Clear();
	KillLeads();
	masAnchors.ReInit();
	mbDetachedMove = false;
	if (!bNew)
	{
		if (bConnected)
		{
			FindLeads(papcComponents, papcTraces, papcFollowing);
			FindAnchors(papcComponents, papcTraces);
		}
		else
		{
			mbDetachedMove = true;
			Rewire(papcComponents, papcTraces);
		}
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
// Called by the move each time the views have moved (or turned or
// flipped): the leads are routed to where they are now.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::MovedViews(void)
{
	if (!mbDetachedMove)
	{
		RouteLeads();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The ends of the moved wires that are joined to something that isn't
// moving, see SMoveAnchor.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FindAnchors(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces)
{
	size			i;
	int				iEnd;
	CTraceView*		pcTrace;
	SInt32Vec2		sEnd;
	SMoveAnchor*	psAnchor;

	for (i = 0; i < papcTraces->NumElements(); i++)
	{
		pcTrace = papcTraces->GetPtr(i);
		for (iEnd = 0; iEnd < 2; iEnd++)
		{
			sEnd = (iEnd == 0) ? pcTrace->GetStart() : pcTrace->GetEnd();
			if (IsJoinedAt(sEnd, papcComponents, papcTraces))
			{
				psAnchor = masAnchors.Add();
				psAnchor->pcTrace = pcTrace;
				psAnchor->bStart = (iEnd == 0);
				psAnchor->sOriginal = sEnd;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a wire that isn't moving is on the grid point, or a port of a
// component that isn't.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::IsJoinedAt(SInt32Vec2 sPoint, CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces)
{
	size				i;
	size				j;
	CTraceView*			pcTrace;
	CComponentView*		pcComponent;

	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (pcTrace->Contains(sPoint) && (papcTraces->Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
		{
			return true;
		}
	}
	for (i = 0; i < mpcCircuit->NumComponents(); i++)
	{
		pcComponent = mpcCircuit->GetComponent(i);
		if (papcComponents->Find(pcComponent) == ARRAY_ELEMENT_NOT_FOUND)
		{
			for (j = 0; j < pcComponent->NumPortViews(); j++)
			{
				if (pcComponent->GetPortView(j)->GetGridPosition() == sPoint)
				{
					return true;
				}
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The new wire joining the anchor's end, where it's been moved to, back to
// where it was: first square to its wire, then along it, so a wire moved
// sideways stretches straight back.  None if it hasn't moved.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::GetAnchorLines(SMoveAnchor* psAnchor, CArrayTraceLine* pasLines)
{
	SInt32Vec2	sMoved;
	SInt32Vec2	sCorner;
	STraceLine	sWire;

	sMoved = psAnchor->bStart ? psAnchor->pcTrace->GetStart() : psAnchor->pcTrace->GetEnd();
	if (sMoved == psAnchor->sOriginal)
	{
		return;
	}
	sWire.Init(psAnchor->pcTrace->GetStart(), psAnchor->pcTrace->GetEnd());
	if (sWire.IsHorizontal())
	{
		sCorner.Init(sMoved.x, psAnchor->sOriginal.y);
	}
	else
	{
		sCorner.Init(psAnchor->sOriginal.x, sMoved.y);
	}
	if (sCorner != sMoved)
	{
		pasLines->Add()->Init(sMoved, sCorner);
	}
	if (sCorner != psAnchor->sOriginal)
	{
		pasLines->Add()->Init(sCorner, psAnchor->sOriginal);
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
// in the simulations.  If anything changed the leads' routes are kept, the
// moved wires, and the wires at the moved components' ports, are merged
// and split again with the wires they touch, and the simulations are
// rewired, which joins detached views to what they're on.  If nothing
// changed the leads are put back and detached views joined again where
// they were.  The views are selected if bSelect: the components and the
// wires along where the moved wires went.
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
	CArrayTraceLine		asAnchorLines;
	CTraceView*			pcTrace;
	CComponentView*		pcComponent;
	STraceLine*			psLine;
	STraceLine			sAdded;
	SInt32Vec2			sPort;

	ClearHover();
	if (bNew)
	{
		CreateComponents(papcComponents);

		//A unit of a part placed as its units is followed by the part's next unit (see ButtonReleased).
		if (papcComponents->NumElements() == 1)
		{
			mpcNextUnit = papcComponents->GetPtr(0)->NewNextUnit();
		}
	}

	asAnchorLines.Init();
	if (bChanged)
	{
		for (i = 0; i < masAnchors.NumElements(); i++)
		{
			GetAnchorLines(masAnchors.Get(i), &asAnchorLines);
		}
	}
	masAnchors.ReInit();

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
		FinishLeads(&apcSeeds);
	}
	else
	{
		RestoreLeads();
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
		mpcCircuit->ReplaceWires(&asAnchorLines, &apcSeeds, &apcAdded);
		Rewire();
	}
	else
	{
		for (i = 0; i < papcTraces->NumElements(); i++)
		{
			apcAdded.Add(papcTraces->GetPtr(i));
		}
		if (mbDetachedMove)
		{
			Rewire();
		}
	}
	mbDetachedMove = false;
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
	asAnchorLines.Kill();

	if (bNew || bChanged)
	{
		PushUndo(false);
	}
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
	PushUndo(false);
}


//////////////////////////////////////////////////////////////////////////
//
// The lead of each port of each component, see SMoveLead.  The wires
// between moved ports are added to papcFollowing.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FindLeads(CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, CArrayTraceViewPtr* papcFollowing)
{
	size				i;
	size				j;
	CComponentView*		pcComponent;

	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		pcComponent = papcComponents->GetPtr(i);
		for (j = 0; j < pcComponent->NumPortViews(); j++)
		{
			FindLead(pcComponent->GetPortView(j), papcComponents, papcTraces, papcFollowing);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Follows the one wire from the port on through the points where only two
// wires meet (and nothing else), to the next junction or port.  There's no
// lead if the port isn't on exactly one wire's end or if the wires go
// through a moved wire.  Wires that end on a moved component's port are
// no lead either: they move with the components, so they're added to
// papcFollowing (once, though they're found from both ends).
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FindLead(CPortView* pcPort, CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces, CArrayTraceViewPtr* papcFollowing)
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
		for (i = 0; i < apcChain.NumElements(); i++)
		{
			pcTrace = apcChain.GetPtr(i);
			if (papcFollowing->Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND)
			{
				papcFollowing->Add(pcTrace);
				mapcFollowing.Add(pcTrace);
			}
		}
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
		psLead->pcNetPort = FindNetPort(sPoint, &apcChain, papcComponents);
		psLead->pcTargetPort = (pcPortAt != NULL) ? pcPortAt : psLead->pcNetPort;
		if (psLead->pcNetPort == NULL)
		{
			psLead->pcNetPort = pcPortAt;
		}
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
// Takes the leads' wires out of the circuit, remembering where each was
// in its wires so they can be put back as they were (see RestoreLeads).
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::LiftLeads(void)
{
	size			i;
	size			j;
	size			k;
	CTraceView*		pcTrace;
	size			uiIndex;

	for (i = 0; i < masLeads.NumElements(); i++)
	{
		for (j = 0; j < masLeads.Get(i)->apcTraces.NumElements(); j++)
		{
			pcTrace = masLeads.Get(i)->apcTraces.GetPtr(j);
			uiIndex = FindTraceIndex(pcTrace);
			if ((uiIndex != ARRAY_ELEMENT_NOT_FOUND) && (mapcLiftedLeads.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
			{
				for (k = 0; (k < maiLiftedLeadIndices.NumElements()) && (maiLiftedLeadIndices.GetValue(k) < (int)uiIndex); k++)
				{
				}
				mapcLiftedLeads.InsertAt(&pcTrace, k);
				maiLiftedLeadIndices.InsertAt((int)uiIndex, k);
			}
		}
	}
	for (i = 0; i < mapcLiftedLeads.NumElements(); i++)
	{
		mpcCircuit->RemoveTrace(mapcLiftedLeads.GetPtr(i));
	}
	mbLeadsLifted = true;
}


//////////////////////////////////////////////////////////////////////////
//
// Routes each moved port again to where its lead went, where the ports
// are now, or wires it there straight (along and then across) if it can't
// be routed.  Leads that end on the same net (pcNetPort) are routed as one
// net, as a port can only be on one.  The routes replace those of the
// last time and are left in the circuit, not merged, split or simulated,
// so they're drawn as disconnected wires.  The leads' wires are taken out
// first.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RouteLeads(void)
{
	size					i;
	size					uiConnection;
	size					uiNet;
	size					uiNetIndex;
	SMoveLead*				psLead;
	CTraceView*				pcTrace;
	CArrayTraceViewPtr		apcBefore;
	CArrayInt				aiConnections;
	CArrayPortViewPtr		apcNetPorts;
	CArrayInt				aiNets;
	CTraceAutoConnector		cConnector;
	SInt32Vec2				sPort;
	bool					bRouted;

	if (masLeads.NumElements() == 0)
	{
		return;
	}
	if (!mbLeadsLifted)
	{
		LiftLeads();
	}
	KillLeadRoutes();

	apcBefore.Init();
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		apcBefore.Add(mpcCircuit->GetTrace(i));
	}

	aiConnections.Init();
	apcNetPorts.Init();
	aiNets.Init();
	cConnector.Init(mpcCircuit);
	uiConnection = 0;
	for (i = 0; i < masLeads.NumElements(); i++)
	{
		psLead = masLeads.Get(i);
		uiNet = 0;
		if (psLead->pcTargetPort != NULL)
		{
			uiNetIndex = apcNetPorts.Find(psLead->pcNetPort);
			if (uiNetIndex == ARRAY_ELEMENT_NOT_FOUND)
			{
				uiNet = cConnector.AddNet();
				apcNetPorts.Add(psLead->pcNetPort);
				aiNets.Add((int)uiNet);
			}
			else
			{
				uiNet = (size)aiNets.GetValue(uiNetIndex);
			}
		}
		if ((psLead->pcTargetPort != NULL) && cConnector.AddConnection(uiNet, psLead->pcPort, psLead->pcTargetPort, TRACE_AUTO_CONNECT_NO_BEND))
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
	aiNets.Kill();
	apcNetPorts.Kill();

	for (i = 0; i < masLeads.NumElements(); i++)
	{
		psLead = masLeads.Get(i);
		bRouted = (aiConnections.GetValue(i) != -1) && cConnector.IsConnected((size)aiConnections.GetValue(i));
		if (!bRouted)
		{
			sPort = psLead->pcPort->GetGridPosition();
			if (sPort.x != psLead->sTarget.x)
			{
				mpcCircuit->AddTrace(sPort.x, sPort.y, psLead->sTarget.x, sPort.y);
			}
			if (sPort.y != psLead->sTarget.y)
			{
				mpcCircuit->AddTrace(psLead->sTarget.x, sPort.y, psLead->sTarget.x, psLead->sTarget.y);
			}
		}
	}
	cConnector.Kill();
	aiConnections.Kill();

	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (apcBefore.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND)
		{
			mapcLeadRoutes.Add(pcTrace);
		}
	}
	apcBefore.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the leads' routes out of the circuit and frees them.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::KillLeadRoutes(void)
{
	size			i;
	CTraceView*		pcTrace;

	for (i = 0; i < mapcLeadRoutes.NumElements(); i++)
	{
		pcTrace = mapcLeadRoutes.GetPtr(i);
		mpcCircuit->RemoveTrace(pcTrace);
		SafeKill(pcTrace);
	}
	mapcLeadRoutes.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The leads' routes are taken out and their wires put back where they
// were, for a move that ends where it started.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::RestoreLeads(void)
{
	size			i;
	CTraceView*		pcTrace;

	KillLeadRoutes();
	for (i = 0; i < mapcLiftedLeads.NumElements(); i++)
	{
		pcTrace = mapcLiftedLeads.GetPtr(i);
		mpcCircuit->InsertTrace(pcTrace, (size)maiLiftedLeadIndices.GetValue(i));
	}
	mapcLiftedLeads.ReInit();
	maiLiftedLeadIndices.ReInit();
	mbLeadsLifted = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The leads' routes, where the moved ports ended up, are kept and their
// old wires freed.  The routes, and the wires at the leads' ends, are
// added to papcSeeds to be merged and split.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::FinishLeads(CArrayTraceViewPtr* papcSeeds)
{
	size			i;
	size			j;
	CTraceView*		pcTrace;

	if (masLeads.NumElements() == 0)
	{
		return;
	}
	RouteLeads();
	for (i = 0; i < mapcLiftedLeads.NumElements(); i++)
	{
		pcTrace = mapcLiftedLeads.GetPtr(i);
		mcSelection.Remove(pcTrace);
		SafeKill(pcTrace);
	}
	mapcLiftedLeads.ReInit();
	maiLiftedLeadIndices.ReInit();
	mbLeadsLifted = false;

	for (i = 0; i < mapcLeadRoutes.NumElements(); i++)
	{
		papcSeeds->Add(mapcLeadRoutes.GetPtr(i));
	}
	mapcLeadRoutes.ReInit();
	for (i = 0; i < mpcCircuit->NumTraces(); i++)
	{
		pcTrace = mpcCircuit->GetTrace(i);
		if (papcSeeds->Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND)
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::KillLeads(void)
{
	size			i;
	CTraceView*		pcTrace;

	for (i = 0; i < masLeads.NumElements(); i++)
	{
		masLeads.Get(i)->apcTraces.Kill();
	}
	masLeads.ReInit();
	for (i = 0; i < mapcLiftedLeads.NumElements(); i++)
	{
		pcTrace = mapcLiftedLeads.GetPtr(i);
		SafeKill(pcTrace);
	}
	mapcLiftedLeads.ReInit();
	maiLiftedLeadIndices.ReInit();
	mapcLeadRoutes.ReInit();
	mbLeadsLifted = false;
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
// anything is selected.  A port on the grid point nearest the pointer is
// hovered, with its component, from any side; ports are on the edge of
// their component's box, or outside it, so the box alone would only find
// them from inside.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitEditor::UpdateHover(void)
{
	SInt32Vec2	sPoint;
	CPortView*	pcPort;

	ClearHover();
	if ((mpcEdit == NULL) && mcSelection.IsEmpty() && mbPointer && (mpcCircuit != NULL))
	{
		sPoint.Init(RoundToGrid(msPointer.x), RoundToGrid(msPointer.y));
		pcPort = mpcCircuit->GetPortViewAt(sPoint);
		if (pcPort != NULL)
		{
			mpcHover = pcPort->GetOwner();
			mbHoverConnection = true;
			msHoverConnection = sPoint;
			return;
		}

		mpcHover = mpcCircuit->GetComponentAt(msPointer);
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
// once for each simulation's top path.  The detached views (either can be
// NULL) join nothing, see CCircuitWiring::SetDetached.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitEditor::Rewire(CArrayComponentViewPtr* papcDetachedComponents, CArrayTraceViewPtr* papcDetachedTraces)
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
		cWiring.SetDetached(papcDetachedComponents, papcDetachedTraces);
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

