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
#ifndef __KICAD_SCHEM_FILE_H__
#define __KICAD_SCHEM_FILE_H__
#include "BaseLib/ListVariable.h"
#include "KicadSchemLibSymbol.h"
#include "KicadSchemSymbol.h"
#include "KicadSchemWire.h"
#include "KicadSchemJunction.h"
#include "KicadSchemNoConnect.h"
#include "KicadSchemBusEntry.h"
#include "KicadSchemLabel.h"
#include "KicadSchemSheet.h"


//One .kicad_sch file: one sheet of a schematic.  Sub-sheets are the files named by its sheets' GetSheetFile and are
//loaded separately.  Graphics (text, polylines, rectangles, images) are not imported; they don't change the circuit.
class CKicadSchemFile
{
protected:
	CChars			mszFileName;
	int32			miVersion;
	CChars			mszGenerator;
	CChars			mszGeneratorVersion;
	CChars			mszUuid;
	CChars			mszPaper;
	CChars			mszTitle;
	CChars			mszDate;
	CChars			mszRevision;
	CChars			mszCompany;

	CListVariable	macLibSymbols;
	CListVariable	macSymbols;
	CListVariable	macWires;
	CListVariable	macJunctions;
	CListVariable	macNoConnects;
	CListVariable	macBusEntries;
	CListVariable	macLabels;
	CListVariable	macSheets;

public:
	void					Init(void);
	void					Kill(void);

	bool					Load(char* szPathName);
	bool					Import(char* szText, size iTextLen, char* szFileName);

	void					SetVersion(int32 iVersion);
	void					SetGenerator(char* szGenerator, char* szGeneratorVersion);
	void					SetUuid(char* szUuid);
	void					SetPaper(char* szPaper);
	void					SetTitleBlock(char* szTitle, char* szDate, char* szRevision, char* szCompany);

	CKicadSchemLibSymbol*	AddLibSymbol(char* szName);
	CKicadSchemSymbol*		AddSymbol(char* szLibId);
	CKicadSchemWire*		AddWire(SKicadSchemPoint* psStart, SKicadSchemPoint* psEnd, bool bBus);
	CKicadSchemJunction*	AddJunction(SKicadSchemPoint* psPosition, float64 fDiameter);
	CKicadSchemNoConnect*	AddNoConnect(SKicadSchemPoint* psPosition);
	CKicadSchemBusEntry*	AddBusEntry(SKicadSchemPoint* psPosition, SKicadSchemPoint* psSize);
	CKicadSchemLabel*		AddLabel(EKicadSchemLabelType eType, char* szText);
	CKicadSchemSheet*		AddSheet(SKicadSchemPoint* psPosition, SKicadSchemPoint* psSize);

	char*					GetFileName(void);
	int32					GetVersion(void);
	char*					GetGenerator(void);
	char*					GetGeneratorVersion(void);
	char*					GetUuid(void);
	char*					GetPaper(void);
	char*					GetTitle(void);
	char*					GetDate(void);
	char*					GetRevision(void);
	char*					GetCompany(void);

	size					NumLibSymbols(void);
	CKicadSchemLibSymbol*	GetLibSymbol(size uiIndex);
	CKicadSchemLibSymbol*	GetLibSymbol(char* szName);
	CKicadSchemLibSymbol*	GetLibSymbol(CKicadSchemSymbol* pcSymbol);

	size					NumSymbols(void);
	CKicadSchemSymbol*		GetSymbol(size uiIndex);
	CKicadSchemSymbol*		GetSymbol(char* szReference);
	CKicadSchemSymbol*		GetSymbolByUuid(char* szUuid);

	size					NumWires(void);
	CKicadSchemWire*		GetWire(size uiIndex);
	size					NumJunctions(void);
	CKicadSchemJunction*	GetJunction(size uiIndex);
	size					NumNoConnects(void);
	CKicadSchemNoConnect*	GetNoConnect(size uiIndex);
	size					NumBusEntries(void);
	CKicadSchemBusEntry*	GetBusEntry(size uiIndex);
	size					NumLabels(void);
	CKicadSchemLabel*		GetLabel(size uiIndex);
	size					NumSheets(void);
	CKicadSchemSheet*		GetSheet(size uiIndex);

	bool					GetPinPosition(SKicadSchemPoint* psDest, CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin);
};


#endif // __KICAD_SCHEM_FILE_H__

