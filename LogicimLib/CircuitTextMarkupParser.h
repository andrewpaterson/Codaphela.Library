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
#ifndef __CIRCUIT_TEXT_MARKUP_PARSER_H__
#define __CIRCUIT_TEXT_MARKUP_PARSER_H__
#include "SupportLib/TextMarkupParser.h"


//The markup of circuit text that isn't edited where it's drawn (labels, port names, component text): KiCad's.  "~{...}"
//is overlined (a negated signal), "_{...}" is subscript and "^{...}" superscript.  Groups nest ("~{A_{0}}") and each
//is closed by a "}".  A "~", "_" or "^" not followed by a "{", and a "}" outside any group, are plain text.
class CCircuitTextMarkupParser : public CTextMarkupParser
{
protected:
	size	ReadOpen(const char* szMarkup, STextStyleOverride* psOverride, int* piKind) override;
	size	ReadClose(const char* szMarkup, int iKind) override;
};


void	GetCircuitPlainText(const char* szMarkup, CChars* pszPlain);
bool	CircuitPlainTextEquals(const char* szMarkup, const char* szPlain);
bool	HasCircuitOverline(const char* szMarkup);


#endif // __CIRCUIT_TEXT_MARKUP_PARSER_H__

