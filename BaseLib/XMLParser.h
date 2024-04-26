/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__
#include "Markup.h"
#include "Logger.h"
#include "TextParser.h"
#include "XMLEntity.h"
#include "XMLProlog.h"


class CXMLParser
{
protected:
	CXMLProlog					mcProlog;

public:
	CTextParser					mcParser;
	int							miDepth;
	CMarkupTag*					mpcCurrent;
	CArrayXMLEntity				macEntities;
	CMarkupDoc*					mpcDoc;
	char*						mszSourceContext;  //Generally a filename.
	CLogger*					mpcLogger;

	void		Init(CMarkup* pcMarkup, CLogger* pcLogger);
	void		Init(CMarkupDoc* pcDoc, CLogger* pcLogger);
	void		Kill(void);

	TRISTATE	Parse(char* szText, char* szSourceContext);
	TRISTATE	ParseProlog(void);
	TRISTATE	ParseStandalone(void);
	TRISTATE	ParseVersion(void);
	TRISTATE	ParseEncoding(void);
	TRISTATE	ParseElement(void);
	TRISTATE	ParseStartTagOrEmptyTag(bool* pbStart, CChars* pszIdentifier);
	TRISTATE	ParseEndTag(char* szIdentifier);
	TRISTATE	ParseContent(void);
	TRISTATE	ParseText(void);
	TRISTATE	ParseAttribute(void);
	TRISTATE	ParseNamedReference(void);
	TRISTATE	ParseText(CChars* pszText);
	TRISTATE	ParseIdentifier(CChars* pszIdentifier);
	TRISTATE	ParseValue(CChars* pszValue);
	TRISTATE	ParseTextReplacement(CChars* pszText);
	TRISTATE	ParseComment(void);
	TRISTATE	ParseComments(void);
	TRISTATE	ParseDocType(void);
	TRISTATE	ParseEntity(void);
	TRISTATE	ParseEntities(void);
	TRISTATE	ParseExternalEntity(char* szName, char* szContent);
	TRISTATE	ParseInternalEntity(char* szName, char* szContent);
	bool		IsAllowedIdentifierChar(bool bFirstChar, uint8 cChar);
	bool		IsAllowedTextChar(uint8 cChar);
	TRISTATE	FailOnErrorReturnOnFalse(TRISTATE tResult, char* szExpected);
	TRISTATE	FailAlways(TRISTATE tResult, char* szExpected);
};


#endif // __XML_PARSER_H__

