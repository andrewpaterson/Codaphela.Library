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

	bResult = CreateComponents(pcViewPath, pcSimulation, pcFamilies);

	cWiring.Init(pcSimulation);
	bResult &= cWiring.Wire(pcViewPath, bStart);
	cWiring.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Each view's component for the path, then those of each instance's
// subcircuit for the path through the instance, which the instance's
// component gives its subcircuit simulation.
//
//////////////////////////////////////////////////////////////////////////
bool CCircuitView::CreateComponents(CViewPath* pcViewPath, CCircuitSimulation* pcSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	size					i;
	CComponentView*			pcComponent;
	CCircuitInstanceView*	pcInstanceView;
	CViewPath*				pcChild;
	bool					bResult;

	bResult = true;
	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcComponent = mapcComponents.GetPtr(i);
		if (pcComponent->MakesComponent())
		{
			if (pcComponent->CreateComponent(pcViewPath, pcSimulation, pcFamilies) == NULL)
			{
				bResult = false;
			}
		}
	}

	for (i = 0; i < mapcComponents.NumElements(); i++)
	{
		pcInstanceView = mapcComponents.GetPtr(i)->GetCircuitInstanceView();
		if (pcInstanceView != NULL)
		{
			pcChild = pcViewPath->GetChild(pcInstanceView);
			if ((pcChild == NULL) || !pcChild->ContainsCircuitSimulation(pcSimulation))
			{
				bResult = false;
			}
			else
			{
				bResult &= pcInstanceView->GetInstanceCircuitView()->CreateComponents(pcChild, pcSimulation, pcFamilies);
			}
		}
	}
	return bResult;
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

