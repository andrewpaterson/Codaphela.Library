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
#include "BaseLib/StringHelper.h"
#include "BaseLib/LogString.h"
#include "MicroprocessorLib/Buffer.h"
#include "InverterPackageView.h"


//The box's edges, the ports either side of it and where the inverter in each row is.
#define INVERTER_PACKAGE_LEFT			-4
#define INVERTER_PACKAGE_RIGHT			5
#define INVERTER_PACKAGE_INPUT_X		-5
#define INVERTER_PACKAGE_OUTPUT_X		6
#define INVERTER_PACKAGE_MARGIN			3


//////////////////////////////////////////////////////////////////////////
//
// szPartType is the part's type after its family ("04" for a 74LVC04).
// Without a description it's named for its count and type ("Hex
// Inverter", "Hex Schmitt Inverter" for a 14).
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType, int iUnitCount, const char* szDescription)
{
	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	mszPartType.Init(szPartType);
	if ((iUnitCount < 1) || (iUnitCount > INVERTER_PACKAGE_MAX_UNITS))
	{
		iUnitCount = 1;
	}
	miUnitCount = iUnitCount;
	mszDescription.Init();
	if (StrEmpty(szDescription))
	{
		GetDescription(&mszDescription, szPartType, iUnitCount);
	}
	else
	{
		mszDescription.Set(szDescription);
	}

	CreateShapes();
	CreatePowerPortViews();
	CreatePackagePortViews();
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::Kill(void)
{
	mszDescription.Kill();
	mszPartType.Kill();
	CIntegratedCircuitView::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The box, then each row's inverter (as an inverter view draws it) with
// lines out to its ports, then the description and part number.
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::CreateShapes(void)
{
	int			i;
	float64		fY;
	float64		fTop;
	float64		fBottom;
	CChars		szPartNumber;

	fTop = GetUnitY(0, miUnitCount) - INVERTER_PACKAGE_MARGIN;
	fBottom = GetUnitY(miUnitCount - 1, miUnitCount) + INVERTER_PACKAGE_MARGIN;
	mcShapes.AddRectangle(INVERTER_PACKAGE_LEFT, fTop, INVERTER_PACKAGE_RIGHT, fBottom, true, true);
	for (i = 0; i < miUnitCount; i++)
	{
		fY = GetUnitY(i, miUnitCount);
		mcShapes.AddLine(INVERTER_PACKAGE_INPUT_X, fY, -1, fY);
		mcShapes.AddTriangle(0.9, fY, -1, fY + 0.75, -1, fY - 0.75, true, true);
		mcShapes.AddCircle(1.5, fY, PORT_BUBBLE_DIAMETER / 2, false, true);
		mcShapes.AddLine(1.5 + PORT_BUBBLE_DIAMETER / 2, fY, INVERTER_PACKAGE_OUTPUT_X, fY);
	}

	mcShapes.AddText((INVERTER_PACKAGE_LEFT + INVERTER_PACKAGE_RIGHT) / 2.0, fTop + 1.2, mszDescription.Text(), CF_SansSerif, INVERTER_PACKAGE_FONT_SIZE, TA_Centre);
	szPartNumber.Init();
	GetPartNumber(&szPartNumber);
	mcShapes.AddText((INVERTER_PACKAGE_LEFT + INVERTER_PACKAGE_RIGHT) / 2.0, fBottom - 1.2, szPartNumber.Text(), CF_SansSerifBold, INVERTER_PACKAGE_FONT_SIZE, TA_Centre);
	szPartNumber.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// All the inputs and then all the outputs, as an inverter view's.
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::CreatePackagePortViews(void)
{
	int				i;
	CArrayChars		aszNames;

	for (i = 0; i < miUnitCount; i++)
	{
		aszNames.Init();
		CLogicGateView::GetPortNames(&aszNames, "Input", i, 1, miUnitCount);
		AddPortView(&aszNames, INVERTER_PACKAGE_INPUT_X, GetUnitY(i, miUnitCount));
		aszNames.Kill();
	}
	for (i = 0; i < miUnitCount; i++)
	{
		aszNames.Init();
		CLogicGateView::GetPortNames(&aszNames, "Output", i, 1, miUnitCount);
		AddPortView(&aszNames, INVERTER_PACKAGE_OUTPUT_X, GetUnitY(i, miUnitCount));
		aszNames.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Rows two grid units apart about the position, the first at the top.
//
//////////////////////////////////////////////////////////////////////////
int32 CInverterPackageView::GetUnitY(int iUnit, int iUnitCount)
{
	return 2 * iUnit - (iUnitCount - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CInverterPackageView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	CInverter*		pcInverter;

	pcInverter = &OMalloc<CInverter>(pcSimulation, mszName.Text(), OMalloc<CBufferPins>(miUnitCount, pcFamily));
	return pcInverter;
}


//////////////////////////////////////////////////////////////////////////
//
// 74, the family and the type: "74LVC04".
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::GetPartNumber(CChars* psz, const char* szFamily, const char* szPartType)
{
	psz->Append("74");
	psz->Append(szFamily);
	psz->Append(szPartType);
}


//////////////////////////////////////////////////////////////////////////
//
// "Hex Inverter" for six, "Dual" for two and so on; a 14 (or 1G14, 2G14)
// is a Schmitt inverter.
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::GetDescription(CChars* psz, const char* szPartType, int iUnitCount)
{
	const char*		aszCounts[] = { "", "Single", "Dual", "Triple", "Quad", "", "Hex", "", "Octal" };
	size			uiLength;

	if ((iUnitCount >= 1) && (iUnitCount <= 8) && !StrEmpty(aszCounts[iUnitCount]))
	{
		psz->Append(aszCounts[iUnitCount]);
		psz->Append(' ');
	}
	uiLength = StrLen(szPartType);
	if ((uiLength >= 2) && (szPartType[uiLength - 2] == '1') && (szPartType[uiLength - 1] == '4'))
	{
		psz->Append("Schmitt ");
	}
	psz->Append("Inverter");
}


//////////////////////////////////////////////////////////////////////////
//
// How many inverters a part of the type has: the count before a small
// part's G ("2G04" is two), otherwise six (an 04, 05, 06 or 14).
//
//////////////////////////////////////////////////////////////////////////
int CInverterPackageView::GetPartUnitCount(const char* szPartType)
{
	if ((szPartType != NULL) && (szPartType[0] >= '1') && (szPartType[0] <= '9') && (szPartType[1] == 'G'))
	{
		return szPartType[0] - '0';
	}
	return 6;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::GetPartNumber(CChars* psz)
{
	GetPartNumber(psz, mszFamily.Text(), mszPartType.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortView* CInverterPackageView::GetUnitInput(int iUnit)
{
	if ((iUnit < 0) || (iUnit >= miUnitCount))
	{
		return NULL;
	}
	return GetPortView((size)(NumPortViews() - 2 * miUnitCount + iUnit));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPortView* CInverterPackageView::GetUnitOutput(int iUnit)
{
	if ((iUnit < 0) || (iUnit >= miUnitCount))
	{
		return NULL;
	}
	return GetPortView((size)(NumPortViews() - miUnitCount + iUnit));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CInverterPackageView::GetType(void)
{
	return "Inverter Package";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CInverterPackageView::GetXMLName(void)
{
	return "InverterPackage";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CIntegratedCircuitView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("partType", mszPartType.Text());
	pcWriter->Attribute("unitCount", (int32)miUnitCount);
	pcWriter->Attribute("description", mszDescription.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInverterPackageView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	char*		szPartType;
	int32		iUnitCount;
	char*		szDescription;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	szPartType = pcTag->GetStringAttribute("partType");
	if (StrEmpty(szPartType))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a partType attribute.", NULL);
	}
	if (!pcTag->GetInt32Attribute("unitCount", &iUnitCount) || (iUnitCount < 1) || (iUnitCount > INVERTER_PACKAGE_MAX_UNITS))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a unitCount from 1 to ", IntToString(INVERTER_PACKAGE_MAX_UNITS), ".", NULL);
	}
	szDescription = pcTag->GetStringAttribute("description");
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, szPartType, iUnitCount, szDescription);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CInverterPackageView::GetPartType(void) { return mszPartType.Text(); }
char* CInverterPackageView::GetDescription(void) { return mszDescription.Text(); }
int CInverterPackageView::GetUnitCount(void) { return miUnitCount; }


//////////////////////////////////////////////////////////////////////////
//
// iUnit is from 0 (Unit A).
//
//////////////////////////////////////////////////////////////////////////
void CInverterUnitView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType, int iUnit)
{
	CChars	szUnit;

	InitBuffer(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, 1, 1, true);
	mszPartType.Init(szPartType);
	if ((iUnit < 0) || (iUnit >= INVERTER_PACKAGE_MAX_UNITS))
	{
		iUnit = 0;
	}
	miUnit = iUnit;

	szUnit.Init("Unit ");
	szUnit.Append(GetUnitLetter());
	mcShapes.AddText(0.5, -1.5, szUnit.Text(), CF_SansSerif, INVERTER_UNIT_FONT_SIZE, TA_Centre);
	szUnit.Kill();
	mcShapes.AddText(0.5, 1.5, szFamily, CF_SansSerif, INVERTER_UNIT_FONT_SIZE, TA_Centre);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInverterUnitView::Kill(void)
{
	mszPartType.Kill();
	CInverterView::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CInverterUnitView::GetUnitLetter(void)
{
	return (char)('A' + miUnit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CInverterUnitView::GetType(void)
{
	return "Inverter Unit";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CInverterUnitView::GetXMLName(void)
{
	return "InverterUnit";
}


//////////////////////////////////////////////////////////////////////////
//
// As an inverter's, with the part's type and the unit's letter.
//
//////////////////////////////////////////////////////////////////////////
void CInverterUnitView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CChars	szUnit;

	CIntegratedCircuitView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("partType", mszPartType.Text());
	szUnit.Init();
	szUnit.Append(GetUnitLetter());
	pcWriter->Attribute("unit", szUnit.Text());
	szUnit.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInverterUnitView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	char*		szPartType;
	char*		szUnit;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	szPartType = pcTag->GetStringAttribute("partType");
	if (StrEmpty(szPartType))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a partType attribute.", NULL);
	}
	szUnit = pcTag->GetStringAttribute("unit");
	if ((szUnit == NULL) || (szUnit[0] < 'A') || (szUnit[0] >= 'A' + INVERTER_PACKAGE_MAX_UNITS) || (szUnit[1] != '\0'))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a unit letter.", NULL);
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, szPartType, szUnit[0] - 'A');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CInverterUnitView::GetPartType(void) { return mszPartType.Text(); }
int CInverterUnitView::GetUnitCount(void) { return CInverterPackageView::GetPartUnitCount(mszPartType.Text()); }


//////////////////////////////////////////////////////////////////////////
//
// The part's next unit where this one is, named, turned and of the family
// as it is.  NULL after the part's last unit.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CInverterUnitView::NewNextUnit(void)
{
	CInverterUnitView*	pcNext;

	if (miUnit + 1 >= GetUnitCount())
	{
		return NULL;
	}
	pcNext = NewMalloc<CInverterUnitView>();
	pcNext->Init(msPosition.x, msPosition.y, meRotation, mszName.Text(), mszFamily.Text(), mbExplicitPowerPorts, mszPartType.Text(), miUnit + 1);
	return pcNext;
}
int CInverterUnitView::GetUnit(void) { return miUnit; }


//////////////////////////////////////////////////////////////////////////
//
// Its part's type ("04"), how many inverters it has and its description,
// after its name and power.
//
//////////////////////////////////////////////////////////////////////////
void CInverterPackageView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	CComponentView::GetProperties(pcProperties, psContext);
	AddPowerProperties(pcProperties, psContext);
	pcProperties->AddText("partType", mszPartType.Text());
	pcProperties->AddInteger("unitCount", miUnitCount, 1, 26);
	pcProperties->AddText("description", mszDescription.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInverterPackageView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), pcProperties->GetText("partType", ""), (int)pcProperties->GetInteger("unitCount", miUnitCount), pcProperties->GetText("description", ""));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its part's type and which of its units it is, after its name and power.
//
//////////////////////////////////////////////////////////////////////////
void CInverterUnitView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	const char*		aszUnits[] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };

	CComponentView::GetProperties(pcProperties, psContext);
	AddPowerProperties(pcProperties, psContext);
	pcProperties->AddText("partType", mszPartType.Text());
	pcProperties->AddChoice("unit", aszUnits, 26, (size)miUnit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInverterUnitView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), pcProperties->GetText("partType", ""), (int)pcProperties->GetChoice("unit", (size)miUnit));
	return true;
}
