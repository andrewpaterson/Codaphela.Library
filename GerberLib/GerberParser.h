/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela GerberLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __GERBER_PARSER_H__
#define __GERBER_PARSER_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/TextParser.h"
#include "GerberCommands.h"


class CGerberParser
{
protected:
	CTextParser			mcParser;
	CChars				mszFilename;
	CGerberCommands*	mpcCommands;
	bool				mbSkipWhitespace;
	CCalculatorSymbols	mcSymbols;

public:
	void		Init(char* szText, size iTextLen, char* szFileName, CGerberCommands* pcCommands);
	void		Kill(void);

	TRISTATE	Parse(void);

protected:
	TRISTATE	ParseCommandG04(void);
	TRISTATE	ParseCommandMO(void);
	TRISTATE	ParseCommandFS(void);
	TRISTATE	ParseCommandAD(void);
	TRISTATE	ParseCommandAM(void);
	TRISTATE	ParseCommandDnn(void);
	TRISTATE	ParseCommandG75(void);
	TRISTATE	ParseCommandG01(void);
	TRISTATE	ParseCommandG02(void);
	TRISTATE	ParseCommandG03(void);
	TRISTATE	ParseCommandD01(void);
	TRISTATE	ParseCommandD02(void);
	TRISTATE	ParseCommandD03(void);
	TRISTATE	ParseCommandLP(void);
	TRISTATE	ParseCommandLM(void);
	TRISTATE	ParseCommandLR(void);
	TRISTATE	ParseCommandLS(void);
	TRISTATE	ParseRegionStatement(void);
	TRISTATE	ParseABStatement(void);
	TRISTATE	ParseSRStatement(void);
	TRISTATE	ParseCommandTF(void);
	TRISTATE	ParseCommandTA(void);
	TRISTATE	ParseCommandTO(void);
	TRISTATE	ParseCommandTD(void);
	TRISTATE	ParseCommandM02(void);

	TRISTATE	ParseExpression(CGerberExpression* pcGerberExpression);
	TRISTATE	ParseAttributeArguments(CGerberCommandAttribute* pcCommandAttribute);

	TRISTATE	ParseApertureMacroPrimitive(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroVariableDefinition(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroComment(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroCircle(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroVectorLine(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroCenterLine(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroOutline(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroPolygon(CGerberCommandApertureMacro* pcApertureMacro);
	TRISTATE	ParseApertureMacroThermal(CGerberCommandApertureMacro* pcApertureMacro);

	TRISTATE	Error(char* szError);

	TRISTATE	GetCommentString(char* szString, size* piLength);
	TRISTATE	GetFieldString(char* szString, size* piLength);

	TRISTATE	GetNameString(char* szString, size* piLength);
	TRISTATE	GetStandardNameString(char* szString, size* piLength);
	TRISTATE	GetUserNameString(char* szString, size* piLength);
};


#define ReturnSyntanxError() return Error("Syntax Error, could not parse Command:")


#define ReturnOnFalseOrSyntaxErrorOnError(tResult) \
if (tResult == TRIFALSE) return TRIFALSE;\
if (tResult == TRIERROR) ReturnSyntanxError()

#define ReturnSyntaxErrorOnErrorOrFalse(tResult) \
if (tResult == TRIFALSE || tResult == TRIERROR) ReturnSyntanxError()

#define ReturnSyntaxErrorOnError(tResult) \
if (tResult == TRIERROR) ReturnSyntanxError()


#endif //  __GERBER_PARSER_H__

