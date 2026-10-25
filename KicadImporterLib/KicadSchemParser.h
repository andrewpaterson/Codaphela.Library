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
#ifndef __KICAD_SCHEM_PARSER_H__
#define __KICAD_SCHEM_PARSER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TextParser.h"
#include "KicadSchemExpressions.h"


//Parses the S-expression text of a .kicad_sch file into a tree of expressions.  It knows nothing about what the
//expressions mean; CKicadSchemImporter turns the tree into a CKicadSchemFile.
class CKicadSchemParser
{
protected:
	CTextParser				mcParser;
	CChars					mszFileName;
	CKicadSchemExpressions*	mpcExpressions;

public:
	bool		Init(char* szText, size iTextLen, char* szFileName, CKicadSchemExpressions* pcExpressions);
	void		Kill(void);

	TRISTATE	Parse(void);

protected:
	TRISTATE	ParseList(CKicadSchemExpression** ppcList);
	TRISTATE	ParseString(CKicadSchemExpression** ppcString);
	TRISTATE	ParseSymbol(CKicadSchemExpression** ppcSymbol);

	TRISTATE	Error(char* szError);
};


void SkipKicadSchemWhitespace(CTextParser* pcTextParser);
bool IsKicadSchemWhitespace(char c);
bool IsKicadSchemDelimiter(char c);


#endif // __KICAD_SCHEM_PARSER_H__

