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


class CGerberParser
{
protected:
	CTextParser	mcParser;
	CChars		mszFilename;

public:
	void		Init(char* szText, size iTextLen, char* szFileName);
	void		Kill(void);

	TRISTATE	Parse(void);

protected:
	TRISTATE	ParseStart(void);
	TRISTATE	ParseEnd(void);

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

	TRISTATE	Error(char* szError);
};


#endif //  __GERBER_PARSER_H__

