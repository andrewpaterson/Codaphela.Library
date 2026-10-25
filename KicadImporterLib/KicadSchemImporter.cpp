/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadImporterLib

Codaphela KicadImporterLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadImporterLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadImporterLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Logger.h"
#include "BaseLib/StringHelper.h"
#include "KicadSchemImporter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemImporter::Init(CKicadSchemFile* pcFile, char* szFileName)
{
	mpcFile = pcFile;
	mszFileName.Init(szFileName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CKicadSchemImporter::Kill(void)
{
	mszFileName.Kill();
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::Import(CKicadSchemExpression* pcRoot)
{
	TRISTATE				tResult;
	size					i;
	size					uiNumChildren;
	CKicadSchemExpression*	pcChild;
	CKicadSchemExpression*	pcSymbolInstances;

	if (!pcRoot->Is("kicad_sch"))
	{
		return Error(pcRoot, "Expected (kicad_sch at the start of the file.");
	}

	tResult = ImportHeader(pcRoot);
	ReturnOnErrorAndFalse(tResult);

	pcSymbolInstances = NULL;
	uiNumChildren = pcRoot->NumChildren();
	for (i = 0; i < uiNumChildren; i++)
	{
		pcChild = pcRoot->GetChild(i);
		if (pcChild->Is("lib_symbols"))
		{
			tResult = ImportLibSymbols(pcChild);
		}
		else if (pcChild->Is("symbol"))
		{
			tResult = ImportSymbol(pcChild);
		}
		else if (pcChild->Is("wire"))
		{
			tResult = ImportWire(pcChild, false);
		}
		else if (pcChild->Is("bus"))
		{
			tResult = ImportWire(pcChild, true);
		}
		else if (pcChild->Is("junction"))
		{
			tResult = ImportJunction(pcChild);
		}
		else if (pcChild->Is("no_connect"))
		{
			tResult = ImportNoConnect(pcChild);
		}
		else if (pcChild->Is("bus_entry"))
		{
			tResult = ImportBusEntry(pcChild);
		}
		else if (pcChild->Is("label"))
		{
			tResult = ImportLabel(pcChild, KSLT_Local);
		}
		else if (pcChild->Is("global_label"))
		{
			tResult = ImportLabel(pcChild, KSLT_Global);
		}
		else if (pcChild->Is("hierarchical_label"))
		{
			tResult = ImportLabel(pcChild, KSLT_Hierarchical);
		}
		else if (pcChild->Is("sheet"))
		{
			tResult = ImportSheet(pcChild);
		}
		else if (pcChild->Is("symbol_instances"))
		{
			//The symbols must all have been imported first.
			pcSymbolInstances = pcChild;
			tResult = TRITRUE;
		}
		else
		{
			tResult = TRITRUE;
		}
		ReturnOnErrorAndFalse(tResult);
	}

	if (pcSymbolInstances != NULL)
	{
		tResult = ImportOldSymbolInstances(pcSymbolInstances);
		ReturnOnErrorAndFalse(tResult);
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportHeader(CKicadSchemExpression* pcRoot)
{
	int32					iVersion;
	CKicadSchemExpression*	pcTitleBlock;

	if (!pcRoot->GetChildInt("version", &iVersion))
	{
		return Error(pcRoot, "Expected (version yyyymmdd) in (kicad_sch.");
	}

	mpcFile->SetVersion(iVersion);
	mpcFile->SetGenerator(pcRoot->GetChildText("generator"), pcRoot->GetChildText("generator_version"));
	mpcFile->SetUuid(pcRoot->GetChildText("uuid"));
	mpcFile->SetPaper(pcRoot->GetChildText("paper"));

	pcTitleBlock = pcRoot->FindChild("title_block");
	if (pcTitleBlock != NULL)
	{
		mpcFile->SetTitleBlock(pcTitleBlock->GetChildText("title"), pcTitleBlock->GetChildText("date"), pcTitleBlock->GetChildText("rev"), pcTitleBlock->GetChildText("company"));
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportLibSymbols(CKicadSchemExpression* pcLibSymbols)
{
	TRISTATE				tResult;
	size					i;
	size					uiNumChildren;
	CKicadSchemExpression*	pcChild;

	uiNumChildren = pcLibSymbols->NumChildren();
	for (i = 0; i < uiNumChildren; i++)
	{
		pcChild = pcLibSymbols->GetChild(i);
		if (pcChild->Is("symbol"))
		{
			tResult = ImportLibSymbol(pcChild);
			ReturnOnErrorAndFalse(tResult);
		}
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportLibSymbol(CKicadSchemExpression* pcExpression)
{
	TRISTATE				tResult;
	char*					szName;
	CKicadSchemLibSymbol*	pcLibSymbol;
	CKicadSchemExpression*	pcChild;
	size					i;
	size					uiNumChildren;

	//(symbol "Device:R" (pin_numbers (hide yes)) (pin_names (offset 0)) ... (property ...) (symbol "R_0_1" ...) (symbol "R_1_1" (pin ...)))
	szName = pcExpression->GetAtomText(0);
	if (szName == NULL)
	{
		return Error(pcExpression, "Expected a name in (symbol.");
	}

	pcLibSymbol = mpcFile->AddLibSymbol(szName);
	pcLibSymbol->SetExtends(pcExpression->GetChildText("extends"));

	//(power) and, in KiCad 10, (power global) or (power local).
	pcLibSymbol->SetPower(pcExpression->FindChild("power") != NULL);

	pcChild = pcExpression->FindChild("pin_names");
	if (pcChild != NULL)
	{
		pcLibSymbol->SetPinNamesHidden(IsHidden(pcChild));
	}

	pcChild = pcExpression->FindChild("pin_numbers");
	if (pcChild != NULL)
	{
		pcLibSymbol->SetPinNumbersHidden(IsHidden(pcChild));
	}

	tResult = ImportProperties(pcExpression, pcLibSymbol->GetProperties());
	ReturnOnErrorAndFalse(tResult);

	uiNumChildren = pcExpression->NumChildren();
	for (i = 0; i < uiNumChildren; i++)
	{
		pcChild = pcExpression->GetChild(i);
		if (pcChild->Is("symbol"))
		{
			tResult = ImportLibSubSymbol(pcChild, pcLibSymbol);
			ReturnOnErrorAndFalse(tResult);
		}
		else if (pcChild->Is("pin"))
		{
			tResult = ImportLibPin(pcChild, pcLibSymbol, 0, 0);
			ReturnOnErrorAndFalse(tResult);
		}
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportLibSubSymbol(CKicadSchemExpression* pcExpression, CKicadSchemLibSymbol* pcLibSymbol)
{
	TRISTATE				tResult;
	char*					szName;
	int32					iUnit;
	int32					iBodyStyle;
	CKicadSchemExpression*	pcChild;
	size					i;
	size					uiNumChildren;

	szName = pcExpression->GetAtomText(0);
	if ((szName == NULL) || !GetUnitAndBodyStyle(szName, &iUnit, &iBodyStyle))
	{
		return Error(pcExpression, "Expected a library sub-symbol name ending in _<unit>_<body style>.");
	}

	uiNumChildren = pcExpression->NumChildren();
	for (i = 0; i < uiNumChildren; i++)
	{
		pcChild = pcExpression->GetChild(i);
		if (pcChild->Is("pin"))
		{
			tResult = ImportLibPin(pcChild, pcLibSymbol, iUnit, iBodyStyle);
			ReturnOnErrorAndFalse(tResult);
		}
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportLibPin(CKicadSchemExpression* pcExpression, CKicadSchemLibSymbol* pcLibSymbol, int32 iUnit, int32 iBodyStyle)
{
	char*					szType;
	char*					szGraphicStyle;
	EKicadSchemPinType		eType;
	CKicadSchemLibPin*		pcPin;
	SKicadSchemPoint		sPosition;
	float64					fAngle;
	float64					fLength;

	//(pin passive line (at 0 3.81 270) (length 1.27) (name "~" ...) (number "1" ...))
	szType = pcExpression->GetAtomText(0);
	szGraphicStyle = pcExpression->GetAtomText(1);
	if ((szType == NULL) || (szGraphicStyle == NULL))
	{
		return Error(pcExpression, "Expected an electrical type and a graphic style in (pin.");
	}

	eType = ParseKicadSchemPinType(szType);
	if (eType == KSPT_Unknown)
	{
		return Error(pcExpression, "Unknown pin electrical type.");
	}

	if (!GetAt(pcExpression, &sPosition, &fAngle))
	{
		return Error(pcExpression, "Expected (at x y angle) in (pin.");
	}

	pcPin = pcLibSymbol->AddPin(eType, szGraphicStyle, iUnit, iBodyStyle);
	pcPin->SetPosition(&sPosition, fAngle);
	if (pcExpression->GetChildDouble("length", &fLength))
	{
		pcPin->SetLength(fLength);
	}
	pcPin->SetName(pcExpression->GetChildText("name"));
	pcPin->SetNumber(pcExpression->GetChildText("number"));
	pcPin->SetHidden(IsHidden(pcExpression));
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportSymbol(CKicadSchemExpression* pcExpression)
{
	TRISTATE				tResult;
	char*					szLibId;
	char*					szMirror;
	CKicadSchemSymbol*		pcSymbol;
	SKicadSchemPoint		sPosition;
	float64					fAngle;
	int32					iValue;
	bool					bValue;
	CKicadSchemExpression*	pcChild;

	//(symbol (lib_id "Device:R") (at 100.33 50.8 90) (mirror x) (unit 1) (body_style 1) (in_bom yes) ... (instances ...))
	szLibId = pcExpression->GetChildText("lib_id");
	if (szLibId == NULL)
	{
		return Error(pcExpression, "Expected (lib_id \"library:name\") in (symbol.");
	}

	if (!GetAt(pcExpression, &sPosition, &fAngle))
	{
		return Error(pcExpression, "Expected (at x y angle) in (symbol.");
	}

	pcSymbol = mpcFile->AddSymbol(szLibId);
	pcSymbol->SetLibName(pcExpression->GetChildText("lib_name"));
	pcSymbol->SetUuid(pcExpression->GetChildText("uuid"));
	pcSymbol->SetPosition(&sPosition, fAngle);

	szMirror = pcExpression->GetChildText("mirror");
	if (szMirror != NULL)
	{
		if (StringCompare(szMirror, "x") == 0)
		{
			pcSymbol->SetMirror(KSM_X);
		}
		else if (StringCompare(szMirror, "y") == 0)
		{
			pcSymbol->SetMirror(KSM_Y);
		}
		else
		{
			return Error(pcExpression, "Expected (mirror x) or (mirror y) in (symbol.");
		}
	}

	if (pcExpression->GetChildInt("unit", &iValue))
	{
		pcSymbol->SetUnit(iValue);
	}

	//KiCad 10 renamed (convert) to (body_style).
	if (pcExpression->GetChildInt("body_style", &iValue) || pcExpression->GetChildInt("convert", &iValue))
	{
		pcSymbol->SetBodyStyle(iValue);
	}

	if (pcExpression->GetChildYesNo("in_bom", &bValue))
	{
		pcSymbol->SetInBom(bValue);
	}
	if (pcExpression->GetChildYesNo("on_board", &bValue))
	{
		pcSymbol->SetOnBoard(bValue);
	}
	if (pcExpression->GetChildYesNo("dnp", &bValue))
	{
		pcSymbol->SetDoNotPopulate(bValue);
	}

	tResult = ImportProperties(pcExpression, pcSymbol->GetProperties());
	ReturnOnErrorAndFalse(tResult);

	pcChild = pcExpression->FindChild("instances");
	if (pcChild != NULL)
	{
		tResult = ImportSymbolInstances(pcChild, pcSymbol);
		ReturnOnErrorAndFalse(tResult);
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportSymbolInstances(CKicadSchemExpression* pcInstances, CKicadSchemSymbol* pcSymbol)
{
	size					i;
	size					j;
	CKicadSchemExpression*	pcProject;
	CKicadSchemExpression*	pcPath;
	char*					szProject;
	char*					szPath;
	char*					szReference;
	int32					iUnit;

	//(instances (project "Name" (path "/uuid" (reference "R1") (unit 1))))
	for (i = 0; i < pcInstances->NumChildren(); i++)
	{
		pcProject = pcInstances->GetChild(i);
		if (!pcProject->Is("project"))
		{
			continue;
		}

		szProject = pcProject->GetAtomText(0);
		for (j = 0; j < pcProject->NumChildren(); j++)
		{
			pcPath = pcProject->GetChild(j);
			if (!pcPath->Is("path"))
			{
				continue;
			}

			szPath = pcPath->GetAtomText(0);
			szReference = pcPath->GetChildText("reference");
			if ((szPath == NULL) || (szReference == NULL))
			{
				return Error(pcPath, "Expected a path and a (reference) in (path.");
			}

			if (!pcPath->GetChildInt("unit", &iUnit))
			{
				iUnit = pcSymbol->GetUnit();
			}
			pcSymbol->AddInstance(szProject, szPath, szReference, iUnit);
		}
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportOldSymbolInstances(CKicadSchemExpression* pcSymbolInstances)
{
	size					i;
	CKicadSchemExpression*	pcPath;
	char*					szPath;
	char*					szUuid;
	char*					szReference;
	int32					iUnit;
	CKicadSchemSymbol*		pcSymbol;

	//KiCad 6 listed every symbol's references at the end of the root sheet: (symbol_instances (path "/sheet/uuid"
	//(reference "R1") (unit 1) ...)).  The last part of the path is the placed symbol's uuid.
	for (i = 0; i < pcSymbolInstances->NumChildren(); i++)
	{
		pcPath = pcSymbolInstances->GetChild(i);
		if (!pcPath->Is("path"))
		{
			continue;
		}

		szPath = pcPath->GetAtomText(0);
		szReference = pcPath->GetChildText("reference");
		if ((szPath == NULL) || (szReference == NULL))
		{
			return Error(pcPath, "Expected a path and a (reference) in (path.");
		}

		szUuid = (char*)FindChar(szPath, '/', true);
		szUuid = (szUuid == NULL) ? szPath : &szUuid[1];
		pcSymbol = mpcFile->GetSymbolByUuid(szUuid);
		if (pcSymbol == NULL)
		{
			//The symbol is on a sub-sheet.
			continue;
		}

		if (!pcPath->GetChildInt("unit", &iUnit))
		{
			iUnit = pcSymbol->GetUnit();
		}
		pcSymbol->AddInstance(NULL, szPath, szReference, iUnit);
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportProperties(CKicadSchemExpression* pcExpression, CKicadSchemProperties* pcProperties)
{
	size					i;
	size					uiNumChildren;
	CKicadSchemExpression*	pcChild;
	CKicadSchemExpression*	pcEffects;
	CKicadSchemProperty*	pcProperty;
	char*					szName;
	char*					szValue;
	SKicadSchemPoint		sPosition;
	float64					fAngle;
	bool					bHidden;

	//(property "Reference" "R1" (at 102.87 49.53 0) (hide yes) (effects (font (size 1.27 1.27)) (justify left)))
	//KiCad 6 put a bare hide in (effects ...), KiCad 9 (hide yes) in (effects ...) and KiCad 10 (hide yes) in (property ...).
	uiNumChildren = pcExpression->NumChildren();
	for (i = 0; i < uiNumChildren; i++)
	{
		pcChild = pcExpression->GetChild(i);
		if (!pcChild->Is("property"))
		{
			continue;
		}

		szName = pcChild->GetAtomText(0);
		szValue = pcChild->GetAtomText(1);
		if ((szName == NULL) || (szValue == NULL))
		{
			return Error(pcChild, "Expected a name and a value in (property.");
		}

		pcProperty = pcProperties->Add(szName, szValue);
		if (GetAt(pcChild, &sPosition, &fAngle))
		{
			pcProperty->SetPosition(&sPosition, fAngle);
		}

		bHidden = IsHidden(pcChild);
		pcEffects = pcChild->FindChild("effects");
		if (pcEffects != NULL)
		{
			bHidden |= IsHidden(pcEffects);
		}
		pcProperty->SetHidden(bHidden);
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportWire(CKicadSchemExpression* pcExpression, bool bBus)
{
	CKicadSchemExpression*	pcPoints;
	SKicadSchemPoint		sStart;
	SKicadSchemPoint		sEnd;
	CKicadSchemWire*		pcWire;

	//(wire (pts (xy 101.6 50.8) (xy 114.3 50.8)) (stroke (width 0) (type default)) (uuid "..."))
	pcPoints = pcExpression->FindChild("pts");
	if ((pcPoints == NULL) || (pcPoints->NumChildren() != 2) || !GetXY(pcPoints->GetChild(0), &sStart) || !GetXY(pcPoints->GetChild(1), &sEnd))
	{
		return Error(pcExpression, "Expected (pts (xy x1 y1) (xy x2 y2)) in a wire or bus.");
	}

	pcWire = mpcFile->AddWire(&sStart, &sEnd, bBus);
	pcWire->SetUuid(pcExpression->GetChildText("uuid"));
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportJunction(CKicadSchemExpression* pcExpression)
{
	SKicadSchemPoint	sPosition;
	float64				fAngle;
	float64				fDiameter;

	if (!GetAt(pcExpression, &sPosition, &fAngle))
	{
		return Error(pcExpression, "Expected (at x y) in (junction.");
	}

	if (!pcExpression->GetChildDouble("diameter", &fDiameter))
	{
		fDiameter = 0.0;
	}
	mpcFile->AddJunction(&sPosition, fDiameter);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportNoConnect(CKicadSchemExpression* pcExpression)
{
	SKicadSchemPoint	sPosition;
	float64				fAngle;

	if (!GetAt(pcExpression, &sPosition, &fAngle))
	{
		return Error(pcExpression, "Expected (at x y) in (no_connect.");
	}

	mpcFile->AddNoConnect(&sPosition);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportBusEntry(CKicadSchemExpression* pcExpression)
{
	SKicadSchemPoint		sPosition;
	SKicadSchemPoint		sSize;
	float64					fAngle;
	CKicadSchemExpression*	pcSize;

	pcSize = pcExpression->FindChild("size");
	if (!GetAt(pcExpression, &sPosition, &fAngle) || (pcSize == NULL) || !pcSize->GetDouble(0, &sSize.fX) || !pcSize->GetDouble(1, &sSize.fY))
	{
		return Error(pcExpression, "Expected (at x y) and (size dx dy) in (bus_entry.");
	}

	mpcFile->AddBusEntry(&sPosition, &sSize);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportLabel(CKicadSchemExpression* pcExpression, EKicadSchemLabelType eType)
{
	char*				szText;
	char*				szShape;
	SKicadSchemPoint	sPosition;
	float64				fAngle;
	CKicadSchemLabel*	pcLabel;

	//(label "CLK" (at 101.6 50.8 0) ...) and (global_label "CLK" (shape input) (at 101.6 50.8 180) ...)
	szText = pcExpression->GetAtomText(0);
	if (szText == NULL)
	{
		return Error(pcExpression, "Expected the label's text.");
	}

	if (!GetAt(pcExpression, &sPosition, &fAngle))
	{
		return Error(pcExpression, "Expected (at x y angle) in a label.");
	}

	pcLabel = mpcFile->AddLabel(eType, szText);
	pcLabel->SetPosition(&sPosition, fAngle);

	szShape = pcExpression->GetChildText("shape");
	if (szShape != NULL)
	{
		pcLabel->SetShape(ParseKicadSchemLabelShape(szShape));
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::ImportSheet(CKicadSchemExpression* pcExpression)
{
	TRISTATE				tResult;
	SKicadSchemPoint		sPosition;
	SKicadSchemPoint		sSize;
	float64					fAngle;
	CKicadSchemExpression*	pcSize;
	CKicadSchemExpression*	pcChild;
	CKicadSchemSheet*		pcSheet;
	CKicadSchemSheetPin*	pcPin;
	char*					szName;
	char*					szShape;
	size					i;

	//(sheet (at 100 50) (size 30 20) ... (property "Sheetname" "LED Display" ...) (property "Sheetfile" "LED Display.kicad_sch" ...)
	//  (pin "D0" input (at 100 55 180) ...))
	pcSize = pcExpression->FindChild("size");
	if (!GetAt(pcExpression, &sPosition, &fAngle) || (pcSize == NULL) || !pcSize->GetDouble(0, &sSize.fX) || !pcSize->GetDouble(1, &sSize.fY))
	{
		return Error(pcExpression, "Expected (at x y) and (size width height) in (sheet.");
	}

	pcSheet = mpcFile->AddSheet(&sPosition, &sSize);
	pcSheet->SetUuid(pcExpression->GetChildText("uuid"));

	tResult = ImportProperties(pcExpression, pcSheet->GetProperties());
	ReturnOnErrorAndFalse(tResult);

	for (i = 0; i < pcExpression->NumChildren(); i++)
	{
		pcChild = pcExpression->GetChild(i);
		if (!pcChild->Is("pin"))
		{
			continue;
		}

		szName = pcChild->GetAtomText(0);
		szShape = pcChild->GetAtomText(1);
		if ((szName == NULL) || (szShape == NULL))
		{
			return Error(pcChild, "Expected a name and a shape in a sheet (pin.");
		}

		if (!GetAt(pcChild, &sPosition, &fAngle))
		{
			return Error(pcChild, "Expected (at x y angle) in a sheet (pin.");
		}

		pcPin = pcSheet->AddPin(szName, ParseKicadSchemLabelShape(szShape));
		pcPin->SetPosition(&sPosition, fAngle);
	}
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemImporter::GetAt(CKicadSchemExpression* pcExpression, SKicadSchemPoint* psPosition, float64* pfAngle)
{
	CKicadSchemExpression*	pcAt;

	//(at x y) or (at x y angle).
	pcAt = pcExpression->FindChild("at");
	if ((pcAt == NULL) || !pcAt->GetDouble(0, &psPosition->fX) || !pcAt->GetDouble(1, &psPosition->fY))
	{
		return false;
	}

	if (pcAt->NumChildren() > 2)
	{
		return pcAt->GetDouble(2, pfAngle);
	}
	*pfAngle = 0.0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemImporter::GetXY(CKicadSchemExpression* pcExpression, SKicadSchemPoint* psPoint)
{
	if ((pcExpression == NULL) || !pcExpression->Is("xy"))
	{
		return false;
	}
	return pcExpression->GetDouble(0, &psPoint->fX) && pcExpression->GetDouble(1, &psPoint->fY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemImporter::IsHidden(CKicadSchemExpression* pcExpression)
{
	bool	bHidden;

	//A bare hide (KiCad 6 and 7) or (hide yes).
	if (pcExpression->ContainsSymbol("hide"))
	{
		return true;
	}
	return pcExpression->GetChildYesNo("hide", &bHidden) && bHidden;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CKicadSchemImporter::GetUnitAndBodyStyle(char* szSubSymbolName, int32* piUnit, int32* piBodyStyle)
{
	CChars	sz;
	size	iBodyStyleSeparator;
	size	iUnitSeparator;
	bool	bResult;

	//"74HC00_2_1" is unit 2, body style 1.  The symbol's own name may contain underscores so the numbers are found from the end.
	sz.Init(szSubSymbolName);
	iBodyStyleSeparator = sz.FindFromEnd('_');
	if ((iBodyStyleSeparator == ARRAY_ELEMENT_NOT_FOUND) || (iBodyStyleSeparator == 0))
	{
		sz.Kill();
		return false;
	}

	iUnitSeparator = sz.FindFromEnd(iBodyStyleSeparator - 1, '_');
	if (iUnitSeparator == ARRAY_ELEMENT_NOT_FOUND)
	{
		sz.Kill();
		return false;
	}

	bResult = ParseKicadSchemInt(&sz.Text()[iBodyStyleSeparator + 1], piBodyStyle);
	sz.Text()[iBodyStyleSeparator] = '\0';
	bResult &= ParseKicadSchemInt(&sz.Text()[iUnitSeparator + 1], piUnit);
	sz.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CKicadSchemImporter::Error(CKicadSchemExpression* pcExpression, char* szError)
{
	CChars			sz;
	SLogConfig		sLogConfig;

	sz.Init(mszFileName);
	sz.Append('(');
	sz.Append(pcExpression->GetLine());
	sz.Append("): ");
	sz.Append(szError);
	gcLogger.GetConfig(&sLogConfig);
	gcLogger.SetBreakOnError(false);
	gcLogger.Error2(__METHOD__, " ", sz.Text(), NULL);
	gcLogger.SetConfig(&sLogConfig);
	sz.Kill();
	return TRIERROR;
}

