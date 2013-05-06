/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __XML_PARSER_H__
#define __XML_PARSER_H__
#include "BaseLib/Markup.h"
#include "BaseLib/Logger.h"
#include "TextParser.h"
#include "XMLEntity.h"


class CXMLParser
{
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

	void		LogError(char* szText, ...);

	TRISTATE	Parse(char* szText, char* szSourceContext);
	TRISTATE	ParseProlog(void);
	TRISTATE	ParseVersion(void);
	TRISTATE	ParseEncoding(void);
	TRISTATE	ParseElement(void);
	TRISTATE	ParseStartTagOrEmptyTag(BOOL* pbStart, CChars* pszIdentifier);
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
	BOOL		IsAllowedIdentifierChar(BOOL bFirstChar, unsigned char cChar);
	BOOL		IsAllowedTextChar(unsigned char cChar);
};


#endif // __XML_PARSER_H__

