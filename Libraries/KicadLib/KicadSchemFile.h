/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela KicadLib

Codaphela KicadLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela KicadLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela KicadLib.  If not, see <http://www.gnu.org/licenses/>.

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
	bool					Import(char* szText, lsize iTextLen, char* szFileName);

	void					SetVersion(int32 iVersion);
	void					SetGenerator(const char* szGenerator, const char* szGeneratorVersion);
	void					SetUuid(const char* szUuid);
	void					SetPaper(const char* szPaper);
	void					SetTitleBlock(const char* szTitle, const char* szDate, const char* szRevision, const char* szCompany);

	CKicadSchemLibSymbol*	AddLibSymbol(const char* szName);
	CKicadSchemSymbol*		AddSymbol(const char* szLibId);
	CKicadSchemWire*		AddWire(SFloat64Vec2* psStart, SFloat64Vec2* psEnd, bool bBus);
	CKicadSchemJunction*	AddJunction(SFloat64Vec2* psPosition, float64 fDiameter);
	CKicadSchemNoConnect*	AddNoConnect(SFloat64Vec2* psPosition);
	CKicadSchemBusEntry*	AddBusEntry(SFloat64Vec2* psPosition, SFloat64Vec2* psSize);
	CKicadSchemLabel*		AddLabel(EKicadSchemLabelType eType, const char* szText);
	CKicadSchemSheet*		AddSheet(SFloat64Vec2* psPosition, SFloat64Vec2* psSize);

	const char*					GetFileName(void) const;
	int32						GetVersion(void) const;
	const char*					GetGenerator(void) const;
	const char*					GetGeneratorVersion(void) const;
	const char*					GetUuid(void) const;
	const char*					GetPaper(void) const;
	const char*					GetTitle(void) const;
	const char*					GetDate(void) const;
	const char*					GetRevision(void) const;
	const char*					GetCompany(void) const;

	size					NumLibSymbols(void) const;
	CKicadSchemLibSymbol*	GetLibSymbol(size uiIndex);
	CKicadSchemLibSymbol*	GetLibSymbol(const char* szName);
	CKicadSchemLibSymbol*	GetLibSymbol(CKicadSchemSymbol* pcSymbol);

	size					NumSymbols(void) const;
	CKicadSchemSymbol*		GetSymbol(size uiIndex);
	CKicadSchemSymbol*		GetSymbol(char* szReference);
	CKicadSchemSymbol*		GetSymbolByUuid(const char* szUuid);

	size					NumWires(void) const;
	CKicadSchemWire*		GetWire(size uiIndex);
	size					NumJunctions(void) const;
	CKicadSchemJunction*	GetJunction(size uiIndex);
	size					NumNoConnects(void) const;
	CKicadSchemNoConnect*	GetNoConnect(size uiIndex);
	size					NumBusEntries(void) const;
	CKicadSchemBusEntry*	GetBusEntry(size uiIndex);
	size					NumLabels(void) const;
	CKicadSchemLabel*		GetLabel(size uiIndex);
	size					NumSheets(void) const;
	CKicadSchemSheet*		GetSheet(size uiIndex);

	bool					GetPinPosition(SFloat64Vec2* psDest, CKicadSchemSymbol* pcSymbol, CKicadSchemLibPin* pcPin);
};


#endif // __KICAD_SCHEM_FILE_H__

