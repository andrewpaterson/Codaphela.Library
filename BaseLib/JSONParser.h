/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __JSON_PARSER_H__
#define __JSON_PARSER_H__
#include "Markup.h"
#include "Logger.h"
#include "BaseParserString.h"
#include "BaseParser.h"


class CJSONParser : public CBaseParser
{
public:
	int					miDepth;
	CMarkupDoc*			mpcDoc;

	void		Init(char* szText, size iTextLen, char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger);
	void		Init(char* szText, size iTextLen, char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger);
	void		Init(char* szText, char* szFileName, CMarkup* pcMarkup, CLogger* pcLogger);
	void		Init(char* szText, char* szFileName, CMarkupDoc* pcDoc, CLogger* pcLogger);
	void		Kill(void);

	TRISTATE	Parse(void);

	TRISTATE	ParseElement(SBaseParserString* psElement);
	TRISTATE	ParseValue(SBaseParserString* psElement);
	TRISTATE	ParseObject(SBaseParserString* psElement);
	TRISTATE	ParseArray(SBaseParserString* psElement);
	TRISTATE	ParseLiteral(char* szElementName);
	TRISTATE	ParseString(char* szElementName);
	TRISTATE	ParseNumber(char* szElementName);
	TRISTATE	ParseMember(void);
};


#endif // __JSON_PARSER_H__

