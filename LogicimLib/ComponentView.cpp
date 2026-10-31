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
#include "BaseLib/StringHelper.h"
#include "BaseLib/Logger.h"
#include "MicroprocessorLib/CircuitComponent.h"
#include "MicroprocessorLib/CircuitSimulation.h"
#include "ComponentView.h"


//////////////////////////////////////////////////////////////////////////
//
// Subclasses add their shapes and port views and then CalculateBounds.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::Init(int32 x, int32 y, ERotation eRotation, const char* szName)
{
	msPosition.Init(x, y);
	meRotation = eRotation;
	mbMirrored = false;
	mszName.Init(szName);
	mcShapes.Init();
	mapcPortViews.Init();
	msBoundsMinimum.Init(0, 0);
	msBoundsMaximum.Init(0, 0);
	mbHasBounds = false;
	masComponents.Init();
}


//////////////////////////////////////////////////////////////////////////
//
// The components belong to their simulations and aren't freed.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::Kill(void)
{
	size		i;
	CPortView*	pcPortView;

	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		pcPortView = mapcPortViews.GetPtr(i);
		SafeKill(pcPortView);
	}
	mapcPortViews.Kill();
	mcShapes.Kill();
	mszName.Kill();
	for (i = 0; i < masComponents.NumElements(); i++)
	{
		masComponents.Get(i)->apcSupplies.Kill();
	}
	masComponents.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SaveXML(CXMLWriter* pcWriter)
{
	pcWriter->Begin(GetXMLName());
	SaveXMLAttributes(pcWriter);
	pcWriter->End();
}


