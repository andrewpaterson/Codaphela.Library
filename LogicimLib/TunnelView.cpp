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
#include <math.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Logger.h"
#include "TunnelView.h"


//////////////////////////////////////////////////////////////////////////
//
// A new tunnel, its box measured to fit its name.
//
//////////////////////////////////////////////////////////////////////////
void CTunnelView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, bool bDoubleSided, CCircuitFonts* pcFonts)
{
	SFloat64Vec2	sBoxMinimum;
	SFloat64Vec2	sBoxMaximum;

	MeasureBox(pcFonts, szName, bDoubleSided, &sBoxMinimum, &sBoxMaximum);
	Init(x, y, eRotation, szName, bDoubleSided, sBoxMinimum, sBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
// The shape comes from the box: it's as wide as the box, pointed at the
// position by half its height (less a little) and, double sided, pointed
// at the far end too.
//
//////////////////////////////////////////////////////////////////////////
void CTunnelView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, bool bDoubleSided, SFloat64Vec2 sBoxMinimum, SFloat64Vec2 sBoxMaximum)
{
	SFloat64Vec2	asPoints[6];
	float64			fHalf;
	float64			fPoint;
	float64			fFar;
	CArrayChars		aszNoPorts;
	CPortView*		pcPortView;

	CComponentView::Init(x, y, eRotation, szName);
	mbDoubleSided = bDoubleSided;
	msBoxMinimum = sBoxMinimum;
	msBoxMaximum = sBoxMaximum;
	mszConnectionName.Init(szName);
	mszConnectionName.StripWhitespace();

	fHalf = (msBoxMaximum.y - msBoxMinimum.y) / 2;
	fPoint = fHalf - 0.125;
	fFar = msBoxMaximum.x;
	asPoints[0].Init(0, 0);
	asPoints[1].Init(fPoint, -fHalf);
	if (!mbDoubleSided)
	{
		asPoints[2].Init(fFar, -fHalf);
		asPoints[3].Init(fFar, fHalf);
		asPoints[4].Init(fPoint, fHalf);
		mcShapes.AddPolygon(asPoints, 5, true, true);
	}
	else
	{
		asPoints[2].Init(fFar - fPoint, -fHalf);
		asPoints[3].Init(fFar, 0);
		asPoints[4].Init(fFar - fPoint, fHalf);
		asPoints[5].Init(fPoint, fHalf);
		mcShapes.AddPolygon(asPoints, 6, true, true);
	}
	mcShapes.AddText(0.5, 0, szName, CF_SansSerifBold, TUNNEL_FONT_SIZE, TA_Left);

	aszNoPorts.Init();
	pcPortView = AddPortView(&aszNoPorts, 0, 0);
	pcPortView->SetDrawn(false);
	if (mbDoubleSided)
	{
		pcPortView = AddPortView(&aszNoPorts, -(int32)floor(0.5 - fFar), 0);
		pcPortView->SetDrawn(false);
	}
	aszNoPorts.Kill();
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTunnelView::Kill(void)
{
	mszConnectionName.Kill();
	CComponentView::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's TunnelView.createGraphics: the name in bold 11, pointed at
// each end by half its height less a quarter of a grid unit.  A double
// sided tunnel is half a grid unit longer and its far end is rounded to a
// whole grid unit.
//
//////////////////////////////////////////////////////////////////////////
void CTunnelView::MeasureBox(CCircuitFonts* pcFonts, const char* szName, bool bDoubleSided, SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum)
{
	SCircuitTextSize	sSize;
	float64				fPoint;
	float64				fLength;

	sSize = pcFonts->Measure(CF_SansSerifBold, TUNNEL_FONT_SIZE, szName);
	fPoint = (sSize.fHeight - 0.25) / 2;
	if (!bDoubleSided)
	{
		fLength = sSize.fWidth + fPoint + 0.4;
	}
	else
	{
		fLength = floor(sSize.fWidth + fPoint + fPoint + 0.5 + 0.5);
	}
	psBoxMinimum->Init(0, -sSize.fHeight / 2);
	psBoxMaximum->Init(fLength, sSize.fHeight / 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTunnelView::GetType(void)
{
	return "Tunnel";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTunnelView::MakesComponent(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CTunnelView::CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The name without the spaces around it.  Names are case sensitive.
//
//////////////////////////////////////////////////////////////////////////
const char* CTunnelView::GetConnectionName(void)
{
	return mszConnectionName.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CTunnelView::GetXMLName(void)
{
	return "Tunnel";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTunnelView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CComponentView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("doubleSided", mbDoubleSided);
	SaveXMLBox(pcWriter, msBoxMinimum, msBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTunnelView::LoadXML(CMarkupTag* pcTag)
{
	int32			x;
	int32			y;
	ERotation		eRotation;
	char*			szName;
	bool			bDoubleSided;
	SFloat64Vec2	sBoxMinimum;
	SFloat64Vec2	sBoxMaximum;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName))
	{
		return false;
	}
	if (!pcTag->GetBoolAttribute("doubleSided", &bDoubleSided))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] doubleSided must be true or false.", NULL);
	}
	if (!LoadXMLBox(pcTag, &sBoxMinimum, &sBoxMaximum))
	{
		return false;
	}
	Init(x, y, eRotation, szName, bDoubleSided, sBoxMinimum, sBoxMaximum);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTunnelView::IsDoubleSided(void) { return mbDoubleSided; }
void CTunnelView::GetBox(SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum) { *psBoxMinimum = msBoxMinimum; *psBoxMaximum = msBoxMaximum; }
