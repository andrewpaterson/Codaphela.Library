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
#include "BaseLib/LogString.h"
#include "BaseLib/StringHelper.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "MicroprocessorLib/SubcircuitSimulation.h"
#include "MicroprocessorLib/Circuit.h"
#include "ComponentViewFactory.h"
#include "CircuitWiring.h"
#include "CircuitView.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::Init(void)
{
	mszTypeName.Init();
	mapcComponents.Init();
	mapcTraces.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The simulation keeps its components and traces.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::Kill(void)
{
	size				i;
	CComponentView*		pcComponent;
	CTraceView*			pcTrace;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		SafeKill(pcTrace);
	}
	mapcTraces.Kill();
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		SafeKill(pcComponent);
	}
	mapcComponents.Kill();
	mszTypeName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit view frees the component when it's killed.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CCircuitView::Add(CComponentView* pcComponent)
{
	mapcComponents.Add(pcComponent);
	return pcComponent;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTraceView* CCircuitView::AddTrace(int32 x0, int32 y0, int32 x1, int32 y1)
{
	CTraceView*		pcTrace;

	pcTrace = NewMalloc<CTraceView>();
	pcTrace->Init(x0, y0, x1, y1);
	mapcTraces.Add(pcTrace);
	return pcTrace;
}


//////////////////////////////////////////////////////////////////////////
//
// A wire from one point to another, horizontal first and then vertical if
// they aren't in a line.  Returns the last piece.
//
//////////////////////////////////////////////////////////////////////////
CTraceView* CCircuitView::AddTraces(int32 x0, int32 y0, int32 x1, int32 y1)
{
	if ((x0 == x1) || (y0 == y1))
	{
		return AddTrace(x0, y0, x1, y1);
	}
	AddTrace(x0, y0, x1, y0);
	return AddTrace(x1, y0, x1, y1);
}


//////////////////////////////////////////////////////////////////////////
//
// Frees every wire.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::KillTraces(void)
{
	size			i;
	CTraceView*		pcTrace;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		SafeKill(pcTrace);
	}
	mapcTraces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit view no longer frees the component.  Fails if it isn't
// here.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::Remove(CComponentView* pcComponent)
{
	size	i;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		if (mapcComponents.GetPtr(i) == pcComponent)
		{
			mapcComponents.RemoveAt(i, true);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the new component where the old one is in the order; the circuit
// view frees the new one and no longer frees the old.  Fails if the old
// one isn't here.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::Replace(CComponentView* pcOld, CComponentView* pcNew)
{
	size	uiIndex;

	uiIndex = mapcComponents.Find(pcOld);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	mapcComponents.SetPtr(uiIndex, pcNew);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The circuit view no longer frees the wire.  Fails if it isn't here.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::RemoveTrace(CTraceView* pcTrace)
{
	size	i;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		if (mapcTraces.GetPtr(i) == pcTrace)
		{
			mapcTraces.RemoveAt(i, true);
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts a wire taken out (see RemoveTrace) back at uiIndex in the wires,
// or at the end if there are fewer.  The circuit owns it again.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::InsertTrace(CTraceView* pcTrace, size uiIndex)
{
	if (uiIndex >= mapcTraces.NumElements())
	{
		mapcTraces.Add(pcTrace);
	}
	else
	{
		mapcTraces.InsertAt(&pcTrace, uiIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds wires along the lines, as Logicim's createTraceViews.  The lines,
// and the wires of every net they or the seed wires (which may be NULL)
// touch, are merged and split again (see MergeTraceLines), and those wires
// are replaced by the result, added to papcAdded unless it's NULL.  The
// replaced wires are freed; a simulated circuit must be rewired after.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::ReplaceWires(CArrayTraceLine* pasLines, CArrayTraceViewPtr* papcSeeds, CArrayTraceViewPtr* papcAdded)
{
	CArrayTraceViewPtr	apcReplaced;
	CArrayTraceLine		asLines;
	CArrayInt32Vec2		asEnds;
	CArrayInt32Vec2		asPorts;
	CArrayPortViewPtr	apcPortViews;
	STraceLine			sLine;
	STraceLine*			psLine;
	CTraceView*			pcTrace;
	SInt32Vec2			sPort;
	size				i;
	size				j;

	apcReplaced.Init();
	if (papcSeeds != NULL)
	{
		for (i = 0; i < papcSeeds->NumElements(); i++)
		{
			pcTrace = papcSeeds->GetPtr(i);
			if ((mapcTraces.Find(pcTrace) != ARRAY_ELEMENT_NOT_FOUND) && (apcReplaced.Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
			{
				apcReplaced.Add(pcTrace);
			}
		}
	}
	for (i = 0; i < pasLines->NumElements(); i++)
	{
		AddTouchingTraces(pasLines->Get(i), &apcReplaced);
	}
	for (i = 0; i < apcReplaced.NumElements(); i++)
	{
		pcTrace = apcReplaced.GetPtr(i);
		sLine.Init(pcTrace->GetStart(), pcTrace->GetEnd());
		AddTouchingTraces(&sLine, &apcReplaced);
	}

	asLines.Init();
	for (i = 0; i < pasLines->NumElements(); i++)
	{
		asLines.Add(pasLines->Get(i));
	}
	for (i = 0; i < apcReplaced.NumElements(); i++)
	{
		pcTrace = apcReplaced.GetPtr(i);
		psLine = asLines.Add();
		psLine->Init(pcTrace->GetStart(), pcTrace->GetEnd());
		RemoveTrace(pcTrace);
		SafeKill(pcTrace);
	}
	apcReplaced.Kill();

	asEnds.Init();
	for (i = 0; i < asLines.NumElements(); i++)
	{
		asEnds.Add(&asLines.Get(i)->sStart);
		asEnds.Add(&asLines.Get(i)->sEnd);
	}
	MergeTraceLines(&asLines);

	asPorts.Init();
	apcPortViews.Init();
	CollectPortViews(&apcPortViews);
	for (i = 0; i < apcPortViews.NumElements(); i++)
	{
		sPort = apcPortViews.GetPtr(i)->GetGridPosition();
		for (j = 0; j < asLines.NumElements(); j++)
		{
			if (asLines.Get(j)->Contains(sPort))
			{
				asPorts.Add(&sPort);
				break;
			}
		}
	}
	apcPortViews.Kill();
	SplitTraceLines(&asLines, &asEnds, &asPorts);
	asPorts.Kill();
	asEnds.Kill();

	for (i = 0; i < asLines.NumElements(); i++)
	{
		psLine = asLines.Get(i);
		pcTrace = AddTrace(psLine->sStart.x, psLine->sStart.y, psLine->sEnd.x, psLine->sEnd.y);
		if (papcAdded != NULL)
		{
			papcAdded->Add(pcTrace);
		}
	}
	asLines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the wires touching the line that aren't already in papcTraces.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::AddTouchingTraces(STraceLine* psLine, CArrayTraceViewPtr* papcTraces)
{
	size			i;
	CTraceView*		pcTrace;
	STraceLine		sLine;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		sLine.Init(pcTrace->GetStart(), pcTrace->GetEnd());
		if (sLine.Touches(psLine) && (papcTraces->Find(pcTrace) == ARRAY_ELEMENT_NOT_FOUND))
		{
			papcTraces->Add(pcTrace);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircuitView::NumComponents(void) { return mapcComponents.NumElements(); }
CComponentView* CCircuitView::GetComponent(size uiIndex) { return mapcComponents.SafeGetPtr(uiIndex); }
size CCircuitView::NumTraces(void) { return mapcTraces.NumElements(); }
CTraceView* CCircuitView::GetTrace(size uiIndex) { return mapcTraces.SafeGetPtr(uiIndex); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::CollectPortViews(CArrayPortViewPtr* papcPortViews)
{
	size				i;
	size				j;
	CComponentView*		pcComponent;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		for (j = 0; j < pcComponent->NumPortViews(); j++)
		{
			papcPortViews->Add(pcComponent->GetPortView(j));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Makes every component of the circuit, and of the subcircuits in it all
// the way down, in the circuit simulation for the path, starts the
// simulation (if bStart) and wires the nets (see CCircuitWiring).  The
// path must end in this circuit.  A top path is given the simulation's
// top simulation; a path through an instance has its subcircuit
// simulation from the instance.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::CreateSimulation(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, bool bStart)
{
	CCircuitWiring	cWiring;
	bool			bResult;

	if (pcViewPath->GetCircuitView() != this)
	{
		return gcLogger.Error2(__METHOD__, " The path doesn't end in this circuit.", NULL);
	}
	if (pcViewPath->IsTop() && !pcViewPath->ContainsCircuitSimulation(pcSimulation))
	{
		pcViewPath->AddSubcircuitSimulation(pcSimulation->GetSubcircuitTopSimulation());
	}

	bResult = CreateComponents(pcViewPath, pcSimulation, pcFamilies, &mapcComponents, NULL);

	cWiring.Init(pcSimulation);
	bResult &= cWiring.Wire(pcViewPath, bStart);
	cWiring.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The component of each of the views (which are in this circuit) for the
// path, then those of each instance's subcircuit for the path through the
// instance, which the instance's component gives its subcircuit
// simulation.  The components made are added to papcCreated unless it's
// NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::CreateComponents(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies, CArrayComponentViewPtr* papcComponents, CArrayCircuitComponentPtr* papcCreated)
{
	size					i;
	CComponentView*			pcComponent;
	CCircuitComponent*		pcCreated;
	CCircuitInstanceView*	pcInstanceView;
	CCircuitView*			pcSubcircuit;
	CViewPath*				pcChild;
	bool					bResult;

	bResult = true;
	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		pcComponent = papcComponents->GetPtr(i);
		if (pcComponent->MakesComponent())
		{
			pcCreated = pcComponent->CreateComponent(pcViewPath, pcSimulation, pcFamilies);
			if (pcCreated == NULL)
			{
				bResult = false;
			}
			else if (papcCreated != NULL)
			{
				papcCreated->Add(pcCreated);
			}
		}
	}

	for (i = 0; i < papcComponents->NumElements(); i++)
	{
		pcInstanceView = papcComponents->GetPtr(i)->GetCircuitInstanceView();
		if (pcInstanceView != NULL)
		{
			pcChild = pcViewPath->GetChild(pcInstanceView);
			if ((pcChild == NULL) || !pcChild->ContainsCircuitSimulation(pcSimulation))
			{
				bResult = false;
			}
			else
			{
				pcSubcircuit = pcInstanceView->GetInstanceCircuitView();
				bResult &= pcSubcircuit->CreateComponents(pcChild, pcSimulation, pcFamilies, &pcSubcircuit->mapcComponents, papcCreated);
			}
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the view's component for the path out of the running simulation,
// as CComponentView::DestroyComponent.  An instance's subcircuit is taken
// out first, all the way down, and the path through the instance loses
// the instance's simulation.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::DestroyComponent(CComponentView* pcComponent, CViewPath* pcViewPath, CCircuitSimulation* pcSimulation)
{
	CCircuitInstanceView*	pcInstanceView;
	CViewPath*				pcChild;
	CSubcircuitSimulation*	pcInstanceSimulation;

	pcInstanceSimulation = NULL;
	pcInstanceView = pcComponent->GetCircuitInstanceView();
	if (pcInstanceView != NULL)
	{
		pcChild = pcViewPath->FindChild(pcInstanceView);
		if (pcChild != NULL)
		{
			pcInstanceSimulation = pcChild->GetSubcircuitSimulation(pcSimulation);
			if (pcInstanceSimulation != NULL)
			{
				pcInstanceView->GetInstanceCircuitView()->DestroySimulation(pcChild, pcSimulation);
			}
		}
	}

	pcComponent->DestroyComponent(pcViewPath, pcSimulation);
	if ((pcInstanceSimulation != NULL) && pcInstanceSimulation->IsInstance())
	{
		pcSimulation->RemoveInstanceSimulation((CSubcircuitInstanceSimulation*)pcInstanceSimulation);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes every component of the path out of the running simulation, all
// the way down, as DestroyComponent; the wires forget their traces and
// the path forgets the simulation.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::DestroySimulation(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation)
{
	size	i;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		DestroyComponent(mapcComponents.GetPtr(i), pcViewPath, pcSimulation);
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		mapcTraces.GetPtr(i)->ClearTraces(pcViewPath, pcSimulation);
	}
	pcViewPath->RemoveSubcircuitSimulation(pcSimulation);
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the components and traces of the path in the circuit
// simulation, and of the paths through the instances in it, which the
// simulation still owns.  The paths forget the simulation.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::DisconnectSimulation(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation)
{
	size					i;
	CCircuitInstanceView*	pcInstanceView;
	CViewPath*				pcChild;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcInstanceView = mapcComponents.GetPtr(i)->GetCircuitInstanceView();
		if (pcInstanceView != NULL)
		{
			pcChild = pcViewPath->FindChild(pcInstanceView);
			if (pcChild != NULL)
			{
				pcInstanceView->GetInstanceCircuitView()->DisconnectSimulation(pcChild, pcSimulation);
			}
		}
	}
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		mapcComponents.GetPtr(i)->DisconnectComponent(pcViewPath, pcSimulation);
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		mapcTraces.GetPtr(i)->ClearTraces(pcViewPath, pcSimulation);
	}
	pcViewPath->RemoveSubcircuitSimulation(pcSimulation);
}


//////////////////////////////////////////////////////////////////////////
//
// A circuit is the top of its own path.
//
//////////////////////////////////////////////////////////////////////////
CCircuitView* CCircuitView::GetInstanceCircuitView(void)
{
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::AppendDescription(CChars* psz)
{
	if (mszTypeName.Empty())
	{
		psz->Append("Circuit");
	}
	else
	{
		psz->Append(mszTypeName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::SetTypeName(const char* szTypeName)
{
	mszTypeName.Set(szTypeName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCircuitView::GetTypeName(void)
{
	return mszTypeName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// A wire's end is a junction where a port is or where three or more wire
// arms meet (a wire passing through counts as two).
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::CalculateJunctions(void)
{
	CArrayPortViewPtr	apcPortViews;
	size				i;
	size				j;
	CTraceView*			pcTrace;
	SInt32Vec2			asEnds[2];
	bool				abJunction[2];
	int					iArms;
	int					iPorts;
	int					iEnd;
	CTraceView*			pcOther;

	apcPortViews.Init();
	CollectPortViews(&apcPortViews);

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		asEnds[0] = pcTrace->GetStart();
		asEnds[1] = pcTrace->GetEnd();
		for (iEnd = 0; iEnd < 2; iEnd++)
		{
			iArms = 0;
			iPorts = 0;
			for (j = 0; j < mapcTraces.NumElements(); j++)
			{
				pcOther = mapcTraces.GetPtr(j);
				if (pcOther->IsEnd(asEnds[iEnd]))
				{
					iArms++;
				}
				else if (pcOther->Contains(asEnds[iEnd]))
				{
					iArms += 2;
				}
			}
			for (j = 0; j < apcPortViews.NumElements(); j++)
			{
				if (apcPortViews.GetPtr(j)->GetGridPosition() == asEnds[iEnd])
				{
					iPorts++;
				}
			}
			abJunction[iEnd] = (iPorts > 0) || (iArms >= 3);
		}
		pcTrace->SetJunctions(abJunction[0], abJunction[1]);
	}
	apcPortViews.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::SaveXML(CXMLWriter* pcWriter)
{
	size	i;

	pcWriter->Begin("Circuit");
	if (!mszTypeName.Empty())
	{
		pcWriter->Attribute("type", mszTypeName.Text());
	}
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		mapcComponents.GetPtr(i)->SaveXML(pcWriter);
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		mapcTraces.GetPtr(i)->SaveXML(pcWriter);
	}
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// Stops at the first element it can't read; what was read before it stays
// in the circuit view.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::LoadXML(CMarkupTag* pcCircuitTag)
{
	STagIterator		sIter;
	CMarkupTag*			pcTag;
	CComponentView*		pcComponent;
	CTraceView*			pcTrace;
	char*				szTypeName;

	if (!pcCircuitTag->Is("Circuit"))
	{
		return gcLogger.Error2(__METHOD__, " Expected a [Circuit] element, not [", pcCircuitTag->GetName(), "].", NULL);
	}
	szTypeName = pcCircuitTag->GetStringAttribute("type");
	if (szTypeName != NULL)
	{
		mszTypeName.Set(szTypeName);
	}

	pcTag = pcCircuitTag->GetTag(&sIter);
	while (pcTag != NULL)
	{
		if (pcTag->Is("Trace"))
		{
			pcTrace = NewMalloc<CTraceView>();
			if (!pcTrace->LoadXML(pcTag))
			{
				free(pcTrace);
				return false;
			}
			mapcTraces.Add(pcTrace);
		}
		else
		{
			pcComponent = NewComponentView(pcTag->GetName());
			if (pcComponent == NULL)
			{
				return gcLogger.Error2(__METHOD__, " Unknown component [", pcTag->GetName(), "].", NULL);
			}
			if (!pcComponent->LoadXML(pcTag))
			{
				free(pcComponent);
				return false;
			}
			if (!pcComponent->LoadXMLMirrored(pcTag))
			{
				SafeKill(pcComponent);
				return false;
			}
			mapcComponents.Add(pcComponent);
		}
		pcTag = pcCircuitTag->GetNextTag(&sIter);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Wires under components.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::Paint(CCircuitPainter* pcPainter)
{
	size	i;

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		mapcTraces.GetPtr(i)->Paint(pcPainter);
	}
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		mapcComponents.GetPtr(i)->Paint(pcPainter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// In grid units.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::GetBoundingBox(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	size			i;
	bool			bHasBounds;
	SFloat64Vec2	sMinimum;
	SFloat64Vec2	sMaximum;
	CTraceView*		pcTrace;

	bHasBounds = false;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		if (mapcComponents.GetPtr(i)->GetBoundingBox(&sMinimum, &sMaximum))
		{
			if (!bHasBounds)
			{
				*psMinimum = sMinimum;
				*psMaximum = sMaximum;
				bHasBounds = true;
			}
			IncludeBoundsPoint(psMinimum, psMaximum, sMinimum.x, sMinimum.y);
			IncludeBoundsPoint(psMinimum, psMaximum, sMaximum.x, sMaximum.y);
		}
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		if (!bHasBounds)
		{
			psMinimum->Init(pcTrace->GetStart().x, pcTrace->GetStart().y);
			psMaximum->Init(pcTrace->GetStart().x, pcTrace->GetStart().y);
			bHasBounds = true;
		}
		IncludeBoundsPoint(psMinimum, psMaximum, pcTrace->GetStart().x, pcTrace->GetStart().y);
		IncludeBoundsPoint(psMinimum, psMaximum, pcTrace->GetEnd().x, pcTrace->GetEnd().y);
	}
	return bHasBounds;
}


//////////////////////////////////////////////////////////////////////////
//
// The component whose box the grid position is in, as Logicim's
// getComponentViewInScreenSpace: of several, the one whose box's centre is
// nearest.  NULL if there's none.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CCircuitView::GetComponentAt(SFloat64Vec2 sPosition)
{
	size				i;
	CComponentView*		pcComponent;
	CComponentView*		pcClosest;
	SFloat64Vec2		sMinimum;
	SFloat64Vec2		sMaximum;
	float64				fDistance;
	float64				fClosest;
	float64				fDX;
	float64				fDY;

	pcClosest = NULL;
	fClosest = 0;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		if (pcComponent->GetBoundingBox(&sMinimum, &sMaximum) && IsInBox(sPosition, sMinimum, sMaximum))
		{
			fDX = sPosition.x - (sMinimum.x + sMaximum.x) / 2;
			fDY = sPosition.y - (sMinimum.y + sMaximum.y) / 2;
			fDistance = fDX * fDX + fDY * fDY;
			if ((pcClosest == NULL) || (fDistance < fClosest))
			{
				pcClosest = pcComponent;
				fClosest = fDistance;
			}
		}
	}
	return pcClosest;
}


//////////////////////////////////////////////////////////////////////////
//
// The wire through the grid point, as Logicim's getTraceViewInScreenSpace:
// of several, the one whose middle is nearest.  NULL if there's none.
//
//////////////////////////////////////////////////////////////////////////
CTraceView* CCircuitView::GetTraceAt(SInt32Vec2 sPoint)
{
	size			i;
	CTraceView*		pcTrace;
	CTraceView*		pcClosest;
	float64			fDistance;
	float64			fClosest;
	float64			fDX;
	float64			fDY;

	pcClosest = NULL;
	fClosest = 0;
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		if (pcTrace->Contains(sPoint))
		{
			fDX = sPoint.x - (pcTrace->GetStart().x + pcTrace->GetEnd().x) / 2.0;
			fDY = sPoint.y - (pcTrace->GetStart().y + pcTrace->GetEnd().y) / 2.0;
			fDistance = fDX * fDX + fDY * fDY;
			if ((pcClosest == NULL) || (fDistance < fClosest))
			{
				pcClosest = pcTrace;
				fClosest = fDistance;
			}
		}
	}
	return pcClosest;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a port or a wire's end is on the grid point, where Logicim has a
// connection.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::IsConnectionAt(SInt32Vec2 sPoint)
{
	size	i;

	if (GetPortViewAt(sPoint) != NULL)
	{
		return true;
	}
	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		if (mapcTraces.GetPtr(i)->IsEnd(sPoint))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A port view on the grid point; NULL if there's none.
//
//////////////////////////////////////////////////////////////////////////
CPortView* CCircuitView::GetPortViewAt(SInt32Vec2 sPoint)
{
	size				i;
	size				j;
	CComponentView*		pcComponent;
	CPortView*			pcPortView;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		for (j = 0; j < pcComponent->NumPortViews(); j++)
		{
			pcPortView = pcComponent->GetPortView(j);
			if (pcPortView->GetGridPosition() == sPoint)
			{
				return pcPortView;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The views a selection rectangle from sStart to sEnd (grid positions)
// picks, as Logicim's getSelectionFromRectangle.  If the corners round to
// the same grid point it's a click there: components whose boxes hold the
// point and wires through it.  Otherwise components and wires wholly in
// the rectangle, or (dragged right to left) touching it.
//
//////////////////////////////////////////////////////////////////////////
void CCircuitView::GetViewsInRectangle(SFloat64Vec2 sStart, SFloat64Vec2 sEnd, CArrayComponentViewPtr* papcComponents, CArrayTraceViewPtr* papcTraces)
{
	size				i;
	CComponentView*		pcComponent;
	CTraceView*			pcTrace;
	SFloat64Vec2		sMinimum;
	SFloat64Vec2		sMaximum;
	SInt32Vec2			sPoint;
	SInt32Vec2			sEndPoint;
	SFloat64Vec2		sPointPosition;
	bool				bPoint;
	bool				bIntersections;

	sPoint.Init(RoundToGrid(sStart.x), RoundToGrid(sStart.y));
	sEndPoint.Init(RoundToGrid(sEnd.x), RoundToGrid(sEnd.y));
	sPointPosition.Init(sPoint.x, sPoint.y);
	bPoint = sPoint == sEndPoint;
	bIntersections = sStart.x > sEnd.x;

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		if (pcComponent->GetBoundingBox(&sMinimum, &sMaximum))
		{
			if (bPoint ? IsInBox(sPointPosition, sMinimum, sMaximum) : IsBoxInRectangle(sStart, sEnd, sMinimum, sMaximum, bIntersections))
			{
				papcComponents->Add(pcComponent);
			}
		}
	}

	for (i = 0; i < mapcTraces.NumElements(); i++)
	{
		pcTrace = mapcTraces.GetPtr(i);
		if (bPoint)
		{
			if (pcTrace->Contains(sPoint))
			{
				papcTraces->Add(pcTrace);
			}
		}
		else
		{
			sMinimum.Init(pcTrace->GetStart().x, pcTrace->GetStart().y);
			sMaximum = sMinimum;
			IncludeBoundsPoint(&sMinimum, &sMaximum, pcTrace->GetEnd().x, pcTrace->GetEnd().y);
			if (IsBoxInRectangle(sStart, sEnd, sMinimum, sMaximum, bIntersections))
			{
				papcTraces->Add(pcTrace);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's Math.round to a grid point.
//
//////////////////////////////////////////////////////////////////////////
int32 RoundToGrid(float64 f)
{
	return (int32)floor(f + 0.5);
}


//////////////////////////////////////////////////////////////////////////
//
// Edges count as in, as Logicim's BoundingBox.containsPoint.
//
//////////////////////////////////////////////////////////////////////////
bool IsInBox(SFloat64Vec2 sPosition, SFloat64Vec2 sMinimum, SFloat64Vec2 sMaximum)
{
	return (sPosition.x >= sMinimum.x) && (sPosition.x <= sMaximum.x) && (sPosition.y >= sMinimum.y) && (sPosition.y <= sMaximum.y);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the box is wholly in the rectangle with corners sStart and sEnd
// or, if bIntersections, touches it, as Logicim's BoundingBox.containsBox.
//
//////////////////////////////////////////////////////////////////////////
bool IsBoxInRectangle(SFloat64Vec2 sStart, SFloat64Vec2 sEnd, SFloat64Vec2 sMinimum, SFloat64Vec2 sMaximum, bool bIntersections)
{
	float64		fLeft;
	float64		fTop;
	float64		fRight;
	float64		fBottom;

	fLeft = (sStart.x < sEnd.x) ? sStart.x : sEnd.x;
	fRight = (sStart.x < sEnd.x) ? sEnd.x : sStart.x;
	fTop = (sStart.y < sEnd.y) ? sStart.y : sEnd.y;
	fBottom = (sStart.y < sEnd.y) ? sEnd.y : sStart.y;

	if (bIntersections)
	{
		return (fLeft <= sMaximum.x) && (fTop <= sMaximum.y) && (fRight >= sMinimum.x) && (fBottom >= sMinimum.y);
	}
	return (fLeft <= sMinimum.x) && (fTop <= sMinimum.y) && (fRight >= sMaximum.x) && (fBottom >= sMaximum.y);
}


//////////////////////////////////////////////////////////////////////////
//
// Logicim's dot grid: none when zoomed far out, single pixel dots, darker
// dots, and from zoom 2 small squares.
//
//////////////////////////////////////////////////////////////////////////
void PaintCircuitGrid(CCircuitPainter* pcPainter)
{
	CCircuitViewport*	pcViewport;
	CCircuitColours*	pcColours;
	CGeometryMap*		pcMap;
	float64				fZoom;
	SFloat64Vec2		sScreen;
	SFloat64Vec2		sTopLeft;
	SFloat64Vec2		sBottomRight;
	SInt32Vec2			sSize;
	int32				iLeft;
	int32				iTop;
	int32				iRight;
	int32				iBottom;
	int32				x;
	int32				y;
	int32				iDot;
	ARGB32				sColour;

	pcViewport = pcPainter->GetViewport();
	pcColours = pcPainter->GetColours();
	pcMap = pcPainter->GetMap();
	fZoom = pcViewport->GetZoom();
	if (fZoom < 0.3)
	{
		return;
	}

	sColour = (fZoom < 1.5) ? pcColours->msSmallGridDot : pcColours->msLargeGridDot;
	iDot = (fZoom < 2.0) ? 1 : (int32)fZoom;

	sSize = pcViewport->GetSize();
	sScreen.Init(0, 0);
	pcViewport->ScreenToGrid(&sTopLeft, &sScreen);
	sScreen.Init(sSize.x, sSize.y);
	pcViewport->ScreenToGrid(&sBottomRight, &sScreen);
	iLeft = (int32)floor(sTopLeft.x) - 1;
	iTop = (int32)floor(sTopLeft.y) - 1;
	iRight = (int32)ceil(sBottomRight.x) + 1;
	iBottom = (int32)ceil(sBottomRight.y) + 1;

	for (y = iTop; y <= iBottom; y++)
	{
		for (x = iLeft; x <= iRight; x++)
		{
			sScreen.Init(pcViewport->GridToScreenX(x), pcViewport->GridToScreenY(y));
			pcMap->AddRectangle((int32)floor(sScreen.x + 0.5) - iDot / 2, (int32)floor(sScreen.y + 0.5) - iDot / 2, iDot, iDot, sColour);
		}
	}
}

