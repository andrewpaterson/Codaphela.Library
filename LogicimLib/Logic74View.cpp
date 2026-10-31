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
#include "MicroprocessorLib/AndGate.h"
#include "MicroprocessorLib/OrGate.h"
#include "MicroprocessorLib/XorGate.h"
#include "MicroprocessorLib/Buffer.h"
#include "MicroprocessorLib/DTypeFlipFlop.h"
#include "AndGateView.h"
#include "OrGateView.h"
#include "BufferView.h"
#include "Logic74View.h"


//The box's edges, the ports either side of it and the room above and below its gates for its titles.
#define LOGIC_74_LEFT				-4
#define LOGIC_74_RIGHT				5
#define LOGIC_74_INPUT_X			-5
#define LOGIC_74_OUTPUT_X			6
#define LOGIC_74_MARGIN				2.5


//////////////////////////////////////////////////////////////////////////
//
// Half the height of a gate as its view draws it: a buffer's triangle or
// a gate's body.
//
//////////////////////////////////////////////////////////////////////////
static float64 GetLogic74GateHalfHeight(EGateFunction eFunction, int iInputCount)
{
	if (CGateArray::IsSingleInput(eFunction))
	{
		return 0.75;
	}
	return CLogicGateView::CalculateWidth(iInputCount);
}


