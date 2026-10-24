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
	mszName.Init(szName);
	mcShapes.Init();
	mapcPortViews.Init();
	msBoundsMinimum.Init(0, 0);
	msBoundsMaximum.Init(0, 0);
	mbHasBounds = false;
	mpcComponent = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The component belongs to its simulation and isn't freed.
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
	mpcComponent = NULL;
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
// Subclasses add their properties after these.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	pcWriter->Attribute("name", mszName.Text());
	pcWriter->Attribute("x", msPosition.x);
	pcWriter->Attribute("y", msPosition.y);
	pcWriter->Attribute("rotation", RotationToString(meRotation));
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
// Draws the shapes and then the ports over them.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::Paint(CCircuitPainter* pcPainter)
{
	pcPainter->SetTransform(msPosition, meRotation);
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
// Remembers the component and gives each port view its ports.
//
//////////////////////////////////////////////////////////////////////////
bool CComponentView::ConnectPortViews(CCircuitComponent* pcComponent)
{
	size	i;
	bool	bResult;

	mpcComponent = pcComponent;
	bResult = true;
	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		bResult &= mapcPortViews.GetPtr(i)->Connect(pcComponent);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Forgets the component, which its simulation still owns.
//
//////////////////////////////////////////////////////////////////////////
void CComponentView::DisconnectComponent(void)
{
	size	i;

	for (i = 0; i < mapcPortViews.NumElements(); i++)
	{
		mapcPortViews.GetPtr(i)->Disconnect();
	}
	mpcComponent = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CComponentView::GetComponent(void)
{
	return mpcComponent;
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
// The box around the shapes and ports, facing North about the position.
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
// In the circuit's grid space, turned and moved.
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
	RotatePoint(meRotation, &sCorner);
	psMinimum->Init(sCorner.x, sCorner.y);
	psMaximum->Init(sCorner.x, sCorner.y);
	sCorner = msBoundsMaximum;
	RotatePoint(meRotation, &sCorner);
	IncludeBoundsPoint(psMinimum, psMaximum, sCorner.x, sCorner.y);

	psMinimum->x += msPosition.x;
	psMinimum->y += msPosition.y;
	psMaximum->x += msPosition.x;
	psMaximum->y += msPosition.y;
	return true;
}