//////////////////////////////////////////////////////////////////////////
//
// The helper that saves and loads the simulation state of this view's
// component; NULL if it keeps none.
//
//////////////////////////////////////////////////////////////////////////
CComponentStateXML* CComponentView::GetComponentStateXML(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Subclasses add their properties after these.  Mirrored is only saved
// when it's true, so files from before mirroring are the same.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	pcWriter->Attribute("name", mszName.Text());
	pcWriter->Attribute("x", msPosition.x);
	pcWriter->Attribute("y", msPosition.y);
	pcWriter->Attribute("rotation", RotationToString(meRotation));
	if (mbMirrored)
	{
		pcWriter->Attribute("mirrored", true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Called on a view LoadXML has initialised, as LoadXML's Init makes it
// unmirrored.  A missing mirrored attribute is false.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::LoadXMLMirrored(CMarkupTag* pcTag)
{
	char*	szMirrored;

	mbMirrored = false;
	szMirrored = pcTag->GetStringAttribute("mirrored");
	if (szMirrored == NULL)
	{
		return true;
	}
	if (!pcTag->GetBoolAttribute("mirrored", &mbMirrored))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has a mirrored attribute that isn't true or false [", szMirrored, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The attributes SaveXMLAttributes writes.  A missing name is empty.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::LoadXMLPlacement(CMarkupTag* pcTag, int32* px, int32* py, ERotation* peRotation, char** pszName)
{
	char*	szRotation;

	*pszName = pcTag->GetStringAttribute("name");
	if (*pszName == NULL)
	{
		*pszName = (char*)"";
	}
	if (!pcTag->GetInt32Attribute("x", px) || !pcTag->GetInt32Attribute("y", py))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs whole number x and y attributes.", NULL);
	}
	szRotation = pcTag->GetStringAttribute("rotation");
	*peRotation = StringToRotation(szRotation);
	if (*peRotation == ROT_Cannot)
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has no rotation or an unknown one [", (szRotation != NULL) ? szRotation : "", "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The box of a component sized to its text, in its unturned grid space, as
// left, top, right and bottom attributes.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SaveXMLBox(CXMLWriter* pcWriter, SInt32Vec2 sBoxMinimum, SInt32Vec2 sBoxMaximum)
{
	pcWriter->Attribute("left", sBoxMinimum.x);
	pcWriter->Attribute("top", sBoxMinimum.y);
	pcWriter->Attribute("right", sBoxMaximum.x);
	pcWriter->Attribute("bottom", sBoxMaximum.y);
}


//////////////////////////////////////////////////////////////////////////
//
// The box SaveXMLBox writes.  It must have some width and height.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::LoadXMLBox(CMarkupTag* pcTag, SInt32Vec2* psBoxMinimum, SInt32Vec2* psBoxMaximum)
{
	if (!pcTag->GetInt32Attribute("left", &psBoxMinimum->x) || !pcTag->GetInt32Attribute("top", &psBoxMinimum->y) ||
		!pcTag->GetInt32Attribute("right", &psBoxMaximum->x) || !pcTag->GetInt32Attribute("bottom", &psBoxMaximum->y))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs whole number left, top, right and bottom attributes.", NULL);
	}
	if ((psBoxMaximum->x <= psBoxMinimum->x) || (psBoxMaximum->y <= psBoxMinimum->y))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] right and bottom must be past left and top.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A box in fractions of a grid unit, as SaveXMLBox's whole one.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SaveXMLBox(CXMLWriter* pcWriter, SFloat64Vec2 sBoxMinimum, SFloat64Vec2 sBoxMaximum)
{
	pcWriter->Attribute("left", sBoxMinimum.x);
	pcWriter->Attribute("top", sBoxMinimum.y);
	pcWriter->Attribute("right", sBoxMaximum.x);
	pcWriter->Attribute("bottom", sBoxMaximum.y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::LoadXMLBox(CMarkupTag* pcTag, SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum)
{
	if (!pcTag->GetFloat64Attribute("left", &psBoxMinimum->x) || !pcTag->GetFloat64Attribute("top", &psBoxMinimum->y) ||
		!pcTag->GetFloat64Attribute("right", &psBoxMaximum->x) || !pcTag->GetFloat64Attribute("bottom", &psBoxMaximum->y))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs left, top, right and bottom attributes.", NULL);
	}
	if ((psBoxMaximum->x <= psBoxMinimum->x) || (psBoxMaximum->y <= psBoxMinimum->y))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] right and bottom must be past left and top.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Most views make a component in the simulation.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::MakesComponent(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL unless the view joins its port views to those of other views of
// the same name.
//
//////////////////////////////////////////////////////////////////////////
const char* CComponentView::GetConnectionName(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the bits the view joins; most join none.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::GetBitJoins(CArrayPortBitJoin* pasJoins)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The next unit of a part placed as its units (see CInverterUnitView),
// to be placed after this one.  NULL for anything else, or after the
// part's last unit.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CComponentView::NewNextUnit(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL unless the view is an instance of a subcircuit.
//
//////////////////////////////////////////////////////////////////////////
CCircuitInstanceView* CComponentView::GetCircuitInstanceView(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Draws the shapes and then the ports over them.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::Paint(CCircuitPainter* pcPainter)
{
	pcPainter->SetTransform(msPosition, meRotation, mbMirrored);
	PaintShapes(pcPainter);
	PaintPorts(pcPainter);
	pcPainter->ClearTransform();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::PaintShapes(CCircuitPainter* pcPainter)
{
	mcShapes.Paint(pcPainter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::PaintPorts(CCircuitPainter* pcPainter)
{
	size	i;

	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		mapcPortViews.GetPtr(i)->Paint(pcPainter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The subcircuit simulation to make the component in, as Logicim's
// validateCanCreateComponent and getSubcircuitSimulation.  NULL if the
// view already has a component for the path and circuit simulation or the
// path isn't simulated in it.
//
//////////////////////////////////////////////////////////////////////////
CSubcircuitSimulation* CComponentView::CanCreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	CSubcircuitSimulation*	pcSimulation;
	CChars					sz;

	if (FindComponent(pcViewPath, pcCircuitSimulation, NULL) != NULL)
	{
		gcLogger.Error2(__METHOD__, " [", GetType(), "] [", mszName.Text(), "] already has a component for this path and simulation.", NULL);
		return NULL;
	}
	pcSimulation = pcViewPath->GetSubcircuitSimulation(pcCircuitSimulation);
	if (pcSimulation == NULL)
	{
		sz.Init();
		pcViewPath->AppendDescription(&sz);
		gcLogger.Error2(__METHOD__, " Path [", sz.Text(), "] isn't simulated in circuit simulation [", pcCircuitSimulation->GetName(), "].", NULL);
		sz.Kill();
	}
	return pcSimulation;
}


//////////////////////////////////////////////////////////////////////////
//
// Remembers the component for the path and circuit simulation, with the
// supplies powering it implicitly (papcSupplies may be NULL), and gives
// each port view its ports, as Logicim's postCreateComponent.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::PostCreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CCircuitComponent* pcComponent, CArrayCircuitComponentPtr* papcSupplies)
{
	size					i;
	bool					bResult;
	SViewPathComponent*		psComponent;

	psComponent = masComponents.Add();
	psComponent->pcViewPath = pcViewPath;
	psComponent->pcCircuitSimulation = pcCircuitSimulation;
	psComponent->pcComponent = pcComponent;
	psComponent->apcSupplies.Init();
	if (papcSupplies != NULL)
	{
		for (i = 0; i < papcSupplies->NumElements(); i++)
		{
			psComponent->apcSupplies.Add(papcSupplies->GetPtr(i));
		}
	}

	bResult = true;
	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		bResult &= mapcPortViews.GetPtr(i)->Connect(pcViewPath, pcCircuitSimulation, pcComponent);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the component, which its simulation still owns.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::DisconnectComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	size					i;
	size					uiIndex;
	SViewPathComponent*		psComponent;

	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		mapcPortViews.GetPtr(i)->Disconnect(pcViewPath, pcCircuitSimulation);
	}
	psComponent = FindComponent(pcViewPath, pcCircuitSimulation, &uiIndex);
	if (psComponent != NULL)
	{
		psComponent->apcSupplies.Kill();
		masComponents.RemoveAt(uiIndex, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the component, and the supplies powering it implicitly, out of
// the running simulation, as Logicim's
// disconnectViewAndDestroyComponents.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::DestroyComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	SViewPathComponent*			psComponent;
	CCircuitComponent*			pcComponent;
	CArrayCircuitComponentPtr	apcSupplies;
	size						i;

	psComponent = FindComponent(pcViewPath, pcCircuitSimulation, NULL);
	if (psComponent == NULL)
	{
		return;
	}

	pcComponent = psComponent->pcComponent;
	apcSupplies.Init();
	for (i = 0; i < psComponent->apcSupplies.NumElements(); i++)
	{
		apcSupplies.Add(psComponent->apcSupplies.GetPtr(i));
	}
	DisconnectComponent(pcViewPath, pcCircuitSimulation);

	DestroySimulatedComponent(pcComponent, pcCircuitSimulation);
	for (i = 0; i < apcSupplies.NumElements(); i++)
	{
		DestroySimulatedComponent(apcSupplies.GetPtr(i), pcCircuitSimulation);
	}
	apcSupplies.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Its events go, the ports on its traces see its ports go, and it's
// removed from the circuit, which frees it.
//
//////////////////////////////////////////////////////////////////////////
void DestroySimulatedComponent(CCircuitComponent* pcComponent, CCircuitSimulation* pcCircuitSimulation)
{
	CTimeline*	pcTimeline;

	pcTimeline = pcCircuitSimulation->GetTimeline();
	pcComponent->Disconnect(pcTimeline);
	pcComponent->DisconnectPorts(pcTimeline);
	pcCircuitSimulation->GetCircuit()->Remove(pcComponent);
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the view has no component for the path and circuit simulation.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CComponentView::GetComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation)
{
	SViewPathComponent*		psComponent;

	psComponent = FindComponent(pcViewPath, pcCircuitSimulation, NULL);
	if (psComponent == NULL)
	{
		return NULL;
	}
	return psComponent->pcComponent;
}


//////////////////////////////////////////////////////////////////////////
//
// The component of the path and circuit simulation being drawn.
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CComponentView::GetComponent(CCircuitPainter* pcPainter)
{
	return GetComponent(pcPainter->GetViewPath(), pcPainter->GetCircuitSimulation());
}


//////////////////////////////////////////////////////////////////////////
//
// How many paths and circuit simulations the view has components for.
//
//////////////////////////////////////////////////////////////////////////
size CComponentView::NumComponents(void)
{
	return masComponents.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SViewPathComponent* CComponentView::FindComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, size* puiIndex)
{
	return FindViewPathSimulation(&masComponents, pcViewPath, pcCircuitSimulation, puiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SetPosition(int32 x, int32 y)
{
	msPosition.Init(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SInt32Vec2 CComponentView::GetPosition(void)
{
	return msPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SetRotation(ERotation eRotation)
{
	meRotation = eRotation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ERotation CComponentView::GetRotation(void)
{
	return meRotation;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SetMirrored(bool bMirrored)
{
	mbMirrored = bMirrored;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::IsMirrored(void)
{
	return mbMirrored;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CComponentView::GetName(void)
{
	return mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponentShapes* CComponentView::GetShapes(void)
{
	return &mcShapes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortView* CComponentView::AddPortView(const char* szPortName, int32 x, int32 y)
{
	CPortView*	pcPortView;

	pcPortView = NewMalloc<CPortView>();
	pcPortView->Init(this, szPortName, x, y);
	mapcPortViews.Add(pcPortView);
	return pcPortView;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortView* CComponentView::AddPortView(CArrayChars* paszPortNames, int32 x, int32 y)
{
	CPortView*	pcPortView;

	pcPortView = NewMalloc<CPortView>();
	pcPortView->Init(this, paszPortNames, x, y);
	mapcPortViews.Add(pcPortView);
	return pcPortView;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CComponentView::NumPortViews(void)
{
	return mapcPortViews.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortView* CComponentView::GetPortView(size uiIndex)
{
	return mapcPortViews.SafeGetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// The port view holding the named port.
//
//////////////////////////////////////////////////////////////////////////
CPortView* CComponentView::GetPortView(const char* szPortName)
{
	size		i;
	size		j;
	CPortView*	pcPortView;

	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		pcPortView = mapcPortViews.GetPtr(i);
		for (j = 0; j < pcPortView->NumPortNames(); j++)
		{
			if (StringCompare(pcPortView->GetPortName(j), szPortName) == 0)
			{
				return pcPortView;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The box around the shapes and ports, facing East about the position.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::CalculateBounds(void)
{
	size	i;

	mbHasBounds = mcShapes.IncludeBounds(&msBoundsMinimum, &msBoundsMaximum, false);
	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		mbHasBounds = mapcPortViews.GetPtr(i)->IncludeBounds(&msBoundsMinimum, &msBoundsMaximum, mbHasBounds);
	}
	mbHasBounds = IncludeExtraBounds(&msBoundsMinimum, &msBoundsMaximum, mbHasBounds);
}


//////////////////////////////////////////////////////////////////////////
//
// For subclasses whose box reaches past their shapes.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds)
{
	return bHasBounds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::GetLocalBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	*psMinimum = msBoundsMinimum;
	*psMaximum = msBoundsMaximum;
	return mbHasBounds;
}


//////////////////////////////////////////////////////////////////////////
//
// In the circuit's grid space, mirrored, turned and moved.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::GetBoundingBox(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum)
{
	SFloat64Vec2	sCorner;

	if (!mbHasBounds)
	{
		return false;
	}

	sCorner = msBoundsMinimum;
	TransformPoint(meRotation, mbMirrored, &sCorner);
	psMinimum->Init(sCorner.x, sCorner.y);
	psMaximum->Init(sCorner.x, sCorner.y);
	sCorner = msBoundsMaximum;
	TransformPoint(meRotation, mbMirrored, &sCorner);
	IncludeBoundsPoint(psMinimum, psMaximum, sCorner.x, sCorner.y);

	psMinimum->x += msPosition.x;
	psMinimum->y += msPosition.y;
	psMaximum->x += msPosition.x;
	psMaximum->y += msPosition.y;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its name, the property every component has, as Logicim's
// ComponentProperties.  Views add theirs after it.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	pcProperties->AddText("name", mszName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the view, which must be new, from its properties, as
// Logicim's ViewFactory makes a view from new properties.  Fails for a view
// with no properties of its own.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	return false;
}
