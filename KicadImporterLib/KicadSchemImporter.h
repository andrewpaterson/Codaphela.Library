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
#ifndef __KICAD_SCHEM_IMPORTER_H__
#define __KICAD_SCHEM_IMPORTER_H__
#include "BaseLib/ErrorTypes.h"
#include "KicadSchemExpression.h"
#include "KicadSchemFile.h"


//Turns the expression tree of a .kicad_sch file into a CKicadSchemFile.  It reads the formats of KiCad 6 (version
//20211123) to KiCad 10 (20260306).  Expressions it doesn't need are skipped, so newer files mostly still import.
class CKicadSchemImporter
{
protected:
	CKicadSchemFile*	mpcFile;
	CChars				mszFileName;

public:
	void		Init(CKicadSchemFile* pcFile, char* szFileName);
	void		Kill(void);

	TRISTATE	Import(CKicadSchemExpression* pcRoot);

protected:
	TRISTATE	ImportHeader(CKicadSchemExpression* pcRoot);
	TRISTATE	ImportLibSymbols(CKicadSchemExpression* pcLibSymbols);
	TRISTATE	ImportLibSymbol(CKicadSchemExpression* pcExpression);
	TRISTATE	ImportLibSubSymbol(CKicadSchemExpression* pcExpression, CKicadSchemLibSymbol* pcLibSymbol);
	TRISTATE	ImportLibPin(CKicadSchemExpression* pcExpression, CKicadSchemLibSymbol* pcLibSymbol, int32 iUnit, int32 iBodyStyle);
	TRISTATE	ImportSymbol(CKicadSchemExpression* pcExpression);
	TRISTATE	ImportSymbolInstances(CKicadSchemExpression* pcInstances, CKicadSchemSymbol* pcSymbol);
	TRISTATE	ImportOldSymbolInstances(CKicadSchemExpression* pcSymbolInstances);
	TRISTATE	ImportProperties(CKicadSchemExpression* pcExpression, CKicadSchemProperties* pcProperties);
	TRISTATE	ImportWire(CKicadSchemExpression* pcExpression, bool bBus);
	TRISTATE	ImportJunction(CKicadSchemExpression* pcExpression);
	TRISTATE	ImportNoConnect(CKicadSchemExpression* pcExpression);
	TRISTATE	ImportBusEntry(CKicadSchemExpression* pcExpression);
	TRISTATE	ImportLabel(CKicadSchemExpression* pcExpression, EKicadSchemLabelType eType);
	TRISTATE	ImportSheet(CKicadSchemExpression* pcExpression);

	bool		GetAt(CKicadSchemExpression* pcExpression, SKicadSchemPoint* psPosition, float64* pfAngle);
	bool		GetXY(CKicadSchemExpression* pcExpression, SKicadSchemPoint* psPoint);
	bool		IsHidden(CKicadSchemExpression* pcExpression);
	bool		GetUnitAndBodyStyle(char* szSubSymbolName, int32* piUnit, int32* piBodyStyle);

	TRISTATE	Error(CKicadSchemExpression* pcExpression, char* szError);
};


#endif // __KICAD_SCHEM_IMPORTER_H__

