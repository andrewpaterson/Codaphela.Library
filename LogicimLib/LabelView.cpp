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
#include "BaseLib/Logger.h"
#include "LabelView.h"


//////////////////////////////////////////////////////////////////////////
//
// A new label, its box measured to fit its text.
//
//////////////////////////////////////////////////////////////////////////
void CLabelView::Init(int32 x, int32 y, ERotation eRotation, const char* szText, ETextAlignment eAlignment, bool bBold, bool bFill, bool bBorder, CCircuitFonts* pcFonts)
{
	SFloat64Vec2	sBoxMinimum;
	SFloat64Vec2	sBoxMaximum;

	MeasureBox(pcFonts, szText, eAlignment, bBold, &sBoxMinimum, &sBoxMaximum);
	Init(x, y, eRotation, szText, eAlignment, bBold, bFill, bBorder, sBoxMinimum, sBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabelView::Init(int32 x, int32 y, ERotation eRotation, const char* szText, ETextAlignment eAlignment, bool bBold, bool bFill, bool bBorder, SFloat64Vec2 sBoxMinimum, SFloat64Vec2 sBoxMaximum)
{
	CComponentView::Init(x, y, eRotation, szText);
	meAlignment = eAlignment;
	mbBold = bBold;
	mbFill = bFill;
	mbBorder = bBorder;
	msBoxMinimum = sBoxMinimum;
	msBoxMaximum = sBoxMaximum;

	if (mbFill || mbBorder)
	{
		mcShapes.AddRectangle(msBoxMinimum.x, msBoxMinimum.y, msBoxMaximum.x, msBoxMaximum.y, mbFill, mbBorder);
	}
	mcShapes.AddText(0, 0, szText, mbBold ? CF_SansSerifBold : CF_SansSerif, LABEL_FONT_SIZE, meAlignment);
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's TextView.calculateTopLeft and calculateBottomRight: the
// text's box, a quarter of its height longer at each end.
//
//////////////////////////////////////////////////////////////////////////
void CLabelView::MeasureBox(CCircuitFonts* pcFonts, const char* szText, ETextAlignment eAlignment, bool bBold, SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum)
{
	SCircuitTextSize	sSize;
	float64				fAdjust;

	sSize = pcFonts->Measure(bBold ? CF_SansSerifBold : CF_SansSerif, LABEL_FONT_SIZE, szText);
	if (eAlignment == TA_Centre)
	{
		fAdjust = -sSize.fWidth / 2;
	}
	else if (eAlignment == TA_Right)
	{
		fAdjust = -sSize.fWidth;
	}
	else
	{
		fAdjust = 0;
	}
	psBoxMinimum->Init(fAdjust - sSize.fHeight / 4, -sSize.fHeight / 2);
	psBoxMaximum->Init(sSize.fWidth + fAdjust + sSize.fHeight / 4, sSize.fHeight / 2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLabelView::GetType(void)
{
	return "Label";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLabelView::MakesComponent(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCircuitComponent* CLabelView::CreateComponent(CViewPath* pcViewPath, CCircuitSimulation* pcCircuitSimulation, CFamilyVoltageConfigurationStore* pcFamilies)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLabelView::GetXMLName(void)
{
	return "Label";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLabelView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CComponentView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("alignment", TextAlignmentToString(meAlignment));
	pcWriter->Attribute("bold", mbBold);
	pcWriter->Attribute("fill", mbFill);
	pcWriter->Attribute("border", mbBorder);
	SaveXMLBox(pcWriter, msBoxMinimum, msBoxMaximum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLabelView::LoadXML(CMarkupTag* pcTag)
{
	int32			x;
	int32			y;
	ERotation		eRotation;
	char*			szText;
	char*			szAlignment;
	ETextAlignment	eAlignment;
	bool			bBold;
	bool			bFill;
	bool			bBorder;
	SFloat64Vec2	sBoxMinimum;
	SFloat64Vec2	sBoxMaximum;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szText))
	{
		return false;
	}
	szAlignment = pcTag->GetStringAttribute("alignment");
	if (!StringToTextAlignment(szAlignment, &eAlignment))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] has no alignment or an unknown one [", (szAlignment != NULL) ? szAlignment : "", "].", NULL);
	}
	if (!pcTag->GetBoolAttribute("bold", &bBold) || !pcTag->GetBoolAttribute("fill", &bFill) || !pcTag->GetBoolAttribute("border", &bBorder))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs bold, fill and border to be true or false.", NULL);
	}
	if (!LoadXMLBox(pcTag, &sBoxMinimum, &sBoxMaximum))
	{
		return false;
	}
	Init(x, y, eRotation, szText, eAlignment, bBold, bFill, bBorder, sBoxMinimum, sBoxMaximum);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ETextAlignment CLabelView::GetAlignment(void) { return meAlignment; }
bool CLabelView::IsBold(void) { return mbBold; }
void CLabelView::GetBox(SFloat64Vec2* psBoxMinimum, SFloat64Vec2* psBoxMaximum) { *psBoxMinimum = msBoxMinimum; *psBoxMaximum = msBoxMaximum; }


//////////////////////////////////////////////////////////////////////////
//
// As Logicim's LabelProperties; its name is its text.
//
//////////////////////////////////////////////////////////////////////////
void CLabelView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	const char*		aszAlignments[] = { "Left", "Centre", "Right" };

	CComponentView::GetProperties(pcProperties, psContext);
	pcProperties->AddChoice("alignment", aszAlignments, 3, (size)meAlignment);
	pcProperties->AddBoolean("bold", mbBold);
	pcProperties->AddBoolean("fill", mbFill);
	pcProperties->AddBoolean("border", mbBorder);
}


//////////////////////////////////////////////////////////////////////////
//
// Measured again with the fonts, as its text may have changed.
//
//////////////////////////////////////////////////////////////////////////
bool CLabelView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), (ETextAlignment)pcProperties->GetChoice("alignment", (size)meAlignment), pcProperties->GetBoolean("bold", mbBold), pcProperties->GetBoolean("fill", mbFill), pcProperties->GetBoolean("border", mbBorder), psContext->pcFonts);
	return true;
}