//////////////////////////////////////////////////////////////////////////
//
// Where a gate's inputs are, from its position: a grid unit back, or two
// for an exclusive OR.
//
//////////////////////////////////////////////////////////////////////////
static int32 GetLogic74GateInputX(EGateFunction eFunction)
{
	if ((eFunction == GF_Xor) || (eFunction == GF_Xnor))
	{
		return -2;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Where a gate's output is, from its position, before any bubble.
//
//////////////////////////////////////////////////////////////////////////
static int32 GetLogic74GateOutputX(EGateFunction eFunction)
{
	if (CGateArray::IsSingleInput(eFunction))
	{
		return 1;
	}
	return 2;
}


//////////////////////////////////////////////////////////////////////////
//
// The gate's body (not its ports) with its position at (x, y).
//
//////////////////////////////////////////////////////////////////////////
static void AddLogic74GateBody(CComponentShapes* pcShapes, EGateFunction eFunction, float64 x, float64 y, int iInputCount)
{
	switch (eFunction)
	{
	case GF_And:
	case GF_Nand:
		CAndGateView::AddAndShapes(pcShapes, x, y, iInputCount);
		break;
	case GF_Or:
	case GF_Nor:
		COrGateView::AddOrShapes(pcShapes, x, y, iInputCount, false);
		break;
	case GF_Xor:
	case GF_Xnor:
		COrGateView::AddOrShapes(pcShapes, x, y, iInputCount, true);
		break;
	case GF_Buffer:
	case GF_Inverter:
		CBufferView::AddBufferShape(pcShapes, x, y, 0.75);
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A part type with no row is kept, so it's saved as it was, but draws an
// empty box.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType)
{
	CIntegratedCircuitView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts);
	mszPartType.Init(szPartType);
	FindLogic74Part(&msType, szPartType);

	CLogicBlockView::GetDefaultParams(LBK_Register, &msParams);
	meKind = LBK_Register;

	CreatePowerPortViews();
	if (msType.psPart == NULL)
	{
		mcShapes.AddRectangle(LOGIC_74_LEFT, -LOGIC_74_MARGIN, LOGIC_74_RIGHT, LOGIC_74_MARGIN, true, true);
		CreateTitles(-LOGIC_74_MARGIN, LOGIC_74_MARGIN);
	}
	else if (msType.psPart->eKind == L74K_Block)
	{
		meKind = msType.psPart->eBlock;
		msParams = msType.psPart->sParams;
		CreatePartBlock();
	}
	else
	{
		CreateGates();
	}
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::Kill(void)
{
	mszPartType.Kill();
	CLogicBlockView::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// How far apart the gates are, so each gate's inputs (and its bubble) are
// clear of the next's: two grid units for buffers.
//
//////////////////////////////////////////////////////////////////////////
int32 CLogic74PackageView::GetGatePitch(int iInputCount)
{
	if (iInputCount <= 1)
	{
		return 2;
	}
	return 2 * (int32)CLogicGateView::CalculateWidth(iInputCount) + 2;
}


//////////////////////////////////////////////////////////////////////////
//
// The gates one above the other about the position, the first at the top,
// all the inputs' ports and then all the outputs'.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::CreateGates(void)
{
	SLogic74Part*	psPart;
	int				iUnitCount;
	int32			iPitch;
	int32			iFirst;
	int				i;
	size			j;
	CArrayInt		aiOffsets;
	CChars			szName;
	float64			fHalfHeight;
	float64			fTop;
	float64			fBottom;

	psPart = msType.psPart;
	iUnitCount = msType.iUnitCount;
	iPitch = GetGatePitch(psPart->iInputCount);
	iFirst = -((iUnitCount - 1) * iPitch) / 2;
	fHalfHeight = CGateArray::IsSingleInput(psPart->eGateFunction) ? 0.5 : CLogicGateView::CalculateWidth(psPart->iInputCount);
	fTop = iFirst - fHalfHeight - LOGIC_74_MARGIN;
	fBottom = iFirst + (iUnitCount - 1) * iPitch + fHalfHeight + LOGIC_74_MARGIN;

	mcShapes.AddRectangle(LOGIC_74_LEFT, fTop, LOGIC_74_RIGHT, fBottom, true, true);
	for (i = 0; i < iUnitCount; i++)
	{
		AddGateShapes(psPart->eGateFunction, iFirst + i * iPitch, psPart->iInputCount, LOGIC_74_INPUT_X, LOGIC_74_OUTPUT_X);
	}

	aiOffsets.Init();
	CLogicGateView::CalculatePortOffsets(&aiOffsets, psPart->iInputCount);
	for (i = 0; i < iUnitCount; i++)
	{
		for (j = 0; j < aiOffsets.NumElements(); j++)
		{
			szName.Init();
			CGateArrayPins::GetInputName(&szName, i, j);
			AddPortView(szName.Text(), LOGIC_74_INPUT_X, iFirst + i * iPitch + aiOffsets.GetValue(j));
			szName.Kill();
		}
	}
	aiOffsets.Kill();
	for (i = 0; i < iUnitCount; i++)
	{
		szName.Init();
		CGateArrayPins::GetOutputName(&szName, i);
		AddPortView(szName.Text(), LOGIC_74_OUTPUT_X, iFirst + i * iPitch);
		szName.Kill();
	}

	CreateTitles(fTop, fBottom);
}


//////////////////////////////////////////////////////////////////////////
//
// A gate at y with lines from its inputs back to iInputX and from its
// output (or its bubble) on to iOutputX.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::AddGateShapes(EGateFunction eFunction, float64 y, int iInputCount, int32 iInputX, int32 iOutputX)
{
	CArrayInt	aiOffsets;
	size		i;
	float64		fOutputX;

	AddLogic74GateBody(&mcShapes, eFunction, 0, y, iInputCount);

	aiOffsets.Init();
	CLogicGateView::CalculatePortOffsets(&aiOffsets, iInputCount);
	for (i = 0; i < aiOffsets.NumElements(); i++)
	{
		mcShapes.AddLine(iInputX, y + aiOffsets.GetValue(i), GetLogic74GateInputX(eFunction), y + aiOffsets.GetValue(i));
	}
	aiOffsets.Kill();

	fOutputX = GetLogic74GateOutputX(eFunction);
	if (CGateArray::IsNegated(eFunction))
	{
		mcShapes.AddCircle(fOutputX + 0.5, y, PORT_BUBBLE_DIAMETER / 2, false, true);
		fOutputX += 0.5 + PORT_BUBBLE_DIAMETER / 2;
	}
	mcShapes.AddLine(fOutputX, y, iOutputX, y);
}


//////////////////////////////////////////////////////////////////////////
//
// The block of the part's generic chip with its description at the top
// and its part number at the bottom.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::CreatePartBlock(void)
{
	CChars	szDescription;
	CChars	szPartNumber;

	szDescription.Init();
	GetDescription(&szDescription);
	szPartNumber.Init();
	GetPartNumber(&szPartNumber);
	CreateBlock(szDescription.Text(), szPartNumber.Text());
	szPartNumber.Kill();
	szDescription.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The description inside the top of the box and the part number inside
// its bottom.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::CreateTitles(float64 fTop, float64 fBottom)
{
	CChars	sz;
	float64	fMiddle;

	fMiddle = (LOGIC_74_LEFT + LOGIC_74_RIGHT) / 2.0;
	if (msType.psPart != NULL)
	{
		sz.Init();
		GetDescription(&sz);
		mcShapes.AddText(fMiddle, fTop + 1.2, sz.Text(), CF_SansSerif, LOGIC_74_FONT_SIZE, TA_Centre);
		sz.Kill();
	}
	sz.Init();
	GetPartNumber(&sz);
	mcShapes.AddText(fMiddle, fBottom - 1.2, sz.Text(), CF_SansSerifBold, LOGIC_74_FONT_SIZE, TA_Centre);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CLogic74PackageView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	SLogic74Part*	psPart;

	psPart = msType.psPart;
	if (psPart == NULL)
	{
		gcLogger.Error2(__METHOD__, " There is no 74 series part [", mszPartType.Text(), "] to simulate [", mszName.Text(), "].", NULL);
		return NULL;
	}
	if (psPart->eKind == L74K_Block)
	{
		return CreateChip(meKind, &msParams, pcSimulation, mszName.Text(), pcFamily);
	}
	return &OMalloc<CGateArray>(pcSimulation, mszName.Text(), OMalloc<CGateArrayPins>(msType.iUnitCount, psPart->iInputCount, pcFamily), psPart->eGateFunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::GetPartNumber(CChars* psz)
{
	GetLogic74PartNumber(psz, mszFamily.Text(), mszPartType.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::GetDescription(CChars* psz)
{
	if (msType.psPart != NULL)
	{
		GetLogic74Description(psz, &msType);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLogic74PackageView::GetType(void)
{
	return "74 Series Package";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLogic74PackageView::GetXMLName(void)
{
	return "Logic74Package";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::SaveXMLAttributes(CXMLWriter* pcWriter)
{
	CIntegratedCircuitView::SaveXMLAttributes(pcWriter);
	pcWriter->Attribute("partType", mszPartType.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogic74PackageView::LoadXML(CMarkupTag* pcTag)
{
	int32		x;
	int32		y;
	ERotation	eRotation;
	char*		szName;
	char*		szFamily;
	bool		bExplicitPowerPorts;
	char*		szPartType;

	if (!LoadXMLPlacement(pcTag, &x, &y, &eRotation, &szName) || !LoadXMLFamily(pcTag, &szFamily, &bExplicitPowerPorts))
	{
		return false;
	}
	szPartType = pcTag->GetStringAttribute("partType");
	if (StrEmpty(szPartType))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a partType attribute.", NULL);
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, szPartType);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its part's type ("00") after its name and power.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74PackageView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	CComponentView::GetProperties(pcProperties, psContext);
	AddPowerProperties(pcProperties, psContext);
	pcProperties->AddText("partType", mszPartType.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogic74PackageView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), pcProperties->GetText("partType", ""));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CLogic74PackageView::GetPartType(void) { return mszPartType.Text(); }
SLogic74Part* CLogic74PackageView::GetPart(void) { return msType.psPart; }
int CLogic74PackageView::GetUnitCount(void) { return msType.iUnitCount; }


//////////////////////////////////////////////////////////////////////////
//
// Whether the part's units are flip flops (a 74LVC74's), not gates.
//
//////////////////////////////////////////////////////////////////////////
static bool IsLogic74FlipFlopPart(SLogic74PartType* psType)
{
	return (psType->psPart != NULL) && (psType->psPart->eKind == L74K_Block) && (psType->psPart->eBlock == LBK_FlipFlops);
}


//////////////////////////////////////////////////////////////////////////
//
// iUnit is from 0 (Unit A).  A unit of a part type there's no row for is
// drawn as a buffer and can't be simulated.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74UnitView::Init(int32 x, int32 y, ERotation eRotation, const char* szName, const char* szFamily, bool bExplicitPowerPorts, const char* szPartType, int iUnit)
{
	EGateFunction	eFunction;
	int				iInputCount;

	FindLogic74Part(&msType, szPartType);
	eFunction = ((msType.psPart != NULL) && (msType.psPart->eKind == L74K_Gates)) ? msType.psPart->eGateFunction : GF_Buffer;
	iInputCount = ((msType.psPart != NULL) && (msType.psPart->eKind == L74K_Gates)) ? msType.psPart->iInputCount : 1;

	CLogicGateView::Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, iInputCount, 1, CGateArray::IsNegated(eFunction));
	mszPartType.Init(szPartType);
	if ((iUnit < 0) || (iUnit >= LOGIC_74_MAX_UNITS))
	{
		iUnit = 0;
	}
	miUnit = iUnit;

	if (IsLogic74FlipFlopPart(&msType))
	{
		CreateFlipFlop();
	}
	else
	{
		CreateGate();
	}
	CalculateBounds();
}


//////////////////////////////////////////////////////////////////////////
//
// The flip flop as the generic D flip flop view draws it, with its unit
// above and its family below.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74UnitView::CreateFlipFlop(void)
{
	SPortViewCreator	asLeft[] = {	{ "CP", "CP",	1, true,  false },
										{ "SD", "SD",	1, false, true },
										{ "D",	"D",	1, false, false },
										{ "RD", "RD",	1, false, true } };
	SPortViewCreator	asRight[] = {	{ "Q",	"Q",	1, false, false },
										{ "Q",	"QB",	1, false, true } };
	CChars				szUnit;

	mcShapes.AddRectangle(-3, -4, 3, 4, true, true);
	CreatePowerPortViews();
	CreateLabelledPortViews(asLeft, 4, asRight, 2, -3, 3, -3, -1, 2);

	szUnit.Init("Unit ");
	szUnit.Append(GetUnitLetter());
	mcShapes.AddText(0, -4.75, szUnit.Text(), CF_SansSerif, LOGIC_74_UNIT_FONT_SIZE, TA_Centre);
	szUnit.Kill();
	mcShapes.AddText(0, 4.75, mszFamily.Text(), CF_SansSerif, LOGIC_74_UNIT_FONT_SIZE, TA_Centre);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogic74UnitView::Kill(void)
{
	mszPartType.Kill();
	CLogicGateView::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The gate as the generic gate view draws it, with its unit above and its
// family below.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74UnitView::CreateGate(void)
{
	EGateFunction	eFunction;
	float64			fHalfHeight;
	CChars			szUnit;

	eFunction = ((msType.psPart != NULL) && (msType.psPart->eKind == L74K_Gates)) ? msType.psPart->eGateFunction : GF_Buffer;
	AddLogic74GateBody(&mcShapes, eFunction, 0, 0, miInputCount);
	if (CGateArray::IsSingleInput(eFunction))
	{
		CreateBufferPortViews();
	}
	else
	{
		CreateGatePortViews(-1 - GetLogic74GateInputX(eFunction));
	}

	fHalfHeight = GetLogic74GateHalfHeight(eFunction, miInputCount) + 0.75;
	szUnit.Init("Unit ");
	szUnit.Append(GetUnitLetter());
	mcShapes.AddText(0.5, -fHalfHeight, szUnit.Text(), CF_SansSerif, LOGIC_74_UNIT_FONT_SIZE, TA_Centre);
	szUnit.Kill();
	mcShapes.AddText(0.5, fHalfHeight, mszFamily.Text(), CF_SansSerif, LOGIC_74_UNIT_FONT_SIZE, TA_Centre);
}


//////////////////////////////////////////////////////////////////////////
//
// A buffer is only as wide as its triangle.
//
//////////////////////////////////////////////////////////////////////////
bool CLogic74UnitView::IncludeExtraBounds(SFloat64Vec2* psMinimum, SFloat64Vec2* psMaximum, bool bHasBounds)
{
	if ((msType.psPart == NULL) || (msType.psPart->eKind != L74K_Gates) || CGateArray::IsSingleInput(msType.psPart->eGateFunction))
	{
		return bHasBounds;
	}
	return CLogicGateView::IncludeExtraBounds(psMinimum, psMaximum, bHasBounds);
}


//////////////////////////////////////////////////////////////////////////
//
// The generic single gate of the unit's function.
//
//////////////////////////////////////////////////////////////////////////
CIntegratedCircuit* CLogic74UnitView::CreateIntegratedCircuit(CSubcircuitSimulation* pcSimulation, CFamilyVoltageConfiguration* pcFamily)
{
	if (msType.psPart == NULL)
	{
		gcLogger.Error2(__METHOD__, " There is no 74 series part [", mszPartType.Text(), "] to simulate [", mszName.Text(), "].", NULL);
		return NULL;
	}
	if (IsLogic74FlipFlopPart(&msType))
	{
		return &OMalloc<CDTypeFlipFlop>(pcSimulation, mszName.Text(), OMalloc<CDTypeFlipFlopPins>(pcFamily));
	}
	if (msType.psPart->eKind != L74K_Gates)
	{
		gcLogger.Error2(__METHOD__, " 74 series part [", mszPartType.Text(), "] has no units to simulate [", mszName.Text(), "].", NULL);
		return NULL;
	}

	switch (msType.psPart->eGateFunction)
	{
	case GF_And:
		return &OMalloc<CAndGate>(pcSimulation, mszName.Text(), OMalloc<CAndGatePins>(miInputCount, pcFamily));
	case GF_Nand:
		return &OMalloc<CNandGate>(pcSimulation, mszName.Text(), OMalloc<CAndGatePins>(miInputCount, pcFamily));
	case GF_Or:
		return &OMalloc<COrGate>(pcSimulation, mszName.Text(), OMalloc<COrGatePins>(miInputCount, pcFamily));
	case GF_Nor:
		return &OMalloc<CNorGate>(pcSimulation, mszName.Text(), OMalloc<COrGatePins>(miInputCount, pcFamily));
	case GF_Xor:
		return &OMalloc<CXorGate>(pcSimulation, mszName.Text(), OMalloc<CXorGatePins>(miInputCount, pcFamily));
	case GF_Xnor:
		return &OMalloc<CXnorGate>(pcSimulation, mszName.Text(), OMalloc<CXorGatePins>(miInputCount, pcFamily));
	case GF_Inverter:
		return &OMalloc<CInverter>(pcSimulation, mszName.Text(), OMalloc<CBufferPins>(1, pcFamily));
	default:
		return &OMalloc<CBuffer>(pcSimulation, mszName.Text(), OMalloc<CBufferPins>(1, pcFamily));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The part's next unit where this one is, named, turned, mirrored and of
// the family as it is.  NULL after the part's last unit.
//
//////////////////////////////////////////////////////////////////////////
CComponentView* CLogic74UnitView::NewNextUnit(void)
{
	CLogic74UnitView*	pcNext;

	if (miUnit + 1 >= GetUnitCount())
	{
		return NULL;
	}
	pcNext = NewMalloc<CLogic74UnitView>();
	pcNext->Init(msPosition.x, msPosition.y, meRotation, mszName.Text(), mszFamily.Text(), mbExplicitPowerPorts, mszPartType.Text(), miUnit + 1);
	pcNext->SetMirrored(mbMirrored);
	return pcNext;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLogic74UnitView::GetType(void)
{
	return "74 Series Unit";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CLogic74UnitView::GetXMLName(void)
{
	return "Logic74Unit";
}


//////////////////////////////////////////////////////////////////////////
//
// As an integrated circuit's, with the part's type and the unit's letter.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74UnitView::SaveXMLAttributes(CXMLWriter* pcWriter)
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
bool CLogic74UnitView::LoadXML(CMarkupTag* pcTag)
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
	if ((szUnit == NULL) || (szUnit[0] < 'A') || (szUnit[0] >= 'A' + LOGIC_74_MAX_UNITS) || (szUnit[1] != '\0'))
	{
		return gcLogger.Error2(__METHOD__, " [", pcTag->GetName(), "] needs a unit letter.", NULL);
	}
	Init(x, y, eRotation, szName, szFamily, bExplicitPowerPorts, szPartType, szUnit[0] - 'A');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Its part's type and which of the part's units it is, after its name and
// power.
//
//////////////////////////////////////////////////////////////////////////
void CLogic74UnitView::GetProperties(CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	const char*		aszUnits[LOGIC_74_MAX_UNITS] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z" };
	int				iUnitCount;

	CComponentView::GetProperties(pcProperties, psContext);
	AddPowerProperties(pcProperties, psContext);
	pcProperties->AddText("partType", mszPartType.Text());
	iUnitCount = GetUnitCount();
	if (iUnitCount <= miUnit)
	{
		iUnitCount = miUnit + 1;
	}
	pcProperties->AddChoice("unit", aszUnits, (size)iUnitCount, (size)miUnit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogic74UnitView::InitFromProperties(int32 x, int32 y, ERotation eRotation, CComponentProperties* pcProperties, SComponentPropertiesContext* psContext)
{
	Init(x, y, eRotation, pcProperties->GetText("name", ""), pcProperties->GetText("family", ""), pcProperties->GetBoolean("explicitPowerPorts", mbExplicitPowerPorts), pcProperties->GetText("partType", ""), (int)pcProperties->GetChoice("unit", (size)miUnit));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CLogic74UnitView::GetPartType(void) { return mszPartType.Text(); }
SLogic74Part* CLogic74UnitView::GetPart(void) { return msType.psPart; }
int CLogic74UnitView::GetUnit(void) { return miUnit; }
char CLogic74UnitView::GetUnitLetter(void) { return (char)('A' + miUnit); }
int CLogic74UnitView::GetUnitCount(void) { return (msType.psPart != NULL) ? msType.iUnitCount : 1; }

